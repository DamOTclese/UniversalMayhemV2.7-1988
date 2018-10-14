
/* **********************************************************************
   * plague.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow the buying or use of biological weapons.			*
   *									*
   * Also allow the seeding of unprotected planets.			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "planets.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long xpos, ypos;
   extern unsigned short docked;
   extern char total_pirate_count;
   extern char pirate;
   extern short user_number;
   extern short the_rnd;

void perform_plague(void)
{
   if (docked == 1 && ships->plague_flag > 0) {
      if (planets.protected != (char)NIL) {
         if (Good_Hold(planets.protected)) {
            c_out(WHITE, "Planet is protected from plague by %s\n\r",
               hold[planets.protected]->names);
         }
         else {
            log_error(114);
         }
         return;
      }
   }

   if (docked == 1 && ships->plague_flag > 0) {
      ships->plague_flag--;
      write_user();
      c_out(LIGHTRED, "Planetary surface water seeded with plague.\n\r");
      read_universe(xpos);

      if (find_specific_planet(xpos, ypos) == 0) {
         return;
      }

      read_planets(xpos);
      planets.plagued = user_number;
      write_planets(xpos);
      return;
   }

   if (docked == 1) {
      c_out(WHITE, "You are docked and have no plague virus!\n\r");
      return;
   }

   if (pirate == 0) {
      c_out(WHITE, "There are no pirates in your area!\n\r");
      return;
   }

   if (ships->ship_credits < 500000L) {
      c_out(WHITE, "You don't have enough cash to buy plague!\n\r");
      return;
   }

   ships->plague_flag++;
   ships->ship_credits -= 500000L;
   c_out(YELLOW, "Class %d Plague brought aboard ship! Contained in sick bay.\n\r",
      ships->plague_flag);
   total_pirate_count = 0;
   write_user();
}

/* **********************************************************************
   * Cause some deaths due to infection.				*
   *									*
   ********************************************************************** */

void trigger_infection(void)
{
   if (ships->ship_crew < 50) {
      return;
   }

   the_rnd = arandom(10L, (long) ships->ship_crew);

   c_out(YELLOW, "* * * Sickbay reporting: Infection strikes %d crew members!\n\r",
      the_rnd);

   ships->ship_crew -= the_rnd;
   ships->sick_bay = arandom(1L, (long) the_rnd);

   c_out(YELLOW, "* * * Sickbay reporting: %d crew members in sick bay!\n\r",
      the_rnd);

   write_user();
}

