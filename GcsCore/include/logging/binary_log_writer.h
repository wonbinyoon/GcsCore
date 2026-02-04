// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_LOGGING_BINARY_LOG_WRITER_H_
#define GCS_CORE_LOGGING_BINARY_LOG_WRITER_H_

#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include "common/event.h"

namespace gcs::interfaces
{
  class IParser;
  class IConverter;
} // namespace gcs::interfaces

namespace gcs::transport
{
  class SerialManager;
} // namespace gcs::transport

namespace gcs::logging
{

  /**
   * @class BinaryLogWriter
   * @brief Handles recording of raw and parsed data to binary files.
   */
  class BinaryLogWriter
  {
  public:
    /**
     * @brief Constructor.
     * @param parser Protocol parser (ownership transferred).
     * @param converter Data converter (ownership transferred).
     * @param log_dir Directory to store log files.
     */
    BinaryLogWriter(std::unique_ptr<gcs::interfaces::IParser> parser,
                    std::unique_ptr<gcs::interfaces::IConverter> converter,
                    const std::string &log_dir);

    /**
     * @brief Destructor.
     */
    ~BinaryLogWriter();

    /**
     * @brief Binds the writer to a SerialManager.
     * @param serial SerialManager to monitor.
     */
    void Bind(gcs::transport::SerialManager &serial);

    /**
     * @brief Starts the logging process. Creates new log files.
     */
    void StartLogging();

    /**
     * @brief Stops the logging process and closes files.
     */
    void StopLogging();

    // Public event tokens for manual management.
    gcs::common::SignalToken on_raw_;
    gcs::common::SignalToken on_packet_;
    gcs::common::SignalToken on_parsed_;
    gcs::common::SignalToken on_opened_connection_;
    gcs::common::SignalToken on_closed_connection_;

  private:
    std::string GetTimestamp() const;

    mutable std::recursive_mutex mutex_;
    std::string log_dir_;
    std::ofstream raw_file_;
    std::ofstream parsed_file_;

    std::unique_ptr<gcs::interfaces::IParser> parser_;
    std::unique_ptr<gcs::interfaces::IConverter> converter_;
  };

} // namespace gcs::logging

#endif // GCS_CORE_LOGGING_BINARY_LOG_WRITER_H_