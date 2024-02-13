#include <iostream>
#include <string>
// 假设你的shared_ptr和weak_ptr实现在"MyMemory.h"头文件中
#include "shared_ptr.h"

class TestClass {
 public:
  TestClass(const std::string& msg) : message(msg) {
    std::cout << "TestClass created with message: " << message << std::endl;
  }
  ~TestClass() {
    std::cout << "TestClass destroyed with message: " << message << std::endl;
  }
  void sayHello() const { std::cout << message << std::endl; }

 private:
  std::string message;
};

void testSharedWeakPtr() {
  // 创建一个shared_ptr
  shared_ptr<TestClass> shared1(new TestClass("Hello, shared_ptr"));

  {
    // 复制shared_ptr
    auto shared2 = shared1;
    std::cout << "Use count after copy: " << shared1.use_count() << std::endl;

    // 创建weak_ptr
    /* weak_ptr<TestClass> weak1 = shared1; */
    /* std::cout << "Use count after weak_ptr creation: " << shared1.use_count()
     */
    /*           << std::endl; */

    // 从weak_ptr提升为shared_ptr
    /* auto shared3 = weak1.lock(); */
    auto shared3 = shared2;
    if (shared3) {
      std::cout << "Successfully locked weak_ptr, use count: "
                << shared1.use_count() << std::endl;
      shared3->sayHello();
    }

    // 测试移动构造函数
    auto shared4 = std::move(shared3);
    if (!shared3) {
      std::cout << "shared3 is null after move" << std::endl;
    }
    shared4->sayHello();
  }  // shared2和shared3被销毁

  std::cout << "Use count before leaving scope: " << shared1.use_count()
            << std::endl;
}

int main() {
  testSharedWeakPtr();
  return 0;
}
