#ifndef GCS_CORE_LOG_PLAYER_H_
#define GCS_CORE_LOG_PLAYER_H_

#include <atomic>
#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "common/event.h"

// 전방 선언
namespace gcs::communication
{
  class IParser;
  class IConverter;
  class IPacket;
}
namespace gcs::data
{
  struct TelemetryData;
}

namespace gcs::logging
{
  /**
   * @enum LogType
   * @brief 로그 파일 형식 정의
   */
  enum class LogType
  {
    kRaw,   ///< 센서에서 수신된 원본 바이너리 스트림 (.bin)
    kParsed ///< 이미 파싱되어 구조체 형태로 저장된 데이터 (.dat)
  };

  /**
   * @class LogPlayer
   * @brief 로그 파일 재생기
   * @details 저장된 로그 파일(Raw 또는 Parsed)을 읽어 실시간 데이터처럼 이벤트를 발생시킵니다.
   *          배속 재생, 일시 정지, 탐색(Seek) 기능을 지원합니다.
   */
  class LogPlayer
  {
  public:
    /**
     * @brief LogPlayer 생성자
     * @param parser Raw 로그 재생 시 사용할 파서 (Parsed 모드만 쓴다면 nullptr 가능)
     * @param converter Raw 로그 재생 시 사용할 컨버터 (Parsed 모드만 쓴다면 nullptr 가능)
     */
    LogPlayer(std::unique_ptr<gcs::communication::IParser> parser,
              std::unique_ptr<gcs::communication::IConverter> converter);
    ~LogPlayer();

    /**
     * @brief 로그 파일을 로드합니다.
     * @param file_path 로그 파일의 절대 경로
     * @param type 로그 파일 형식 (kRaw 또는 kParsed)
     * @return 파일 열기 성공 여부
     */
    bool Load(const std::string &file_path, LogType type);

    /**
     * @brief 로그 재생을 시작합니다.
     * @details 별도의 스레드에서 재생 루프가 실행됩니다.
     */
    void Play();

    /**
     * @brief 로그 재생을 일시 정지합니다.
     */
    void Pause();

    /**
     * @brief 로그 재생을 완전히 멈추고 파일 포인터를 처음으로 되돌립니다.
     */
    void Stop(); 

    /**
     * @brief 재생 속도를 설정합니다.
     * @param speed 배속 비율 (1.0 = 정속, 2.0 = 2배속, 0.5 = 0.5배속)
     */
    void SetSpeed(double speed);

    /**
     * @brief 파일의 특정 위치로 탐색(Seek)합니다.
     * @param percent 탐색할 위치의 퍼센트 (0.0 ~ 1.0)
     */
    void SeekTo(double percent);

    /**
     * @brief 현재 재생 중인지 여부를 반환합니다.
     * @return true이면 재생 중 (일시 정지 포함), false이면 정지 상태
     */
    bool IsPlaying() const { return is_playing_; }

    /**
     * @brief 현재 재생 위치를 퍼센트로 반환합니다.
     * @return 재생 위치 (0.0 ~ 1.0)
     */
    double GetCurrentPercent() const;

    /// @brief 텔레메트리 데이터가 복원되었을 때 발생하는 이벤트
    gcs::common::Signal<const gcs::data::TelemetryData &> OnTelemetry;
    
    /// @brief (Raw 모드) CRC 검사 실패 시 발생하는 이벤트
    gcs::common::Signal<const std::vector<std::uint8_t> &> OnCrcFailed; 

  private:
    void PlayLoop();
    bool HandleRawChunk();
    bool HandleParsedFrame();

    // 타임스탬프 동기화
    void SyncTiming(std::uint32_t timestamp);

    // 멤버 변수
    std::unique_ptr<gcs::communication::IParser> parser_;
    std::unique_ptr<gcs::communication::IConverter> converter_;

    // 파일 및 스레드
    std::ifstream file_;
    LogType type_;
    std::string file_path_;
    size_t file_size_ = 0;

    std::thread play_thread_;
    std::atomic<bool> is_playing_ = false; // 스레드 실행 여부
    std::atomic<bool> is_paused_ = false;  // 일시정지 여부
    std::atomic<bool> stop_flag_ = false;  // 스레드 종료 신호

    // 재생 제어
    std::atomic<double> speed_ = 1.0;
    std::mutex file_mutex_; // Seek와 Read 간 동기화

    // 타이밍 계산용
    std::uint32_t last_pkt_timestamp_ = 0;

    // 이벤트 연결 티켓
    gcs::common::SignalToken on_packet_;
    gcs::common::SignalToken on_crc_fail_;
    gcs::common::SignalToken on_converted_;
  };

} // namespace gcs::logging

#endif // GCS_CORE_LOG_PLAYER_H_