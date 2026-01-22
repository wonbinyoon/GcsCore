#ifndef GCS_CORE_BINARY_LOG_WRITER_H_
#define GCS_CORE_BINARY_LOG_WRITER_H_

#include <fstream>
#include <memory>
#include <string>

#include "common/event.h"

// 네임스페이스 및 클래스 전방 선언
namespace gcs::communication
{
  class IParser;
  class IConverter;
  class SerialManager;
}

namespace gcs::logging
{

  /**
   * @class BinaryLogWriter
   * @brief 데이터 로깅 클래스
   * @details 시리얼 포트에서 수신된 원본 데이터(Raw)와 파싱/변환된 데이터(Parsed)를 파일로 저장합니다.
   *          Raw 데이터는 재생(Replay)용으로, Parsed 데이터는 분석용으로 사용됩니다.
   */
  class BinaryLogWriter
  {
  public:
    /**
     * @brief BinaryLogWriter 생성자
     * @param parser 데이터를 해석할 파서 인스턴스 (소유권 이전)
     * @param converter 데이터를 변환할 컨버터 인스턴스 (소유권 이전)
     * @param log_dir 로그 파일을 저장할 디렉토리 경로
     */
    BinaryLogWriter(std::unique_ptr<gcs::communication::IParser> parser,
                    std::unique_ptr<gcs::communication::IConverter> converter,
                    const std::string &log_dir);
    ~BinaryLogWriter();

    /**
     * @brief 시리얼 매니저와 연결하여 로깅을 시작할 준비를 합니다.
     * @param serial 모니터링할 SerialManager 인스턴스
     * @details 포트 연결/해제 이벤트에 자동으로 구독하여 파일 생성 및 닫기를 관리합니다.
     */
    void Bind(gcs::communication::SerialManager &serial);

    // 내부 이벤트 리스너 관리를 위한 토큰
    gcs::common::SignalToken on_raw_;
    gcs::common::SignalToken on_packet_;
    gcs::common::SignalToken on_parsed_;
    gcs::common::SignalToken on_opened_connection_;
    gcs::common::SignalToken on_closed_connection_;

  private:
    void StartLogging();
    void StopLogging();
    std::string GetTimestamp() const;

    std::string log_dir_;
    std::ofstream raw_file_;    ///< 원본 바이너리 로그 파일 스트림
    std::ofstream parsed_file_; ///< 파싱된 데이터 로그 파일 스트림

    std::unique_ptr<gcs::communication::IParser> parser_;
    std::unique_ptr<gcs::communication::IConverter> converter_;
  };

} // namespace gcs::logging

#endif // GCS_CORE_BINARY_LOG_WRITER_H_