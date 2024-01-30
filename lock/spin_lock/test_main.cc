#include <iostream>
#include <thread>
#include <vector>

#include "spin_lock.h"

const int thread_cnt = 10;
const int inc_cnt = 100000;
int shared_var;
SpinLock spin_lock;

void do_increment() {
  for (int i = 0; i < inc_cnt; i++) {
    spin_lock.lock();
    shared_var++;
    spin_lock.unlock();
  }
}
int main(int argc, char** argv) {
  std::cout << "------------------Test lock------------------" << std::endl;
  std::vector<std::thread> workers;

  for (int i = 0; i < thread_cnt; i++) {
    workers.emplace_back(std::thread(do_increment));
  }
  for (auto& i : workers) {
    i.join();
  }

  std::cout << "Expected shared_var:" << thread_cnt * inc_cnt << std::endl;
  std::cout << "shared_var:" << shared_var << std::endl;
  if (shared_var == thread_cnt * inc_cnt) {
    std::cout << "passed!" << std::endl;
  } else {
    std::cout << "failed!" << std::endl;
  }
  return 0;
}
