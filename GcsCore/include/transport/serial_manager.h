#ifndef GCS_CORE_SERIAL_MANAGER_H_
#define GCS_CORE_SERIAL_MANAGER_H_

#include <cstdint>
#include <vector>

#include <winrt/Windows.Devices.SerialCommunication.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>

#include "common/event.h"

namespace gcs::communication
{

  /**
   * @brief 시리얼 포트 정보를 담는 구조체
   */
  struct SerialPortInfo
  {
    winrt::hstring name; ///< 사람이 읽을 수 있는 포트 이름 (예: "USB-Serial CH340 (COM3)")
    winrt::hstring id;   ///< 시스템 내부 장치 ID (연결 시 OpenAsync에 전달해야 함)
  };

  /**
   * @class SerialManager
   * @brief WinRT 기반 시리얼 통신 관리자
   * @details 포트 검색, 연결, 데이터 송수신 및 이벤트를 비동기적으로 처리합니다.
   */
  class SerialManager : public std::enable_shared_from_this<SerialManager>
  {
  public:
    SerialManager();
    ~SerialManager();

    /**
     * @brief 시스템에 존재하는 사용 가능한 시리얼 포트 목록을 조회합니다.
     * @return 검색된 시리얼 포트 정보(이름, ID)의 벡터
     * @note 동기 함수이므로 실행 중 스레드가 잠시 차단될 수 있습니다.
     */
    static std::vector<SerialPortInfo> GetPortList();

    /**
     * @brief 현재 연결된 포트의 정보를 반환합니다.
     * @return 연결된 포트 정보. 연결되어 있지 않으면 빈 문자열을 포함한 구조체를 반환합니다.
     */
    SerialPortInfo GetConnectedPortInfo() const;

    /**
     * @brief 지정된 장치 ID로 시리얼 포트에 비동기적으로 연결합니다.
     * @param device_id 연결할 장치의 고유 ID (SerialPortInfo::id)
     * @return 연결 성공 여부 (true: 성공, false: 실패)
     */
    winrt::Windows::Foundation::IAsyncOperation<bool> OpenAsync(winrt::hstring device_id);

    /**
     * @brief 연결된 포트로 데이터를 비동기적으로 전송합니다.
     * @param data 전송할 바이트 배열의 뷰
     * @return 실제로 전송된 바이트 수
     */
    winrt::Windows::Foundation::IAsyncOperation<std::uint32_t> WriteAsync(winrt::array_view<std::uint8_t const> data);

    /**
     * @brief 포트 연결을 해제하고 리소스를 정리합니다.
     */
    void Close();

    // 이벤트
    
    /// @brief 포트가 성공적으로 열렸을 때 발생하는 이벤트 (인자: 연결된 포트 정보)
    gcs::common::Signal<const SerialPortInfo &> OnPortOpened;
    
    /// @brief 포트가 닫혔을 때 발생하는 이벤트 (인자: 닫힌 포트 정보)
    gcs::common::Signal<const SerialPortInfo &> OnPortClosed;
    
    /// @brief 원본(Raw) 데이터가 수신되었을 때 발생하는 이벤트 (인자: 수신된 바이트 벡터)
    gcs::common::Signal<const std::vector<std::uint8_t> &> OnRawDataReceived;

    /**
     * @brief 현재 포트가 열려 있는지 확인합니다.
     * @return true이면 연결됨, false이면 연결 안 됨
     */
    bool IsOpened() const { return serial_device_ != nullptr; }

  private:
    /// @brief 백그라운드에서 데이터를 계속 읽어오는 비동기 루프
    winrt::fire_and_forget StartReadLoop();

    winrt::Windows::Devices::SerialCommunication::SerialDevice serial_device_{nullptr};
    winrt::Windows::Storage::Streams::DataReader reader_{nullptr};
    SerialPortInfo current_port_info_;

    bool is_reading_ = false;
  };

} // namespace gcs::communication

#endif // GCS_CORE_SERIAL_MANAGER_H_