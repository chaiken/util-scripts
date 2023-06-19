#include <endian.h>

#include <cstdbool>
#include <cstddef>
#include <cstdlib>
#include <iostream>

namespace endian {

constexpr size_t INTSIZE = sizeof(int);

// Return true iff the first byte in the representation of val matches
// first_byte. Since std::byte is under the hood unsigned char, first_byte must
// be positive.
bool is_little_endian(const unsigned val, const std::byte first_byte);

// Return true iff the first byte in the representation of val matches
// first_byte. Since char is signed, first_byte can be negative.
bool is_little_endian2(const int val, const char first_byte);
bool is_little_endian3(const int val, const char first_byte);

// Return true iff val is big-endian.
template <std::size_t HOST_INTSIZE> bool is_big_endian(const unsigned val) {
  if constexpr (HOST_INTSIZE == 2U)
    return (htobe16(val) == val);
  if constexpr (HOST_INTSIZE == 4U)
    return (htobe32(val) == val);
  if constexpr (HOST_INTSIZE == 8U)
    return (htobe64(val) == val);
  std::cerr << "Unknown architecture" << std::endl;
  return false;
}

} // namespace endian
