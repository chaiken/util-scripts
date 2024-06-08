#include "timerlat_pipe_load.h"

#include <cstdint>
#include <fcntl.h>
#include <sched.h>
#include <sys/stat.h>

#include "gtest/gtest.h"

using namespace std;

namespace timerlat_load {
namespace local_testing {

constexpr uint64_t ats = 391875376454005;
constexpr uint64_t bts = 391875376458684;

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

  // Create a file for rtla-tickler to read.
  const fs::path tmpdir{fs::path{ft.fifopath()}.parent_path()};
  ASSERT_TRUE(fs::is_directory(tmpdir));
  std::fstream tlfs(tmpdir.string() + "/rtlafile");
  std::string trash{"111111111"};
  tlfs.write(trash.c_str(), trash.length());
  tlfs.close();
  // Re-open as ifstream.
  std::ifstream tlfs0(tmpdir.string() + "/rtlafile");

  ft.calculate_roundtrip_delays(tlfs0);
  tlfs0.close();
}

} // namespace local_testing
} // namespace timerlat_load
