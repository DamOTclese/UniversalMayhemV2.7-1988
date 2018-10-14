
/* **********************************************************************
   * remote.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Perform scan from a remote, place a remote, recall a remote, or	*
   * attach a remote to an enemy ship.					*
   *									*
   * Attaching a remote is done by warping onto an enemy ship and then	*
   * deploying the remote. We will scan to see if we are on top of a	*
   * ship when the deploy is done. If so, we tell the operator that the	*
   * enemy ship is attached.						*
   *									*
   * Remote syntax is:							*
   *   remnC		 where n is remote number 1, 2, or 3		*
   *			 and C is P for place, R for recall.		*
   *									*
   * If the P or R is missing, then a scan should be performed from the	*
   * remote.								*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdio.h"
#include "stdlib.h"
#include "function.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long dloop;
   extern long xpos, ypos;
   extern UC zpos;
   extern char *record;
   extern char from_remote;
   extern short players;
   extern short bail_out;
   extern short user_number;
   extern short rpt_loop;

void perform_remote(void)
{
   unsigned short remote_number;
   char *tpoint;

   tpoint = record;
   tpoint += 3;
   remote_number = (*tpoint) - 0x30;
   tpoint++;

   read_user();

   if (remote_number < 0 || remote_number > 9) {
      c_out(WHITE, "Valid remote numbers are 0 through 9.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (strlen(record) == 4) {
      remote_from(remote_number);
      return;
   }

   if ((*tpoint) == 'r' || (*tpoint) == 'R') {
      recall_remote(remote_number);
      return;
   }

   if ((*tpoint) == 'P' || (*tpoint) == 'P') {
      place_remote(remote_number);
      return;
   }

   c_out(WHITE, "Syntax for remote is bad. Use R to recall, or P to place.\n\r");
   bail_out = 1;
   rpt_loop = 0;
}

/* **********************************************************************
   * Check to see if the requested remote is valid. If so, request a	*
   * scan from it.							*
   *									*
   * We can do a scan from the remote by shoveing the xpos and ypos of	*
   * the ship into holding variables. We then perform the scan after	*
   * setting xpos and ypos to that of the remote. When we return, set	*
   * the variables back.						*
   *									*
   ********************************************************************** */

void remote_from(unsigned short remote_number)
{
   long holdx, holdy;
   UC holdz;

   read_user();

   if (ships->rem_xpos[remote_number] == ONBOARD) {
      c_out(WHITE, "Remote %d has not been deployed yet.\n\r", remote_number);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[remote_number] == 0) {
      c_out(WHITE, "Remote %d has not been bought.\n\r", remote_number);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[remote_number] < 1) {
      if (Good_Hold(abs(ships->rem_xpos[remote_number]))) {
         c_out(LIGHTRED, "Remote %d was destroyed by %s\n\r", remote_number,
            hold[abs(ships->rem_xpos[remote_number])]->names);
      }
      else {
         log_error(114);
      }
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   holdx = xpos;
   holdy = ypos;
   holdz = zpos;
   from_remote = TRUE;
   xpos = hold[user_number]->xremotes[remote_number];
   ypos = hold[user_number]->yremotes[remote_number];
   zpos = hold[user_number]->remote_universe[remote_number];
   perform_scan();
   from_remote = FALSE;
   xpos = holdx;
   ypos = holdy;
   zpos = holdz;
}

/* **********************************************************************
   * Place the remote. Check to see if it has been bought first, then	*
   * check to see if it is already deployed.				*
   *									*
   ********************************************************************** */

void place_remote(unsigned short remote_number)
{
   if (ships->rem_xpos[remote_number] == 0) {
      c_out(WHITE, "You have not bought remote %d yet.\n\r", remote_number);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[remote_number] < 0 &&
      ships->rem_xpos[remote_number] != ONBOARD) {
      if (Good_Hold(abs(ships->rem_xpos[remote_number]))) {
         c_out(LIGHTRED, "Remote %d was destroyed by %s!\n\r",
            remote_number, hold[abs(ships->rem_xpos[remote_number])]->names);
      }
      else {
         log_error(114);
      }
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[remote_number] != ONBOARD) {
      c_out(WHITE, "Remote %d already deployed.\n\r", remote_number);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   ships->rem_xpos[remote_number] = xpos;
   ships->rem_ypos[remote_number] = ypos;
   ships->rem_universe[remote_number] = zpos;
   hold[user_number]->xremotes[remote_number] = xpos;
   hold[user_number]->yremotes[remote_number] = ypos;
   hold[user_number]->remote_universe[remote_number] = zpos;

/*
   If the remote is being placed on another ship, display a
   message that is descriptive. We call it 'attaching'.

   START AT 1 HERE! So that we can't attach the Galactic Police!
*/

   for (dloop = 1; dloop < players; dloop++) {
      if (Good_Hold(dloop) && dloop != user_number) {
         if (hold[dloop]->sxpos == xpos && hold[dloop]->sypos == ypos &&
            hold[dloop]->szpos == zpos) {
	    c_out(YELLOW,
	       "Attaching remote %d to ship %s at {%ld-%ld} (Universe %d)\n\r",
	       remote_number,
	       hold[dloop]->names,
	       hold[dloop]->sxpos,
	       hold[dloop]->sypos,
	       hold[dloop]->szpos);
	    write_user();
	    return;
         }
      }
   }

   c_out(WHITE,
      "Remote %d placed at {%ld-%ld} (Universe %d)\n\r",
      remote_number, xpos, ypos, zpos);
   write_user();
}

/* **********************************************************************
   * Here is where we recall the remote. We must check to see if the	*
   * remote is deployed at the current posistion. To do this, and to	*
   * offer the proper error message, we check to see if the remote is	*
   * on board, destroyed, or at this posistion.				*
   *									*
   * We also check to see if we are unattaching an enemy ship so that a	*
   * message may be displayed.						*
   *									*
   ********************************************************************** */

void recall_remote(unsigned short remote_number)
{
   if (ships->rem_xpos[remote_number] == 0) {
      c_out(WHITE, "You havn't bought remote %d yet.\n\r", remote_number);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[remote_number] < 0 &&
      ships->rem_xpos[remote_number] != ONBOARD) {
      if (Good_Hold(abs(ships->rem_xpos[remote_number]))) {
         c_out(LIGHTRED, "Remote %d was destroyed by %s!\n\r",
            remote_number, hold[abs(ships->rem_xpos[remote_number])]->names);
      }
      else {
         log_error(114);
      }
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[remote_number] == ONBOARD) {
      c_out(WHITE, "You havn't deployed remote %d yet!\n\r", remote_number);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->rem_xpos[remote_number] != xpos ||
      ships->rem_ypos[remote_number] != ypos ||
      ships->rem_universe[remote_number] != zpos) {
      c_out(WHITE, "You are not on top of remote %d!\n\r", remote_number);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

/*
   If the remote is being taken off of a ship, display a message that
   is descriptive. We call it 'unattaching'.

   START AT 1 HERE! Cops can't be attached so why bother checking?
   If it is, it would simply drop off at this point so let it!
*/

   for (dloop = 1; dloop < players; dloop++) {
      if (Good_Hold(dloop) && dloop != user_number) {
         if (hold[dloop]->sxpos == xpos && hold[dloop]->sypos == ypos &&
            hold[dloop]->szpos == zpos) {
	    c_out(YELLOW, "Unataching remote %d from %s.\n\r",
               remote_number, hold[dloop]->names);
         }
      }
   }

   c_out(WHITE, "Recalling remote %d from {%ld-%ld} and storing it on board.\n\r",
      remote_number, xpos, ypos);

   ships->rem_xpos[remote_number] = ONBOARD;
   ships->rem_ypos[remote_number] = ONBOARD;
   ships->rem_universe[remote_number] = 0;
   hold[user_number]->xremotes[remote_number] = 0;
   hold[user_number]->yremotes[remote_number] = 0;
   hold[user_number]->remote_universe[remote_number]=0;
   write_user();
}

