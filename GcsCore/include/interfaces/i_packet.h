#ifndef GCS_CORE_I_PACKET_H_
#define GCS_CORE_I_PACKET_H_

namespace gcs::communication
{
  /**
   * @interface IPacket
   * @brief 통신 패킷 인터페이스
   * @details 모든 프로토콜 패킷 클래스가 구현해야 하는 기본 인터페이스입니다.
   */
  class IPacket
  {
  public:
    virtual ~IPacket() = default;

    /**
     * @brief 패킷 데이터를 직렬화(Serialize)하여 바이트 벡터로 반환합니다.
     * @return 직렬화된 바이트 데이터
     */
    virtual std::vector<std::uint8_t> Serialize() const = 0;
    
    /**
     * @brief 패킷의 고유 ID를 반환합니다.
     * @return 패킷 ID
     */
    virtual int GetId() const = 0;
  };
}

#endif // GCS_CORE_I_PACKET_H_