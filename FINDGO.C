
/* **********************************************************************
   * findgo.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Find and display those planets which fall within scanner range.	*
   * Also offer the warp commands which will allow travel to them.	*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "universe.h"
#include "planets.h"
#include "stdio.h"
#include "function.h"
#include "stdlib.h"
#include "conio.h"

/* **********************************************************************
   * The 'to_save' value determines if planets may be named. It		*
   * contains the numeric value of the string that is normally used to	*
   * contains the planets name, (actually the length of the string).	*
   *									*
   ********************************************************************** */

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char to_save;
   extern long xsize;
   extern long dloop, count;
   extern long xpos, ypos;
   extern unsigned short atpoint;
   extern char *record, *go_string;
   extern char *point;
   extern short bail_out;
   extern short rpt_loop;
   extern short the_remove;

   static short low_warp;

void perform_find(void)
{
   int distance;

   low_warp = 1000;
   distance = ships->ship_sensor;

   if (distance > 50) {
      distance = 50;
   }

   c_out(WHITE, "Finding planets within a sensor range of: %d\n\r", distance);
   c_out(WHITE, "-----\n\r");

   for (count = xpos - distance; count < xpos + distance; count++) {
      if (count > 1 && count < xsize - 1) {
         read_universe(count);
         for (dloop = 0; dloop < 4; dloop++) {
            if (abs(universe.planets[dloop] - ypos) < distance + 1) {
              offer_planets_position();
            }
         }
      }
   }
   c_out(WHITE, "-----\n\r");

   if (low_warp != 1000) {
      c_out(WHITE, "Closest planet is %d sectors away.\n\r", low_warp);
   }
   else {
      c_out(WHITE, "Unable to locate a close planet.\n\r");
      bail_out = 1;
      rpt_loop = 0;
   }
}

/* **********************************************************************
   * Here is where we determine the location and the warp commands	*
   * which are needed to get to a planet.				*
   *									*
   * Various themes are under consideration. The closest planet could	*
   * be determined and the warp commands needed to get to it be fed 	*
   * into the warp drive. Because the warp unit may not be high enough	*
   * to perform the command, the string would have to be rebuilt to use	*
   * the maximum warp capability.					*
   *									*
   ********************************************************************** */

void offer_planets_position(void)
{
   int test_warp, small_jump;
   char hold_go[10], go_test[201], appending[5];

   read_planets(count);

   if (to_save == 0) {
      if (strncmp(planets.named, "NONE", 4)) {
	 c_out(WHITE, "%16s: ", planets.named);
      }
      else {
	 c_out(WHITE, "                  ");
      }
   }

   c_out(WHITE, "(%ld-%d) Warp: ", count, universe.planets[dloop]);
   go_test[0] = (char)NULL;
   test_warp = 0;

   if (xpos == count) {
      goto dont_xpos;
   }

   if (xpos - count < 1) {
      c_out(WHITE, "W8");
      strcpy(go_test, "W8");
      strcpy(appending, ";W8");
   }
   else {
      c_out(WHITE, "W2");
      strcpy(go_test, "W2");
      strcpy(appending, ";W2");
   }

   test_warp = (short) abs(xpos - count);
   small_jump = test_warp;
   c_out(WHITE, "%d", test_warp);

   while (small_jump > ships->ship_warp) {
      sprintf(hold_go, "%d", ships->ship_warp);
      strcat(go_test, hold_go);
      small_jump -= ships->ship_warp;
      if (small_jump > 0) {
         strcat(go_test, appending);
      }
   }

   if (small_jump > 0) {
      sprintf(hold_go, "%d", small_jump);
      strcat(go_test, hold_go);
   }

dont_xpos:
   if (xpos != count) {
      c_out(WHITE, ";");
      strncat(go_test, ";", 1);
   }

   if (ypos == universe.planets[dloop]) {
      goto dont_ypos;
   }

   if (ypos - universe.planets[dloop] < 1) {
      c_out(WHITE, "W6");
      strcat(go_test, "W6");
      strcpy(appending, ";W6");
   }
   else {
      c_out(WHITE, "W4");
      strcat(go_test, "W4");
      strcpy(appending, ";W4");
   }

   small_jump = (short) abs(ypos - universe.planets[dloop]);
   test_warp += small_jump;
   c_out(WHITE, "%d", small_jump);

   while (small_jump > ships->ship_warp) {
      sprintf(hold_go, "%d", ships->ship_warp);
      strcat(go_test, hold_go);
      small_jump -= ships->ship_warp;
      if (small_jump > 0) {
	 strcat(go_test, appending);
      }
   }

   if (small_jump > 0) {
      sprintf(hold_go, "%d", small_jump);
      strcat(go_test, hold_go);
   }

dont_ypos:
   if (xpos == count && ypos == universe.planets[dloop]) {
      c_out(LIGHTGREEN, "CURRENT PLANET");
      test_warp = 0;
   }

   if (planets.protected != (char)NIL) {
      c_out(LIGHTRED, " ---> Protected <---");
   }
  
   c_out(WHITE, "\n\r");

   if (test_warp < low_warp && test_warp != 0) {
      low_warp = test_warp;
      strcpy(go_string, go_test);
   }
}

/* **********************************************************************
   * So that we don't destroy the 'string command parser' routines      *
   * pointers and buffers and such, let'd do a quick and dirty command  *
   * parser and extraction here.                                        *
   *                                                                    *
   ********************************************************************** */

char strip_go_command(void)
{
   char *gopoint;

   gopoint = record;
   record[atpoint] = (char)NULL;

   if (! perform_warp()) return(FALSE);

   gopoint += atpoint + 1;
   strcpy(record, gopoint);
   return(TRUE);
}

/* **********************************************************************
   * Take the 'record' and put it into hold_record. Then parse out the	*
   * 'go_string' so that it uses the maximum warp speed possible. Then	*
   * call the warp routine to move the ship. When done, restore the	*
   * holding string back to 'record' and return to caller.		*
   *									*
   * This command will move a ship to the closest planet possible.	*
   *									*
   ********************************************************************** */

void perform_go(void)
{
   if (go_string[0] != 'W') {
      c_out(LIGHTRED, "GO will not work unless you request FIND first.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   the_remove = 0;
   find_power_needed(ships->ship_warp);

   if (the_remove > ships->ship_power) {
      c_out(LIGHTRED, "Navagational computers need %d units of power to operate!\n\r",
	 (short) the_remove);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }
   else {
      ships->ship_power -= the_remove;
   }

   strcpy(record, go_string);

do_go_move_again:
   if ((atpoint = match()) != 0) {
      if (strip_go_command()) {
         goto do_go_move_again;
      }
      go_string[0] = (char)NULL;
      point++; point++;
      return;
   }
   else if (strlen(record) > 1) {
      if (! perform_warp()) {
         go_string[0] = (char)NULL;
         point++; point++;
         return;
      }
   }

   go_string[0] = (char)NULL;		/* Set go string to NULL no go again */
   point++; point++;			/* point past the 'GO' command in    */
					/* the command string parser pointer */
}


