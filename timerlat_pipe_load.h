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
#include <ctime>
#include <fstream>
#include <iostream>
#include <optional>

// The directory in which the timerlat file descriptor opened by RTLA appears.
constexpr char TRACETLD[] = "/sys/kernel/tracing/osnoise/per_cpu/cpu";
// The core count on both of my test systems.
constexpr uint16_t CORES = 8U;
// Container size which receives characters from the pipe read.
constexpr size_t PIPE_BUF_SIZE = sizeof(struct timespec);
// 1 millisecond.
constexpr size_t THRESHOLD = 100 * 100 * 100;

namespace timerlat_load {

constexpr uint64_t convert_ns(const struct timespec &ts) {
  return (1e9 * ts.tv_sec) + ts.tv_nsec;
}

// Get the current CLOCK_MONOTONIC time in ns.
std::optional<uint64_t> get_ns();

std::string create_fifo_path();

pid_t spawn_responding_thread(const std::string &fifopath);

void calculate_roundtrip_delays(std::ifstream &tlfs);

} // namespace timerlat_load

#endif
