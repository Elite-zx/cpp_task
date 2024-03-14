#include <atomic>

class Spin_Lock {
 public:
  Spin_Lock() = default;

  void lock() {
    while (af.test_and_set())
      ;
  }

  void unlock() { af.clear(); }

 private:
  std::atomic_flag af;
}
