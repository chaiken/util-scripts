#include "timerlat_pipe_load.h"

#include <cassert>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace timerlat_load {

using namespace std::chrono_literals;

void wait_one_ms() { std::this_thread::sleep_for(1ms); }

std::optional<uint64_t> get_ns() {
  struct timespec ts;
  if (-1 == clock_gettime(CLOCK_MONOTONIC, &ts)) {
    std::cerr << "Cannot read monotonic time " << strerror(errno) << std::endl;
    return {};
  }
  return convert_ns(ts);
}

void responding_fn(const std::string &fifopath) {
  int write_fd =
      openat(-1 /*NOT USED*/, fifopath.c_str(), O_WRONLY | O_NONBLOCK);
  if (-1 == write_fd) {
    std::cerr << "Unable to open FIFO " << fifopath
              << " for writing: " << strerror(errno) << std::endl;
    std::cerr.flush();
    return;
  }
  size_t ctr = 0;
  while (ctr++ < LIMIT) {
    std::optional<uint64_t> ns = get_ns();
    if (!ns.has_value()) {
      return;
    }
    std::ostringstream timestr;
    timestr << std::hex << ns.value() << std::dec;
    if (!timestr.str().length()) {
      std::cerr << "timestring is empty!" << std::endl;
    }
    ssize_t bytes_written =
        write(write_fd, timestr.str().c_str(), timestr.str().length());
    if ((-1 == bytes_written) &&
        ((EAGAIN != errno) || (EWOULDBLOCK != errno))) {
      std::cerr << "Unable to write pipe: " << strerror(errno) << std::endl;
      break;
    }
    if (0 == bytes_written) {
      std::cerr << "Wrote 0 bytes from : " << timestr.str() << std::endl;
    }
  }
  close(write_fd);
}

std::string create_fifo_path() {
  char path_template[] = "/tmp/fifodirXXXXXX";
  // Actually creates the directory.  mkdtempat() does not exist.
  char *path = mkdtemp(path_template);
  if (!path) {
    std::cerr << "Temporary direction path creation failed." << std::endl;
    return std::string{};
  }
  return (std::string{path} + std::string{"/myfifo"});
}

void FifoTimer::calculate_roundtrip_delays(std::ifstream &tlfs) {
  if (!tlfs.good()) {
    return;
  }
  while (1) {
    // Tickle the timerlat file descriptor.
    std::string trash(2, '\0');
    tlfs.read(&trash[0], 1);

    char pipe_buffer[PIPE_BUF_SIZE + 1U];
    ifs.readsome(pipe_buffer, PIPE_BUF_SIZE);
    if (!ifs.good()) {
      std::cerr << "Pipe read failed: " << strerror(errno) << std::endl;
      if ((EAGAIN != errno) && (EWOULDBLOCK != errno)) {
        break;
      }
    }
    size_t bytes_read = ifs.gcount();
    if (!bytes_read) {
      continue;
    }
    uint64_t then = 0;
    if (0 < bytes_read) {
      memcpy(&then, pipe_buffer, PIPE_BUF_SIZE - 1U);
    }
    std::optional<uint64_t> now = get_ns();
    if ((!now.has_value()) || (!then)) {
      break;
    }
    uint64_t delay = 0U;
    delay = now.value() - then;
    if (delay > THRESHOLD) {
      std::cout << std::to_string(delay) + " ns" << std::endl;
    }
  }
}

} // namespace timerlat_load
