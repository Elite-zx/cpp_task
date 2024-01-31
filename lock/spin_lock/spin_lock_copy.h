#include <atomic>

class SpinLock {
 public:
  SpinLock() = default;

  void lock() {
    while (af.test_and_set())
      ;
  }

  void unlock() { af.clear(); }

 private:
  std::atomic_flag af;
};
