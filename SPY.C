
/* **********************************************************************
   * spy.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow the relocation of a remote which is already deployed.	*
   * 									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "universe.h"
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

   extern long xsize, ysize;
   extern long xpos, ypos, txpos, typos;
   extern UC zpos, tzpos;
   extern char *record;
   extern short players;
   extern short bail_out;
   extern short user_number;
   extern short rpt_loop;
   extern short directions[18];

void perform_spy(void)
{
   char *tpoint, attach_flag;
   short rem_num, the_dir, the_vel, hold_index, stest;
   long rxpos, rypos, oldx, oldy;
   UC rzpos, oldz;

   plug_close_objects(CLOSE_NO_ION);
   attach_flag = 0;

   if (strlen(record) != 6) {
      bad_spy();
      return;
   }

   tpoint = record;
   tpoint += 3;
   rem_num = *tpoint++ - 0x30;
   the_dir = *tpoint++ - 0x30;
   the_vel = *tpoint - 0x30;

   if (rem_num < 0 || rem_num > 9 || the_dir < 1 || the_dir > 9 ||
      the_dir == 5 || the_vel < 1 || the_vel > 5) {
      bad_spy();
      return;
   }

   if (ships->rem_xpos[rem_num] == 0) {
      c_out(WHITE, "That remote hasn't been bought yet!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[rem_num] == ONBOARD) {
      c_out(WHITE, "That remote has not been deployed yet!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[rem_num] < 0) {
      if (Good_Hold(abs(ships->rem_xpos[rem_num]))) {
         c_out(LIGHTRED, "That remote had been destroyed by %s!\n\r",
            hold[abs(ships->rem_xpos[rem_num])]->names);
      }
      else {
         log_error(114);
      }
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   hold_index = ((the_dir - 1) * 2);
   txpos = ships->rem_xpos[rem_num] + (long) the_vel * directions[hold_index];
   typos = ships->rem_ypos[rem_num] + (long) the_vel * directions[++hold_index];
   tzpos = ships->rem_universe[rem_num];

   if (txpos < 0 || txpos > xsize - 1 || typos < 0 || typos > ysize - 1) {
      c_out(LIGHTRED, "Sensor tried to leave the universe!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   oldx = ships->rem_xpos[rem_num];
   oldy = ships->rem_ypos[rem_num];
   oldz = ships->rem_universe[rem_num];
   ships->rem_xpos[rem_num] = txpos;
   ships->rem_ypos[rem_num] = typos;
   hold[user_number]->xremotes[rem_num] = txpos;
   hold[user_number]->yremotes[rem_num] = typos;
   write_user();

/*
   See if the remote was on a ship. If so, display that it was
   unattached, otherwise say that it just moved.
*/

   for (stest = 1; stest < players; stest++) {  /* Not on a cop */
      if (Good_Hold(stest)) {
         if (hold[stest]->sxpos == oldx &&
	    hold[stest]->sypos == oldy &&
	    hold[stest]->szpos == oldz) {
            if (stest != user_number) {

               c_out(LIGHTRED, "Remote unattaching from ship %s\n\r",
                  hold[stest]->names);

               attach_flag = 1;
            }
	 }
      }
   }

   if (attach_flag == 0) {
      c_out(WHITE, "Remote %d - {%ld-%ld}\n\r", rem_num, txpos, typos);
   }

/*
   See if the remote was deposited into a star, mine, or black hole. If it was
   dropped into a star or mine, destroy it. If a black hole, make it come out
   at another place.
*/

   read_universe(txpos);

   if (typos == universe.star) {
      c_out(LIGHTRED, "Remote destroyed by star!\n\r");
      remove_destroyed_remote(rem_num);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ypos == universe.mine) {
      c_out(LIGHTRED, "Remote destroyed by mine!\n\r");
      remove_destroyed_remote(rem_num);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

/*
   Enter a black hole. If there is a white hole, then exit it
   but one sector away.
*/

   if (ypos == universe.black_hole) {
      c_out(YELLOW, "Remote entered black hole!\n\r");
      if (universe.white_hole == 0) {
         rxpos = arandom(1L, xsize - 1);
	 rypos = arandom(1L, ysize - 1);
	 rzpos = arandom(1L, 8L);
      }
      else {
         rxpos = universe.white_hole;
         read_universe(rxpos);
	 rypos = universe.black_hole - 1;
	 rzpos = zpos;
      }
      ships->rem_xpos[rem_num] = rxpos;
      ships->rem_ypos[rem_num] = rypos;
      ships->rem_universe[rem_num] = rzpos;
      hold[user_number]->xremotes[rem_num] = rxpos;
      hold[user_number]->yremotes[rem_num] = rypos;
      hold[user_number]->remote_universe[rem_num] = rzpos;
      return;
   }

   if (find_specific_planet(xpos, ypos) == 1) {
      c_out(LIGHTGREEN, "Remote is in orbit.\n\r");
      return;
   }

/*
   See if remote went onto a ship. If so, say that it was attached.
*/

   for (stest = 1; stest < players; stest++) {  /* Not on a cop */
      if (Good_Hold(stest)) {
         if (hold[stest]->sxpos == txpos &&
	    hold[stest]->sypos == typos &&
	    hold[stest]->szpos == tzpos) {
            if (stest != user_number) {

               c_out(LIGHTBLUE, "Remote %d attaching to ship %s\n\r",
                  rem_num, hold[stest]->names);

            }
	 }
      }
   }
}

/* **********************************************************************
   * The spy command way bad. Offer a message and set bail-outs.	*
   *									*
   ********************************************************************** */

void bad_spy(void)
{
   c_out(WHITE, "Spy command syntax is:\n\r");
   c_out(WHITE, "   SPYrdv  Where r - The remote number 0 through 9\n\r");
   c_out(WHITE, "                 d - The direction, (0 to 9 except 5)\n\r");
   c_out(WHITE, "                 v - The velocity, (0 to 5 sectors)\n\r\n\r");
   bail_out = 1;
   rpt_loop = 0;
}

/* **********************************************************************
   * A remote has been destroyed. Get rid of it.			*
   *									*
   ********************************************************************** */

void remove_destroyed_remote(short rem_num)
{
   ships->rem_xpos[rem_num] = 0;
   ships->rem_ypos[rem_num] = 0;
   ships->rem_universe[rem_num] = 0;
   hold[user_number]->xremotes[rem_num] = 0;
   hold[user_number]->yremotes[rem_num] = 0;
   hold[user_number]->remote_universe[rem_num] = 0;
   write_user();
}

