#include <iostream>
#include <memory>
#include <string>

#include "shared_ptr.h"

class TestClass : public enable_shared_from_this<TestClass> {
 public:
  TestClass(const std::string& msg) : message(msg) {
    std::cout << "TestClass created with message: " << message << std::endl;
  }
  ~TestClass() {
    std::cout << "TestClass destroyed with message: " << message << std::endl;
  }
  void say_hello() const { std::cout << message << std::endl; }
  void test_shared_from_this() {
    auto self = shared_from_this();
    std::cout << "Using shared_from_this: ";
    self->say_hello();
  }

 private:
  std::string message;
};

void test_shared_weak_ptr() {
  auto shared1 = make_shared<TestClass>("Hello, shared_ptr");

  {
    auto shared2 = shared1;
    std::cout << "Use count after copy (2): " << shared1.use_count()
              << std::endl;

    shared1->test_shared_from_this();

    weak_ptr<TestClass> weak1 = shared1;
    std::cout << "Use count after weak_ptr creation (2): "
              << shared1.use_count() << std::endl;

    if (auto shared3 = weak1.lock()) {
      std::cout << "Successfully locked weak_ptr, use count (3): "
                << shared1.use_count() << std::endl;
      shared3->say_hello();
    } else {
      std::cerr << "Failed to lock weak_ptr, object might be destroyed."
                << std::endl;
    }
  }

  std::cout << "Use count before leaving scope (1): " << shared1.use_count()
            << std::endl;

  // 模拟weak_ptr所指对象已被释放
  weak_ptr<TestClass> expired_weak = shared1;
  shared1.reset();  // 显式释放shared1所指对象

  auto shared_from_expired = expired_weak.lock();
  if (!shared_from_expired) {
    std::cout << "expiredWeak is expired, lock() return a empty shared_ptr"
              << std::endl;
  }

  try {
    TestClass tc("Throw bad_weak_ptr");
    tc.test_shared_from_this();
  } catch (const bad_weak_ptr& e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }
}

int main() {
  test_shared_weak_ptr();
  return 0;
}
