#ifndef GCS_CORE_I_CONVERTER_H_
#define GCS_CORE_I_CONVERTER_H_

#include <memory>

#include "common/event.h"
#include "data/telemetry.h"
#include "interfaces/i_packet.h"

namespace gcs::communication
{
  /**
   * @interface IConverter
   * @brief 패킷 변환기 인터페이스
   * @details 로우 레벨 패킷(IPacket)을 애플리케이션에서 사용할 수 있는 고수준 데이터 구조(TelemetryData 등)로 변환합니다.
   */
  class IConverter
  {
  public:
    virtual ~IConverter() = default;

    /**
     * @brief 패킷을 받아 변환 작업을 수행합니다.
     * @param packet 변환할 소스 패킷
     * @details 변환이 성공하면 OnTelemetryConverted 이벤트가 발생합니다.
     */
    virtual void Convert(const std::shared_ptr<IPacket> &packet) = 0;

    /**
     * @brief 컨버터의 내부 상태를 초기화합니다.
     * @details 누적된 보정 데이터나 이전 패킷 참조 등의 상태를 모두 비웁니다.
     */
    virtual void Reset() = 0;

    /// @brief 텔레메트리 데이터로 변환이 완료되었을 때 발생하는 이벤트
    gcs::common::Signal<const gcs::data::TelemetryData &> OnTelemetryConverted;
  };
}

#endif // GCS_CORE_I_CONVERTER_H_