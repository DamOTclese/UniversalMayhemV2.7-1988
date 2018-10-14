
/* **********************************************************************
   * protect.c								*
   *                                                                    *
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Protect a planet. The cost is 1 million credits times the 		*
   * technology level!							*
   *									*
   * We also buy the planet thus incrimenting the ownership value in	*
   * the users file. The number of owned planets are used to determine	*
   * the standings.							*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "planets.h"
#include "universe.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long xpos, ypos, dloop;
   extern unsigned short docked;
   extern char *record;
   extern short user_number;
   extern char to_save;

void perform_protect(void)
{
   short floop;

   if (docked == 0) {
      c_out(LIGHTRED, "You are not docked with any planets.\n\r");
      return;
   }

   if (ships->ship_credits < (long) 1000000L * (long) planets.technology) {
      c_out(LIGHTRED, "You need %d million credits to buy and protect this planet.\n\r",
	 planets.technology);
      return;
   }

   read_universe(xpos);

   if (find_specific_planet(xpos, ypos) == 0) {
      c_out(LIGHTRED, "This planet appears to be dead!\n\r");
      return;
   }

   read_planets(xpos);

   if (planets.technology == 0) {
      if (Good_Hold(planets.visited)) {
         c_out(LIGHTRED, "Too late! This planet was destroyed by %s!\n\r",
            hold[planets.visited]->names);
      }
      else {
         log_error(114);
      }
      return;
   }

   if (planets.protected == user_number) {
      c_out(LIGHTRED, "Are you sure you want to unprotect this planet? ");
      timed_input(0);
      ucase(record);
      if (record[0] != 'Y') {
	 return;
      }
      for (floop = 0; floop < OWNABLE; floop++) {
         if (ships->owned_planets[floop] == xpos) {
            planets.protected = (char)NIL;
	    write_planets(xpos);
	    read_user();
	    hold[user_number]->standings--;
            ships->owned_planets[floop] = (short)NIL;
            ships->ship_credits -= (long) 10000L * (long) planets.technology;
	    write_user();
	    c_out(LIGHTGREEN, "Planet unprotected, sold for %ld credits\n\r",
	       (long) 10000L * (long) planets.technology);
	    perform_stand(TRUE);
	    return;
	 }
      }
      c_out(LIGHTRED,
	 "Ships computer failed to find this planet in galactic records\n\r");
      return;
   }

   if (planets.protected != (char)NIL) {
      if (Good_Hold(planets.protected)) {
         c_out(LIGHTRED, "This planet has already been bought by %s.\n\r",
            hold[planets.protected]->names);
      }
      else {
         log_error(114);
      }
      return;
   }

   for (floop = 0; floop < OWNABLE; floop++) {
      if (ships->owned_planets[floop] == (short)NIL ||
         ships->owned_planets[floop] == xpos) {
	 planets.protected = user_number;
	 read_user();
         ships->ship_credits -= (long) 1000000L * (long) planets.technology;
         ships->planets_owned++;
	 hold[user_number]->standings++;
         ships->owned_planets[floop] = xpos;
	 write_user();
	 write_planets(xpos);
	 c_out(LIGHTGREEN, "Planet cost %ld credits to buy. Planet protected.\n\r",
	    (long) 1000000L * (long) planets.technology);
	 perform_stand(TRUE);
	 return;
      }
   }

   c_out(LIGHTRED, "Ships computer can't store that many ownership records!\n\r");
}

/* **********************************************************************
   * Display the locations of the planets this ship owns.               *
   *                                                                    *
   * Make sure to pause if there are many of them so that they don't    *
   * scroll off of the screen.                                          *
   *                                                                    *
   ********************************************************************** */

void perform_own(void)
{
    char floop, lc;
    short total;
    long highestx, highesty;
    short highest;

    for (floop = lc = total = highest = 0; floop < OWNABLE; floop++) {
        if (ships->owned_planets[floop] != (short)NIL) {
            read_universe(ships->owned_planets[floop]);
            for (dloop = 0; dloop < 4; dloop++) {
                read_planets(ships->owned_planets[floop]);
                if (planets.protected == user_number) {
                    c_out(LIGHTBLUE, "Planet at {%d-%d} Technology %d",
                        (short)ships->owned_planets[floop],
                        (short)universe.planets[dloop],
                        planets.technology);

                    if (planets.technology > highest) {
                        highest = planets.technology;
                        highestx = ships->owned_planets[floop];
                        highesty = universe.planets[dloop];
                    }

                    if (to_save == 0) {
                        if (strncmp(planets.named, "NONE", 4)) {
                            c_out(LIGHTGREEN, " Name: %s", planets.named);
                        }
                        else {
                            c_out(LIGHTGREEN, " Not named");
                        }
                    }

                    c_out(WHITE, "\n\r");
                    lc++;
                    total++;
                    if (lc == 20) {
                        c_out(WHITE, "Hit [ENTER] to continue: ");
                        timed_input(FALSE);
                        lc = 0;
                    }
                }
            }
        }
    }

    c_out(WHITE, "\n\rYou own %d planets.\n\r", total);

    if (total > 1) {
        c_out(WHITE, "Highest technology is [%d] at {%ld-%ld}\n\r",
            highest, highestx, highesty);
    }
}

