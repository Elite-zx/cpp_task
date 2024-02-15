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
  void sayHello() const { std::cout << message << std::endl; }
  void testSharedFromThis() {
    auto self = shared_from_this();
    std::cout << "Using shared_from_this: ";
    self->sayHello();
  }

 private:
  std::string message;
};

void testSharedWeakPtr() {
  auto shared1 = make_shared<TestClass>("Hello, shared_ptr");

  {
    auto shared2 = shared1;
    std::cout << "Use count after copy (2): " << shared1.use_count()
              << std::endl;

    shared1->testSharedFromThis();

    weak_ptr<TestClass> weak1 = shared1;
    std::cout << "Use count after weak_ptr creation (2): "
              << shared1.use_count() << std::endl;

    if (auto shared3 = weak1.lock()) {
      std::cout << "Successfully locked weak_ptr, use count (3): "
                << shared1.use_count() << std::endl;
      shared3->sayHello();
    } else {
      std::cerr << "Failed to lock weak_ptr, object might be destroyed."
                << std::endl;
    }
  }

  std::cout << "Use count before leaving scope (1): " << shared1.use_count()
            << std::endl;

  // 模拟weak_ptr所指对象已被释放
  weak_ptr<TestClass> expiredWeak = shared1;
  shared1.reset();  // 显式释放shared1所指对象

  auto sharedFromExpired = expiredWeak.lock();
  if (!sharedFromExpired) {
    std::cout << "expiredWeak is expired, lock() return a empty shared_ptr"

              << std::endl;
  }
}

int main() {
  testSharedWeakPtr();
  return 0;
}
