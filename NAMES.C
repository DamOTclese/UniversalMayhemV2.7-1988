
/* **********************************************************************
   * names.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Display the players ship names, personal names, if they are	*
   * active, destroyed, or havn't moved, and how many planets they	*
   * have named.							*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"
#include "string.h"
#include "mem.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char to_save;
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern long count;
   extern unsigned short ocount;
   extern short players;
   extern char *record;
   extern char is_redirected;

   static char pause_state;

/* **********************************************************************
   * If running from the console (local) allow the correction of the    *
   * faulty record.                                                     *
   *                                                                    *
   ********************************************************************** */

static void rebuild_record(short count)
{
    c_out(LIGHTRED, "\n\r---> Entry %d is CORRUPTED! <---\n\r", count);
    if (is_redirected != 0) return;

    c_out(LIGHTRED, "Should I correct this entry? ");
    timed_input(0);
    ucase(record);
    if (record[0] != 'Y') return;
    make_zero_record();

    memcpy(enemy, ships, sizeof(struct ship_file));

/*    *enemy = *ships;  FRED */
    write_enemy(count);
    read_user();
}

/* **********************************************************************
   * See if the players record is valid.                                *
   *                                                                    *
   ********************************************************************** */

static char is_acceptable(short count)
{
    char ctest;

    if (count == 0) return(TRUE);
        
    if (strlen(enemy->ship_name) != 4) {
        rebuild_record(count);
        return(FALSE);
    }

    for (ctest = 0; ctest < 4; ctest++) {
        if (enemy->ship_name[ctest] < 'A' || enemy->ship_name[ctest] > 'Z') {
            if (enemy->ship_name[ctest] < '0' || enemy->ship_name[ctest] > '9') {
               rebuild_record(count);
               return(FALSE);
            }
        }
    }

    return(TRUE);
}

static char display_this_enemy(void)
{
   char hold_name[32];

   STRNCPY(hold_name, enemy->ship_person, 30);
   hold_name[21] = (char)NULL;

   c_out(WHITE, "%4s - %-20s  ", enemy->ship_name, hold_name);

   if (enemy->ship_xpos == xsize / 2 &&
      enemy->ship_ypos == ysize / 2 && enemy->tax_warnings != (char)AUCTION_SHIP) {
      c_out(WHITE, " hasn't moved yet");
   }
   else if (enemy->tax_warnings == (char)AUCTION_SHIP) {
      c_out(LIGHTRED, " Impounded       ");
   }
   else if (enemy->ship_xpos == 0 && enemy->ship_ypos == 0) {
      c_out(LIGHTGREEN, " murdered by %-4s", enemy->who_destroyed);
   }
   else {
      if (enemy->local == 0) {
         c_out(WHITE, " active          ");
      }
      else {
         c_out(LIGHTBLUE, " remote player!  ");
      }
   }

   c_out(WHITE, " owns ");

   if (enemy->planets_owned == 0) {
      c_out(YELLOW, "no ");
   }
   else {
      c_out(YELLOW, "%03d", enemy->planets_owned);
   }

   c_out(WHITE, " planets");

   if (enemy->outstanding_bid != (char)NIL) {
      if (Good_Hold(enemy->outstanding_bid)) {
         c_out(LIGHTRED, " Bid on %s\n\r", hold[enemy->outstanding_bid]->names);
      }
      else {
         log_error(114);
      }
   }
   else {
     c_out(WHITE, "\n\r");
   }

   if (ocount++ != 20) {
      return(TRUE);
   }

   if (pause_state) {
      c_out(WHITE,
          "\n\rHit [ENTER] to continue, (S)top listing, or (N)o pause: ");

      timed_input(1);
      ucase(record);

      if (record[0] == 'S') {
         return(FALSE);
      }

      if (record[0] == 'N') {
         pause_state = FALSE;
      }
   }

   ocount = 0;
   return(TRUE);
}

/* **********************************************************************
   * Perform a list of all players.                                     *
   *                                                                    *
   ********************************************************************** */

void perform_names(void)
{
   ocount = 0;
   pause_state = TRUE;

   for (count = 0; count < players; count++) {
      if (Good_Hold(count)) {
         read_enemy(count);

         if (strcmp(enemy->ship_name, "NONE")) {
            if (is_acceptable(count)) {
               if (! display_this_enemy()) {
                  return;
               }
            }
         }
      }
   }
}


