#include <mutex>

class Singleton {
 public:
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;
  Singleton(Singleton&&) = delete;
  Singleton& operator=(Singleton&&) = delete;

  static Singleton& get_instance() {
    std::call_once(init_flag, &Singleton::init);
    return *instance_ptr;
  }

 private:
  Singleton() = default;
  static std::once_flag init_flag;
  static Singleton* instance_ptr;
  static void init() { instance_ptr = new Singleton(); }
};
