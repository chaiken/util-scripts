/*
 * The mechanism for invoking googletest unit tests is copied from Mike Long's
 * work at https://github.com/meekrosoft.
 */
#include <memory>

#include "gtest/gtest.h"

#define TESTING

#include "cpumask.c"

TEST(SimpleCpuMaskTest, ParseSingleCore) { ASSERT_EQ(3, parse_core("3,")); }

TEST(SimpleCpuMaskTest, BadCore) {
  EXPECT_EXIT(parse_core(","), testing::ExitedWithCode(1),
              "Illegal core value");
  EXPECT_EXIT(parse_core("-1,"), testing::ExitedWithCode(1),
              "Illegal core value");
  EXPECT_EXIT(parse_core("128,"), testing::ExitedWithCode(1),
              "Illegal core value");
}

TEST(SimpleCpuMaskTest, ParseRange) {
  /*
    clang-format off
    Causes ASAN to SEGV:
  [ RUN      ] SimpleCpuMaskTest.ParseRange
AddressSanitizer:DEADLYSIGNAL
=================================================================
==773215==ERROR: AddressSanitizer: SEGV on unknown address 0x55b280c6a430 (pc 0x7f869582d357 bp 0x000000000002 sp 0x7fffb251bfc0 T0)
==773215==The signal is caused by a WRITE memory access.
    #0 0x7f869582d357 in bool __sanitizer::atomic_compare_exchange_strong<__sanitizer::atomic_uint8_t>(__sanitizer::atomic_uint8_t volatile*, __sanitizer::atomic_uint8_t::Type*, __sanitizer::atomic_uint8_t::Type, __sanitizer::memory_order) ../../../../src/libsanitizer/sanitizer_common/sanitizer_atomic_clang.h:80
    #1 0x7f869582d357 in __asan::Allocator::AtomicallySetQuarantineFlagIfAllocated(__asan::AsanChunk*, void*, __sanitizer::BufferedStackTrace*) ../../../../src/libsanitizer/asan/asan_allocator.cpp:620
    #2 0x7f869582d357 in __asan::Allocator::Deallocate(void*, unsigned long, unsigned long, __sanitizer::BufferedStackTrace*, __asan::AllocType) ../../../../src/libsanitizer/asan/asan_allocator.cpp:696
    #3 0x7f86958ba39d in operator delete(void*, unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:164
    #4 0x55b280c19616 in std::_Sp_counted_ptr<char const*, (__gnu_cxx::_Lock_policy)2>::_M_dispose() /usr/include/c++/12/bits/shared_ptr_base.h:428
    #5 0x55b280c09bff in std::_Sp_counted_base<(__gnu_cxx::_Lock_policy)2>::_M_release() /usr/include/c++/12/bits/shared_ptr_base.h:346
    #6 0x55b280c0cc7b in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::~__shared_count() /usr/include/c++/12/bits/shared_ptr_base.h:1071
    #7 0x55b280c0c595 in std::__shared_ptr<char const, (__gnu_cxx::_Lock_policy)2>::~__shared_ptr() /usr/include/c++/12/bits/shared_ptr_base.h:1524
    #8 0x55b280c0c603 in std::shared_ptr<char const>::~shared_ptr() /usr/include/c++/12/bits/shared_ptr.h:175
    #9 0x55b280c08847 in SimpleCpuMaskTest_ParseRange_Test::TestBody() /home/alison/gitsrc/util-scripts/cpumask_testsuite.cc:27

   std::shared_ptr<const char> str1("0-1");
   EXPECT_EQ(3, parse_range(str1.get(), 1U));
   clang-format on
  */
  // works with single-digit delimiters
  EXPECT_EQ(3, parse_range("0-1,", 1U));
  // works with multi-digit delimiters
  EXPECT_EQ((1 << 10) + (1 << 11), parse_range("10-11,", 2U));
  // Empty range.
  EXPECT_EQ(0, parse_range("1-0,", 1U));
  // Single-core equivalent.
  EXPECT_EQ(2, parse_range("1-1,", 1U));
}

TEST(SimpleCpuMaskTest, BadRanges) {
  EXPECT_EXIT(parse_range("0123,", 0U), testing::ExitedWithCode(1),
              "Illegal range endpoints");
  EXPECT_EXIT(parse_range("0123,", 150U), testing::ExitedWithCode(1),
              "Malformed core range");
  EXPECT_EXIT(parse_range("---,", 0U), testing::ExitedWithCode(1),
              "Illegal range endpoints");
  EXPECT_EXIT(parse_range("---,", 1U), testing::ExitedWithCode(1),
              "Illegal range endpoints");
  EXPECT_EXIT(parse_range("01-,", 1U), testing::ExitedWithCode(1),
              "Illegal range endpoints");
  EXPECT_EXIT(parse_range("01-,", 2U), testing::ExitedWithCode(1),
              "Illegal range endpoints");
  EXPECT_EXIT(parse_range("127-128,", 3U), testing::ExitedWithCode(1),
              "Illegal core value");
}

TEST(SimpleCpuMaskTest, CalcMask) {
  // Single core.
  EXPECT_EQ(7, calc_mask("0-2,"));
  // Cores and ranges.
  EXPECT_EQ(3 + (1 << 10) + (1 << 11), calc_mask("0,1,10-11,"));
  EXPECT_EQ(3 + (1 << 10) + (1 << 11), calc_mask("0,10-11,1,"));
  EXPECT_EQ(3 + (1 << 10) + (1 << 11), calc_mask("10-11,1,0,"));
  // Ranges plus single cores.
  EXPECT_EQ(3 + (1 << 10), calc_mask(std::string("0-1,10,").c_str()));
  EXPECT_EQ(3 + (1 << 10), calc_mask(std::string("10,0-1,").c_str()));
  // Examples from "man taskset".
  EXPECT_EQ(1, calc_mask(std::string("0,").c_str()));
  EXPECT_EQ(3, calc_mask(std::string("0-1,").c_str()));
  EXPECT_EQ(0xFFFFFFFF, calc_mask(std::string("0-31,").c_str()));
  EXPECT_EQ(0x32, calc_mask(std::string("1,4,5,").c_str()));
}
