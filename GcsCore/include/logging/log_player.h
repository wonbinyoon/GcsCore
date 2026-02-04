// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_LOGGING_LOG_PLAYER_H_
#define GCS_CORE_LOGGING_LOG_PLAYER_H_

#include <atomic>
#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "common/event.h"

namespace gcs::data
{
  struct TelemetryData;
} // namespace gcs::data

namespace gcs::interfaces
{
  class IParser;
  class IConverter;
  class IPacket;
} // namespace gcs::interfaces

namespace gcs::logging
{

  /**
   * @enum LogType
   * @brief Supported log file formats.
   */
  enum class LogType
  {
    kRaw,   ///< Raw binary stream (.bin)
    kParsed ///< Pre-parsed telemetry structures (.dat)
  };

  /**
   * @class LogPlayer
   * @brief Replays log files and fires telemetry events.
   *
   * Supports speed adjustment, pause, and seeking.
   */
  class LogPlayer
  {
  public:
    /**
     * @brief Constructor.
     * @param parser Protocol parser for raw logs.
     * @param converter Data converter for raw logs.
     */
    LogPlayer(std::unique_ptr<gcs::interfaces::IParser> parser,
              std::unique_ptr<gcs::interfaces::IConverter> converter);

    /**
     * @brief Destructor.
     */
    ~LogPlayer();

    /**
     * @brief Loads a log file.
     * @param file_path Absolute path to the file.
     * @param type File format type.
     * @return True if successfully loaded.
     */
    bool Load(const std::string &file_path, LogType type);

    /**
     * @brief Starts playback in a background thread.
     */
    void Play();

    /**
     * @brief Pauses playback.
     */
    void Pause();

    /**
     * @brief Stops playback and resets to the beginning.
     */
    void Stop();

    /**
     * @brief Sets playback speed.
     * @param speed Ratio (1.0 = normal, 2.0 = double).
     */
    void SetSpeed(double speed);

    /**
     * @brief Seeks to a specific percentage.
     * @param percent Position (0.0 to 1.0).
     */
    void SeekTo(double percent);

    /**
     * @brief Checks if playback is active.
     */
    bool IsPlaying() const { return is_playing_; }

    /**
     * @brief Gets current playback position as a percentage.
     */
    double GetCurrentPercent() const;

    /**
     * @brief Event fired when telemetry data is recovered.
     */
    gcs::common::Signal<const gcs::data::TelemetryData &> OnTelemetry;

    /**
     * @brief Event fired when CRC fails (raw mode).
     */
    gcs::common::Signal<const std::vector<std::uint8_t> &> OnCrcFailed;

    /**
     * @brief Event fired when the end of the file is reached.
     */
    gcs::common::Signal<> OnEof;

  private:
    void PlayLoop();
    bool HandleRawChunk();
    bool HandleParsedFrame();
    void SyncTiming(std::uint32_t timestamp);

    std::unique_ptr<gcs::interfaces::IParser> parser_;
    std::unique_ptr<gcs::interfaces::IConverter> converter_;

    std::ifstream file_;
    LogType type_;
    std::string file_path_;
    size_t file_size_ = 0;

    std::thread play_thread_;
    std::atomic<bool> is_playing_ = false;
    std::atomic<bool> is_paused_ = false;
    std::atomic<bool> stop_flag_ = false;

    std::atomic<double> speed_ = 1.0;
    std::mutex file_mutex_;

    std::uint32_t last_pkt_timestamp_ = 0;

    gcs::common::SignalToken on_packet_;
    gcs::common::SignalToken on_crc_fail_;
    gcs::common::SignalToken on_converted_;
  };

} // namespace gcs::logging

#endif // GCS_CORE_LOGGING_LOG_PLAYER_H_