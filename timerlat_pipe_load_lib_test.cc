#include "timerlat_pipe_load.h"

#include <cstdint>
#include <fcntl.h>
#include <sched.h>
#include <sys/stat.h>

#include "gtest/gtest.h"

using namespace std;

namespace timerlat_load {
namespace local_testing {

TEST(TimerlatPipeLoadTest, SpawnResponder) {
  FifoTimer ft;
  EXPECT_TRUE(ft.ifs.good());

  struct stat statbuf {};
  ASSERT_EQ(0, stat(ft.fifopath().c_str(), &statbuf));
  ASSERT_NE(0UL, statbuf.st_ino);
  const fs::path tmpfile{ft.fifopath()};
  ASSERT_TRUE(is_fifo(tmpfile));

  std::ifstream rfs(ft.fifopath());
  ASSERT_TRUE(rfs.good());

  wait_one_ms();

  char pipe_buffer[PIPE_BUF_SIZE + 1U];
  ft.ifs.readsome(pipe_buffer, PIPE_BUF_SIZE);
  std::cout << ft.ifs.gcount() << " bytes read from pipe: " << pipe_buffer
            << std::endl;
  EXPECT_GT(ft.ifs.gcount(), 0);
}

TEST(TimerlatPipeLoadTest, CalculateDelay) {
  FifoTimer ft;

  // Apparently we cannot reuse the existing tmpdir.
  char path_template[] = "/tmp/fifodirXXXXXX";
  char *path = mkdtemp(path_template);
  std::cout << path << std::endl;
  std::ofstream tlfs(std::string(path) + "/rtlafile");
  std::string trash{"111111111"};
  tlfs.write(trash.c_str(), trash.length());
  const size_t pos = tlfs.tellp();
  ASSERT_EQ(trash.length(), pos);
  tlfs.close();
  // Re-open as ifstream.
  std::ifstream tlfs0(std::string(path) + "/rtlafile");

  ft.calculate_roundtrip_delays(tlfs0);
  tlfs0.close();
  fs::remove_all(path);
}

} // namespace local_testing
} // namespace timerlat_load
