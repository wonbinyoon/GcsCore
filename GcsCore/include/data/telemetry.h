// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_DATA_TELEMETRY_H_
#define GCS_CORE_DATA_TELEMETRY_H_

#include <array>
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace gcs::data
{

  /**
   * @struct Vec3
   * @brief 3D Vector (X, Y, Z) structure.
   */
  struct Vec3
  {
    std::array<double, 3> data = {0.0, 0.0, 0.0}; ///< [X, Y, Z] data array.

    double x() const { return data[0]; }
    double y() const { return data[1]; }
    double z() const { return data[2]; }
    double &x() { return data[0]; }
    double &y() { return data[1]; }
    double &z() { return data[2]; }

    /**
     * @brief Accesses components by index.
     * @param index 0(X), 1(Y), 2(Z).
     * @return Value of the component.
     * @throws std::out_of_range if the index is out of bounds.
     */
    double operator[](size_t index) const
    {
      assert(index < 3 && "Vec3 index out of bounds!");
      if (index >= 3)
      {
        throw std::out_of_range("Vec3 index must be 0, 1, or 2");
      }
      return data[index];
    }

    double &operator[](size_t index)
    {
      assert(index < 3 && "Vec3 index out of bounds!");
      if (index >= 3)
      {
        throw std::out_of_range("Vec3 index must be 0, 1, or 2");
      }
      return data[index];
    }
  };

  /**
   * @struct Quat
   * @brief Quaternion (W, X, Y, Z) structure.
   */
  struct Quat
  {
    std::array<double, 4> data = {0.0, 0.0, 0.0, 0.0}; ///< [W, X, Y, Z] data array.

    double w() const { return data[0]; }
    double x() const { return data[1]; }
    double y() const { return data[2]; }
    double z() const { return data[3]; }
    double &w() { return data[0]; }
    double &x() { return data[1]; }
    double &y() { return data[2]; }
    double &z() { return data[3]; }

    /**
     * @brief Accesses components by index.
     * @param index 0(W), 1(X), 2(Y), 3(Z).
     * @return Value of the component.
     * @throws std::out_of_range if the index is out of bounds.
     */
    double operator[](size_t index) const
    {
      assert(index < 4 && "Quat index out of bounds!");
      if (index >= 4)
      {
        throw std::out_of_range("Quat index must be 0, 1, 2, or 3");
      }
      return data[index];
    }

    double &operator[](size_t index)
    {
      assert(index < 4 && "Quat index out of bounds!");
      if (index >= 4)
      {
        throw std::out_of_range("Quat index must be 0, 1, 2, or 3");
      }
      return data[index];
    }
  };

  /**
   * @struct TelemetryData
   * @brief Integrated telemetry data structure.
   *
   * Contains all major information such as system status, position, attitude, etc.
   * This structure can be directly written to binary log files.
   */
  struct TelemetryData
  {
    std::uint32_t timestamp = 0; ///< System uptime (ms).
    Vec3 pos;                    ///< Position (m).
    Vec3 vel;                    ///< Velocity (m/s).
    Vec3 acc;                    ///< Acceleration (m/s^2).
    Quat quat;                   ///< Attitude quaternion (W, X, Y, Z).
    Vec3 euler;                  ///< Attitude euler (roll, pitch, yaw).
    std::uint32_t rx_count = 0;  ///< Received packet count.
    std::uint32_t tx_count = 0;  ///< Transmitted packet count.
    std::uint8_t fsm = 0;        ///< Finite State Machine (FSM) state value.
    std::uint8_t sensor = 0;     ///< Sensor status flags.
    std::uint8_t ejection = 0;   ///< Ejection Type
  };

} // namespace gcs::data

#endif // GCS_CORE_DATA_TELEMETRY_H_