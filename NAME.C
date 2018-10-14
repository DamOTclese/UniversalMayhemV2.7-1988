
/* **********************************************************************
   * name.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow a planet to be named. We must make sure that the planet is	*
   * not already named. In addition, of course, we must make sure that	*
   * the ship is docked at a planet.					*
   *									*
   * It is very important to allocate a tempporary string to hold the	*
   * input. This is because the player may exceed the 30 character	*
   * limit on the string. If this is done during entry of the string,	*
   * it could overwrite data or code memory which could crash the	*
   * program, leaving the BBS it is running on hung! This is totally	*
   * unacceptable behavior of a program!				*
   *									*
   * To make sure that dloop is pointint to the proper planet, we do a	*
   * read universe, find, and then the name. We also, you might notice,	*
   * update the ships named-planet counter.				*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "planets.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "stdlib.h"
#include "ctype.h"
#include "conio.h"
#include "string.h"

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
   extern long xpos, ypos;
   extern unsigned short docked;
   extern char *record;
   extern short user_number;

void perform_name(void)
{
   char hold_name[81];

   if (to_save > 0) {
      c_out(LIGHTRED,
	 "Planetary governments do not allow the nameing of planets.\n\r");
      return;
   }

   hold_name[0] = (char)NULL;

   if (docked == 0) {
      c_out(WHITE, "You are not docked at a planet.\n\r");
      return;
   }

   read_universe(xpos);

   if (find_specific_planet(xpos, ypos) == 0) {
      c_out(WHITE, "You can't name a dead planet.\n\r");
      return;
   }

   read_planets(xpos);

   if (strcmp(planets.named, "NONE") &&
      strcmp(planets.named, hold[user_number]->names)) {
      c_out(WHITE, "You can't rename a planet! That's not nice!\n\r");
      return;
   }

   while (strlen(hold_name) < 1 || strlen(hold_name) > 15) {
      c_out(LIGHTGREEN, "What would you like to name this planet? ");
      timed_input(0);
      strcpy(hold_name, record);

      if (strlen(hold_name) > 15) {
	 c_out(LIGHTRED, "Name must be less than 15 characters long.\n\r");
      }
   }

   strcpy(planets.named, hold_name);
   write_planets(xpos);
   write_user();
}

