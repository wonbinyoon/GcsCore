// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include "logging/log_player.h"

#include <chrono>
#include <filesystem>
#include <vector>

#include "data/telemetry.h"
#include "interfaces/i_converter.h"
#include "interfaces/i_packet.h"
#include "interfaces/i_parser.h"

namespace gcs::logging
{

  namespace
  {
    constexpr size_t kChunkSize = 256;
  }

  LogPlayer::LogPlayer(std::unique_ptr<gcs::interfaces::IParser> parser,
                       std::unique_ptr<gcs::interfaces::IConverter> converter)
      : parser_(std::move(parser)), converter_(std::move(converter))
  {
    if (parser_)
    {
      on_packet_ = parser_->OnPacketReceived.Connect(
          [this](std::shared_ptr<gcs::interfaces::IPacket> packet)
          {
            if (converter_)
              converter_->Convert(packet);
          });

      on_crc_fail_ = parser_->OnCrcFailed.Connect(
          [this](const std::vector<std::uint8_t> &data)
          {
            OnCrcFailed.Invoke(data);
          });
    }

    if (converter_)
    {
      on_converted_ = converter_->OnTelemetryConverted.Connect(
          [this](const gcs::data::TelemetryData &data)
          {
            SyncTiming(data.timestamp);
            OnTelemetry.Invoke(data);
          });
    }
  }

  LogPlayer::~LogPlayer() { Stop(); }

  bool LogPlayer::Load(const std::string &file_path, LogType type)
  {
    Stop();

    std::lock_guard<std::mutex> lock(file_mutex_);
    file_path_ = file_path;
    type_ = type;

    file_.open(file_path, std::ios::binary | std::ios::ate);
    if (!file_.is_open())
    {
      return false;
    }

    file_size_ = static_cast<size_t>(file_.tellg());
    file_.clear();
    file_.seekg(0, std::ios::beg);

    last_pkt_timestamp_ = 0;
    return true;
  }

  void LogPlayer::Play()
  {
    if (is_playing_)
    {
      is_paused_ = false;
      return;
    }

    if (!file_.is_open())
      return;

    stop_flag_ = false;
    is_playing_ = true;
    is_paused_ = false;

    play_thread_ = std::thread(&LogPlayer::PlayLoop, this);
  }

  void LogPlayer::Pause() { is_paused_ = true; }

  void LogPlayer::Stop()
  {
    stop_flag_ = true;
    if (play_thread_.joinable())
    {
      play_thread_.join();
    }
    is_playing_ = false;
    is_paused_ = false;

    if (file_.is_open())
    {
      file_.clear();
      file_.seekg(0, std::ios::beg);
    }
    last_pkt_timestamp_ = 0;
  }

  void LogPlayer::SetSpeed(double speed)
  {
    if (speed > 0.0)
    {
      speed_ = speed;
    }
  }

  void LogPlayer::SeekTo(double percent)
  {
    if (!file_.is_open())
      return;
    if (percent < 0.0)
      percent = 0.0;
    if (percent > 1.0)
      percent = 1.0;

    std::lock_guard<std::mutex> lock(file_mutex_);

    size_t offset = static_cast<size_t>(file_size_ * percent);

    if (type_ == LogType::kParsed)
    {
      size_t struct_size = sizeof(gcs::data::TelemetryData);
      offset = (offset / struct_size) * struct_size;
    }

    file_.clear();
    file_.seekg(offset, std::ios::beg);

    last_pkt_timestamp_ = 0;

    if (parser_)
      parser_->Reset();
    if (converter_)
      converter_->Reset();
  }

  double LogPlayer::GetCurrentPercent() const
  {
    // Implementation omitted for brevity.
    return 0.0;
  }

  void LogPlayer::PlayLoop()
  {
    while (!stop_flag_)
    {
      if (is_paused_)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }

      bool success = false;
      if (type_ == LogType::kParsed)
      {
        success = HandleParsedFrame();
      }
      else
      {
        success = HandleRawChunk();
      }

      if (!success)
      {
        stop_flag_ = true;
        OnEof.Invoke();
        break;
      }
    }
    is_playing_ = false;
  }

  bool LogPlayer::HandleParsedFrame()
  {
    gcs::data::TelemetryData data;
    {
      std::lock_guard<std::mutex> lock(file_mutex_);
      if (!file_.good())
        return false;
      file_.read(reinterpret_cast<char *>(&data), sizeof(data));
    }

    if (file_.gcount() == sizeof(data))
    {
      SyncTiming(data.timestamp);
      OnTelemetry.Invoke(data);
      return true;
    }
    return false;
  }

  bool LogPlayer::HandleRawChunk()
  {
    if (!parser_)
      return false;

    std::vector<std::uint8_t> buffer(kChunkSize);
    size_t bytes_read = 0;
    {
      std::lock_guard<std::mutex> lock(file_mutex_);
      if (!file_.good())
        return false;
      file_.read(reinterpret_cast<char *>(buffer.data()), kChunkSize);
      bytes_read = static_cast<size_t>(file_.gcount());
    }

    if (bytes_read > 0)
    {
      buffer.resize(bytes_read);
      parser_->PushData(buffer);
      return true;
    }
    return false;
  }

  void LogPlayer::SyncTiming(std::uint32_t current_ts)
  {
    if (last_pkt_timestamp_ == 0)
    {
      last_pkt_timestamp_ = current_ts;
      return;
    }

    long long delta_ms = static_cast<long long>(current_ts) - last_pkt_timestamp_;
    if (delta_ms > 0 && delta_ms < 5000)
    {
      double wait_ms = delta_ms / speed_;
      if (wait_ms > 1.0)
      {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<long long>(wait_ms)));
      }
    }
    last_pkt_timestamp_ = current_ts;
  }

} // namespace gcs::logging