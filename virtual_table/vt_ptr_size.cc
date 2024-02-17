#include <iostream>

class A {};
class B {
 public:
  virtual void vfunc() {}
};

int main(int argc, char **argv) {
  A a;
  std::cout << sizeof(a) << std::endl;
  B b;
  std::cout << sizeof(b) << std::endl;
  return 0;
}
