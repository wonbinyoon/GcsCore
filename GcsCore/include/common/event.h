#ifndef GCS_CORE_EVENT_H_
#define GCS_CORE_EVENT_H_

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
   * @brief 이벤트 연결을 관리하는 RAII 객체
   * @details 이 객체가 파괴될 때 자동으로 이벤트 구독(Callback)을 해제합니다.
   *          따라서 리스너의 수명 주기를 안전하게 관리할 수 있습니다.
   */
  class ScopedConnection
  {
  public:
    ScopedConnection() = default;
    
    /**
     * @brief 생성자
     * @param unregister 연결 해제 로직이 담긴 함수 객체
     */
    ScopedConnection(std::function<void()> unregister) : m_unregister(unregister) {}
    
    /**
     * @brief 소멸자. 구독을 해제합니다.
     */
    ~ScopedConnection()
    {
      if (m_unregister)
        m_unregister();
    }

    // 이동은 허용, 복사는 금지 (소유권 명확화)
    ScopedConnection(const ScopedConnection &) = delete;
    ScopedConnection &operator=(const ScopedConnection &) = delete;
    ScopedConnection(ScopedConnection &&) = default;
    ScopedConnection &operator=(ScopedConnection &&) = default;

  private:
    std::function<void()> m_unregister;
  };

  // SignalToken: 명확한 타입 별칭 제공
  using SignalToken = std::unique_ptr<ScopedConnection>;

  /**
   * @class Signal
   * @brief 다중 리스너 이벤트 발행자 (Observer Pattern)
   * @tparam Args 이벤트 발생 시 전달할 인자들의 타입
   * @details 스레드 안전하게 여러 콜백 함수를 등록하고 호출할 수 있습니다.
   */
  template <typename... Args>
  class Signal
  {
  public:
    using Callback = std::function<void(Args...)>;

    /**
     * @brief 이벤트 리스너(콜백)를 등록합니다.
     * @param cb 등록할 콜백 함수
     * @return 연결 토큰 (이 토큰을 멤버 변수로 저장해야 구독이 유지됨)
     */
    [[nodiscard]] std::unique_ptr<ScopedConnection> Connect(Callback cb)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      std::uint64_t id = next_id_++;
      callbacks_[id] = std::move(cb);

      // 반환된 객체가 파괴될 때 이 람다가 실행되어 자동 등록 해제됨
      return std::make_unique<ScopedConnection>([this, id]()
                                                {
            std::lock_guard<std::mutex> lock(mutex_);
            callbacks_.erase(id); });
    }

    /**
     * @brief 이벤트를 발생시켜 등록된 모든 콜백을 호출합니다.
     * @param args 전달할 인자들
     */
    void Invoke(Args... args)
    {
      std::vector<Callback> safe_callbacks;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        safe_callbacks.reserve(callbacks_.size());
        for (auto const &[id, cb] : callbacks_)
        {
          safe_callbacks.push_back(cb);
        }
      }

      for (auto const &cb : safe_callbacks)
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

#endif // GCS_CORE_EVENT_H_