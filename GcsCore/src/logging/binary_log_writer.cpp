#include "logging/binary_log_writer.h"

#include <chrono>
#include <filesystem>
#include <iomanip>

#include "data/telemetry.h"
#include "interfaces/i_converter.h"
#include "interfaces/i_packet.h"
#include "interfaces/i_parser.h"
#include "transport/serial_manager.h"

namespace gcs::logging
{

  BinaryLogWriter::BinaryLogWriter(std::unique_ptr<gcs::communication::IParser> parser,
                                   std::unique_ptr<gcs::communication::IConverter> converter,
                                   const std::string &log_dir)
      : log_dir_(log_dir),
        parser_(std::move(parser)),
        converter_(std::move(converter))
  {
    if (!std::filesystem::exists(log_dir_))
    {
      std::filesystem::create_directories(log_dir_);
    }

    // 패킷 수신 시 -> 컨버터로 전달
    on_packet_ = parser_->OnPacketReceived.Connect([this](std::shared_ptr<gcs::communication::IPacket> packet)
                                                   { converter_->Convert(packet); });

    // 텔레메트리 변환 시 -> 파일 저장
    on_parsed_ = converter_->OnTelemetryConverted.Connect([this](const gcs::data::TelemetryData &data)
                                                          {
    if (parsed_file_.is_open()) {
      parsed_file_.write(reinterpret_cast<const char*>(&data), sizeof(gcs::data::TelemetryData));
    } });
  }

  BinaryLogWriter::~BinaryLogWriter()
  {
    StopLogging();
  }

  void BinaryLogWriter::Bind(gcs::communication::SerialManager &serial)
  {
    // 1. 포트가 열리면 새 파일 생성
    on_opened_connection_ = serial.OnPortOpened.Connect([this](const gcs::communication::SerialPortInfo &)
                                                        { StartLogging(); });

    // 2. 포트가 닫히면 파일 안전하게 닫기
    on_closed_connection_ = serial.OnPortClosed.Connect([this](const gcs::communication::SerialPortInfo &)
                                                        { StopLogging(); });

    // 3. Raw 데이터 저장 (재생용 바이너리)
    on_raw_ = serial.OnRawDataReceived.Connect([this](const std::vector<std::uint8_t> &data)
                                               {
            if (raw_file_.is_open()) {
                raw_file_.write(reinterpret_cast<const char*>(data.data()), data.size());
            }

            // 파서로 데이터 전달
            parser_->PushData(data); });
  }

  void BinaryLogWriter::StartLogging()
  {
    StopLogging(); // 기존에 열린 파일이 있다면 닫음

    std::string ts = GetTimestamp();
    raw_file_.open(log_dir_ + "/" + ts + "_raw.bin", std::ios::binary);
    parsed_file_.open(log_dir_ + "/" + ts + "_parsed.dat", std::ios::binary);
  }

  void BinaryLogWriter::StopLogging()
  {
    if (raw_file_.is_open())
      raw_file_.close();
    if (parsed_file_.is_open())
      parsed_file_.close();
  }

  std::string BinaryLogWriter::GetTimestamp() const
  {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt{};
    localtime_s(&bt, &in_time_t);
    std::ostringstream ss;
    ss << std::put_time(&bt, "%Y%m%d_%H%M%S");
    return ss.str();
  }

} // namespace gcs::logging