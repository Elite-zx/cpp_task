#include <iostream>
#include <ostream>

class __class_on_stack {
 public:
  __class_on_stack() { std::cout << "costructor" << std::endl; }
  static void* operator new(std::size_t) = delete;
  static void* operator new[](std::size_t) = delete;
  static void operator delete(void*) = delete;
  static void operator delete[](void*) = delete;

 private:
  int x;
};

class Foo {
  __class_on_stack obj;
};

int main(int argc, char** argv) {
  Foo* foo = new Foo();

  __class_on_stack* p = ::new __class_on_stack();
  return 0;
}
