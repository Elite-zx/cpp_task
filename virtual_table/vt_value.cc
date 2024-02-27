#include <sys/types.h>

#include <iostream>

class Base {
 public:
  virtual void vfunc1() {}
  virtual void vfunc2() { std::cout << "Base vfunc2" << std::endl; }
  virtual void vfunc3() {}
};

class Derived final : public Base {
 public:
  virtual void vfunc2() override {
    std::cout << "Derived vfunc2" << std::endl;
  };
};

using func = void (*)();

int main(int argc, char **argv) {
  auto *b = new Base();
  auto *d = new Derived();
  uintptr_t *vt_p1 = reinterpret_cast<uintptr_t *>(b);
  uintptr_t *vt1 = reinterpret_cast<uintptr_t *>(*vt_p1);
  std::cout << "------Base------" << std::endl;
  for (int i = 0; i < 3; i++) {
    std::cout << std::hex << vt1[i] << std::endl;
  }

  std::cout << "------Derived------" << std::endl;
  uintptr_t *vt_p2 = reinterpret_cast<uintptr_t *>(d);
  uintptr_t *vt2 = reinterpret_cast<uintptr_t *>(*vt_p2);
  for (int i = 0; i < 3; i++) {
    std::cout << std::hex << vt2[i] << std::endl;
  }

  func Base_vfunc2 = reinterpret_cast<func>(vt1[1]);
  func Derived_vfunc2 = reinterpret_cast<func>(vt2[1]);
  std::cout << "the size of vt item: " << sizeof(vt1[0]) << std::endl;
  Base_vfunc2();
  Derived_vfunc2();
  return 0;
}
