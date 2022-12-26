/*
 *
 * Given a list and/or range of CPU cores as input, print out the CPU mask.
 * Rely on the math library function pow() rather than the shift operator given
 * that current systems may have a huge number of cores.
 * Alison Chaiken (alison@she-devel.com)
 * GPLv2 or greater.
 *
 */
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void usage(void) {
  fprintf(stderr, "Provide a comma-separated list of cores with optional\n");
  fprintf(stderr,
          "dashes to indicate ranges. Core numbering starts at 0.  End\n");
  fprintf(stderr, "the input with a comma.\n");
  fprintf(stderr, "Example output:\n");
  fprintf(stderr, "$ cpumask 1,4,5\n");
  fprintf(stderr, "0x32\n");
  fprintf(stderr, "$ cpumask 1,4-5\n");
  fprintf(stderr, "0x32\n");
}

/* Convert a string to a core number. */
uint64_t parse_core(const char *core_name) {
  errno = 0;
  uint64_t core = strtoul(core_name, NULL, 10);
  if (errno || !isdigit(*core_name) || (core > 63U)) {
    fprintf(stderr, "Illegal core value %s.\n", core_name);
    if (core > 63U) {
      fprintf(stderr, "Core values larger than 63U overflow 64-bit masks.\n");
    }
    exit(EXIT_FAILURE);
  }
  return core;
}

/* Return the mask corresponding to a core range indicated by a dash-separated
 * pair of numbers in increasing order. */
uint64_t parse_range(const char *core_name, const size_t dash_offset) {
  uint64_t increment = 0U;
  char *range_start = strndup(core_name, dash_offset);
  char *range_end = strndup(core_name + dash_offset + 1U,
                            (strlen(core_name) - strlen(range_start)) - 1U);
  if (!range_start || !range_end) {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
  }
  if (!strlen(range_start) || !strlen(range_end)) {
    fprintf(stderr, "Illegal range endpoints %s and %s.\n", range_start,
            range_end);
    exit(EXIT_FAILURE);
  }
  uint64_t start = parse_core(range_start);
  const uint64_t end = parse_core(range_end);
  while (start <= end) {
    increment += pow(2, start);
    start++;
  }
  free(range_start);
  free(range_end);
  return increment;
}

/* Perform the calculation based on the full input string. */
uint64_t calc_mask(const char *corelist) {
  uint64_t mask = 0U;
  char *templist = strndup(corelist, strlen(corelist));
  if (!templist) {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
  }
  /* Initialize strtok(). */
  char *core_name = strtok(templist, ",");
  /* Guard against empty strings or ","  */
  if (!core_name || !strlen(core_name)) {
    fprintf(stderr, "Invalid core name %s\n", core_name);
    exit(EXIT_FAILURE);
  }
  do {
    char *dash_pos;
    if ((dash_pos = index(core_name, '-')) != NULL) {
      mask += parse_range(core_name, dash_pos - core_name);
    } else {
      uint64_t core = parse_core(core_name);
      mask += pow(2, core);
    }
    /* Perform the test only after processing the initial input. */
  } while ((core_name = strtok(NULL, ",")) != NULL);
  free(templist);
  return mask;
}

#ifndef TESTING
int main(int argc, char *argv[]) {
  if ((argc < 2) || (!index(argv[1], ','))) {
    usage();
    exit(EXIT_FAILURE);
  }

  printf("%#lx\n", calc_mask(argv[1]));

  exit(EXIT_SUCCESS);
}
#endif
