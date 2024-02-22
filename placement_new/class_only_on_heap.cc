#include <iostream>

class __class_in_heap {
 public:
  __class_in_heap() { std::cout << "constructor" << std::endl; }
  void destroy() { delete this; }

 private:
  ~__class_in_heap(){};
};

class Foo {
 public:
  ~Foo();
  static Foo *createFoo() { return new Foo(); }

  Foo(const Foo &) = delete;
  Foo &operator=(const Foo &) = delete;
  Foo(Foo &&) = delete;
  Foo &operator=(Foo &&) = delete;

 private:
  Foo() { std::cout << "constructor" << std::endl; };
};

int main(int argc, char **argv) {
  auto p = Foo::createFoo();
  return 0;
}
