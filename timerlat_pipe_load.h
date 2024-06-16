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
    sizeof(std::chrono::duration<uint64_t, std::nano>) + 1U;
constexpr size_t LIMIT = 100;
constexpr std::chrono::duration<int, std::micro> TEN_MICROS =
    std::chrono::duration<int, std::micro>{10};
// Copied from
// https://github.com/frc971/971-Robot-Code/blob/acfda878d17c2981040c6904fab3d718e2d4bc67/aos/ipc_lib/named_pipe_latency.cc#L76
constexpr char STOP_WORD[] = "00000000";
namespace fs = std::filesystem;

constexpr std::chrono::nanoseconds convert_ns(const struct timespec &ts) {
  std::chrono::duration<int, std::nano> nanosecs{
      static_cast<int>((1e9 * ts.tv_sec) + ts.tv_nsec)};
  return nanosecs;
}

void wait_one_ms();

std::optional<std::filesystem::path> create_fifo_dir();
void responding_fn(const std::filesystem::path &fifofile);

class FifoTimer {
public:
  FifoTimer() = default;
  bool start();
  bool create_responder();
  void responding_fn();
  void calculate_roundtrip_delays(std::ifstream &tlfs);
  std::string fifofile() const { return fifofile_; }
  // Only for unit tests.
  void set_fifofile(const std::string &fifofile) { fifofile_ = fifofile; }
  std::ifstream ifs;
  void stop() { responder_.join(); }

  ~FifoTimer() {
    std::cerr << "Destructor" << std::endl;
    if (responder_.joinable()) {
      stop();
    }
    ifs.close();
    fs::remove_all(fifofile_.parent_path());
  }

private:
  std::thread responder_;
  std::filesystem::path fifofile_;
};

} // namespace timerlat_load

#endif
