// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_INTERFACES_I_PACKET_H_
#define GCS_CORE_INTERFACES_I_PACKET_H_

#include <cstdint>
#include <vector>

namespace gcs::interfaces
{

  /**
   * @interface IPacket
   * @brief Communication packet interface.
   *
   * Base interface that all protocol packet classes must implement.
   */
  class IPacket
  {
  public:
    virtual ~IPacket() = default;

    /**
     * @brief Serializes packet data into a byte vector.
     * @return Serialized byte data.
     */
    virtual std::vector<std::uint8_t> Serialize() const = 0;

    /**
     * @brief Deserializes byte data into the packet object.
     * @param data Raw byte data.
     */
    virtual void Deserialize(const std::vector<std::uint8_t> &data) = 0;

    /**
     * @brief Returns the unique ID of the packet.
     * @return Packet ID.
     */
    virtual int GetId() const = 0;
  };

} // namespace gcs::interfaces

#endif // GCS_CORE_INTERFACES_I_PACKET_H_