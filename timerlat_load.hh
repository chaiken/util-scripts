#ifndef TIMERLAT_LOAD_LIB
#define TIMERLAT_LOAD_LIB

// A reimplementation of
// https://github.com/torvalds/linux/blob/master/tools/tracing/rtla/sample/timerlat_load.py
// in C++. See
// https://github.com/torvalds/linux/blob/master/tools/tracing/rtla/README.txt
// and https://bristot.me/linux-scheduling-latency-debug-and-analysis/
// As of v6.9-rc5, the file Documentation/tools/rtla/common_timerlat_options.rst
// appears in git on localhost, but not at github.com/torvalds.

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

// A file that the test reads just to keep busy since it is never empty.
constexpr char DEVPATH[] = "/dev/full";
// The directory in which the timerlat file descriptor opened by RTLA appears.
constexpr char TRACETLD[] = "/sys/kernel/tracing/osnoise/per_cpu/cpu";
// The core count on both of my test systems.
constexpr uint16_t CORES = 8U;
// The size of the reads from /dev/full.
constexpr uint32_t BYTES = 20 * 1024 * 1024;
// 20 is perhaps already too high for safety on a non-PREEMPT_RT system.
constexpr int MAX_PRIO = 20;

namespace timerlat_load {

// Set the test process' scheduler to SCHED_FIFO and bind it to a core.
// Requires root privilege.
int set_affinity(const pid_t pid, const uint16_t cpu);

// Set the test process' priority.
// Requires root privilege for RT priorities < 0.
int set_prio(const pid_t pid, const int prio);

// Read the file paths.   Reading tracefs requires root privilege.
ssize_t read_buffs(std::ifstream &tlfs, std::ifstream &devfs);

} // namespace timerlat_load

#endif
