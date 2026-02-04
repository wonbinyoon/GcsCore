// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_INTERFACES_PACKET_FACTORY_H_
#define GCS_CORE_INTERFACES_PACKET_FACTORY_H_

#include <functional>
#include <map>
#include <memory>

#include "interfaces/i_packet.h"

namespace gcs::interfaces
{

  /**
   * @class PacketFactory
   * @brief Factory for creating IPacket instances based on ID.
   */
  class PacketFactory
  {
  public:
    using Creator = std::function<std::shared_ptr<IPacket>()>;

    /**
     * @brief Registers a packet type with its ID.
     * @tparam T Packet class type.
     * @param id Unique ID for the packet type.
     */
    template <typename T>
    static void Register(int id)
    {
      GetRegistry()[id] = []()
      { return std::make_shared<T>(); };
    }

    /**
     * @brief Creates a packet instance based on ID.
     * @param id Packet ID.
     * @return Shared pointer to the created packet, or nullptr if not registered.
     */
    static std::shared_ptr<IPacket> Create(int id)
    {
      auto &registry = GetRegistry();
      auto it = registry.find(id);
      if (it != registry.end())
      {
        return it->second();
      }
      return nullptr;
    }

  private:
    // Meyers' Singleton to hold the registry
    static std::map<int, Creator> &GetRegistry()
    {
      static std::map<int, Creator> registry;
      return registry;
    }
  };

  /**
   * @brief Helper for automatic packet registration.
   */
  template <typename T>
  struct PacketRegistrar
  {
    PacketRegistrar(int id)
    {
      PacketFactory::Register<T>(id);
    }
  };

} // namespace gcs::interfaces

/**
 * @def REGISTER_PACKET
 * @brief Macro to register a packet type at startup.
 */
#define REGISTER_PACKET(ClassName, ID) \
  static inline gcs::interfaces::PacketRegistrar<ClassName> ClassName##Registrar(ID)

#endif // GCS_CORE_INTERFACES_PACKET_FACTORY_H_
