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
  FifoTimer ft;
  EXPECT_NE(-1, ft.read_fd());

  struct stat statbuf {};
  ASSERT_EQ(0, stat(ft.fifopath().c_str(), &statbuf));
  ASSERT_NE(0UL, statbuf.st_ino);

  int read_fd = open(ft.fifopath().c_str(), O_RDONLY + O_NONBLOCK);
  ASSERT_GT(read_fd, 0);

  usleep(10 * 1000);

  char pipe_buffer[PIPE_BUF_SIZE + 1U];
  ssize_t bytes_read = read(read_fd, pipe_buffer, PIPE_BUF_SIZE);
  std::cout << bytes_read << " bytes read from pipe: " << pipe_buffer
            << std::endl;
  EXPECT_GT(bytes_read, 0);
  ft.stop();
}

} // namespace local_testing
} // namespace timerlat_load
