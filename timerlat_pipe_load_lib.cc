#include "timerlat_pipe_load.h"

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

void FifoTimer::responding_fn() {
  int write_fd = openat(-1 /*NOT USED*/, fifofile_.c_str(), O_WRONLY);
  if (-1 == write_fd) {
    std::cerr << "Unable to open FIFO " << fifofile_
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

std::optional<fs::path> create_fifo_dir() {
  char path_template[] = "/tmp/fifodirXXXXXX";
  char *randdir_name = mkdtemp(path_template);
  if (!randdir_name) {
    std::cerr << "Temporary directory " << randdir_name
              << " creation failed: " << strerror(errno) << std::endl;
    return std::nullopt;
  }
  return fs::path{std::string{randdir_name}};
}

// Convenient for tests.
bool FifoTimer::create_responder() {
  responder_ = std::thread([this]() { responding_fn(); });
  // Check something here.
  return true;
}

bool FifoTimer::start() {
  std::optional<fs::path> fp_res = create_fifo_dir();
  if (!fp_res.has_value()) {
    std::cerr << "FifoTimer creation failed." << std::endl;
    return false;
  }
  fifofile_ = fs::path(fp_res.value().string() + std::string{"/myfifo"});
  if (-1 == mkfifoat(-1 /*NOT USED*/, fifofile_.c_str(), 0777)) {
    std::cerr << "Unable to create FIFO at " << fifofile_.string() << ": "
              << strerror(errno) << std::endl;
    return false;
  }
  std::cout << "Created fifo at " << fifofile_.string() << std::endl;

  if (!create_responder()) {
    std::cerr << "Unable to spawn responder thread." << std::endl;
    return false;
  }
  ifs = std::ifstream{fifofile_.string(), std::ifstream::in};
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

    if (!(ifs.is_open() && ifs.good() && fs::is_fifo(fifofile_))) {
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
