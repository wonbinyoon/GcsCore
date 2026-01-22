#include "transport/serial_manager.h"

#include <winrt/Windows.Devices.Enumeration.h>

#include "common/config.h"

namespace winrt_foundation = winrt::Windows::Foundation;
namespace winrt_collections = winrt::Windows::Foundation::Collections;

namespace gcs::communication
{

  SerialManager::SerialManager() {}

  SerialManager::~SerialManager()
  {
    Close();
  }

  std::vector<SerialPortInfo> SerialManager::GetPortList()
  {
    namespace winrt_enum = winrt::Windows::Devices::Enumeration;
    namespace winrt_serial = winrt::Windows::Devices::SerialCommunication;

    // 동기적으로 대기 (.get())
    auto devices = winrt_enum::DeviceInformation::FindAllAsync(winrt_serial::SerialDevice::GetDeviceSelector()).get();

    std::vector<SerialPortInfo> port_list;
    port_list.reserve(devices.Size());

    for (auto const &device : devices)
    {
      port_list.push_back({device.Name(), device.Id()});
    }

    return port_list;
  }

  SerialPortInfo SerialManager::GetConnectedPortInfo() const
  {
    return current_port_info_;
  }

  winrt_foundation::IAsyncOperation<bool> SerialManager::OpenAsync(winrt::hstring device_id)
  {
    namespace winrt_serial = winrt::Windows::Devices::SerialCommunication;
    namespace winrt_stream = winrt::Windows::Storage::Streams;
    namespace winrt_enum = winrt::Windows::Devices::Enumeration;

    try
    {
      serial_device_ = co_await winrt_serial::SerialDevice::FromIdAsync(device_id);

      if (serial_device_ != nullptr)
      {
        // 연결된 장치 정보 저장
        auto device_info = co_await winrt_enum::DeviceInformation::CreateFromIdAsync(device_id);
        current_port_info_ = {device_info.Name(), device_info.Id()};

        // 1. 시리얼 포트 설정
        serial_device_.BaudRate(gcs::common::kSerialBaudRate);
        serial_device_.IsDataTerminalReadyEnabled(true); // DTR 신호 활성화
        serial_device_.IsRequestToSendEnabled(true);     // RTS 신호 활성화
        serial_device_.DataBits(8);
        serial_device_.StopBits(winrt_serial::SerialStopBitCount::One);
        serial_device_.Parity(winrt_serial::SerialParity::None);
        serial_device_.Handshake(winrt_serial::SerialHandshake::None);

        // 2. 타임아웃 설정 (매우 중요: 응답성 결정)
        serial_device_.ReadTimeout(gcs::common::kSerialReadTimeout);
        serial_device_.WriteTimeout(gcs::common::kSerialWriteTimeout);

        // 3. DataReader 생성
        reader_ = winrt_stream::DataReader(serial_device_.InputStream());

        // 4. 수신 루프 시작
        StartReadLoop();

        // 5. 열림 이벤트 발생
        OnPortOpened.Invoke(current_port_info_);

        co_return true;
      }
    }
    catch (...)
    {
      // 연결 실패 처리
    }
    co_return false;
  }

  winrt_foundation::IAsyncOperation<std::uint32_t> SerialManager::WriteAsync(winrt::array_view<std::uint8_t const> data)
  {
    namespace winrt_stream = winrt::Windows::Storage::Streams;

    if (!serial_device_)
      co_return 0;

    winrt_stream::DataWriter writer(serial_device_.OutputStream());
    writer.WriteBytes(data);

    std::uint32_t bytes_written = co_await writer.StoreAsync();
    co_await writer.FlushAsync();
    writer.DetachStream(); // 스트림 소유권 해제 (장치 닫힘 방지)

    co_return bytes_written;
  }

  void SerialManager::Close()
  {
    const bool was_opened = (serial_device_ != nullptr);
    SerialPortInfo last_info = current_port_info_; // 닫히기 전 정보 백업

    is_reading_ = false;
    if (reader_)
    {
      reader_.Close();
      reader_ = nullptr;
    }
    if (serial_device_)
    {
      serial_device_.Close();
      serial_device_ = nullptr;
    }
    
    // 현재 포트 정보 초기화
    current_port_info_ = {L"", L""};

    if (was_opened)
    {
      OnPortClosed.Invoke(last_info); // 닫힘 이벤트 발생 (백업된 정보 사용)
    }
  }

  winrt::fire_and_forget SerialManager::StartReadLoop()
  {
    auto lifetime = shared_from_this(); // 객체 생존 보장
    is_reading_ = true;

    try
    {
      while (is_reading_ && serial_device_)
      {
        // 최대 64바이트 요청 (타임아웃 발생 시 가진 만큼만 리턴함)
        std::uint32_t bytes_read = co_await reader_.LoadAsync(64);

        if (bytes_read > 0)
        {
          // 1. 읽어온 데이터를 바이트 배열로 추출
          std::vector<std::uint8_t> buffer(bytes_read);
          reader_.ReadBytes(buffer);

          // 2. 로우 데이터 로깅: m_logger->Write(buffer);
          OnRawDataReceived.Invoke(buffer);
        }
      }
    }
    catch (winrt::hresult_error const &ex)
    {
      // 장치 연결 해제 등 예외 발생 시 루프 종료
      // 0x800703E3: I/O 작업이 스레드 종료 또는 애플리케이션 요청으로 인해 중단됨
      if (ex.code() == winrt::hresult(0x800703E3))
      {
        // 종료 시 발생하는 정상적인 상황이므로 로그만 찍거나 무시
      }
      is_reading_ = false;
      Close();
    }
  }

} // namespace gcs::communication