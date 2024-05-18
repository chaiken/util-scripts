#include "timerlat_pipe_load.h"

#include <cassert>

#include <cstring>
#include <fstream>

namespace timerlat_load {

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
    std::string timestr = std::to_string(ns.value());
    //    char *ts = strdup(timestr.c_str());
    ssize_t bytes_read = write(write_fd, timestr.c_str(), timestr.length());
    //        write(write_fd, static_cast<const void *>(ts), timestr.length());
    if ((-1 == bytes_read) && ((EAGAIN != errno) || (EWOULDBLOCK != errno))) {
      std::cerr << "Unable to write pipe: " << strerror(errno) << std::endl;
      break;
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
    ssize_t bytes_read = read(read_fd_, pipe_buffer, PIPE_BUF_SIZE);
    if (errno && (EAGAIN != errno) && (EWOULDBLOCK != errno)) {
      std::cerr << "Pipe read failed: " << strerror(errno) << std::endl;
      break;
    }
    uint64_t then = 0;
    if (PIPE_BUF_SIZE == bytes_read) {
      memcpy(&then, pipe_buffer, PIPE_BUF_SIZE);
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
