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

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <thread>

namespace timerlat_load {

// The directory in which the timerlat file descriptor opened by RTLA appears.
constexpr char TRACETLD[] = "/sys/kernel/tracing/osnoise/per_cpu/cpu";
// The core count on both of my test systems.
constexpr uint16_t CORES = 8U;
// Size of container which holds a std::chrono::duration<uint64_t,
// std::nano>.count().
constexpr size_t PIPE_BUF_SIZE =
    sizeof(std::chrono::steady_clock::duration::max()) + 1U;
constexpr size_t LIMIT = 100;
constexpr std::chrono::duration<int, std::nano> SLEEP_TIME =
    std::chrono::duration<int, std::nano>{1};
constexpr std::chrono::duration<int, std::nano> THRESHOLD =
    std::chrono::duration<int, std::nano>{100};
// Copied from
// https://github.com/frc971/971-Robot-Code/blob/acfda878d17c2981040c6904fab3d718e2d4bc67/aos/ipc_lib/named_pipe_latency.cc#L76
constexpr char STOP_WORD[] = "00000000";
namespace fs = std::filesystem;

constexpr std::chrono::nanoseconds convert_ns(const struct timespec &ts) {
  std::chrono::duration<int, std::nano> nanosecs{
      static_cast<int>((1e9 * ts.tv_sec) + ts.tv_nsec)};
  return nanosecs;
}

std::optional<std::filesystem::path> create_fifo_dir();
void responding_fn(const std::string &fifopath);

class FifoTimer {
public:
  FifoTimer();
  FifoTimer(const fs::path &fifodir)
      : fifodir_(fs::path(fifodir.string() + "/myfifo")) {}
  // Note that any open file is automatically closed when the fstream object is
  // destroyed.
  ~FifoTimer() {
    if (responder_.joinable()) {
      stop();
    }
    if (fs::exists(fifodir_)) {
      fs::remove_all(fifodir_);
    }
  }

  bool start();
  bool create_responder(std::function<void(const std::string &)> fn);
  void calculate_roundtrip_delays(std::ifstream &tlfs);
  std::string fifodir() const { return fifodir_.string(); }
  // Only for unit tests.
  void set_fifodir(const std::string &fifodir) { fifodir_ = fifodir; }
  std::ifstream ifs;
  void stop() { responder_.join(); }

private:
  std::thread responder_;
  std::filesystem::path fifodir_;
};

} // namespace timerlat_load

#endif
