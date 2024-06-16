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

void wait_one_ms() { std::this_thread::sleep_for(1ms); }

void FifoTimer::responding_fn() {
  int write_fd = openat(-1 /*NOT USED*/, fifofile_.c_str(), O_WRONLY);
  if (-1 == write_fd) {
    std::cerr << "Unable to open FIFO " << fifofile_
              << " for writing: " << strerror(errno) << std::endl;
    std::cerr.flush();
    return;
  }
  size_t ctr = 0;
  std::string timestr{};
  while (ctr++ < LIMIT) {
    time_point<steady_clock> tp = steady_clock::now();
    duration<uint64_t, std::nano> now_ref = tp.time_since_epoch();
    // Accessing a std::string after it is cleared is UB.   What good then is
    // clear()? timestr.clear();
    timestr.assign(std::to_string(now_ref.count()));
    if (!timestr.length()) {
      std::cerr << "timestring is empty!" << std::endl;
      continue;
    }
    ssize_t bytes_written = write(write_fd, timestr.c_str(), timestr.length());
    if ((-1 == bytes_written) &&
        ((EAGAIN != errno) || (EWOULDBLOCK != errno))) {
      std::cerr << "Unable to write pipe: " << strerror(errno) << std::endl;
      break;
    }
    if (0 == bytes_written) {
      std::cerr << "Wrote 0 bytes from : " << timestr << std::endl;
    } else {
      if (!(ctr % 10)) {
        std::cout << bytes_written << " written." << std::endl;
      }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  timestr.assign(std::string{STOP_WORD});
  write(write_fd, timestr.c_str(), timestr.length());
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
  size_t ctr = 0U;
  while (!ifs.eof()) {
    // Tickle the timerlat file descriptor.
    tlfs.read(&trash[0], 1);

    if (!(ifs.is_open() && ifs.good() && fs::is_fifo(fifofile_))) {
      std::cerr << "Pipe is closed." << std::endl;
      return;
    }
    errno = 0;
    ifs.read(pipe_buffer, PIPE_BUF_SIZE - 1U);
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
    if (PIPE_BUF_SIZE - 1U != bytes_read) {
      std::cerr << "Short read of " << bytes_read << " bytes." << std::endl;
      continue;
    }
    time_point<steady_clock> then;
    memcpy(&then, pipe_buffer, PIPE_BUF_SIZE - 1U);

    time_point<steady_clock> tp = steady_clock::now();
    // The following without "auto:  does not coimpile:
    //    duration<microseconds> delay = duration_cast<microseconds>(tp - then);
    auto delay = duration_cast<microseconds>(tp - then);
    if (delay > TEN_MICROS) {
      std::cout << std::to_string(delay.count()) << +" ms" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    if (!(ctr++ % 10)) {
      std::cout << bytes_read << " bytes read." << std::endl;
    }
  }
}

} // namespace timerlat_load
