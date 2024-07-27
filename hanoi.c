/******************************************************************************
 *									      *
 *	File:     hanoi.c						      *
 *	Author:   Alison Chaiken <alison@bonnet.sonic.net>		      *
 *	Created:  Mon Jun 29 14:35:50 PDT 2015				      *
 *	Contents: The key insight needed to complete this exercise is to      *
 *       realize that two recursive calls are needed, as the solution of       *
 *       n disks requires the solution of (n-1) disks twice.  First (n-1)      *
 *       are moved to uncover the	nth, largest disk. The largest disk   *
 *       is moved, and then the (n-1) are moved again, on top of it.           *
 *                                                                             *
 *       Another key insight is that the number of disks to be moved in the    *
 *       next recursive action must be passed downward during the 'winding'    *
 *       phase.   There's nothing the state of the pegs that allows any        *
 *       algorithm to determine the direction that disks should move, thereby  *
 *       preventing oscillation.   The status information passed down as the   *
 *       number of disks to be moved eliminates any chance of oscillation      *
 *                                                                             *
 *       The real benefit of recursion is that the solution for n disks is the *
 *       independent of n as long as (n >= 3).  Notice that the number of      *
 *       moves needed to solve n disks is the number needed to solve (n-1)     *
 *       plus the number needed to solve (n-2) plus . . . which sums to the    *
 *       value (pow(2, n-1)).   The reason is obvious from the recursive       *
 *       nature of the problem.                                                *
 *									      *
 *	The 3 arrays are made global variables for ease in printing them.     *
 *	That means that the function is not re-entrant.  To restore           *
 *       threadsafe behavior, the operations in popstack() and movechar()      *
 *       would have to be protected by a lock.                                 *
 *									      *
 ******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL "ABCDE"
/*
  To initialize character arrays, include the terminating NULL
  explicitly.   This initialization looks very weird, but works fine,
  and allows the strcpy in main() to be omitted.
*/
char first[6] = "ABCDE\0", second[6] = "\0", third[6] = "\0";

static int ctr = 0;

char popstack(char **instr) {
  int len = strlen(*instr);
  char lastchar = *(*instr + (len - 1));
  *(*instr + (len - 1)) = '\0';
  return (lastchar);
}

void movechar(char **from, char **to) {
  char topchar = popstack(from);
  int len = strlen(*to);
  *(*to + len) = topchar;
  *(*to + len + 1) = '\0';
  return;
}

void printset() {

  int i;

  if ((ctr >= pow(2, strlen(INITIAL))) || (ctr < 0)) {
    printf("Oops, not terminating.\n");
    exit(EXIT_FAILURE);
  }

  printf("Iteration %d:\n", ctr);
  ctr++;
  for (i = (strlen(INITIAL) - 1); i >= 0; i--)
    printf("%c\t%c\t%c\n", first[i], second[i], third[i]);

  printf("@\t@\t@\n\n");

  return;
}

void restack(char origin[], char holder[], char goal[], int tomove) {

  printf("Moving %d ", tomove);

  /* trivial case of two disks */
  if (tomove == 2) {
    printf("right now:\n");
    movechar(&origin, &goal);
    printset();
    movechar(&origin, &holder);
    printset();
    movechar(&goal, &holder);
    printset();

    return;
  }

  printf(". . . Winding\n\n");

  restack(origin, goal, holder, (tomove - 1));

  printf("UnWinding . . . move new bottom disk.\n");

  /* the 'complexity-reducing' step: move the bottom disk */
  movechar(&origin, &holder);

  printset();

  restack(goal, holder, origin, (tomove - 1));

  if ((!strcmp(origin, INITIAL)) || (!strcmp(goal, INITIAL)) ||
      (!strcmp(holder, INITIAL))) {
    printf("\nFinished!\n");
    exit(EXIT_SUCCESS);
  }

  return;
}

int main(void) {

  /*
    version which allows the size of the char arrays to be declared
    as the length of the initializer

    strcpy(first, INITIAL);
    strcpy(second, "");
    strcpy(third, "");
  */

  printset();

  restack(first, second, third, strlen(INITIAL));

  printset();

  return (EXIT_SUCCESS);
}
