// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_INTERFACES_I_CONVERTER_H_
#define GCS_CORE_INTERFACES_I_CONVERTER_H_

#include <memory>

#include "common/event.h"
#include "data/telemetry.h"

namespace gcs::interfaces
{

  class IPacket;

  /**
   * @interface IConverter
   * @brief Packet converter interface.
   *
   * Converts low-level packets (IPacket) into high-level data structures
   * (TelemetryData, etc.) usable by the application.
   */
  class IConverter
  {
  public:
    virtual ~IConverter() = default;

    /**
     * @brief Receives a packet and performs the conversion.
     * @param packet Source packet to convert.
     *
     * If the conversion is successful, the OnTelemetryConverted event occurs.
     */
    virtual void Convert(const std::shared_ptr<IPacket> &packet) = 0;

    /**
     * @brief Initializes the internal state of the converter.
     *
     * Clears all states such as accumulated calibration data or previous packet
     * references.
     */
    virtual void Reset() = 0;

    /**
     * @brief Event that occurs when conversion to telemetry data is complete.
     */
    gcs::common::Signal<const gcs::data::TelemetryData &> OnTelemetryConverted;
  };

} // namespace gcs::interfaces

#endif // GCS_CORE_INTERFACES_I_CONVERTER_H_