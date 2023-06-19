#include "endian.h"

#include <cstring>

namespace endian {

using std::byte;

bool is_little_endian(const unsigned val, const byte first_byte) {
  byte bytes[INTSIZE];
  memcpy(&bytes, &val, INTSIZE);
  return (first_byte == bytes[0]);
}

bool is_little_endian2(const int val, const char first_byte) {
  char bytes[INTSIZE];
  memcpy(&bytes, &val, INTSIZE);
  return (first_byte == bytes[0]);
}

bool is_little_endian3(const int val, const char first_byte) {
  // A stack allocation unlike std::aligned_alloc.
  std::aligned_storage_t<INTSIZE, alignof(unsigned)> byte_store{};
  // Because of the "placement new", bytes on the stack.
  char *bytes = new (&byte_store) char;
  // A simple assignment works because bytes is properly aligned and sized.
  *bytes = val;
  return (first_byte == bytes[0]);
}

} // namespace endian
