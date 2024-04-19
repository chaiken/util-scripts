#include "timerlat_load.h"

#include <sched.h>
#include <unistd.h>

#include <array>
#include <memory>

namespace timerlat_load {

int set_affinity(const pid_t pid, const uint16_t cpu) {
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(cpu, &cpu_set);
  if (-1 == sched_setaffinity(pid, sizeof(cpu_set), &cpu_set)) {
    const int save_errno = errno;
    std::cerr << "Unable to set CPU affinity " << std::to_string(cpu)
              << " for PID " << std::to_string(pid) << ": "
              << strerror(save_errno) << std::endl;
    return save_errno;
  }
  return 0;
}

int set_prio(const pid_t pid, const int prio) {
  const struct sched_param param {
    prio
  };
  if (-1 == sched_setscheduler(pid, SCHED_FIFO, &param)) {
    const int save_errno = errno;
    std::cerr << "Unable to set priority " << std::to_string(prio)
              << " for pid " << std::to_string(pid) << ": " << strerror(errno)
              << std::endl;
    return save_errno;
  }
  return 0;
}

ssize_t read_buffs(std::ifstream &tlfs, std::ifstream &devfs) {
  //  The timerlatfd  is always EOF.
  if (!tlfs.good()) {
    return 0;
  }
  while (devfs.good()) {
    std::string snippet(BYTES, '\0');
    tlfs.read(&snippet[0], 1);
    devfs.read(&snippet[0], BYTES - 1);
  }
  return (tlfs.gcount() + devfs.gcount());
}

} // namespace timerlat_load
