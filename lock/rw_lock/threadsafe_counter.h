#include <mutex>
#include <shared_mutex>

class ThreadSafeCounter {
 public:
  ThreadSafeCounter() : cnt_value(0) {}
  ThreadSafeCounter(ThreadSafeCounter&) = delete;

  unsigned int get() const {
    std::shared_lock<std::shared_mutex> lck(rw_mutex);
    return cnt_value;
  }

  unsigned int inc() {
    std::unique_lock<std::shared_mutex> lck(rw_mutex);
    return ++cnt_value;
  }

  void reset() {
    std::unique_lock<std::shared_mutex> lck(rw_mutex);
    cnt_value = 0;
  }

 private:
  mutable std::shared_mutex rw_mutex;
  unsigned int cnt_value;
};
