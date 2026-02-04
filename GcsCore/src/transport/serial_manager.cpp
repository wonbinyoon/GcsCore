// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include "transport/serial_manager.h"

#include <winrt/Windows.Devices.Enumeration.h>

#include <vector>

#include "common/config.h"
#include "logging_internal.h"

namespace gcs::transport
{

  namespace winrt_foundation = winrt::Windows::Foundation;
  namespace winrt_serial = winrt::Windows::Devices::SerialCommunication;
  namespace winrt_stream = winrt::Windows::Storage::Streams;
  namespace winrt_enum = winrt::Windows::Devices::Enumeration;

  SerialManager::SerialManager()
  {
    gcs::logging::InitLogger();
  }

  SerialManager::~SerialManager() { Close(); }

  std::vector<SerialPortInfo> SerialManager::GetPortList()
  {
    auto devices = winrt_enum::DeviceInformation::FindAllAsync(
                       winrt_serial::SerialDevice::GetDeviceSelector())
                       .get();

    std::vector<SerialPortInfo> port_list;
    port_list.reserve(devices.Size());

    for (const auto &device : devices)
    {
      port_list.push_back({winrt::to_string(device.Name()), winrt::to_string(device.Id())});
    }

    return port_list;
  }

  SerialPortInfo SerialManager::GetConnectedPortInfo() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_port_info_;
  }

  winrt_foundation::IAsyncOperation<bool> SerialManager::OpenAsync(
      std::string device_id)
  {
    try
    {
      GCS_LOG_INFO("Attempting to open port: {}", device_id);

      // Prevent multiple simultaneous open calls
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (serial_device_)
          co_return false;
      }

      auto hstring_id = winrt::to_hstring(device_id);
      auto device = co_await winrt_serial::SerialDevice::FromIdAsync(hstring_id);

      if (device != nullptr)
      {
        auto device_info =
            co_await winrt_enum::DeviceInformation::CreateFromIdAsync(hstring_id);

        SerialPortInfo info_copy;
        {
          std::lock_guard<std::mutex> lock(mutex_);
          serial_device_ = device;
          current_port_info_ = {winrt::to_string(device_info.Name()), winrt::to_string(device_info.Id())};

          serial_device_.BaudRate(gcs::common::kSerialBaudRate);
          serial_device_.IsDataTerminalReadyEnabled(true);
          serial_device_.IsRequestToSendEnabled(true);
          serial_device_.DataBits(8);
          serial_device_.StopBits(winrt_serial::SerialStopBitCount::One);
          serial_device_.Parity(winrt_serial::SerialParity::None);
          serial_device_.Handshake(winrt_serial::SerialHandshake::None);

          serial_device_.ReadTimeout(gcs::common::kSerialReadTimeout);
          serial_device_.WriteTimeout(gcs::common::kSerialWriteTimeout);

          reader_ = winrt_stream::DataReader(serial_device_.InputStream());
          reader_.InputStreamOptions(winrt_stream::InputStreamOptions::Partial);
          info_copy = current_port_info_;
        } // Lock is released here

        StartReadLoop();

        GCS_LOG_INFO("Successfully connected to {}", info_copy.name);
        OnPortOpened.Invoke(info_copy);
        co_return true;
      }
      else
      {
        GCS_LOG_ERROR("Failed to open port: Device not found or access denied.");
      }
    }
    catch (const winrt::hresult_error &ex)
    {
      GCS_LOG_ERROR("Error opening port: {}", winrt::to_string(ex.message()));
    }
    catch (...)
    {
      GCS_LOG_ERROR("Unknown error occurred while opening port.");
    }
    co_return false;
  }

  winrt_foundation::IAsyncOperation<std::uint32_t> SerialManager::WriteAsync(
      winrt::array_view<std::uint8_t const> data)
  {
    winrt_serial::SerialDevice device{nullptr};
    {
      std::lock_guard<std::mutex> lock(mutex_);
      device = serial_device_;
    }

    if (!device)
    {
      GCS_LOG_WARN("Attempted to write to a closed port.");
      co_return 0;
    }

    winrt_stream::DataWriter writer(device.OutputStream());
    writer.WriteBytes(data);

    std::uint32_t bytes_written = co_await writer.StoreAsync();
    // co_await writer.FlushAsync();
    writer.DetachStream();

    GCS_LOG_DEBUG("Sent {} bytes", bytes_written);
    co_return bytes_written;
  }

  void SerialManager::Close()
  {
    SerialPortInfo last_info;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!serial_device_)
        return;

      last_info = current_port_info_;
      GCS_LOG_INFO("Closing port: {}", last_info.name);

      is_reading_ = false;
      if (reader_)
      {
        reader_.Close();
        reader_ = nullptr;
      }
      if (serial_device_)
      {
        serial_device_.Close();
        serial_device_ = nullptr;
      }

      current_port_info_ = {"", ""};
    } // Lock is released here

    OnPortClosed.Invoke(last_info);
  }

  winrt::fire_and_forget SerialManager::StartReadLoop()
  {
    auto lifetime = shared_from_this();
    {
      std::lock_guard<std::mutex> lock(mutex_);
      is_reading_ = true;
    }

    GCS_LOG_DEBUG("Background read loop started.");

    try
    {
      while (true)
      {
        winrt_stream::DataReader current_reader{nullptr};
        {
          std::lock_guard<std::mutex> lock(mutex_);
          if (!is_reading_ || !reader_)
            break;
          current_reader = reader_;
        }

        std::uint32_t bytes_read = co_await current_reader.LoadAsync(gcs::common::kSerialReadBufferSize);

        if (bytes_read > 0)
        {
          std::vector<std::uint8_t> buffer(bytes_read);
          current_reader.ReadBytes(buffer);
          GCS_LOG_TRACE("Received {} bytes", bytes_read);
          OnRawDataReceived.Invoke(buffer);
        }
      }
    }
    catch (const winrt::hresult_error &ex)
    {
      if (ex.code() != winrt::hresult(0x800703E3))
      {
        GCS_LOG_ERROR("Read loop error: {}", winrt::to_string(ex.message()));
      }
      Close();
    }
  }

} // namespace gcs::transport