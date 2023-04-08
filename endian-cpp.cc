#include <cassert>
#include <cstdbool>
#include <cstdlib>
#include <cstring>
#include <iostream>

constexpr size_t INTSIZE = sizeof(int);

bool is_little_endian(const int val, const char byteval) {
  char bytes[INTSIZE];
  std::memcpy(&bytes, &val, INTSIZE);
  return (byteval == bytes[0]);
}

bool is_little_endian2(const unsigned val, const char byteval) {
  // A stack allocation unlike std::aligned_alloc.
  std::aligned_storage_t<INTSIZE, alignof(unsigned)> byte_store{};
  // Because of the "placement new", bytes on the stack.
  char *bytes = new (&byte_store) char;
  // A simple assignment works because bytes is properly aligned and sized.
  *bytes = val;
  return (byteval == bytes[0]);
}

template <std::size_t HOST_INTSIZE> bool is_big_endian(const unsigned val) {
  switch (HOST_INTSIZE) {
  case 2U:
    return (htobe16(val) == val);
  case 4U:
    return (htobe32(val) == val);
  case 8U:
    return (htobe64(val) == val);
  default:
    std::cerr << "Unknown architecture";
    return false;
  }
}

int main() {
  unsigned container = htole64(1);
  assert(true == is_little_endian(container, 1));
  unsigned container_be = htobe64(container);
  assert(false == is_little_endian(container_be, 1));
  assert(true == is_big_endian<INTSIZE>(container_be));

  assert(true == is_little_endian2(container, 1));
  assert(false == is_little_endian2(container_be, 1));
  exit(EXIT_SUCCESS);
}
