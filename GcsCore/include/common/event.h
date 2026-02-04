// Copyright 2026 윤원빈. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GCS_CORE_COMMON_EVENT_H_
#define GCS_CORE_COMMON_EVENT_H_

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace gcs::common
{

  /**
   * @class ScopedConnection
   * @brief An RAII object that manages an event connection.
   *
   * Automatically unsubscribes from the event when this object is destroyed.
   * This ensures the safe management of the listener's lifecycle.
   */
  class ScopedConnection
  {
  public:
    ScopedConnection() = default;

    /**
     * @brief Constructor.
     * @param unregister Function object containing the logic to unregister the
     * connection.
     */
    explicit ScopedConnection(std::function<void()> unregister)
        : unregister_(std::move(unregister)) {}

    /**
     * @brief Destructor. Unsubscribes from the event.
     */
    ~ScopedConnection()
    {
      if (unregister_)
      {
        unregister_();
      }
    }

    // Allow movement, forbid copying for clear ownership.
    ScopedConnection(const ScopedConnection &) = delete;
    ScopedConnection &operator=(const ScopedConnection &) = delete;
    ScopedConnection(ScopedConnection &&) = default;
    ScopedConnection &operator=(ScopedConnection &&) = default;

  private:
    std::function<void()> unregister_;
  };

  /**
   * @brief Type alias for a managed connection token.
   */
  using SignalToken = std::unique_ptr<ScopedConnection>;

  /**
   * @class Signal
   * @brief Multi-listener event publisher (Observer Pattern).
   * @tparam Args Argument types to be passed when the event occurs.
   *
   * Allows multiple callback functions to be registered and invoked in a
   * thread-safe manner.
   */
  template <typename... Args>
  class Signal
  {
  public:
    using Callback = std::function<void(Args...)>;

    /**
     * @brief Registers an event listener (callback).
     * @param cb Callback function to register.
     * @return Connection token. Subscription is maintained as long as this token
     * is kept.
     */
    [[nodiscard]] std::unique_ptr<ScopedConnection> Connect(Callback cb)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      std::uint64_t id = next_id_++;
      callbacks_[id] = std::move(cb);

      // Unregister automatically when the returned object is destroyed.
      return std::make_unique<ScopedConnection>([this, id]()
                                                {
      std::lock_guard<std::mutex> lock(mutex_);
      callbacks_.erase(id); });
    }

    /**
     * @brief Invokes the event, calling all registered callbacks.
     * @param args Arguments to pass to the callbacks.
     */
    void Invoke(Args... args)
    {
      std::vector<Callback> safe_callbacks;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        safe_callbacks.reserve(callbacks_.size());
        for (const auto &[id, cb] : callbacks_)
        {
          safe_callbacks.push_back(cb);
        }
      }

      for (const auto &cb : safe_callbacks)
      {
        cb(args...);
      }
    }

  private:
    std::map<std::uint64_t, Callback> callbacks_;
    std::uint64_t next_id_ = 0;
    std::mutex mutex_;
  };

} // namespace gcs::common

#endif // GCS_CORE_COMMON_EVENT_H_