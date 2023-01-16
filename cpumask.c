/*
 *
 * Given a list and/or range of CPU cores as input, print out the CPU mask.
 * Systems with more than 64 cores will need to choose a different method of
 * calculation since the bit shift will overflow.
 * Alison Chaiken (alison@she-devel.com)
 * GPLv2 or greater.
 *
 */
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
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
  fprintf(stderr, "$ cpumask 1,4,5,\n");
  fprintf(stderr, "0x32\n");
  fprintf(stderr, "$ cpumask 1,4-5,\n");
  fprintf(stderr, "0x32\n");
  fprintf(stderr, "Optionally specify a stride with a colon: ");
  fprintf(stderr, "$ cpumask 0-10:3,\n");
  fprintf(
      stderr,
      "The range specification must come at the end of the input string.\n");
  fprintf(stderr, "Only a single range is supported.\n");
  exit(EXIT_SUCCESS);
}

bool chars_are_numeric(const char *to_test) {
  // Empty string is not numeric.
  if (!strlen(to_test)) {
    return false;
  }
  char *copy = strdup(to_test);
  char *save = copy;
  if (!copy) {
    perror(strerror(ENOMEM));
    exit(EXIT_FAILURE);
  }
  while (*copy) {
    if (0 == isdigit(*copy)) {
      free(save);
      return false;
    }
    // Ranges can be multidigit.
    copy++;
  }
  free(save);
  return true;
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

/* Returns the requested stride when passed the input string and the position of
 * the first colon. */
uint64_t get_stride(const char *core_names, size_t colon_pos) {
  const size_t stride_len = (strlen(core_names) - colon_pos) - 1U;
  if (0U == stride_len) {
    fprintf(stderr, "Empty stride, defaulting to single-core increment.\n");
    return 1U;
  }
  char *stride_string = strndup(core_names + colon_pos + 1U, stride_len);
  if (!stride_string) {
    perror(strerror(ENOMEM));
    return 1U;
  }
  if (!chars_are_numeric(stride_string)) {
    fprintf(stderr, "Invalid stride %s, defaulting to single-core increment.\n",
            stride_string);
    free(stride_string);
    return 1U;
  }
  uint64_t result = strtoul(stride_string, NULL, 10);
  free(stride_string);
  return result;
}

/* Return the mask corresponding to a core range indicated by a dash-separated
 * pair of numbers in increasing order. */
uint64_t parse_range(const char *core_names, const size_t dash_offset) {
  uint64_t increment = 0U;

  if (dash_offset > strlen(core_names)) {
    fprintf(stderr, "Malformed core range: %s\n", core_names);
    exit(EXIT_FAILURE);
  }

  char *range_start = strndup(core_names, dash_offset);
  char *range_end = strndup(core_names + dash_offset + 1U,
                            (strlen(core_names) - strlen(range_start)) - 1U);
  if (!range_start || !range_end) {
    fprintf(stderr, "Out of memory.\n");
    exit(EXIT_FAILURE);
  }
  if (!strlen(range_start) || !strlen(range_end) || !isdigit(*range_start) ||
      !isdigit(*range_end)) {
    fprintf(stderr, "Illegal range endpoints %s and %s.\n", range_start,
            range_end);
    exit(EXIT_FAILURE);
  }

  const char *stride_start;
  uint64_t stride = 1U;
  if (NULL != (stride_start = index(core_names, ':'))) {
    const size_t colon_pos = stride_start - core_names;
    if (range_end <= stride_start) {
      fprintf(stderr,
              "Stride must come at the end of a range specification: %s\n",
              core_names);
      exit(EXIT_FAILURE);
    }
    stride = get_stride(core_names, colon_pos);
  }
  uint64_t start = parse_core(range_start);
  const uint64_t end = parse_core(range_end);
  while (start <= end) {
    increment += (1ULL << start);
    start += stride;
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
  // Check the bytes ahead of the first instance of ','.
  do {
    char *dash_pos;
    if ((dash_pos = index(core_name, '-')) != NULL) {
      mask += parse_range(core_name, dash_pos - core_name);
    } else {
      uint64_t core = parse_core(core_name);
      mask += (1ULL << core);
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
