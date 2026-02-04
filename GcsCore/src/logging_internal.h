// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_LOGGING_INTERNAL_H_
#define GCS_CORE_LOGGING_INTERNAL_H_

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace gcs::logging
{
  /**
   * @brief Initializes the internal logger.
   * Called once when the library is used.
   */
  inline void InitLogger()
  {
    static bool initialized = false;
    if (!initialized)
    {
      auto console = spdlog::get("GcsCore");
      if (!console)
      {
        console = spdlog::stdout_color_mt("GcsCore");
        console->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
#ifdef _DEBUG
        console->set_level(spdlog::level::debug);
#else
        console->set_level(spdlog::level::info);
#endif
      }
      initialized = true;
    }
  }

  inline std::shared_ptr<spdlog::logger> GetLogger()
  {
    InitLogger();
    return spdlog::get("GcsCore");
  }
}

#define GCS_LOG_TRACE(...) if (auto l = gcs::logging::GetLogger()) l->trace(__VA_ARGS__)
#define GCS_LOG_DEBUG(...) if (auto l = gcs::logging::GetLogger()) l->debug(__VA_ARGS__)
#define GCS_LOG_INFO(...)  if (auto l = gcs::logging::GetLogger()) l->info(__VA_ARGS__)
#define GCS_LOG_WARN(...)  if (auto l = gcs::logging::GetLogger()) l->warn(__VA_ARGS__)
#define GCS_LOG_ERROR(...) if (auto l = gcs::logging::GetLogger()) l->error(__VA_ARGS__)

#endif // GCS_CORE_LOGGING_INTERNAL_H_
