#include "classify_process_affinity.h"

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
}

TEST(SimpleClassificationTest, ReadProcfs) {
  // The set ctor must take the comparator as a parameter.
  //  https://stackoverflow.com/questions/2620862/using-custom-stdset-comparator
  std::set<struct tid_data, decltype(tid_data_compare) *> tset{
      tid_data_compare};
  tset.emplace(1422, false, "foo");
  read_thread_data(tset, "procfs");
  ASSERT_EQ(2U, tset.size());
  EXPECT_EQ(tset.begin(), tset.find(tid_data(14, false, "ksoftirqd/0")));
  EXPECT_EQ(false, tset.begin()->is_settable);
  EXPECT_EQ("ksoftirqd/0", tset.begin()->thread_name);
  EXPECT_EQ(++tset.begin(), tset.find(tid_data(1422, false, "foo")));
  EXPECT_EQ(false, (++tset.begin())->is_settable);
  EXPECT_EQ("foo", (++tset.begin())->thread_name);
  // Fails because only the set key is compared.
  // EXPECT_EQ(tset.end(), tset.find(tid_data(1422, true, "unattended-upgr")));
  EXPECT_EQ(tset.find(tid_data(1422, true, "unattended-upgr")),
            tset.find(tid_data(1422, false, "foo")));
  EXPECT_EQ(tset.end(), tset.find(tid_data(0, false, "ksoftirqd/0")));
  EXPECT_EQ(tset.end(), tset.find(tid_data(1, false, "ksoftirqd/0")));
}

} // namespace local_testing
} // namespace process_affinity
