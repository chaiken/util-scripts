#include "classify_process_affinity.hh"

#include "gtest/gtest.h"

namespace process_affinity {
namespace local_testing {

TEST(SimpleClassificationTest, ReadThreadStat) {
  std::optional<std::string> tstat = read_thread_stat("procfs/14/stat");
  ASSERT_TRUE(tstat.has_value());
  EXPECT_EQ("14 (ksoftirqd/0) S 2 0 0 0 -1 69238848 0 0 0 0 0 8499 0 0 20 0 1 "
            "0 21 0 0 18446744073709551615 0 0 0 0 0 0 0 2147483647 0 1 0 0 17 "
            "0 0 0 0 0 0 0 0 0 0 0 0 0 0\n",
            tstat.value());
}

TEST(SimpleClassificationTest, BadFileRead) {
  std::optional<std::string> tstat = read_thread_stat("/etc/shadow");
  ASSERT_FALSE(tstat.has_value());
  tstat = read_thread_stat("procfs/11111111111111111111/stat");
  ASSERT_FALSE(tstat.has_value());
}

TEST(SimpleClassificationTest, ThreadAffinityIsSettable) {
  // unattended upgrades is an ordinary userspace process.
  std::string flagstr = read_thread_stat("procfs/1422/stat").value();
  // Googletest does treat 1 == true.
  EXPECT_EQ(1, thread_affinity_is_settable(flagstr));
  // kworker/0 is a kernel per-CPU thread whose affinity is not settable.
  flagstr = read_thread_stat("procfs/14/stat").value();
  // Googletest does not consider 0 == false.
  EXPECT_EQ(0, thread_affinity_is_settable(flagstr));
  // Thread name contains spaces.
  flagstr = read_thread_stat("procfs/140857/stat").value();
  EXPECT_EQ(1, thread_affinity_is_settable(flagstr));
}

TEST(SimpleClassificationTest, ReadProcfs) {
  // The set ctor must take the comparator as a parameter; otherwise insert()
  // and emplace() will trigger a SEGV.
  //  https://stackoverflow.com/questions/2620862/using-custom-stdset-comparator
  std::set<struct tid_data, decltype(tid_data_compare) *> tset{
      tid_data_compare};
  tset.emplace(21, false, "kworker");
  read_thread_data(tset, "procfs", true);

  std::set<struct tid_data, decltype(tid_data_compare) *> expected(
      tid_data_compare);
  ASSERT_TRUE(expected.emplace(10851, 1, "(sd-pam)").second);
  ASSERT_TRUE(expected.emplace(140857, 1, "Isolated Web Co").second);
  ASSERT_TRUE(expected.emplace(14, 0, "ksoftirqd/0").second);
  ASSERT_TRUE(expected.emplace(21, 0, "kworker").second);
  ASSERT_TRUE(expected.emplace(1422, 1, "unattended-upgr").second);

  ASSERT_EQ(expected.size(), tset.size());
  auto it1 = expected.cbegin();
  auto it2 = tset.cbegin();
  for (; it1 != expected.cend() && it2 != tset.cend(); it1++, it2++) {
    EXPECT_EQ(it1->thread_name, it2->thread_name);
  }
  // Fails because only the set key is compared.
  // EXPECT_EQ(tset.end(), tset.find(tid_data(21, false, "unattended-upgr")));
  EXPECT_EQ(tset.find(tid_data(1422, true, "unattended-upgr")),
            tset.find(tid_data(21, false, "unattended-upgr")));
  EXPECT_EQ(tset.end(), tset.find(tid_data(14, false, "systemd")));
}

} // namespace local_testing
} // namespace process_affinity
