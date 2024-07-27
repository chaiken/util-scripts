/* Inspired by
https://lore.kernel.org/util-linux/20220805154820.407972387@osadl.org/T/#t

clang-format off
In kernel's include/linux/sched.h:
#define PF_NO_SETAFFINITY	0x04000000	Userland is not allowed to
meddle with cpus_mask

The value in /proc/<pid>/stat is the unsigned int flags field from struct
task_struct. Output in procfs occurs in fs/proc/array.c in do_task_stat():

0	seq_put_decimal_ull(m, "", pid_nr_ns(pid, ns));
1	seq_puts(m, " (");
1	proc_task_name(m, task, false);
1	seq_puts(m, ") ");
2	seq_putc(m, state);
3	seq_put_decimal_ll(m, " ", ppid);
4	seq_put_decimal_ll(m, " ", pgid);
5	seq_put_decimal_ll(m, " ", sid);  task_session_nr
6	seq_put_decimal_ll(m, " ", tty_nr);
7	seq_put_decimal_ll(m, " ", tty_pgrp);
8	seq_put_decimal_ull(m, " ", task->flags);


$ cat /proc/93/stat
93 (irq/27-aerdrv) S 2 0 0 0 -1 2129984 0 0 0 0 0 0 0 0 -51 0 1 0 88 0 0
18446744073709551615 0 0 0 0 0 0 0 2147483647 0 0 0 0 17 5 50 1 0 0 0 0 0 0 0 0
0 0 0 clang-format on
*/

#include "classify_process_affinity.hh"

#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace process_affinity {

namespace {

std::string extract_thread_name(const std::string &thread_stat) {
  size_t name_start = thread_stat.find('(') + 1U;
  size_t name_end = thread_stat.find(')') - 1U;
  // Some thread names end with ')', because they hate us.
  if ((')' == thread_stat.at(name_end + 1U)) &&
      (')' == thread_stat.at(name_end + 2U))) {
    name_end++;
  }
  return thread_stat.substr(name_start, (name_end - name_start) + 1U);
}

// Since the code already extracted the thread name, it's safe to assume that
// the stat string is well-formed.
std::optional<std::string>
extract_thread_flags(const std::string &thread_stat) {
  std::string affinity_str{};
  // The name string is enclosed in parentheses.  Go past the following space.
  size_t space_pos = thread_stat.find(")") + 1U;
  if (')' == thread_stat.at(space_pos)) {
    space_pos++;
  }
  size_t start_pos = space_pos + 1U;
  size_t field_num = 1U; // First space is already located.
  std::string remainder{};
  while (std::string::npos != space_pos) {
    // Found start of flags field?
    if (field_num == PF_NO_SET_AFFINITY_POSITION) {
      break;
    }
    remainder = thread_stat.substr(start_pos);
    space_pos = remainder.find(" ");
    start_pos += space_pos + 1U;
    field_num++;
  }
  // Find the space that terminates the flags string.
  space_pos = remainder.find(" ");
  affinity_str = remainder.substr(0U, space_pos);
  if (isdigit(*affinity_str.begin()) &&
      (field_num == PF_NO_SET_AFFINITY_POSITION)) {
    return affinity_str;
  }
  std::cerr << "Malformed affinity string " << remainder << std::endl;
  return std::nullopt;
}

} // namespace

bool tid_data_compare(const struct tid_data &a, const struct tid_data &b) {
  return a.thread_name < b.thread_name;
}

std::optional<std::string> read_thread_stat(const std::string &pathname) {
  std::ifstream statstream(pathname, std::ifstream::in);
  std::string statstring{};
  std::filebuf *contents = statstream.rdbuf();
  if (statstream.good()) {
    char ch = contents->sgetc();
    // EOF detection works with actual /proc files on x86_64 but not on ARM64.
    // Newline detection works on both.
    while ((ch != EOF) && (ch != '\n')) {
      statstring += ch;
      ch = contents->snextc();
    }
    statstring += '\n';
  } else {
    std::cerr << "Unable to read procfs file " + pathname << std::endl;
  }
  statstream.close();
  return statstring.empty() ? std::nullopt
                            : std::optional<std::string>(statstring);
}

std::optional<bool> thread_affinity_is_settable(const std::string &stat_str) {
  std::optional<std::string> flags_str = extract_thread_flags(stat_str);
  if (flags_str.has_value()) {
    uint64_t affinity = std::strtoul(flags_str.value().c_str(), NULL, 10);
    return (!(affinity & PF_NO_SETAFFINITY));
  }
  return std::nullopt;
}

void read_thread_data(
    std::set<struct tid_data, decltype(tid_data_compare) *> &tid_set,
    const std::string &procfs_top, bool verbose) {
  fs::path proc_path(procfs_top);
  for (const fs::directory_entry &entry : fs::directory_iterator(proc_path)) {
    std::string tid_string = entry.path().stem().string();
    // Entries in /proc whose filenames start with a digit are tids.
    if (!isdigit(*tid_string.begin())) {
      continue;
    }
    pid_t tid = std::strtoul(tid_string.c_str(), NULL, 10);
    if (0U == tid) {
      continue;
    }
    std::optional<std::string> stat_str =
        read_thread_stat(procfs_top + "/" + tid_string + "/stat");
    if (stat_str.has_value()) {
      std::string thread_name = extract_thread_name(stat_str.value());
      std::optional<bool> is_movable =
          thread_affinity_is_settable(stat_str.value());
      if (!is_movable.has_value()) {
        continue;
      }
      std::pair<std::set<struct tid_data>::iterator, bool> result =
          tid_set.emplace(tid, is_movable.value(), thread_name);
      if ((!result.second) && verbose) {
        std::cerr << "Thread " << thread_name << " already present in set."
                  << std::endl;
      }
    }
  }
}

} // namespace process_affinity
