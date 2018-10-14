
/* **********************************************************************
   * peace.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Declare peace with someone.					*
   *									*
   * Display the ships the player is already allied with. See if there	*
   * is an empty spot to make another alliance. Then ask for the new	*
   * name to make alliance with. Scan the names of the ships and make	*
   * sure that the name is valid. For good measure, make sure that the	*
   * name entered is not already an ally.				*
   *									*
   * If the ship has a command file that is attacking the ship that an	*
   * alliance is desired with, then issue a message and return without	*
   * allowing it until the command file is updated.			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
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

   extern FILE *command_file;
   extern long count;
   extern char *record;
   extern short players;
   extern short user_number;

void perform_peace(void)
{
   short ally_count, empty_spot;

   ally_count = 0;
   empty_spot = (short)NIL;

   c_out(WHITE, "\n\rYou are allied with: ");

   for (count = 0; count < 5; count++) {
      if (strcmp(ships->allies[count], "NONE")) {
         c_out(WHITE, "%s ", ships->allies[count]);
         ally_count++;
      }
      else {
         if (empty_spot == (short)NIL) {
	    empty_spot = count;
	 }
      }
   }

   if (ally_count == 0) {
      c_out(WHITE, "No one\n\r\n\r");
   }
   else {
      c_out(WHITE, "\n\r\n\r");
   }

   if (ally_count == 5) {
      c_out(WHITE, "You can't make another alliance.\n\r");
      return;
   }

   c_out(WHITE, "Enter the name of the new ship to make alliance to: ");
   timed_input(0);
   ucase(record);
   record[4] = (char)NULL;

   if (strlen(record) != 4) {
      return;
   }

   if (! strcmp(hold[user_number]->names, record)) {
      c_out(LIGHTRED, "Make an alliance with yourself?\n\r");
      return;
   }

   if (! strcmp(record, "<GP>")) {
      c_out(LIGHTRED, "Make friends with the cops? ");
      c_out(LIGHTRED, "You can't make friends with cops!\n\r");
      return;
   }

   for (count = 0; count < 5; count++) {
      if (! strcmp(ships->allies[count], record)) {
	 c_out(YELLOW, "Pay attention! You are already allied to that ship!\n\r");
         return;
      }
   }

   if (check_command_file()) {
      c_out(YELLOW, "Your COMMAND file is logged for attacking %s!\n\r", record);
      c_out(YELLOW, "Change COMMAND file or don't make this alliance.\n\r");
      return;
   }

   for (count = 0; count < players; count++) {
      if (Good_Hold(count)) {
         if (! strcmp(hold[count]->names, record)) {
            if (empty_spot == (short)NIL) {
	       c_out(LIGHTRED, "Can't make alliance with that ship!\n\r");
	       return;
	    }
            strcpy(ships->allies[empty_spot], hold[count]->names);
	    hold[count]->is_friendly = TRUE;
	    write_user();
	    c_out(WHITE, "Alliance to ship %s has been logged.\n\r",
	       hold[count]->names);
            return;
         }
      }
   }

   c_out(WHITE, "That is not an active ship.\n\r");
}

/* **********************************************************************
   * If an alliance exists, remove it.					*
   *									*
   ********************************************************************** */

void fire_at_friend(short attacked_ship, short friend_number)
{
   c_out(LIGHTRED, "You have fired at an allied ship! \n\r");

   if (Good_Hold(attacked_ship)) {
      c_out(LIGHTRED, "Alliance between %s and %s broken!\n\r",
         hold[attacked_ship]->names, ships->allies[friend_number]);
   }
   else {
      log_error(114);
   }

   read_user();
   strcpy(ships->allies[friend_number], "NONE");
   write_user();
}

/* **********************************************************************
   * See if the command file, if any, for this ship is set up to attack	*
   * the ship that peace is desired with. If so, don't allow it. We	*
   * return with TRUE if there is an alliance.				*
   *									*
   ********************************************************************** */

unsigned char check_command_file(void)
{
   char file_name[20], arec[201], *tpoint;

   strcpy(file_name, hold[user_number]->names);
   strcat(file_name, ".SHP");

   if ((command_file = mayhem_fopen(file_name, "r", command_file)) == (FILE *)NULL) {
      return(FALSE);
   }

   while (! feof(command_file)) {
      fgets(arec, 200, command_file);
      if (! feof(command_file)) {
	 tpoint = arec;
         skipspace(tpoint);
         if (! strncmp(tpoint, "attack", 6)) {
            tpoint += 7;
            skipspace(tpoint);
            if (! strncmp(tpoint, record, 4)) {
               mayhem_fclose(&command_file);
	       return(TRUE);
	    }
	 }
      }
   }

   mayhem_fclose(&command_file);
   return(FALSE);
}


