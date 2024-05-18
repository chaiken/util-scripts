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
#include <fstream>
#include <iostream>
#include <optional>
#include <thread>

// The directory in which the timerlat file descriptor opened by RTLA appears.
constexpr char TRACETLD[] = "/sys/kernel/tracing/osnoise/per_cpu/cpu";
// The core count on both of my test systems.
constexpr uint16_t CORES = 8U;
// Size of container which holds a uint64_t timestamp from the pipe read.
constexpr size_t PIPE_BUF_SIZE = 8;
// 1 millisecond.
constexpr size_t THRESHOLD = 100 * 100 * 100;
constexpr size_t LIMIT = 100;

namespace timerlat_load {

constexpr uint64_t convert_ns(const struct timespec &ts) {
  return (1e9 * ts.tv_sec) + ts.tv_nsec;
}

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
    read_fd_ = open(fifopath_.c_str(), O_RDONLY | O_NONBLOCK);
    if (-1 == read_fd_) {
      std::cerr << "Unable to open FIFO for reading: " << strerror(errno)
                << std::endl;
    }
  }
  void calculate_roundtrip_delays(std::ifstream &tlfs);
  std::string fifopath() const { return fifopath_; }
  int read_fd() const { return read_fd_; }
  void stop() { responder_.join(); }

  ~FifoTimer() {
    // responder_.join();
    close(read_fd_);
    unlink(fifopath_.c_str());
  }

private:
  std::thread responder_;
  const std::string fifopath_;
  int read_fd_ = -1;
};

// Get the current CLOCK_MONOTONIC time in ns.
std::optional<uint64_t> get_ns();

} // namespace timerlat_load

#endif
