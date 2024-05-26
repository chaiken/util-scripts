#ifndef TIMERLAT_LOAD_LIB
#define TIMERLAT_LOAD_LIB

// A reimplementation of
// https://github.com/torvalds/linux/blob/master/tools/tracing/rtla/sample/timerlat_load.py
// in C++. See
// https://github.com/torvalds/linux/blob/master/tools/tracing/rtla/README.txt
// and https://bristot.me/linux-scheduling-latency-debug-and-analysis/
// As of v6.9-rc5, the file Documentation/tools/rtla/common_timerlat_options.rst
// appears in git on localhost, but not at github.com/torvalds.

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <thread>

namespace timerlat_load {

// The directory in which the timerlat file descriptor opened by RTLA appears.
constexpr char TRACETLD[] = "/sys/kernel/tracing/osnoise/per_cpu/cpu";
// The core count on both of my test systems.
constexpr uint16_t CORES = 8U;
// Size of container which holds a uint64_t timestamp from the pipe read.
constexpr size_t PIPE_BUF_SIZE = sizeof(uint64_t);
// 1 millisecond.
constexpr size_t THRESHOLD = 100 * 100 * 100;
constexpr size_t LIMIT = 100;

namespace fs = std::filesystem;

constexpr uint64_t convert_ns(const struct timespec &ts) {
  return (1e9 * ts.tv_sec) + ts.tv_nsec;
}

void wait_one_ms();

std::string create_fifo_path();
void responding_fn(const std::string &fifopath);

class FifoTimer {
public:
  FifoTimer() : fifopath_(create_fifo_path()) {
    if (-1 == mkfifoat(-1 /*NOT USED*/, fifopath_.c_str(), 0777)) {
      std::cerr << "Unable to create FIFO: " << strerror(errno) << std::endl;
      return;
    }
    std::cout << "Created fifo at " << fifopath_ << std::endl;

    responder_ = std::thread([this]() { responding_fn(this->fifopath_); });
    ifs = std::ifstream{fifopath_, std::ifstream::in};
    if (!ifs.good()) {
      std::cerr << "Unable to open FIFO for reading: " << strerror(errno)
                << std::endl;
    }
  }
  void calculate_roundtrip_delays(std::ifstream &tlfs);
  std::string fifopath() const { return fifopath_; }
  std::ifstream ifs;
  void stop() { responder_.join(); }

  ~FifoTimer() {
    stop();
    ifs.close();
    fs::remove_all(fs::path(fifopath_).parent_path());
  }

private:
  std::thread responder_;
  const std::string fifopath_;
};

// Get the current CLOCK_MONOTONIC time in ns.
std::optional<uint64_t> get_ns();

} // namespace timerlat_load

#endif
