#include <cstdint>
#include <cstring>
#include <iostream>

class A {
 public:
  int x;
  A() { std::memset(this, 0, sizeof(x) + sizeof(uintptr_t *)); }

  virtual void vfunc() { std::cout << "virtual me" << std::endl; }
};

int main(int argc, char **argv) {
  A *a = new A();
  a->vfunc();
  return 0;
}
