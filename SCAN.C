
/* **********************************************************************
   * scan.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   ********************************************************************** */

#include "defines.h"
#include "universe.h"
#include "comets.h"
#include "holder.h"
#include "goal.h"
#include "scout.h"
#include "stdio.h"
#include "function.h"
#include "ctype.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern UC gateway_access[13][4][2];
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern UC zpos;
   extern short pxpos, pypos;
   extern long aloop, tloop, dloop;
   extern long ion_field[ION_COUNT][2];
   extern unsigned short ion_trail;
   extern unsigned short comet_count;
   extern unsigned short close_ship[TOTAL_PLAYERS];
   extern unsigned short close_base[TOTAL_PLAYERS];
   extern unsigned short close_remotes[TOTAL_PLAYERS];
   extern unsigned short close_swarms[TOTAL_PLAYERS];
   extern unsigned short close_comets[40];
   extern unsigned short close_goals[10];
   extern unsigned short close_scouts[10];
   extern char *record;
   extern char goals_count;
   extern char from_remote;
   extern short ship_count;
   static short swarm_flag;
   static long anum, count;
   extern short base_count;
   extern short remote_count;
   extern short user_number;
   extern short swarm_count;
   extern char scout_count;
   extern char ham_version;

/* **********************************************************************
   * Do a sensor scan of the area. Report objects within the scan range	*
   * which are visible. If a cloaked ship, display an ion trail.	*
   *									*
   ********************************************************************** */

void perform_scan(void)
{
   ion_trail = FALSE;
   swarm_flag = 0;

   plug_close_objects(CLOSE_NORMAL);

   if (ion_trail) {
      c_out(LIGHTRED, "\n\rSensors report an ion trail in this area...\n\r");
   }

   if (from_remote) {
      c_out(LIGHTGREEN, "Scanning from remote...\n\r");
   }

   c_out(LIGHTRED, "   - - - Universe {%d} - - -", zpos);

   c_out(WHITE, "\n\r       ");
   for (count = ypos - 8; count <= ypos + 8; count++) {
      anum = (int) count / 10000;
      c_out(WHITE, "%4ld", anum);
   }

   c_out(WHITE, "\n\r       ");

   for (count = ypos - 8; count <= ypos + 8; count++) {
      anum = count;

      if (count < 0 || count > ysize - 1) {
         anum = 0;
      }

      if (anum > 9999) {
         anum -= 10000;
      }

      c_out(WHITE, "%4.4ld", anum);
   }

   c_out(WHITE, "\n\r       ----++++----++++----++++----++++----++++");
   c_out(WHITE, "----++++----++++----++++----\n\r");

   for (count = xpos - 5; count <= xpos + 5; count++) {
      if (count >= 0 && count < xsize - 1) {
         read_universe(count);
      }
      else {
         universe.planets[0] = universe.planets[1] = 0;
         universe.planets[2] = universe.planets[3] = 0;
         universe.star = universe.mine = 0;
         universe.black_hole = universe.white_hole = 0;
      }
      anum = count;

      if (count < 0 || count > xsize - 1) {
         anum = 0;
      }

      c_out(WHITE, "%05.5ld", anum);
      if (! ham_version) {
         c_out(WHITE, " |");
      }
      else {
         c_out(WHITE, " :");
      }

      for (dloop = ypos - 8; dloop <= ypos + 8; dloop++) {
	 scan_elements();
      }
      if (! ham_version) {
         c_out(WHITE, "|\n\r");
      }
      else {
         c_out(WHITE, ":\n\r");
      }
   }

   c_out(WHITE, "       ----++++----++++----++++----++++----++++");
   c_out(WHITE, "----++++----++++----++++----\n\r\n\r");

   if (comet_count > 0) {
      check_test_comet_scan();
   }
}

/* **********************************************************************
   * After all of the area has been scanned for objects, display the	*
   * various things.							*
   *									*
   ********************************************************************** */

void scan_elements(void)
{
   short int xion, x;
   char afloop;

/*
   See if we are still in the universe
*/

   if (count < 0 || count > ysize - 1 || dloop < 0 || dloop > xsize - 1) {
      c_out(CYAN, "####");
      return;
   }

/*
   If the scanning ship is at the position being scanned, display its
   name, otherwise continue.
*/

   if (count == xpos && dloop == ypos && from_remote == FALSE) {
      c_out(LIGHTGREEN, hold[user_number]->names);
      return;
   }

/*
   If the scan is from the active ships remote and we are scanning the
   location of the remote, display the remote symbol.
*/

   if (count == xpos && dloop == ypos && from_remote == TRUE) {
      c_out(YELLOW, "<RR>");
      return;
   }

/*
   See if we are scanning an ion field.
*/

   for (xion = 0; xion < ION_COUNT; xion++) {
      if (ion_field[xion][0] == count && ion_field[xion][1] == dloop) {
	 c_out(YELLOW, "@@@@");
         return;
      }
   }

/*
   To keep planets, stars, mines and black holes from showing up at
   the upper left side of the universe, we check and return blank at
   this spot always. When a ship gets destroyed, its location is set to
   0000-0000. This may be something of a bug as perhaps the location
   of the ship should go to -1 -1. That would help because the #### is
   displayed when the end of the universe is displayed.

   The gateway access points are also displayed.
*/

   if (count == 0 && dloop == 0) {
      if (gateway_access[zpos][0][0] == 99)
	 c_out(WHITE, "    ");
      else
	 c_out(LIGHTBLUE, "-%02d-", gateway_access[zpos][0][0]);
      return;
   }

   if (count == 0 && dloop == ysize - 1) {
      if (gateway_access[zpos][1][0] == 99)
	 c_out(WHITE, "    ");
      else
	 c_out(LIGHTBLUE, "-%02d-", gateway_access[zpos][1][0]);
      return;
   }

   if (count == xsize - 1 && dloop == 0) {
      if (gateway_access[zpos][2][0] == 99)
	 c_out(WHITE, "    ");
      else
	 c_out(LIGHTBLUE, "-%02d-", gateway_access[zpos][2][0]);
      return;
   }

   if (count == xsize - 1 && dloop == ysize - 1) {
      if (gateway_access[zpos][3][0] == 99)
	 c_out(WHITE, "    ");
      else
	 c_out(LIGHTBLUE, "-%02d-", gateway_access[zpos][3][0]);
      return;
   }

/*
   See if there is a ship at this spot. This is done before planets
   so that ships docked will show up on scanners.
*/

   for (aloop = 0; aloop < ship_count; aloop++) {
      if (hold[close_ship[aloop]]->sxpos == count &&
	 hold[close_ship[aloop]]->sypos == dloop) {
	    c_out(LIGHTRED, hold[close_ship[aloop]]->names);
            return;
      }
   }

/*
   See if a planet is sitting here.
*/

   if (universe.planets[0] == dloop || universe.planets[1] == dloop ||
      universe.planets[2] == dloop || universe.planets[3] == dloop) {
	 c_out(LIGHTGREEN, "<00>");
         return;
   }

/*
   See if a star is here.
*/

   if (universe.star == dloop) {
      c_out(YELLOW, "<**>");
      return;
   }

/*
   Note that we display 4 of the % characters. This is needed to get
   two of them on the display.
*/

   if (universe.mine == dloop) {
      c_out(YELLOW, "<%%%%>");
      return;
   }

/*
   How about a black hole
*/

   if (universe.black_hole == dloop) {
      c_out(CYAN, "<  >");
      return;
   }

/*
   See if a remote is being used and it is scanning the owners ship.
*/

   if (from_remote &&
      count == hold[user_number]->sxpos &&
      dloop == hold[user_number]->sypos) {
	 c_out(LIGHTGREEN, "%s", hold[user_number]->names);
      return;
   }

/*
   See if there is a base at this spot
*/

   for (aloop = 0; aloop < base_count; aloop++) {
      if (hold[close_base[aloop]]->bxpos == count &&
         hold[close_base[aloop]]->bypos == dloop) {
	    for (x = 0; x < 4; x++) {
	       c_out(LIGHTRED, "%c", tolower(hold[close_base[aloop]]->names[x]));
	    }
            return;
      }
   }

/*
   See if there is a remote at this spot
*/

   for (aloop = 0; aloop < remote_count; aloop++) {
      for (tloop = 0; tloop < 10; tloop++) {
         if (hold[close_remotes[aloop]]->xremotes[tloop] == count &&
	    hold[close_remotes[aloop]]->yremotes[tloop] == dloop &&
	    hold[close_remotes[aloop]]->remote_universe[tloop] == zpos) {
	       c_out(LIGHTGREEN, "<RR>");
               return;
         }
      }
   }

/*
   See if this spot has an attack swarm sitting on it
*/

   for (aloop = 0; aloop < swarm_count; aloop++) {
      for (tloop = 0; tloop < 15; tloop++) {
	 if (hold[close_swarms[aloop]]->xswarm[tloop] == count &&
	    hold[close_swarms[aloop]]->yswarm[tloop] == dloop) {
	       c_out(LIGHTRED, " .  ");
	       return;
	 }
      }
   }

/*
   See if its a pirate
*/

   if (pxpos == count && pypos == dloop) {
      c_out(MAGENTA, "<OX>");
      return;
   }

/*
   See if it's a comet
*/

   for (aloop = 0; aloop < comet_count; aloop++) {
      if (comets[close_comets[aloop]] != (struct comets_file *)NULL) {
         if (comets[close_comets[aloop]]->location[0] == count &&
            comets[close_comets[aloop]]->location[1] == dloop) {
            if (comets[close_comets[aloop]]->direction > 5) {
               c_out(YELLOW, "O---");
            }
            else {
               c_out(YELLOW, "---O");
            }
            return;
         }
      }
   }

/*
   After testing, it was desided that it would be faster to have "close"
   find those goal items that are within scanning range.
*/

   for (afloop = 0; afloop < goals_count; afloop++) {
      if (goal_item[close_goals[afloop]] != (struct goal_elements *)NULL) {
         if (goal_item[close_goals[afloop]]->goal_xpos == (short)count &&
            goal_item[close_goals[afloop]]->goal_ypos == (short)dloop) {
            c_out(LIGHTRED, "#%02d#", close_goals[afloop]);
            return;
         }
      }
   }

/*
   See if there is a scout ship out here. Display a symbol that will
   depict an inbound, outbound, or on station scout.
*/

   for (afloop = 0; afloop < scout_count; afloop++) {
      if (scouts[close_scouts[afloop]]->scout_xpos == count &&
         scouts[close_scouts[afloop]]->scout_ypos == dloop) {
         switch (scouts[close_scouts[afloop]]->scout_direction) {
            case SCOUT_INBOUND:
               c_out(YELLOW, "<-%d-", close_scouts[afloop]);
               break;
            case SCOUT_OUTBOUND:
               c_out(YELLOW, "-%d->", close_scouts[afloop]);
               break;
            case SCOUT_STATION:
               c_out(YELLOW, "<>-%d", close_scouts[afloop]);
               break;
         }
         return;
      }
   }

/*
   If there is nothing here, display a blank item.
*/

   c_out(WHITE, "    ");
}

/* **********************************************************************
   * See if the comet(s) in the area have been scanned before. If not,	*
   * update their information. Allow them to be named.			*
   *									*
   ********************************************************************** */

void check_test_comet_scan(void)
{
   for (aloop = 0; aloop < comet_count; aloop++) {
      if (comets[close_comets[aloop]] != (struct comets_file *)NULL) {
         if (! comets[close_comets[aloop]]->flag) {
            comets[close_comets[aloop]]->flag = TRUE;
            strcpy(comets[close_comets[aloop]]->ship, hold[user_number]->names);
            c_out(WHITE, "You have found a new comet!\n\r");
            c_out(WHITE, "WHAT would you like to name it? ");
            timed_input(0);
            record[14] = (char)NULL;
            if (strlen(record) < 2) {
               strcpy(record, "Not named.\n\r");
            }
            else {
               strcpy(comets[close_comets[aloop]]->name, record);
               write_comets();
            }
	 }
      }
   }
}

