#include "timerlat_load.hh"

#include <fcntl.h>
#include <sched.h>
#include <sys/stat.h>

#include "gtest/gtest.h"

using namespace std;

namespace timerlat_load {
namespace local_testing {

constexpr char TESTFILE0[] = "/etc/hosts";
constexpr char TESTFILE1[] = "/dev/null";

// Test which runs only with root UID.
struct TimerlatLoadCoresTest : public testing::TestWithParam<int> {
  TimerlatLoadCoresTest() {
    CPU_ZERO(&cpu_set_in);
    CPU_ZERO(&cpu_set_out);
    testpid = getpid();
  }

  int cpu = -1;
  pid_t testpid = -1;
  cpu_set_t cpu_set_in;
  cpu_set_t cpu_set_out;
};

TEST_P(TimerlatLoadCoresTest, SetAffinity) {
  if (!geteuid()) {
    cpu = GetParam();
    CPU_SET(cpu, &cpu_set_in);
    ASSERT_EQ(0, set_affinity(testpid, cpu));
    ASSERT_EQ(0, sched_getaffinity(testpid, sizeof(cpu_set_t), &cpu_set_out));

    //   CPU_EQUAL()  and  CPU_EQUAL_S()  return nonzero if the two CPU sets are
    //   equal; otherwise they return 0.
    EXPECT_NE(0, CPU_EQUAL(&cpu_set_in, &cpu_set_out));
    EXPECT_EQ(1, CPU_COUNT(&cpu_set_out));
    // CPU_ISSET() and CPU_ISSET_S() return nonzero if cpu is in  set;
    // otherwise, it returns 0.
    EXPECT_NE(0, CPU_ISSET(cpu, &cpu_set_out));
    EXPECT_EQ(0, errno);
  }
}
INSTANTIATE_TEST_SUITE_P(AllCores, TimerlatLoadCoresTest,
                         testing::Values(0, 1, 2, 3, 4, 5, 6, 7));

// Without the new name, INSTANTIATE_TEST_SUITE below will rerun the test above
// with the bad values.
struct TimerlatLoadBadCoresTest : public TimerlatLoadCoresTest {};

TEST_P(TimerlatLoadBadCoresTest, SetAffinityError) {
  if (!geteuid()) {
    cpu = GetParam();
    CPU_SET(cpu, &cpu_set_in);
    ASSERT_EQ(0, errno);

    ASSERT_EQ(EINVAL, set_affinity(testpid, cpu));
  }
  // Needed to prevent pollution of the next test.
  errno = 0;
}
INSTANTIATE_TEST_SUITE_P(NoSuchCores, TimerlatLoadBadCoresTest,
                         testing::Values(-1, 110));

// Test which runs only with root UID.
struct TimerlatLoadPriosTest : public testing::TestWithParam<int> {
  TimerlatLoadPriosTest() { testpid = getpid(); }

  int priority = 0;
  pid_t testpid = -1;
  struct sched_param param_in;
  struct sched_param param_out;
};

TEST_P(TimerlatLoadPriosTest, SetPrio) {
  if (!geteuid()) {
    param_in.sched_priority = GetParam();
    ASSERT_EQ(0, set_prio(testpid, param_in.sched_priority));
    EXPECT_EQ(SCHED_FIFO, sched_getscheduler(testpid));
    ASSERT_EQ(0, sched_getparam(testpid, &param_out));
    EXPECT_EQ(param_in.sched_priority, param_out.sched_priority);
  }
}
INSTANTIATE_TEST_SUITE_P(AllCores, TimerlatLoadPriosTest,
                         testing::Values(1, 2, 3));

struct TimerlatLoadBadPriosTest : public TimerlatLoadPriosTest {};

TEST_P(TimerlatLoadBadPriosTest, SetPrioError) {
  if (!geteuid()) {
    param_in.sched_priority = GetParam();
    ASSERT_EQ(EINVAL, set_prio(testpid, param_in.sched_priority));
  }
}
INSTANTIATE_TEST_SUITE_P(BadPrios, TimerlatLoadBadPriosTest,
                         testing::Values(-120, 500));

// Test which runs with ordinary UID.
TEST(TimerlatLoadTest, ReadBuffs) {
  struct statx stats {};
  ASSERT_EQ(0,
            statx(0 /*dirfd */, TESTFILE0, 0 /* flags */, STATX_SIZE, &stats));
  std::ifstream tlfs0(TESTFILE0);
  std::ifstream devfs0(TESTFILE0);
  // stx_size for the read of devfs plus one for read of the timerlatfd.
  // /etc/hosts is not empty.
  EXPECT_EQ(static_cast<ssize_t>(stats.stx_size) + 1,
            read_buffs(tlfs0, devfs0));
  tlfs0.close();
  devfs0.close();

  std::ifstream tlfs1(TESTFILE1);
  std::ifstream devfs1(TESTFILE1);
  // /dev/null is still empty.
  EXPECT_EQ(0, read_buffs(tlfs1, devfs1));
  tlfs1.close();
  devfs1.close();
}

} // namespace local_testing
} // namespace timerlat_load
