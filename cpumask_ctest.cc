
#include "catch_amalgamated.hpp"

// #include <memory>
#include <string>

#define TESTING

#include "cpumask.c"

using Catch::Matchers::ContainsSubstring;

TEST_CASE("parse a single core correctly") {
  REQUIRE(parse_core(std::string("3,").c_str()) == 3U);
}

/*
  Causes the test itself to exit.
  Catch2 appears to have no equivalent to Googletest's EXPECT_EXIT().
TEST_CASE("bad core strings are rejected") {
  CHECK(1 == parse_core(std::string(",").c_str()));
}
*/

TEST_CASE("range parsing works") {
  // works with single-digit delimiters
  CHECK(3 == parse_range("0-1,", 1U));
  /*
    clang-format off
    As with the Googletest equivalent, triggers an ASAN SEGV.
   std::shared_ptr<const char> str1("0-1");
   CHECK(3 == parse_range(str1.get(), 1U));
   clang-format on
  */
  // works with multi-digit delimiters
  CHECK((1 << 10) + (1 << 11) == parse_range("10-11,", 2U));
  // Empty range.
  CHECK(0 == parse_range("1-0,", 1U));
  // Single-core equivalent.
  CHECK(2 == parse_range("1-1,", 1U));
}
