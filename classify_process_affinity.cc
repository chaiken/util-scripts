#include "classify_process_affinity.hh"

#include <cstdlib>
#include <iostream>

using namespace process_affinity;

int main(int arg, char **argv) {
  std::set<struct tid_data, decltype(tid_data_compare) *> tset{
      tid_data_compare};
  read_thread_data(tset);
  for (struct tid_data td : tset) {
    std::cout << td.thread_name << ": ";
    if (td.is_settable) {
      std::cout << "pinnable." << std::endl;
    } else {
      std::cout << "unpinnable" << std::endl;
    }
  }
  exit(EXIT_SUCCESS);
}
