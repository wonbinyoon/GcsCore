// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_TRANSPORT_SERIAL_MANAGER_H_
#define GCS_CORE_TRANSPORT_SERIAL_MANAGER_H_

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <string>

#include <winrt/Windows.Devices.SerialCommunication.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>

#include "common/event.h"

namespace gcs::transport
{

  /**
   * @struct SerialPortInfo
   * @brief Structure containing serial port information.
   */
  struct SerialPortInfo
  {
    std::string name; ///< Human-readable port name (e.g., "COM3").
    std::string id;   ///< Internal device ID.
  };

  /**
   * @class SerialManager
   * @brief WinRT-based serial communication manager.
   *
   * Handles port discovery, connection, data transmission, and events
   * asynchronously.
   */
  class SerialManager : public std::enable_shared_from_this<SerialManager>
  {
  public:
    SerialManager();
    ~SerialManager();

    /**
     * @brief Retrieves a list of available serial ports.
     * @return Vector of SerialPortInfo objects.
     */
    static std::vector<SerialPortInfo> GetPortList();

    /**
     * @brief Returns information about the currently connected port.
     * @return Current SerialPortInfo. Empty if not connected.
     */
    SerialPortInfo GetConnectedPortInfo() const;

    /**
     * @brief Asynchronously opens a serial port.
     * @param device_id Unique device ID.
     * @return Async operation returning true on success.
     */
    winrt::Windows::Foundation::IAsyncOperation<bool> OpenAsync(
        std::string device_id);

    /**
     * @brief Asynchronously writes data to the connected port.
     * @param data Byte array view to send.
     * @return Async operation returning the number of bytes written.
     */
    winrt::Windows::Foundation::IAsyncOperation<std::uint32_t> WriteAsync(
        winrt::array_view<std::uint8_t const> data);

    /**
     * @brief Closes the port and releases resources.
     */
    void Close();

    /**
     * @brief Checks if the port is currently open.
     */
    bool IsOpened() const { return serial_device_ != nullptr; }

    /**
     * @brief Event fired when a port is successfully opened.
     */
    gcs::common::Signal<const SerialPortInfo &> OnPortOpened;

    /**
     * @brief Event fired when a port is closed.
     */
    gcs::common::Signal<const SerialPortInfo &> OnPortClosed;

    /**
     * @brief Event fired when raw data is received.
     */
    gcs::common::Signal<const std::vector<std::uint8_t> &> OnRawDataReceived;

  private:
    /**
     * @brief Internal background read loop.
     */
    winrt::fire_and_forget StartReadLoop();

    winrt::Windows::Devices::SerialCommunication::SerialDevice serial_device_{
        nullptr};
    winrt::Windows::Storage::Streams::DataReader reader_{nullptr};
    SerialPortInfo current_port_info_;
    bool is_reading_ = false;
    mutable std::mutex mutex_;
  };

} // namespace gcs::transport

#endif // GCS_CORE_TRANSPORT_SERIAL_MANAGER_H_