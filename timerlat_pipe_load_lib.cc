#include "timerlat_pipe_load.hh"

#include <cassert>
#include <charconv>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace timerlat_load {

using namespace std::chrono;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

void responding_fn(const std::string &fifopath) {
  const std::string fifoname{fifopath + "/myfifo"};
  int write_fd = openat(-1 /*NOT USED*/, fifoname.c_str(), O_WRONLY);
  if (-1 == write_fd) {
    std::cerr << "Unable to open FIFO " << fifoname
              << " for writing: " << strerror(errno) << std::endl;
    std::cerr.flush();
    return;
  }
  size_t ctr = 0;
  char pipe_buf[PIPE_BUF_SIZE];
  while (ctr++ < LIMIT) {
    time_point<steady_clock> tp = steady_clock::now();
    duration<uint64_t, std::nano> now_ref = tp.time_since_epoch();
    memcpy(pipe_buf, &now_ref, sizeof(now_ref));
    //    pipe_buf[PIPE_BUF_SIZE-1U] = '\0';
    // Include terminating NULL.
    ssize_t bytes_written = write(write_fd, pipe_buf, PIPE_BUF_SIZE);
    if ((-1 == bytes_written) &&
        ((EAGAIN != errno) || (EWOULDBLOCK != errno))) {
      std::cerr << "Unable to write pipe: " << strerror(errno) << std::endl;
      break;
    }
    if (0 == bytes_written) {
      std::cerr << "Wrote 0 bytes from : " << pipe_buf << std::endl;
    }
    /*
      Use of sched_yield() with nondeterministic scheduling  policies  such  as
      SCHED_OTHER is unspecified and very likely means your application design
      is broken. If the calling thread is the only thread in the highest
      priority list at that time, it will continue to run after a call to
      sched_yield().
    */
    // sched_yield();
    std::this_thread::sleep_for(SLEEP_TIME);
  }
  strncpy(pipe_buf, STOP_WORD, PIPE_BUF_SIZE);
  write(write_fd, pipe_buf, PIPE_BUF_SIZE);
  close(write_fd);
}

FifoTimer::FifoTimer() {
  char path_name[L_tmpnam];
  std::string randdir{tmpnam(path_name)};
  fifodir_ = fs::path(randdir);
}

// Convenient for tests.
bool FifoTimer::create_responder(std::function<void(const std::string &)> fn) {
  if (!fn) {
    std::cerr << "Supplied thread function is not executable." << std::endl;
    return false;
  }
  responder_ = std::thread(fn, fifodir_.string());
  if (!responder_.joinable()) {
    std::cerr << "Failed to launch responder thread." << std::endl;
    return false;
  }
  return true;
}

bool FifoTimer::start() {
  if (!fs::create_directory(fifodir_)) {
    std::cerr << "Fifo directory creation at " << fifodir_.string() << " failed"
              << std::endl;
    return false;
  }
  const std::string fifoname(fifodir_.string() + "/myfifo");
  if (-1 == mkfifoat(-1 /*NOT USED*/, fifoname.c_str(), 0777)) {
    std::cerr << "Unable to create FIFO at " << fifoname << ": "
              << strerror(errno) << std::endl;
    return false;
  }
  std::cout << "Created fifo at " << fifoname << std::endl;

  std::function<void(const std::string &)> fn = responding_fn;
  if (!create_responder(fn)) {
    std::cerr << "Unable to spawn responder thread." << std::endl;
    return false;
  }
  ifs = std::ifstream{fifoname, std::ifstream::in};
  if (!ifs.good()) {
    std::cerr << "Unable to open FIFO for reading: " << strerror(errno)
              << std::endl;
    return false;
  }
  return true;
}

void FifoTimer::calculate_roundtrip_delays(std::ifstream &tlfs) {
  if (!tlfs.good()) {
    return;
  }
  std::string trash(2, '\0');
  char pipe_buffer[PIPE_BUF_SIZE];
  while (!ifs.eof()) {
    // Tickle the timerlat file descriptor.
    tlfs.read(&trash[0], 1);

    const fs::path fifopath(fifodir_.string() + "/myfifo");
    if (!(ifs.is_open() && ifs.good() && fs::is_fifo(fifopath))) {
      std::cerr << "Pipe is closed." << std::endl;
      return;
    }
    errno = 0;
    ifs.read(pipe_buffer, PIPE_BUF_SIZE);
    if (!strcmp(STOP_WORD, pipe_buffer)) {
      std::cout << "DONE" << std::endl;
      return;
    }
    if (!ifs.good()) {
      std::cerr << "Pipe read failed: " << strerror(errno) << std::endl;
      if ((EAGAIN != errno) && (EWOULDBLOCK != errno)) {
        return;
      }
    }
    size_t bytes_read = ifs.gcount();
    if (PIPE_BUF_SIZE != bytes_read) {
      std::cerr << "Bad read of " << bytes_read << " bytes." << std::endl;
      continue;
    }
    duration<uint64_t, std::nano> then;
    // Don't need the NULL for a time_point.
    memcpy(&then, pipe_buffer, PIPE_BUF_SIZE - 1U);

    time_point<steady_clock> tp = steady_clock::now();
    duration<uint64_t, std::nano> delay = tp.time_since_epoch() - then;
    if ((delay / 1000) > THRESHOLD) {
      std::cout << std::to_string(delay.count()) << +" micros" << std::endl;
    }
  }
}

} // namespace timerlat_load
