#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "threadsafe_counter.h"

std::mutex print_mtx;
ThreadSafeCounter cnt;
void do_read(int id) {
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::lock_guard<std::mutex> lck(print_mtx);
    std::cout << "read " << id << ": get value " << cnt.get() << std::endl;
  }
}

void do_write(int id) {
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::lock_guard<std::mutex> lck(print_mtx);
    std::cout << "write " << id << ": increment value to " << cnt.inc()
              << std::endl;
  }
}
int main(int argc, char** argv) {
  std::vector<std::thread> read;
  std::vector<std::thread> write;
  for (int i = 0; i < 10; i++) {
    read.emplace_back(std::thread(do_read, i));
  }
  for (int i = 0; i < 10; i++) {
    write.emplace_back(std::thread(do_write, i));
  }

  for (auto& i : read) {
    i.join();
  }
  for (auto& i : write) {
    i.join();
  }

  return 0;
}
