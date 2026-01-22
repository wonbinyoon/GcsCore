#ifndef GCS_CORE_CONFIG_H_
#define GCS_CORE_CONFIG_H_

#include <chrono>
#include <cstdint>

namespace gcs::common
{
  // --- Serial Port Settings ---

  /// @brief 시리얼 통신의 기본 보드레이트 (Baud Rate)
  constexpr std::uint32_t kSerialBaudRate = 115200;

  /// @brief 시리얼 읽기 작업의 타임아웃
  constexpr std::chrono::milliseconds kSerialReadTimeout(10);

  /// @brief 시리얼 쓰기 작업의 타임아웃
  constexpr std::chrono::milliseconds kSerialWriteTimeout(10);

  // --- Replay Settings ---

  /// @brief 로우 로그 리플레이 시 한 번에 읽어올 청크 크기 (바이트)
  constexpr size_t kRawLogReplayChunkSize = 64;

  /// @brief 리플레이 시 허용되는 최대 지연 시간 (밀리초)
  constexpr std::uint32_t kReplayMaxDelayMs = 1000;

  /// @brief 리플레이 루프 내에서 CPU 점유율을 조절하기 위한 최소 수면 시간
  constexpr std::chrono::milliseconds kReplayBusyLoopSleep(1);

} // namespace gcs::common

#endif // GCS_CORE_CONFIG_H_
