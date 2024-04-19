// Reimplement linux/tools/tracing/rtla/sample/timerlat_load.py as C++.

#include "timerlat_load.h"

#include <sched.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;
using namespace timerlat_load;

void usage(const std::string &prog) {
  cerr << prog << " PRIORITY (<= " << MAX_PRIO << ") CPU (<" << CORES << ")"
       << endl;
}

int main(int argc, char **argv) {
  if (geteuid()) {
    cerr << argv[0] << " is only runnable as root." << endl;
    exit(EXIT_FAILURE);
  }
  if ((3 > argc) || (4 < argc)) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  const int32_t prio = strtol(argv[1], nullptr, 10);
  if (errno || (0 >= prio) || (MAX_PRIO < prio)) {
    cerr << "Illegal priority " << argv[1] << endl;
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  const uint16_t cpu = strtol(argv[2], nullptr, 10);
  if (errno || (CORES <= cpu)) {
    cerr << "Illegal cpu " << argv[2] << endl;
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  const pid_t pid = getpid();
  if ((set_affinity(pid, cpu)) || set_prio(pid, prio)) {
    exit(EXIT_FAILURE);
  }

  const string tl_path = string{TRACETLD} + to_string(cpu) + "/timerlat_fd"s;
  ifstream tlfs(tl_path, ifstream::in);
  if (!tlfs.good()) {
    cerr << "Unable to open file " << tl_path << endl;
    exit(EXIT_FAILURE);
  }
  const string dev_path = DEVPATH;
  ifstream devfs(dev_path, ifstream::in);
  if (!devfs.good()) {
    cerr << "Unable to open file " << dev_path << endl;
    exit(EXIT_FAILURE);
  }
  read_buffs(tlfs, devfs);
  tlfs.close();
  devfs.close();
  exit(EXIT_SUCCESS);
}
