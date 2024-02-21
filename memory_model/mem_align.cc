#include <iostream>

class A {
 public:
  char b;
  int a;
};

class alignas(8) B {
 public:
  char b;
  int a;
};

int main(int argc, char **argv) {
  std::cout << alignof(B) << std::endl;
  std::cout << sizeof(B) << std::endl;

  return 0;
}
