// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_COMMON_CONFIG_H_
#define GCS_CORE_COMMON_CONFIG_H_

#include <chrono>
#include <cstdint>

/**
 * @namespace gcs::common
 * @brief Common utilities and configuration constants.
 */
namespace gcs::common
{

  // --- Serial Port Settings ---

  /**
   * @brief Default baud rate for serial communication.
   */
  constexpr std::uint32_t kSerialBaudRate = 9600;

  /**
   * @brief Timeout for serial read operations.
   */
  constexpr std::chrono::milliseconds kSerialReadTimeout(100);

  /**
   * @brief Timeout for serial write operations.
   */
  constexpr std::chrono::milliseconds kSerialWriteTimeout(100);

  /**
   * @brief Buffer size for reading from the serial port.
   */
  constexpr std::uint32_t kSerialReadBufferSize = 64;

  // --- Replay Settings ---

  /**
   * @brief Chunk size (in bytes) to read at once during raw log replay.
   */
  constexpr size_t kRawLogReplayChunkSize = 64;

  /**
   * @brief Maximum allowed delay during replay in milliseconds.
   */
  constexpr std::uint32_t kReplayMaxDelayMs = 1000;

  /**
   * @brief Minimum sleep time to control CPU usage within the replay loop.
   */
  constexpr std::chrono::milliseconds kReplayBusyLoopSleep(1);

} // namespace gcs::common

#endif // GCS_CORE_COMMON_CONFIG_H_