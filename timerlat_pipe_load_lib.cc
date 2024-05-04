#include "timerlat_pipe_load.h"

#include <cassert>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>

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

void *responding_fn(void *arg) {
  char *fifopath = static_cast<char *>(arg);
  assert(nullptr != fifopath);
  int write_fd = open(fifopath, O_WRONLY | O_NONBLOCK);
  free(arg);
  if (-1 == write_fd) {
    std::cerr << "Unable to open FIFO for writing: " << strerror(errno)
              << std::endl;
    return NULL;
  }
  while (1) {
    std::optional<uint64_t> ns = get_ns();
    if (!ns.has_value()) {
      return NULL;
    }
    std::string timestr = std::to_string(ns.value());
    char *ts = strdup(timestr.c_str());
    ssize_t bytes_read =
        write(write_fd, static_cast<const void *>(ts), timestr.length());
    free(ts);
    if ((-1 == bytes_read) && ((EAGAIN != errno) || (EWOULDBLOCK != errno))) {
      std::cerr << "Unable to write pipe: " << strerror(errno) << std::endl;
      break;
    }
  }
  close(write_fd);
  return NULL;
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

pid_t spawn_responding_thread(const std::string &fifopath) {
  pthread_t tid;
  // Freed by thread.
  char *temp = strdup(fifopath.c_str());
  if (-1 ==
      pthread_create(&tid, nullptr, responding_fn, static_cast<void *>(temp))) {
    std::cerr << "Unable to create pthread: " << strerror(errno) << std::endl;
    free(temp);
    return -1;
  }
  std::cout << "Spawned TID " << tid << std::endl;
  // "Sometimes, we don’t care about the thread’s return status; we simply want
  // the system to automatically clean up and remove the thread when it
  // terminates."
  pthread_detach(tid);
  return tid;
}

void calculate_roundtrip_delays(std::ifstream &tlfs) {
  if (!tlfs.good()) {
    return;
  }
  const std::string fifopath = create_fifo_path();
  if (-1 == mkfifoat(-1 /*NOT USED*/, fifopath.c_str(), 0777)) {
    std::cerr << "Unable to create FIFO: " << strerror(errno) << std::endl;
    return;
  }
  const pid_t child = spawn_responding_thread(fifopath);
  if (-1 == child) {
    return;
  }
  int read_fd = open(fifopath.c_str(), O_RDONLY | O_NONBLOCK);
  if (-1 == read_fd) {
    std::cerr << "Unable to open FIFO for reading: " << strerror(errno)
              << std::endl;
  }

  while (1) {
    // Tickle the timerlat file descriptor.
    std::string trash(2, '\0');
    tlfs.read(&trash[0], 1);

    char pipe_buffer[PIPE_BUF_SIZE + 1U];
    ssize_t bytes_read = read(read_fd, pipe_buffer, PIPE_BUF_SIZE);
    if (errno && (EAGAIN != errno) && (EWOULDBLOCK != errno)) {
      std::cerr << "Pipe read failed: " << strerror(errno) << std::endl;
      break;
    }
    uint64_t then = 0;
    if (PIPE_BUF_SIZE == bytes_read) {
      struct timespec ts;
      memcpy(&ts, pipe_buffer, PIPE_BUF_SIZE);
      then = convert_ns(ts);
    }
    std::optional<uint64_t> now = get_ns();
    if (!now.has_value()) {
      break;
    }
    uint64_t delay = 0U;
    if (then) {
      delay = now.value() - then;
    }
    if (delay > THRESHOLD) {
      std::cout << std::to_string(delay) + " ns" << std::endl;
    }
  }
  close(read_fd);
}

} // namespace timerlat_load
