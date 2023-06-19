#include "endian_lib.h"

#include <cassert>

using namespace endian;

int main() {
  unsigned container = htole64(1);
  assert(true == is_little_endian(container, std::byte{1}));
  const unsigned container_be = htobe64(container);
  assert(false == is_little_endian(container_be, std::byte{1}));
  assert(true == is_big_endian<INTSIZE>(container_be));

  assert(true == is_little_endian2(container, 1));
  assert(false == is_little_endian2(container_be, 1));
  exit(EXIT_SUCCESS);
}
