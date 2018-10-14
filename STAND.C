
/* **********************************************************************
   * stand.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991                                    *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Display the standings.						*
   *									*
   * Show the top 10 killers and the top 10 property owners. Display	*
   * the location of the biggest property owners ship.			*
   *									*
   * We must sort the array kind of. We find the strongest then display	*
   * it. We then set the value to negative. When we are done with the	*
   * top ten, we set them back to positives.				*
   *									*
   * Before we do this, however, we need to update the standings data	*
   * for the current ship. This is done for the fired upon ships as	*
   * well to keep the values fair.					*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "stdlib.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern FILE *ship_std;
   extern long dloop;
   extern long xpos, ypos;
   extern UC zpos;
   extern short players;

void perform_stand(char to_file)
{
   int dloop, sloop, top_count;
   int gave_stand;
   char out_buffer[75];

   gave_stand = 0;

   if (!to_file) {
      c_out(YELLOW, "\n\rThese top ten ships own the most planets:\n\r");
      c_out(YELLOW, "------------------------------------------\n\r");
   }
   else {
      if ((ship_std = mayhem_fopen("SHIP.STD", "w", ship_std)) == (FILE *)NULL) {
	 log_error(40);
	 return;
      }
      fputs("\n\rThese top ten ships own the most planets:\n\r", ship_std);
      fputs("------------------------------------------\n\r", ship_std);
   }

   for (sloop = 0; sloop < 10; sloop++) {
      top_count = 0;
      for (dloop = 0; dloop < players; dloop++) {
         if (Good_Hold(dloop)) {
            if (hold[dloop]->standings > hold[top_count]->standings) {
               top_count = dloop;
            }
         }
      }

      if (hold[top_count]->standings > 0) {
	 if (!to_file) {
	    c_out(LIGHTBLUE,
	       "   ship %s is #%d", hold[top_count]->names, sloop + 1);
	 }
	 else {
	    sprintf(out_buffer,
	       "   ship %s is #%d", hold[top_count]->names, sloop + 1);
	    fputs(out_buffer, ship_std);
	 }
         hold[top_count]->standings = -hold[top_count]->standings;
         gave_stand = 1;
      }

      if (sloop == 0 && gave_stand == 1) {
	 if (!to_file) {
	    c_out(LIGHTBLUE, " position: (%ld-%ld) {Universe %d}\n\r",
	       hold[top_count]->sxpos,
	       hold[top_count]->sypos,
	       hold[top_count]->szpos);
	 }
	 else {
	    sprintf(out_buffer, " position: (%ld-%ld) {Universe %d}\n\r",
	       hold[top_count]->sxpos,
	       hold[top_count]->sypos,
	       hold[top_count]->szpos);
	    fputs(out_buffer, ship_std);
	 }
      }
      else if (gave_stand == 1) {
	 if (!to_file) {
	    c_out(WHITE, "\n\r");
	 }
	 else {
	    fputs("\n\r", ship_std);
	 }
      }

      gave_stand = 0;
   }

   for (dloop = 0; dloop < players; dloop++) {
      if (Good_Hold(dloop)) {
         if (hold[dloop]->standings < 0) {
            hold[dloop]->standings = abs(hold[dloop]->standings);
         }
      }
   }

/*
   Basically a duplicate of the above nested loops but useing
   different data to determine its display parameters.
*/

   if (!to_file) {
      c_out(YELLOW, "\n\r\n\rHere are the top ten killers:\n\r");
      c_out(YELLOW, "-------------------------------\n\r");
   }
   else {
      fputs("\n\r\n\rHere are the top ten killers:\n\r", ship_std);
      fputs("-------------------------------\n\r", ship_std);
   }

   for (sloop = 0; sloop < 10; sloop++) {
      top_count = 0;
      for (dloop = 0; dloop < players; dloop++) {
         if (Good_Hold(dloop)) {
            if (hold[dloop]->kills > hold[top_count]->kills) {
               top_count = dloop;
            }
         }
      }

      if (hold[top_count]->kills > 0) {
	 if (!to_file) {
	    c_out(LIGHTBLUE, "   ship %s killed %d times\n\r",
	       hold[top_count]->names, hold[top_count]->kills);
	 }
	 else {
	    sprintf(out_buffer, "   ship %s killed %d times\n\r",
	       hold[top_count]->names, hold[top_count]->kills);
	    fputs(out_buffer, ship_std);
	 }

         hold[top_count]->kills = -hold[top_count]->kills;
      }
   }

   for (dloop = 0; dloop < players; dloop++) {
      if (Good_Hold(dloop)) {
         if (hold[dloop]->kills < 0) {
            hold[dloop]->kills = abs(hold[dloop]->kills);
         }
      }
   }
   if (to_file) {
      mayhem_fclose(&ship_std);
   }
}

