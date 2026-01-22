#ifndef GCS_CORE_TELEMETRY_H_
#define GCS_CORE_TELEMETRY_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace gcs::data
{

  /**
   * @struct Vec3
   * @brief 3차원 벡터 (X, Y, Z) 구조체
   */
  struct Vec3
  {
    std::array<double, 3> data = {0.0, 0.0, 0.0}; ///< [X, Y, Z] 데이터 배열

    // 이름으로 접근하고 싶을 때를 위한 편의 기능 (C++11 이상)
    double x() const { return data[0]; }
    double y() const { return data[1]; }
    double z() const { return data[2]; }
    double &x() { return data[0]; }
    double &y() { return data[1]; }
    double &z() { return data[2]; }

    /**
     * @brief 인덱스로 성분에 접근합니다.
     * @param index 0(X), 1(Y), 2(Z)
     * @return 해당 성분의 값
     * @throws std::out_of_range 인덱스가 범위를 벗어날 경우 발생
     */
    double operator[](size_t index) const
    {
      // 개발 단계에서 문제를 잡기 위한 어설션
      assert(index < 3 && "Vec3 index out of bounds!");

      // 런타임 에러 처리가 필요하다면 예외 발생
      if (index >= 3)
      {
        throw std::out_of_range("Vec3 index must be 0, 1, or 2");
      }
      return data[index];
    }
    double &operator[](size_t index)
    {
      // 개발 단계에서 문제를 잡기 위한 어설션
      assert(index < 3 && "Vec3 index out of bounds!");

      // 런타임 에러 처리가 필요하다면 예외 발생
      if (index >= 3)
      {
        throw std::out_of_range("Vec3 index must be 0, 1, or 2");
      }
      return data[index];
    }
  };

  /**
   * @struct Quat
   * @brief 사원수 (Quaternion, W, X, Y, Z) 구조체
   */
  struct Quat
  {
    std::array<double, 4> data = {0.0, 0.0, 0.0, 0.0}; ///< [W, X, Y, Z] 데이터 배열

    // 이름으로 접근하고 싶을 때를 위한 편의 기능 (C++11 이상)
    double w() const { return data[0]; }
    double x() const { return data[1]; }
    double y() const { return data[2]; }
    double z() const { return data[3]; }
    double &w() { return data[0]; }
    double &x() { return data[1]; }
    double &y() { return data[2]; }
    double &z() { return data[3]; }

    /**
     * @brief 인덱스로 성분에 접근합니다.
     * @param index 0(W), 1(X), 2(Y), 3(Z)
     * @return 해당 성분의 값
     * @throws std::out_of_range 인덱스가 범위를 벗어날 경우 발생
     */
    double operator[](size_t index) const
    {
      // 개발 단계에서 문제를 잡기 위한 어설션
      assert(index < 4 && "Quat index out of bounds!");

      // 런타임 에러 처리가 필요하다면 예외 발생
      if (index >= 4)
      {
        throw std::out_of_range("Quat index must be 0, 1, 2, or 3");
      }
      return data[index];
    }
    double &operator[](size_t index)
    {
      // 개발 단계에서 문제를 잡기 위한 어설션
      assert(index < 4 && "Quat index out of bounds!");

      // 런타임 에러 처리가 필요하다면 예외 발생
      if (index >= 4)
      {
        throw std::out_of_range("Quat index must be 0, 1, 2, or 3");
      }
      return data[index];
    }
  };

  /**
   * @struct TelemetryData
   * @brief 통합 텔레메트리 데이터 구조체
   * @details 시스템의 상태, 위치, 자세 등 모든 주요 정보를 포함합니다.
   *          이 구조체는 바이너리 로그 파일(.dat)에 직접 쓰여질 수 있습니다.
   */
  struct TelemetryData
  {
    std::uint32_t timestamp = 0; ///< 시스템 가동 시간 (ms)
    Vec3 pos;                    ///< 위치 (m)
    Vec3 vel;                    ///< 속도 (m/s)
    Vec3 acc;                    ///< 가속도 (m/s^2)
    Quat quat;                   ///< 자세 사원수 (W, X, Y, Z)
    std::uint32_t rxCount = 0;   ///< 수신 패킷 카운트
    std::uint32_t txCount = 0;   ///< 송신 패킷 카운트
    std::uint8_t fsm = 0;        ///< 유한 상태 기계(FSM) 상태값
    std::uint8_t sensor = 0;     ///< 센서 상태 플래그
  };

} // namespace gcs::data

#endif // GCS_CORE_TELEMETRY_H_