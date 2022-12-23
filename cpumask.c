/*
 *
 * A trivial program that spits out cpumasks.
 * Alison Chaiken (alison@she-devel.com)
 * GPLv2 or greater.
 *
 */
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void usage(void) {
  fprintf(stderr, "Provide a comma-separated list of cores with optional "
                  "dashes\n");
  fprintf(stderr, "to indicate ranges. Core numbering starts at 0.  End the "
                  "input with a comma.\n");
  fprintf(stderr, "Cores in ranges must be in increasing order.\n");
}

/* Convert a string to a core number. */
uint32_t parse_core(const char *core_name) {
  errno = 0;
  uint32_t core = strtoul(core_name, NULL, 10);
  if (errno || (0U == strlen(core_name))) {
    fprintf(stderr, "Illegal core value %s\n", core_name);
    exit(EXIT_FAILURE);
  }
  return core;
}

/* Return the mask corresponding to a core range indicated by a dash-separated
 * pair of numbers in increasing order. */
uint64_t parse_range(char *core_name, char *dash_pos) {
  uint64_t increment = 0U;
  char *range_start = strndup(core_name, dash_pos - core_name);
  char *range_end =
      strndup(dash_pos + 1, strlen(core_name) - strlen(range_start));
  if (!range_start || !range_end) {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
  }
  uint32_t start = parse_core(range_start);
  const uint32_t end = parse_core(range_end);
  while (start <= end) {
    increment += pow(2, start);
    start++;
  }
  free(range_start);
  free(range_end);
  return increment;
}

/* Perform the calculation based on the full input string. */
uint64_t calc_mask(char *corelist) {
  uint64_t mask = 0U;
  /* Initialize strtok(). */
  char *core_name = strtok(corelist, ",");
  /* Guard against empty strings or ","  */
  if (!core_name || !strlen(core_name)) {
    return mask;
  }
  /* Perform the test only after processing the initial input. */
  do {
    char *dash_pos;
    if ((dash_pos = index(core_name, '-')) != NULL) {
      mask += parse_range(core_name, dash_pos);
    } else {
      uint32_t core = parse_core(core_name);
      mask += pow(2, core);
    }
  } while ((core_name = strtok(NULL, ",")) != NULL);
  return mask;
}

int main(int argc, char *argv[]) {
  if ((argc < 2) || (!index(argv[1], ','))) {
    usage();
    exit(EXIT_FAILURE);
  }

  printf("mask is %#lx\n", calc_mask(argv[1]));

  exit(EXIT_SUCCESS);
}
