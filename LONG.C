
/* **********************************************************************
   * long.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Here we will offer the general direction of an enemy ship. This 	*
   * is done by employing the following algorythem:			*
   *									*
   * if xpos(enemy) < xpos(ship) - 50 then pos(1...3) = 1, 2, 3		*
   * if xpos(enemy) > xpos(ship) + 50 then pos(1...3) = 7, 8, 9 else	*
   *					   pos(1...3) = 4, 5, 6		*
   *									*
   * And then when:							*
   *									*
   * if ypos(enemy) < ypos(ship) - 50 then dir is pos(1) else		*
   * if ypos(enemy) > ypos(ship) + 50 then dir is pos(3) else		*
   *					   dir is pos(2)		*
   *									*
   * By searching outside of a squair of xpos subtracted 50 and added	*
   * 50, and ypos subtracted 50 and added 50, we can exclude those 	*
   * ships which fall within 100 sectors of the scanning ship. If a	*
   * ship being looked for is within this 100 sectors, direction 5 will	*
   * be returned.							*
   *									*
   * We don't want to be too acurate because it would be simple enough	*
   * to simply find a corner of a scan and jump into the center to	*
   * find the ship. Because of this, we add in a random number from 1	*
   * to 10 to make it inaccurate.					*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long count;
   extern UC zpos;
   extern char *record;
   extern short players;
   extern short user_number;

void perform_long(void)
{
   char hold_record[81], *atpoint;
   char what_ship[81];

   strcpy(hold_record, record);
   record[0] = (char)NULL;
   atpoint = hold_record;

   atpoint += 4;

   while(*atpoint == 0x20) {
      atpoint++;
   }

   while (strlen(atpoint) != 4) {
      c_out(WHITE, "Enter name of ship to look for: ");
      timed_input(0);
      ucase(record);
      atpoint = record;
      if (strlen(atpoint) != 4) {
         c_out(WHITE, "Ships name MUST be 4 characters.\n\r");
      }
   }

   strcpy(what_ship, atpoint);
   strcpy(record, hold_record);

   if (! strcmp(what_ship, hold[user_number]->names)) {
      c_out(LIGHTRED, "You don't need to search for your own ship!\n\r");
      return;
   }

   if (! strcmp(what_ship, "NONE")) {
      c_out(WHITE, "Invalid ship name to search for!\n\r");
      return;
   }

   for (count = 0; count < players; count++) {
      if (Good_Hold(count)) {
	 if (! strcmp(hold[count]->names, what_ship)) {
            if (hold[count]->sxpos == 0 && hold[count]->sypos == 0) {
                c_out(WHITE, "That ship has been destroyed!\n\r");
            }
            else if (zpos == hold[count]->szpos) {
	       found_this_ship(count);
	    }
	    else {
	       c_out(WHITE, "Ship is in universe %d\n\r",
		  hold[count]->szpos);
	    }
            return;
         }
      }
   }

   c_out(WHITE, "Ship %s is not active!\n\r", what_ship);
}

void found_this_ship(short target)
{
   short the_dir, offset;

   offset = arandom(1L, 10L);

   the_dir =
      compute_direction(user_number, target, 40 + offset, 40 + offset);

   if (the_dir == (short)NIL) {
      c_out(LIGHTRED, "That ship has been destroyed already!\n\r");
      return;
   }

   if (the_dir == 5) {
      c_out(LIGHTRED, "Ships transponder has been turned off!\n\r");
      return;
   }

   c_out(WHITE, "General direction: %d\n\r", the_dir);
}

