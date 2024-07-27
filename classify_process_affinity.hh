#ifndef CLASSIFY_PROCESS_AFFINITY_H
#define CLASSIFY_PROCESS_AFFINITY_H

#include <filesystem>
#include <optional>
#include <set>
#include <stdint.h>
#include <string>

namespace process_affinity {

constexpr uint32_t PF_NO_SETAFFINITY = 0x04000000;
// Differs from the upstream patch because flags processing begins after the
// thread name.
constexpr uint32_t PF_NO_SET_AFFINITY_POSITION = 8U;

struct tid_data {
  tid_data(pid_t tidval, bool tset, std::string tname)
      : tid(tidval), is_settable(tset), thread_name(tname) {}
  pid_t tid = 0U;
  bool is_settable = true;
  std::string thread_name{};
};

// The thread with the lexigraphically lower thread name is ordered first.
// That choice means that duplicate thread names, all with same pinnability, are
// only listed once.
bool tid_data_compare(const struct tid_data &a, const struct tid_data &b);

// Return the contents of the stat file at the given path.
// Paths in /proc may suddenly disappear when the underlying thread terminates.
std::optional<std::string>
read_thread_stat(const std::string &stat_file_pathname);

// Given the contents of the flags field of a procfs stat file, indicate whether
// its affinity can be set.
std::optional<bool> thread_affinity_is_settable(const std::string &stat_str);

// Populate a caller-provided set with data about affinity-settability of
// threads in the indicated directory, by default /proc. The path is for unit
// tests.   Note that the comparator function is provided as a function pointer,
// thus decltype() *.  See
//  https://stackoverflow.com/questions/2620862/using-custom-stdset-comparator
void read_thread_data(
    std::set<struct tid_data, decltype(tid_data_compare) *> &tid_set,
    const std::string &procfs_top = "/proc/", bool verbose = false);

} // namespace process_affinity

#endif
