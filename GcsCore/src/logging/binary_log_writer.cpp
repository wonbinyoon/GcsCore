// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include "logging/binary_log_writer.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "data/telemetry.h"
#include "interfaces/i_converter.h"
#include "interfaces/i_packet.h"
#include "interfaces/i_parser.h"
#include "transport/serial_manager.h"
#include "logging_internal.h"

namespace gcs::logging
{

  BinaryLogWriter::BinaryLogWriter(
      std::unique_ptr<gcs::interfaces::IParser> parser,
      std::unique_ptr<gcs::interfaces::IConverter> converter,
      const std::string &log_dir)
      : log_dir_(log_dir),
        parser_(std::move(parser)),
        converter_(std::move(converter))
  {
    InitLogger();

    if (!std::filesystem::exists(log_dir_))
    {
      std::error_code ec;
      if (std::filesystem::create_directories(log_dir_, ec))
      {
        GCS_LOG_INFO("Created log directory: {}", log_dir_);
      }
      else
      {
        GCS_LOG_ERROR("Failed to create log directory: {} ({})", log_dir_, ec.message());
      }
    }
    else
    {
      GCS_LOG_DEBUG("Log directory already exists: {}", log_dir_);
    }

    on_packet_ = parser_->OnPacketReceived.Connect(
        [this](std::shared_ptr<gcs::interfaces::IPacket> packet)
        {
          std::lock_guard<std::recursive_mutex> lock(mutex_);
          if (converter_)
            converter_->Convert(packet);
        });

    on_parsed_ = converter_->OnTelemetryConverted.Connect(
        [this](const gcs::data::TelemetryData &data)
        {
          std::lock_guard<std::recursive_mutex> lock(mutex_);
          if (parsed_file_.is_open())
          {
            parsed_file_.write(reinterpret_cast<const char *>(&data),
                               sizeof(gcs::data::TelemetryData));
            // GCS_LOG_TRACE("Wrote parsed telemetry data to file.");
          }
        });
  }

  BinaryLogWriter::~BinaryLogWriter()
  {
    on_raw_.reset();
    on_packet_.reset();
    on_parsed_.reset();
    on_opened_connection_.reset();
    on_closed_connection_.reset();
    StopLogging();
  }

  void BinaryLogWriter::Bind(gcs::transport::SerialManager &serial)
  {
    on_opened_connection_ = serial.OnPortOpened.Connect(
        [this](const gcs::transport::SerialPortInfo &info)
        { 
          GCS_LOG_INFO("Port opened: {}. Starting loggers.", info.name);
          StartLogging(); 
        });

    on_closed_connection_ = serial.OnPortClosed.Connect(
        [this](const gcs::transport::SerialPortInfo &info)
        { 
          GCS_LOG_INFO("Port closed: {}. Stopping loggers.", info.name);
          StopLogging(); 
        });

    on_raw_ = serial.OnRawDataReceived.Connect(
        [this](const std::vector<std::uint8_t> &data)
        {
          std::lock_guard<std::recursive_mutex> lock(mutex_);
          if (raw_file_.is_open())
          {
            raw_file_.write(reinterpret_cast<const char *>(data.data()),
                            data.size());
            GCS_LOG_TRACE("Wrote {} raw bytes to log file.", data.size());
          }
          if (parser_)
            parser_->PushData(data);
        });
  }

  void BinaryLogWriter::StartLogging()
  {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (raw_file_.is_open())
      raw_file_.close();
    if (parsed_file_.is_open())
      parsed_file_.close();

    std::string ts = GetTimestamp();
    std::string raw_path = log_dir_ + "/" + ts + "_raw.bin";
    std::string parsed_path = log_dir_ + "/" + ts + "_parsed.dat";

    raw_file_.open(raw_path, std::ios::binary);
    if (raw_file_.is_open())
    {
      GCS_LOG_INFO("Started raw logging: {}", raw_path);
    }
    else
    {
      GCS_LOG_ERROR("Failed to open raw log file: {}", raw_path);
    }

    parsed_file_.open(parsed_path, std::ios::binary);
    if (parsed_file_.is_open())
    {
       GCS_LOG_INFO("Started parsed logging: {}", parsed_path);
    }
    else
    {
      GCS_LOG_ERROR("Failed to open parsed log file: {}", parsed_path);
    }
  }

  void BinaryLogWriter::StopLogging()
  {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    bool was_open = raw_file_.is_open() || parsed_file_.is_open();

    if (raw_file_.is_open())
      raw_file_.close();
    if (parsed_file_.is_open())
      parsed_file_.close();

    if (was_open)
    {
      GCS_LOG_INFO("Stopped logging.");
    }
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