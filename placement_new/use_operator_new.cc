#include <iostream>

class A {
 public:
  A() { std::cout << "constructor" << std::endl; }
  ~A() { std::cout << "destructor" << std::endl; }

  int a;
  int b;
};

template <typename T>
void test() {
  T *p =
      static_cast<A *>(::operator new(sizeof(T)));  // allocate memory for a T
  new (p) T;             // construct a T into the allocated memory
  p->~T();               // destroy the T again
  ::operator delete(p);  // deallocate the memory
}

int main(int argc, char **argv) {
  test<A>();
  return 0;
}
