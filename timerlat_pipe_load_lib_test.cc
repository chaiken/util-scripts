#include "timerlat_pipe_load.h"

#include <fcntl.h>
#include <sched.h>
#include <sys/stat.h>

#include "gtest/gtest.h"

using namespace std;

namespace timerlat_load {
namespace local_testing {

TEST(TimerlatLoadTest, GetNs) {
  std::optional<uint64_t> ts0 = get_ns();
  ASSERT_TRUE(ts0.has_value());
  std::optional<uint64_t> ts1 = get_ns();
  ASSERT_TRUE(ts1.has_value());
  EXPECT_GT(ts1.value(), ts0.value());
}

TEST(TimerLatLoadTest, SpawnResponder) {
  const std::string fifopath = create_fifo_path();
  ASSERT_FALSE(fifopath.empty());
  ASSERT_EQ(0, mkfifoat(-1 /*NOT USED*/, fifopath.c_str(), 0777));

  struct stat statbuf {};
  ASSERT_EQ(0, stat(fifopath.c_str(), &statbuf));
  ASSERT_NE(0UL, statbuf.st_ino);

  const pid_t child_fd = spawn_responding_thread(fifopath);
  EXPECT_GT(child_fd, 0);
  int read_fd = open(fifopath.c_str(), O_RDONLY + O_NONBLOCK);
  ASSERT_GT(read_fd, 0);

  usleep(10 * 1000);

  char pipe_buffer[PIPE_BUF_SIZE + 1U];
  ssize_t bytes_read = read(read_fd, pipe_buffer, PIPE_BUF_SIZE);
  EXPECT_GT(bytes_read, 0);
}

} // namespace local_testing
} // namespace timerlat_load
