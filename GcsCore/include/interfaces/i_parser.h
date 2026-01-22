#ifndef GCS_CORE_I_PARSER_H_
#define GCS_CORE_I_PARSER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include <winrt/Windows.Foundation.h>

#include "common/event.h"
#include "interfaces/i_packet.h"

namespace gcs::communication
{
  /**
   * @interface IParser
   * @brief 통신 프로토콜 파서 인터페이스
   * @details 수신된 원본 바이트 스트림을 해석하여 의미 있는 패킷 객체(IPacket)로 변환하는 역할을 수행합니다.
   */
  class IParser
  {
  public:
    virtual ~IParser() = default;

    /**
     * @brief 수신된 바이트 데이터를 파서에 주입합니다.
     * @param data 처리할 바이트 배열 뷰 (복사 없이 전달됨)
     * @details 내부 버퍼에 데이터를 누적하고 패킷 완성을 시도합니다. 패킷이 완성되면 OnPacketReceived 이벤트가 발생합니다.
     */
    virtual void PushData(winrt::array_view<std::uint8_t const> data) = 0;

    /**
     * @brief 파서의 내부 상태를 초기화합니다.
     * @details 누적된 버퍼나 진행 중인 파싱 상태를 모두 비웁니다.
     */
    virtual void Reset() = 0;

    /// @brief 완전한 패킷이 파싱되었을 때 발생하는 이벤트
    gcs::common::Signal<std::shared_ptr<IPacket>> OnPacketReceived;

    /// @brief CRC 체크 등 무결성 검사에 실패했을 때 발생하는 이벤트 (선택적 구현)
    gcs::common::Signal<const std::vector<std::uint8_t> &> OnCrcFailed;
  };
}

#endif // GCS_CORE_I_PARSER_H_