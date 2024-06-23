#include "timerlat_pipe_load.h"

#include <cstdint>
#include <fcntl.h>
#include <sched.h>
#include <sys/stat.h>

#include "gtest/gtest.h"

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace timerlat_load {
namespace local_testing {

TEST(TimelatPipeLoadTest, CreateFifoDir) {
  std::optional<fs::path> fdir_res = create_fifo_dir();
  ASSERT_TRUE(fdir_res.has_value());
  fs::path fdir = fdir_res.value();
  EXPECT_NE(std::string::npos, fdir.string().find("/tmp/fifodir"));
  EXPECT_TRUE(fs::exists(fdir));
  fs::remove_all(fdir);
}

TEST(TimerlatPipeLoadTest, CreateResponder) {
  FifoTimer ft;
  std::optional<fs::path> fdir_res = create_fifo_dir();
  const std::string fifopath = fdir_res.value().string() + "/myfifo";
  ft.set_fifofile(fifopath);
  EXPECT_NE(-1, mkfifoat(-1 /*NOT USED*/, ft.fifofile().c_str(), 0777));
  // The writer must be created before the ifstream, as otherwise open() will
  // block forever.
  EXPECT_TRUE(ft.create_responder());
  ft.ifs = std::ifstream{fifopath, std::ifstream::in};
  EXPECT_TRUE(ft.ifs.good());
}

TEST(TimerlatPipeLoadTest, Start) {
  FifoTimer ft;
  ASSERT_TRUE(ft.start());
  EXPECT_TRUE(ft.ifs.good());

  ASSERT_TRUE(fs::exists(ft.fifofile()));
  ASSERT_TRUE(fs::is_fifo(ft.fifofile()));

  ifstream rfs(ft.fifofile());
  ASSERT_TRUE(rfs.good());

  char pipe_buffer[PIPE_BUF_SIZE];
  ft.ifs.readsome(pipe_buffer, PIPE_BUF_SIZE);
  EXPECT_EQ(static_cast<size_t>(ft.ifs.gcount()), PIPE_BUF_SIZE);
}

TEST(TimerlatPipeLoadTest, CalculateDelay) {
  FifoTimer ft;
  ASSERT_TRUE(ft.start());

  // Apparently we cannot reuse the existing tmpdir.
  char path_template[] = "/tmp/fifodirXXXXXX";
  char *path = mkdtemp(path_template);
  ofstream tlfs(string(path) + "/rtlafile");
  string trash{"111111111"};
  tlfs.write(trash.c_str(), trash.length());
  const size_t pos = tlfs.tellp();
  ASSERT_EQ(trash.length(), pos);
  tlfs.close();
  // Re-open as ifstream.
  ifstream tlfs0(string(path) + "/rtlafile");

  ft.calculate_roundtrip_delays(tlfs0);
  tlfs0.close();
  fs::remove_all(path);
}

} // namespace local_testing
} // namespace timerlat_load
