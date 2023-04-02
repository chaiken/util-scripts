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

bool is_little_endian2(int val, const char byteval) {
  std::aligned_storage_t<INTSIZE, alignof(char)> byte_store;
  char *bytes = new (&byte_store) char;
  *bytes = val;
  return (byteval == bytes[0]);
}

int main() {
  int container = 1;
  assert(true == is_little_endian(container, 1));
  int container_be = htobe32(container);
  assert(false == is_little_endian(container_be, 1));

  assert(true == is_little_endian2(container, 1));
  assert(false == is_little_endian2(container_be, 1));
  exit(EXIT_SUCCESS);
}
