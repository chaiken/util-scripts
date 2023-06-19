#include "endian.h"

#include "gtest/gtest.h"

namespace endian {
namespace local_testing {

// From endian.h
constexpr bool IS_LE(__BYTE_ORDER == __LITTLE_ENDIAN);

TEST(LETest, SingleByte) {
  EXPECT_EQ(IS_LE, is_little_endian(255, std::byte{255}));
  EXPECT_EQ(IS_LE, is_little_endian2(255, 255));
  EXPECT_EQ(IS_LE, is_little_endian3(255, 255));
  EXPECT_NE(IS_LE, is_big_endian<INTSIZE>(255));
}

TEST(LETest, Multibyte) {
  EXPECT_EQ(IS_LE, is_little_endian2(1025, 1));
  EXPECT_EQ(IS_LE, is_little_endian3(4097, 1));
  if constexpr (INTSIZE == 2U) {
    EXPECT_NE(IS_LE, htobe16(4095) == 4095);
  }
  if constexpr (INTSIZE == 4U) {
    EXPECT_NE(IS_LE, htobe32(4095) == 4095);
  }
  if constexpr (INTSIZE == 8U) {
    EXPECT_NE(IS_LE, htobe64(4095) == 4095);
  }
}

TEST(LETest, Negative) {
  EXPECT_EQ(IS_LE, is_little_endian2(-1, -1));
  EXPECT_NE(IS_LE, is_little_endian2(-1, 1));

  EXPECT_EQ(IS_LE, is_little_endian3(-1, -1));
  EXPECT_NE(IS_LE, is_little_endian3(-1, 1));
}

} // namespace local_testing
} // namespace endian
