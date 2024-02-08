#include <iostream>
#include <memory>

class B;
class A {
 public:
  std::shared_ptr<B> b;
  ~A() { std::cout << "~A()\n"; }
};

class B {
 public:
  std::weak_ptr<A> a;
  ~B() { std::cout << "~B()\n"; }
};

void useAnB() {
  auto a = std::make_shared<A>();
  auto b = std::make_shared<B>();
  a->b = b;
  b->a = a;
}

int main() {
  useAnB();
  std::cout << "Finished using A and B\n";
}
