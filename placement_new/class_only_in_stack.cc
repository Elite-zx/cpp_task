#include <iostream>

class __class_in_stack {
 public:
  void* operator new(std::size_t) {}
  void operator delete(void*) {}
};
