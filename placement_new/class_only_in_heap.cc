#include <iostream>

class __class_in_heap {
 public:
  __class_in_heap() { std::cout << "constructor" << std::endl; }
  void destroy() { delete this; }

 private:
  ~__class_in_heap();
}
