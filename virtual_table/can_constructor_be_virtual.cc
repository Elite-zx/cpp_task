#include <iostream>

class A {
 public:
  virtual A() { std::cout << "constructor" << std::endl; }
};

int main(int argc, char **argv) {
  A a;
  return 0;
}
