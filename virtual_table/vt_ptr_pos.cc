#include <iostream>

class A {
 public:
  int x;
};

class B {
 public:
  int x;
  virtual void vfunc() {}
};

int main(int argc, char** argv) {
  A a;
  B b;
  A* pta = &a;
  B* ptb = &b;
  std::cout << reinterpret_cast<void*>(pta) << std::endl;
  std::cout << &a.x << std::endl;

  std::cout << reinterpret_cast<void*>(ptb) << std::endl;
  std::cout << &b.x << std::endl;

  return 0;
}
