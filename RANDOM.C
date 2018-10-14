
/* **********************************************************************
   * random.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * This function returns a random number from low to high.		*
   *									*
   * We try 40 times for a number suitable and within bounds before we	*
   * offer a random number generation failure.				*
   *									*
   ********************************************************************** */

#include "stdlib.h"
#include "stdio.h"

long arandom(long low, long high)
{
   long the_test;
   short tcount, try_random;

   the_test = 0;
   try_random = 0;

   for (tcount = 0; tcount < 40; tcount++, try_random++) {
      if (high < 5) {
	 the_test = rand() / 8000;
      }
      else if (high < 9) {
	 the_test = rand() / 4000;
      }
      else if (high < 17) {
	 the_test = rand() / 2000;
      }
      else if (high < 32) {
         the_test = rand() / 1000;
      }
      else if (high < 327) {
         the_test = rand() / 100;
      }
      else if (high < 3276) {
         the_test = rand() / 10;
      }
      else if (high < 32766) {
         the_test = rand();
      }
      else if (high < ((long) 32766 * 2)) {
         the_test = (long) rand() * 2;
      }
      else if (high < ((long) 32766 * 3)) {
         the_test = (long) rand() * 3;
      }

      if (the_test > low && the_test < high) {
         return((long) the_test);
      }
   }

/*
   c_out(WHITE, "Random number fail: low %ld high %ld last test %ld\n\r",
      low, high, the_test);
*/

   return(low);
}

