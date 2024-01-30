#include <atomic>

class SpinLock {
 public:
  SpinLock() : ab(false) {}

  void lock() {
    bool expected = false;
    while (!ab.compare_exchange_weak(expected, true)) {
      expected = false;
    };
  }

  void unlock() { ab.store(false); }

 private:
  std::atomic_bool ab;
};
