#include "timerlat_pipe_load.hh"

#include <sched.h>
#include <signal.h>

#include <cstdint>
#include <exception>
#include <stdexcept>

#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace timerlat_load {
namespace local_testing {

TEST(TimerlatPipeLoadTest, CreateResponder) {
  FifoTimer ft;
  ASSERT_TRUE(fs::create_directory(ft.fifodir()));
  const std::string fifoname{ft.fifodir() + "/myfifo"};

  EXPECT_NE(-1, mkfifoat(-1 /*NOT USED*/, fifoname.c_str(), 0777));
  // The writer must be created before the ifstream, as otherwise open() will
  // block forever.
  std::function<void(const std::string &)> fn = responding_fn;
  EXPECT_TRUE(ft.create_responder(fn));
  ft.ifs = std::ifstream{fifoname, std::ifstream::in};
  EXPECT_TRUE(ft.ifs.good());
}

void do_nothing(const std::string &nothing) {
  size_t ignored = nothing.length();
  ignored++;
  // SIGCHLD is ignored. Substituting SIGPIPE causes the test to exit with error
  // 141.
  // https://stackoverflow.com/questions/18880606/socket-connection-getting-closed-abruptly-with-code-141
  // the shell adds 128 so you can distinguish between exit codes (usually low
  // numbers) and fatal signals (also low numbers). Otherwise death by SIGHUP
  // would look the same as exit(1).
  raise(SIGCHLD);
  return;
}

TEST(TimerlatPipeLoadTest, MinimalResponder) {
  FifoTimer ft;
  std::function<void(const std::string &)> fn = do_nothing;
  EXPECT_TRUE(ft.create_responder(fn));
}

void do_nothing_fail(const std::string &nothing) {
  size_t ignored = nothing.length();
  ignored++;
  raise(SIGPIPE);
  return;
}

TEST(TimerlatPipeLoadTest, BadMinimalResponder) {
  FifoTimer ft;

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGPIPE);
  EXPECT_EQ(0, pthread_sigmask(SIG_BLOCK, &set, nullptr));

  std::function<void(const std::string &)> fn = do_nothing_fail;
  EXPECT_TRUE(ft.create_responder(fn));

  // The following causes SIGABRT.  The catch of the exception doesn't work,
  // probably because googletest receives it rather than the test's own code.
  // std::function<void(const std::string &)> empty_fn;
  //  try {
  //    EXPECT_FALSE(ft.create_responder(empty_fn));
  //  } catch (std::bad_function_call &e) {
  //    std::cerr << "Oops, an exception." << std::endl;
  //  }
  std::function<void(const std::string &)> empty_fn;
  ::testing::internal::CaptureStderr();
  EXPECT_FALSE(ft.create_responder(empty_fn));
  const std::string output = ::testing::internal::GetCapturedStderr();
  EXPECT_THAT(output, ::testing::HasSubstr(
                          "Supplied thread function is not executable."));
}

TEST(TimerlatPipeLoadTest, Start) {
  FifoTimer ft;
  ASSERT_TRUE(ft.start());
  EXPECT_TRUE(ft.ifs.good());

  const std::string fifoname{ft.fifodir() + "/myfifo"};
  ASSERT_TRUE(fs::exists(fifoname));
  ASSERT_TRUE(fs::is_fifo(fifoname));

  ifstream rfs(fifoname);
  ASSERT_TRUE(rfs.good());

  char pipe_buffer[PIPE_BUF_SIZE];
  ft.ifs.readsome(pipe_buffer, PIPE_BUF_SIZE);
  EXPECT_EQ(static_cast<size_t>(ft.ifs.gcount()), PIPE_BUF_SIZE);
}

TEST(TimerlatPipeLoadTest, CalculateDelay) {
  FifoTimer ft;
  ASSERT_TRUE(ft.start());

  ofstream tlfs(ft.fifodir() + "/rtlafile");
  string trash{"111111111"};
  tlfs.write(trash.c_str(), trash.length());
  const size_t pos = tlfs.tellp();
  ASSERT_EQ(trash.length(), pos);
  tlfs.close();
  // Re-open as ifstream.
  ifstream tlfs0(ft.fifodir() + "/rtlafile");

  ft.calculate_roundtrip_delays(tlfs0);
  tlfs0.close();
}

} // namespace local_testing
} // namespace timerlat_load
