// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_INTERFACES_I_PARSER_H_
#define GCS_CORE_INTERFACES_I_PARSER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include <winrt/Windows.Foundation.h>

#include "common/event.h"

namespace gcs::interfaces
{

  class IPacket;

  /**
   * @interface IParser
   * @brief Communication protocol parser interface.
   *
   * Responsible for interpreting received raw byte streams and converting them
   * into meaningful packet objects (IPacket).
   */
  class IParser
  {
  public:
    virtual ~IParser() = default;

    /**
     * @brief Injects received byte data into the parser.
     * @param data Byte array view to process (passed without copying).
     *
     * Accumulates data in an internal buffer and attempts to complete packets.
     * When a packet is completed, the OnPacketReceived event occurs.
     */
    virtual void PushData(winrt::array_view<std::uint8_t const> data) = 0;

    /**
     * @brief Initializes the internal state of the parser.
     *
     * Clears all accumulated buffers or parsing states in progress.
     */
    virtual void Reset() = 0;

    /**
     * @brief Event that occurs when a complete packet is parsed.
     */
    gcs::common::Signal<std::shared_ptr<IPacket>> OnPacketReceived;

    /**
     * @brief Event that occurs when integrity checks like CRC fail (optional
     * implementation).
     */
    gcs::common::Signal<const std::vector<std::uint8_t> &> OnCrcFailed;
  };

} // namespace gcs::interfaces

#endif // GCS_CORE_INTERFACES_I_PARSER_H_