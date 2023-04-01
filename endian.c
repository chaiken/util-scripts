/* Inspired by Robert Love's _Linux Kernel Development_, Third Edition, p. 390.
 */
#include <assert.h>
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

union multirep {
  int ival;
  char bval[4];
};

bool is_little_endian(const int x) {
  if (*(char *)&x == 1) {
    printf("Little-endian, low-order byte containing 1 is at %p and high-order "
           "bytes are at %p.\n",
           &x, (&x + sizeof(x / 2)));
    return true;
  }
  printf("Big-endian, high-order byte containing 1 is at %p and low-order "
         "bytes are at %p.\n",
         &x, (&x - sizeof(x / 2)));
  return false;
}

bool is_little_endian2(const int x) {
  // Find last bit.
  char *be_end = (char *)&x + (sizeof(x) - sizeof(char));
  char be = *(be_end);
  if (1 == be) {
    return false;
  }
  return true;
}

bool is_big_endian(const union multirep x, const int byteval) {
  return (byteval == x.bval[3]);
}

int main() {
  const int le_x = 1;
  const int be_x = htobe32(le_x);
  union multirep reps;

  assert(true == is_little_endian(le_x));
  assert(true == is_little_endian2(le_x));

  assert(false == is_little_endian(be_x));
  assert(false == is_little_endian2(be_x));

  reps.ival = le_x;
  assert(false == is_big_endian(reps, 1));
  reps.ival = be_x;
  assert(true == is_big_endian(reps, 1));

  exit(0);
}
