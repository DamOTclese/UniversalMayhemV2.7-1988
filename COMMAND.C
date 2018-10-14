
/* **********************************************************************
   * command.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * The plugging routines here create a linked list of structures	*
   * containing information on the automation of the inactive ships->   *
   *									*
   *             ----          						*
   * FIRST ---> |DATA|                                                  *
   *             ----       ----                                        *
   *            |NEXT|---> |DATA|                                       *
   *             ----       ----       ----                             *
   *                       |NEXT|---> |DATA| <--- LAST <--- RUN         *
   *                        ----       ----                             *
   *                                  |NEXT|---> NULL                   *
   *                                   ----                             *
   *									*
   * Several pointers are maintained: m_first, m_last, and m_run. The	*
   * example above shows that three ships have automation elements	*
   * active against the currently active ship. It shows which of the	*
   * structures is the first and last, with the last structure pointing	*
   * to NULL. 								*
   * 									*
   * Most importantly, it shows where the next structure to be "run" 	*
   * is being pointed to. The running structure is to be executed by	*
   * checking the elements to see if the automated ship should execute	*
   * an activity against the currently active ship.			*
   *									*
   * The RUN pointer would start from FIRST, work linearly through the	*
   * list to LAST. When a ship "bails-out", (stops being automated),	*
   * the RUN pointer is brought to the FIRST, rather than taking up	*
   * execution to the next structure, thus making some ships have more	*
   * than their share of automative movements.				*
   *									*
   * In the above example, the last structure is to be executed. When	*
   * its automation is done what it must, it points back to the FIRST.	*
   *									*
   ********************************************************************** */

/* **********************************************************************
   * If the console is redirected, then show the messages that give the	*
   * SysOp a little something more interesting to look at.		*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "universe.h"
#include "holder.h"
#include "command.h"
#include "stdio.h"
#include "function.h"
#include "stdlib.h"
#include "alloc.h"
#include "io.h"
#include "string.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char is_redirected;
   extern FILE *command_file;
   extern long xsize, ysize;
   extern long xpos, ypos, txpos;
   extern UC zpos, tzpos;
   extern long dloop;
   extern char *record;
   extern char suspended_real_time;
   extern short players;
   extern short bail_out;
   extern short user_number;
   extern short time_remaining;
   extern short the_remove;
   extern short directions[18];
   extern char valid_command_file;
   extern char the_date[27];
   extern unsigned short close_ship[TOTAL_PLAYERS];
   extern short ship_count;

/* **********************************************************************
   * This module makes use of a lot of string array data to contain	*
   * information that isn't needed by any of the other modules. We	*
   * normally make a lot of data available to all modules but here I	*
   * made an exception.							*
   *									*
   ********************************************************************** */

   static short ccount;
   static char file_name[20], *tpoint, aanswer[10];
   static char attack[MAX_COMMAND][5], defend[MAX_COMMAND][5];
   static char assist[MAX_COMMAND][5], run_from[MAX_COMMAND][5];
   static long power_bail;
   static short torp_bail, time_bail, decoy_bail, damage_bail;
   static short ass_count, def_count, att_count, run_count;

   short last_priority;
   char attacked_ships[TOTAL_PLAYERS];

/* **********************************************************************
   * Define the function prototypes that exist elsewhere we need.       *
   *                                                                    *
   ********************************************************************** */

   void erase_command_file(void);
   void download_command_file(void);
   void remove_attacked_ship(char this_ship);

/* **********************************************************************
   *									*
   * Allow the maintenance of the command file for the ship. If the	*
   * player would like, she may upload the command file and use this    *
   * option to instruct the sysop to move it to the proper area.	*
   *									*
   ********************************************************************** */

void perform_command(void)
{
   char newly_created;
   UL phaser_value;
   char default_fight;
         
   strcpy(file_name, hold[user_number]->names);
   strcat(file_name, ".SHP");

/*
   Turn automation of other ships off so that the active player is not
   interrupted while maintaining its command file.
*/

   suspended_real_time = TRUE;
   newly_created = FALSE;

/*
   See if the command file exists. If not, ask if it should be created.
   Either way, after that, zero out the variables we will be needing to
   make sure that only those elements of information the player want are
   saved to the command file at the end.
*/

   if ((command_file = mayhem_fopen(file_name, "rt", command_file)) == (FILE *)NULL) {
      c_out(WHITE, "Do you want to create a command file? ");
      timed_input(0);
      ucase(record);

      if (record[0] != 'Y') {
         suspended_real_time = FALSE;
         return;
      }

      if ((command_file = mayhem_fopen(file_name, "wt", command_file)) == (FILE *)NULL) {
         c_out(WHITE, "Can't create command file!\n\r");
         return;
      }

      newly_created = TRUE;
   }

   ass_count = def_count = att_count = run_count = 0;
   torp_bail = time_bail = decoy_bail = damage_bail = 0;
   power_bail = 0L;
   phaser_value = 0L;
   default_fight = 0;

/*
   Read out the command file one line at a time. As we find commands
   we are interested in, extract the information and put the ship names
   into the various arrays.

   Anything that doesn't match what we want is ignored (allows comments).
*/

   while (! feof(command_file)) {
      fgets(record, 200, command_file);
      if (! feof(command_file)) {
         ucase(record);
         tpoint = record;
         skipspace(tpoint);

         if (! strncmp(tpoint, "ASSIST", 6)) {
            tpoint += 7;
            skipspace(tpoint);
            if (ass_count < MAX_COMMAND) {
               STRNCPY(assist[ass_count], tpoint, 4);
               assist[ass_count++][4] = (char)NULL;
            }
         }
         else if (! strncmp(tpoint, "DEFEND", 6)) {
            tpoint += 7;
            skipspace(tpoint);
            if (def_count < MAX_COMMAND) {
               STRNCPY(defend[def_count], tpoint, 4);
               defend[def_count++][4] = (char)NULL;
            }
         }
         else if (! strncmp(tpoint, "ATTACK", 6)) {
            tpoint += 7;
            skipspace(tpoint);
            if (att_count < MAX_COMMAND) {
                STRNCPY(attack[att_count], tpoint, 4);
                attack[att_count++][4] = (char)NULL;
            }
         }
         else if (! strncmp(tpoint, "PHASER", 6)) {
            tpoint += 7;
            skipspace(tpoint);
            phaser_value = atol(tpoint);
         }
         else if (! strncmp(tpoint, "RUN-FROM", 8)) {
            tpoint += 9;
            skipspace(tpoint);
            if (run_count < MAX_COMMAND) {
                STRNCPY(run_from[run_count], tpoint, 4);
                run_from[run_count++][4] = (char)NULL;
            }
         }
         else if (! strncmp(tpoint, "BAIL-OUT", 8)) {
            tpoint += 9;
            skipspace(tpoint);
            if (! strncmp(tpoint, "POWER", 5)) {
               tpoint += 6;
               power_bail = atol(tpoint);
            }
            else if (! strncmp(tpoint, "TORP", 4)) {
               tpoint += 5;
               torp_bail = atoi(tpoint);
            }
            else if (! strncmp(tpoint, "TIME", 4)) {
               tpoint += 5;
               time_bail = atoi(tpoint);
            }
            else if (! strncmp(tpoint, "DAMAGE", 6)) {
               tpoint += 7;
               damage_bail = 1;
            }
            else if (! strncmp(tpoint, "DECOY", 5)) {
               tpoint += 6;
               decoy_bail = atoi(tpoint);
            }
            else {
	       record[0] = (char)NULL;
            }
         }
         else if (! strncmp(tpoint, "DEFAULT", 7)) {
            tpoint += 8;
            default_fight = atoi(tpoint);
         }
      }
   }

   if (! newly_created) {
ask_it_again:
      c_out(LIGHTGREEN, "Do you want to:\n\r   (D)ownload your command file:\n\r");
      c_out(LIGHTGREEN, "   (E)rase your command file:\n\r");
      c_out(LIGHTGREEN, "   (M)odify your command file: ");
      timed_input(0);
      ucase(record);
      if (record[0] == 'D') {
         download_command_file();
         return;
      }
      if (record[0] == 'E') {
         erase_command_file();
         return;
      }
      if (record[0] != 'M') {
         goto ask_it_again;
      }
   }

/*
   Display assist information and allow the removal or addition to the
   ship names and information.
*/

   c_out(WHITE, "\n\rHere are the ships you want to ASSIST:\n\r");

   for (ccount = 0; ccount < ass_count; ccount++) {
      c_out(WHITE, "%d - %s\n\r", (short) ccount, assist[ccount]);
   }

   aanswer[0] = (char)NULL;

   while (aanswer[0] != 'S') {
      c_out(WHITE,
	 "\n\rDo you want to %s (R)emove, (Q)uit, or (S)ave and continue: ",
         ass_count < MAX_COMMAND ? "(A)dd," : "");

      timed_input(0);
      ucase(record);
      STRNCPY(aanswer, record, 1);
      if (aanswer[0] == 'Q') {
         mayhem_fclose(&command_file);
         return;
      }
      else if (aanswer[0] == 'A' && ass_count < MAX_COMMAND) {
         c_out(WHITE, "Enter ship name to add: ");
         timed_input(0);
         ucase(record);
	 record[4] = (char)NULL;
         if (! strcmp(record, "ALL") || strlen(record) == 4) {
            strcpy(assist[ass_count++], record);
         }
         else {
            c_out(WHITE, "Ship name MUST be 'ALL' or 4 letters long only!\n\r");
         }
      }
      else if (aanswer[0] == 'R') {
         c_out(WHITE, "Enter item number to remove: ");
         timed_input(0);
         ccount = atoi(record);
         if (ccount < 0 || ccount > ass_count) {
            c_out(WHITE, "Invalid item number.\n\r");
         }
         else {
	    assist[ccount][0] = (char)NULL;
         }
      }
   }

/*
   Display defend information and allow the removal or addition to the
   ship names and information.
*/

   c_out(WHITE, "Here are the ships you want to DEFEND:\n\r");

   for (ccount = 0; ccount < def_count; ccount++) {
      c_out(WHITE, "%d - %s\n\r", ccount, defend[ccount]);
   }

   aanswer[0] = (char)NULL;

   while (aanswer[0] != 'S') {
      c_out(WHITE, "\n\rDo you want to %s (R)emove, (Q)uit, or (S)ave and continue: ",
         def_count < MAX_COMMAND ? "(A)dd," : "");

      timed_input(0);
      ucase(record);
      STRNCPY(aanswer, record, 1);
      if (aanswer[0] == 'Q') {
         mayhem_fclose(&command_file);
         return;
      }
      else if (aanswer[0] == 'A' && ccount < MAX_COMMAND) {
         c_out(WHITE, "Enter ship name to add: ");
         timed_input(0);
         ucase(record);
	 record[4] = (char)NULL;
	 if (! strcmp(record, "ALL") || strlen(record) == 4) {
            strcpy(defend[def_count++], record);
         }
         else {
            c_out(WHITE, "Ship name MUST be 'ALL' or 4 letters long only!\n\r");
         }
      }
      else if (aanswer[0] == 'R') {
         c_out(WHITE, "Enter item number to remove: ");
         timed_input(0);
         ccount = atoi(record);
         if (ccount < 0 || ccount > def_count) {
            c_out(WHITE, "Invalid item number.\n\r");
         }
         else {
	    defend[ccount][0] = (char)NULL;
         }
      }
   }

/*
   Display attack information and allow the removal or addition to the
   ship names and information.
*/

   c_out(WHITE, "Here are the ships you want to ATTACK:\n\r");

   for (ccount = 0; ccount < att_count; ccount++) {
      c_out(WHITE, "%d - %s\n\r", ccount, attack[ccount]);
   }

   aanswer[0] = (char)NULL;

   while (aanswer[0] != 'S') {
      c_out(WHITE, "\n\rDo you want to %s (R)emove, (Q)uit, or (S)ave and continue: ",
         att_count < MAX_COMMAND ? "(A)dd," : "");

      timed_input(0);
      ucase(record);
      STRNCPY(aanswer, record, 1);
      if (aanswer[0] == 'Q') {
         mayhem_fclose(&command_file);
         return;
      }
      else if (aanswer[0] == 'A' && ccount < MAX_COMMAND) {
         c_out(WHITE, "Enter ship name to add: ");
         timed_input(0);
         ucase(record);
	 record[4] = (char)NULL;
         if (! strcmp(record, "ALL") || strlen(record) == 4) {
            strcpy(attack[att_count++], record);
         }
         else {
            c_out(WHITE, "Ship name MUST be 'ALL' or 4 letters long only!\n\r");
         }
      }
      else if (aanswer[0] == 'R') {
         c_out(WHITE, "Enter item number to remove: ");
         timed_input(0);
         ccount = atoi(record);
         if (ccount < 0 || ccount > att_count) {
            c_out(WHITE, "Invalid item number.\n\r");
         }
         else {
            attack[ccount][0] = (char)NULL;
         }
      }
   }

/*
   Display run-from information and allow the removal or addition to the
   ship names and information.
*/

   c_out(WHITE, "Here are the ships you want to RUN-FROM:\n\r");

   for (ccount = 0; ccount < run_count; ccount++) {
      c_out(WHITE, "%d - %s\n\r", ccount, run_from[ccount]);
   }

   aanswer[0] = (char)NULL;

   while (aanswer[0] != 'S') {
      c_out(WHITE, "\n\rDo you want to %s (R)emove, (Q)uit, or (S)ave and continue: ",
         run_count < MAX_COMMAND ? "(A)dd," : "");

      timed_input(0);
      ucase(record);
      STRNCPY(aanswer, record, 1);
      if (aanswer[0] == 'Q') {
         mayhem_fclose(&command_file);
         return;
      }
      else if (aanswer[0] == 'A' && ccount < MAX_COMMAND) {
         c_out(WHITE, "Enter ship name to add: ");
         timed_input(0);
         ucase(record);
         record[4] = (char)NULL;
         if (! strcmp(record, "ALL") || strlen(record) == 4) {
            strcpy(run_from[run_count++], record);
         }
         else {
            c_out(WHITE, "Ship name MUST be 'ALL' or 4 letters long only!\n\r");
         }
      }
      else if (aanswer[0] == 'R') {
         c_out(WHITE, "Enter item number to remove: ");
         timed_input(0);
         ccount = atoi(record);
         if (ccount < 0 || ccount > run_count) {
            c_out(WHITE, "Invalid item number.\n\r");
         }
         else {
            run_from[ccount][0] = (char)NULL;
         }
      }
   }

   if (! examine_command_data()) {
      mayhem_fclose(&command_file);
      return;
   }

/*
   See if there is a bail-out value. At any rate, ask if there should be
   one. If so, ask for the value.
*/

   if (power_bail == 0L) {
      c_out(WHITE, "You don't have a bail-out on power.");
   }
   else {
      c_out(WHITE, "Your bail-out on power is %ld", power_bail);
   }
   c_out(WHITE, "\n\rDo you want to change this? ");
   timed_input(0);
   ucase(record);
   if (record[0] == 'Y') {
      c_out(WHITE, "Enter new bail-out value for power (0 to turn off): ");
      timed_input(0);
      power_bail = atol(record);
   }

   if (torp_bail == 0L) {
      c_out(WHITE, "You don't have a bail-out on torpedo.");
   }
   else {
      c_out(WHITE, "Your bail-out on torpedo count is %d", torp_bail);
   }
   c_out(WHITE, "\n\rDo you want to change this? ");
   timed_input(0);
   ucase(record);
   if (record[0] == 'Y') {
      c_out(WHITE, "Enter new bail-out value for torp (0 to turn off): ");
      timed_input(0);
      torp_bail = atoi(record);
   }

/*
   See if there is a bail-out value. At any rate, ask if there should be
   one. If so, ask for the value.
*/

   if (time_bail == 0L) {
      c_out(WHITE, "You don't have a bail-out on time remaining.");
   }
   else {
      c_out(WHITE, "Your bail-out on time remaining is %d", time_bail);
   }
   c_out(WHITE, "\n\rDo you want to change this? ");
   timed_input(0);
   ucase(record);
   if (record[0] == 'Y') {
      c_out(WHITE, "Enter new bail-out value for minutes remaining: ");
      timed_input(0);
      time_bail = atoi(record);
   }

/*
   See if there is a decoy class value. At any rate, ask if there should be
   one. If so, ask for the value.
*/

   if (decoy_bail == 0L) {
      c_out(WHITE, "You don't have a bail-out with decoy.");
   }
   else {
      c_out(WHITE, "Your bail-out with decoy class is %d", decoy_bail);
   }
   c_out(WHITE, "\n\rDo you want to change this? ");
   timed_input(0);
   ucase(record);
   if (record[0] == 'Y') {
      c_out(WHITE, "Enter new bail-out with decoy class: (0 to turn off): ");
      timed_input(0);
      decoy_bail = atoi(record);
   }

/*
   See if there should be damage bail-out. At any rate, ask if there should be
   one. If so, ask for the value.
*/

   c_out(WHITE, "Do you want to bail-out when you receive damage in a fight? ");
   timed_input(0);
   ucase(record);
   if (record[0] == 'Y') {
      damage_bail = 1;
   }

/*
   See if there is a phaser value to use
*/

   if (phaser_value == (UL)0) {
      c_out(WHITE, "You don't have a phaser value.");
   }
   else {
      c_out(WHITE, "Your phaser value is %ld", phaser_value);
   }
   c_out(WHITE, "\n\rDo you want to change this? ");
   timed_input(0);
   ucase(record);
   if (record[0] == 'Y') {
      c_out(WHITE, "Enter new phaser value: (0 to turn off): ");
      timed_input(0);
      phaser_value = atol(record);
   }

/*
   Added a little later, the default fight
*/

    if (default_fight == 0) {
        c_out(WHITE, "\n\rYour ship will not continue the fight if it is\n\r");
        c_out(WHITE, "attacked and then ignored. Do you want to have your\n\r");
        c_out(WHITE, "ship follow the attacker and continue the engagement? ");
        timed_input(FALSE);
        ucase(record);
        if (record[0] == 'Y') {
            default_fight = 1;
        }
    }
    else {
        default_fight = 0;
        c_out(WHITE, "\n\rYour ship will follow an attacker and continue\n\r");
        c_out(WHITE, "the engagement even after the attacker moves off.\n\r");
        c_out(WHITE, "Do you want to continue this way? ");
        timed_input(FALSE);
        ucase(FALSE);
        if (record[0] == 'Y') {
            c_out(WHITE, "\n\rThis means that your ship, upon being attacked\n\r");
            c_out(WHITE, "by an enemy ship that you have no command file\n\r");
            c_out(WHITE, "directives for, will continue the engagement. Is\n\r");
            c_out(WHITE, "what you want? ");
            timed_input(FALSE);
            ucase(record);
            if (record[0] == 'Y') {
                default_fight = 1;
            }
        }
    }

/*
   Now that we have all of the information for the ships command file, let's
   dump that information back into the file. We will create the file over
   again so that we get a clean file.
*/

   if (command_file != (FILE *)NULL) {
      mayhem_fclose(&command_file);
   }

   if ((command_file = mayhem_fopen(file_name, "wt", command_file)) == (FILE *)NULL) {
      c_out(WHITE, "Can't create the new command file!\n\r");
      suspended_real_time = FALSE;
      return;
   }

   for (ccount = 0; ccount < ass_count; ccount++) {
      if (assist[ccount][0] != (char)NULL) {
         sprintf(record,
            "assist %s          ;assist this ship when it attacks\n",
	    assist[ccount]);
         fputs(record, command_file);
      }
   }

   for (ccount = 0; ccount < def_count; ccount++) {
      if (defend[ccount][0] != (char)NULL) {
         sprintf(record, "defend %s             ;when its attacked\n",
	    defend[ccount]);
         fputs(record, command_file);
      }
   }

   for (ccount = 0; ccount < att_count; ccount++) {
      if (attack[ccount][0] != (char)NULL) {
         sprintf(record, "attack %s             ;when it logs on\n",
	    attack[ccount]);
         fputs(record, command_file);
      }
   }

   for (ccount = 0; ccount < run_count; ccount++) {
      if (run_from[ccount][0] != (char)NULL) {
         sprintf(record, "run-from %s           ;when its too close\n",
	    run_from[ccount]);
         fputs(record, command_file);
      }
   }

   if (power_bail > 0) {
      sprintf(record, "bail-out power %ld       ;when low\n",
	 power_bail);
      fputs(record, command_file);
   }

   if (torp_bail > 0) {
      sprintf(record, "bail-out torp %d ;when low\n",
	 torp_bail);
      fputs(record, command_file);
   }

   if (time_bail > 0) {
      sprintf(record, "bail-out time %d         ;when minutes remaining\n",
	 time_bail);
      fputs(record, command_file);
   }

   if (decoy_bail > 0) {
      sprintf(record, "bail-out decoy %d        ;class decoy\n",
	 decoy_bail);
      fputs(record, command_file);
   }

   if (damage_bail > 0) {
      fputs("bail-out damage            ;if we get damaged\n",
	 command_file);
   }

   if (phaser_value > 0) {
      sprintf(record, "phaser %ld        ;Phaser value to use\n",
         phaser_value);
      fputs(record, command_file);
   }

   if (default_fight == 0)
      sprintf(record, "default 0         ;Do not continue the engagement\n");
   else
      sprintf(record, "default 1         ;Continue the engagement\n");

   fputs(record, command_file);

   mayhem_fclose(&command_file);
   c_out(WHITE, "New command file has been written.\n\r");
   valid_command_file = TRUE;
   suspended_real_time = FALSE;
}

/* **********************************************************************
   * Scan the array of attacked ships and if not already in the array,  *
   * append the ship number.                                            *
   *                                                                    *
   ********************************************************************** */

void ship_attacking(char the_ship)
{
   char scanning;
   char empty_spot;

   for (scanning = 0, empty_spot = NIL; scanning < players; scanning++) {
      if (attacked_ships[scanning] == the_ship) {
         return;
      }
      if (attacked_ships[scanning] == (char)NIL && empty_spot == (char)NIL) {
         empty_spot = scanning;
      }
   }

   if (empty_spot != players)
      attacked_ships[empty_spot] = the_ship;
}

/* **********************************************************************
   * See if the ship offered is in the array of attacked ships-> If the  *
   * parameter is NIL, if there are ANY entries, return TRUE.           *
   *                                                                    *
   ********************************************************************** */

char ship_attack(char the_ship)
{
   char scanning;

   for (scanning = 0; scanning < players; scanning++) {
      if (the_ship == (char)NIL && attacked_ships[scanning] != (char)NIL) {
         return(TRUE);
      }
      if (the_ship == attacked_ships[scanning]) {
         return(TRUE);
      }
   }

   return(FALSE);
}

/* **********************************************************************
   * Here is where we take the first ship in the queue and branch to	*
   * determine what is to be performed. After that, we reposiion the	*
   * active entry pointer, (m_run).					*
   *									*
   * If any of the flags are set, we will process the command.		*
   *									*
   * History:								*
   *									*
   * 23/Jun/89 - A bug was detected here. If a ship assists another and	*
   * gets destroyed for its trouble, command__assist() would return	*
   * after the ship was removed. The next command executed was a test	*
   * to see if there was a command__defend() outstanding. Because the	*
   * entry had been removed, the value would be pointing to some 	*
   * unknown place in memory. Now we check the value of hold[ship]	*
   * after each command declairative.					*
   *									*
   ********************************************************************** */

void skim_command_file_queue(void)
{
   char running_ship;
   char allies_check;
   char allow_attack;

   if (m_first == (struct command_options *)NULL) return;

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   running_ship = m_run->ship;

   if (Good_Hold(running_ship) && m_run->assist == 1) command_assist();

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   if (Good_Hold(running_ship) && m_run->defend == 1) command_defend();

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   allow_attack = TRUE;

   if (Good_Hold(running_ship) && m_run->attack == 1) {
        read_enemy(running_ship);
        for (allies_check = 0; allies_check < 5; allies_check++) {
            if (! strncmp(hold[user_number]->names, enemy->allies[allies_check], 4)) {
                allow_attack = FALSE;
            }
        }
        if (allow_attack) command_attack();
   }

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   if (Good_Hold(running_ship) && m_run->run_from == 1) command_run_from();

/*
   Bail out on power
*/

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   if (Good_Hold(running_ship) && m_run->bail_1_out == 1)
      if (bail_1()) return;

/*
   Bail out on torp
*/

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   if (Good_Hold(running_ship) && m_run->bail_2_out == 1)
      if (bail_2())  return;

/*
   Bail out on time
*/

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   if (Good_Hold(running_ship) && m_run->bail_3_out == 1)
      if (bail_3())  return;

/*
   Bail out on damage
*/

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   if (Good_Hold(running_ship) && m_run->bail_4_out == 1)
      if (bail_4())  return;

/*
   See if the ship that's running wants to default attack.
   If it does, see if it has been attacked.
   If it has, perform a command attack.
*/

   if (Good_Hold(running_ship) && m_run->default_fight == 1) {
      if (ship_attack(running_ship)) {
         command_attack();
      }
   }

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

/*
   See if there is another object after the one we just executed. If so,
   reposition the run pointer to that object. If none follow, then point
   back to the start of the linked list.
*/

   if (m_run == (struct command_options *)NULL) {
      log_error(105);
      m_run = m_first;
      return;
   }

   if (m_run->next == (struct command_options *)NULL)
      m_run = m_first;
   else
      m_run = m_run->next;
}

/* **********************************************************************
   * Some SysOps have asked for something more interesting to look at.  *
   * This will display what command file operating ships are doing to   *
   * the console but not to the modem. It is only done when the console *
   * has been redirected.                                               *
   *                                                                    *
   ********************************************************************** */

static void sysop_message(char *the_name, short the_direction, char *doing_what)
{
   char sysop_output[80];

   if (is_redirected == 1) {

      sprintf(sysop_output,
         "SYSOP: Ship %s is %sing. Heading: %d\n",
             the_name, doing_what, the_direction);

      fputs(sysop_output, stderr);
   }
}

/* **********************************************************************
   * Have ship 'attacking_ship' fire back at ship 'm_run->ship'. This   *
   * is because the automated ship has fired at the ship the currently  *
   * active ship has fired at in an assist declairative.                *
   *                                                                    *
   ********************************************************************** */

static void counter_assist(short attacking_ship, short m_run_ship)
{
   UL hit_value;

/*
   If the attacking_ship has enough power to fight back, have it
   determine how much power to use and then deduct it from its ship.
*/

   read_enemy(attacking_ship);
   if (enemy->ship_power < 5000)    return;
   hit_value = arandom(2000L, enemy->ship_power / 2);
   enemy->ship_power -= hit_value;
   write_enemy(attacking_ship);

/*
   Deduct the power from the assisting ships shields.
*/

   read_enemy(m_run_ship);
   enemy->ship_shield -= hit_value;
   if (enemy->ship_shield < 0)   enemy->ship_shield = 0;
   if (enemy->ship_shield < 17)  enemy->ship_hull -= arandom(6L, 15L);
   if (enemy->ship_hull < 0)     enemy->ship_hull = 0;

   c_out(LIGHTRED,
      "%s counter attacked %s, %ld units remain in assisting ship\n\r",
      hold[attacking_ship]->names,
      hold[m_run_ship]->names,
      enemy->ship_shield);

/*
   Redistribute power on assisting ship if we need to.
*/

   if (enemy->ship_shield < 1000 && enemy->ship_power > 5000) {
      hit_value = enemy->ship_shield + enemy->ship_power;
      enemy->ship_shield = enemy->ship_power = hit_value / 2;

      c_out(LIGHTGREEN,
         "Assisting ship %s redistributes power\n\r",
         hold[m_run_ship]->names);
   }

/*
   If the assisting ships hull is strong enough, return.
*/

   if (enemy->ship_hull > 17) {
      write_enemy(m_run_ship);
      return;
   }

/*
   The assisting ship got destroyed!
*/

   enemy->leashed_to = enemy->leashed_by = (char)NIL;
   enemy->ship_xpos = enemy->ship_ypos = 0;
   strcpy(enemy->who_destroyed, hold[attacking_ship]->names);
   write_enemy(m_run_ship);
   hand_over_planets(m_run_ship, attacking_ship);
   return_slaver_parts(m_run_ship, attacking_ship);

/*
   Make sure that the attacking_ship gets credit for the kill!
*/

   read_enemy(attacking_ship);
   enemy->total_kills++;
   hold[attacking_ship]->kills++;
   write_enemy(attacking_ship);

   inform_kill(hold[attacking_ship]->names, hold[m_run->ship]->names, TRUE, 3);

   c_out(LIGHTRED,
      "Assisting ship %s was destroyed by %s!\n\r",
      hold[m_run_ship]->names,
      hold[attacking_ship]->names);

   memory_freed((UL)sizeof(struct holder));
   if_any_bounce_it(m_run_ship);
   farfree(hold[m_run_ship]);
   hold[m_run_ship] = (struct holder *)NULL;
   perform_stand(TRUE);
}

/* **********************************************************************
   * If the active ship is attacking someone, go and assist it in its	*
   * attack. That is, fire at the ship the active ship is fighting. 	*
   *									*
   ********************************************************************** */

void command_assist(void)
{
   short the_dir, hold_ship;
   UL hit_value, total_enemy_power;
   char scanning, scan2;

   if (! ship_attack(NIL)) return;

/*
   Compute the direction ship 'm_run->ship' needs to travel to
   get near the ship user_number.
*/

   if (m_run == (struct command_options *)NULL) {
      log_error(107);
      m_run = m_first;
      return;
   }

   if (m_run->ship == user_number) {
      log_error(142);
      return;
   }

   the_dir = compute_direction(m_run->ship, user_number,
      X_WARP_WITHIN, Y_WARP_WITHIN);

   if (the_dir == (short)NIL)  return;

/*
   If we are not close, warp to it.
*/

   if (the_dir != 5) {
      sysop_message(hold[m_run->ship]->names, the_dir, "assist");
      warp_ship(m_run->ship, the_dir, Y_WARP_WITHIN, 0, 1);
      return;
   }

/*
   The assisting ship has warped into the area. Now fire at the
   ship that the active ship was attacking.

   In the versions _before_ 1.30, we did things wrong, causing the
   duplication of the assisting ship and the removal of the active ship.
   What I did was make the assisting ship the current one and then fire
   at the ship the real active ship is killing, then restoring the
   original ship to active status.

   This didn't work very well. Badly, in fact.

   o 'attacked_ships[]' is an array that holds the ship numbers to fired on.

   o Fire on the ship with phasers if there is enough power available
     to the assisting ship, otherwise simply return and do nothing.
     Make sure that other ships in the area are not hit.

   o Allow the 'attacked_ships[]' ships to fire at the assisting ship,
     allowing destruction or damage to either ships->

   Rather than use the routines already in place, I created some new
   ones. This is because I didn't want to disturb the other routines by
   'dropping' in a new feature into the middle of them. The reason I
   didn't want to do that was because I didn't have the time to take
   debugging any problems likely to arise and this bug that I have
   removed and recoded is bad enough to cause at least one system I
   know of remove it temporarily until I get it fixed.
*/

   read_enemy(m_run->ship);			/* Read assisting ship	*/
   if (enemy->ship_power < 3000) {              /* and check its power. */
      return;					/* Return if not enough	*/
   }						/* to fight well.	*/

/*
   Deduct power from assisting ship
*/

   hit_value = arandom(1000L, (long)enemy->ship_power / 2);
   enemy->ship_power -= hit_value;
   write_enemy(m_run->ship);

/*
   Deduct shielding from ship(s) active ship is attacking for
   all close ships in the area.
*/

   for (scanning = 0; scanning < ship_count; scanning++) {
      for (scan2 = 0; scan2 < players; scan2++) {
         if (close_ship[scanning] == (short)attacked_ships[scan2]) {
            if (Good_Hold(attacked_ships[scan2])) {
               read_enemy(attacked_ships[scan2]);
               enemy->ship_shield -= (hit_value / ship_count);
               if (enemy->ship_shield < 0) enemy->ship_shield = 0;

               c_out(LIGHTRED,
                  "Assisting ship %s attacking %s. %ld unit hit, %ld remaining\n\r",
                  hold[m_run->ship]->names,
                  hold[attacked_ships[scan2]]->names,
                  (long)hit_value / ship_count,
                  (long)enemy->ship_shield);

/*
   If the ship the active ship was attacking has shields less than 17
   units worth, cause some damage to the ships hull.
*/

               if (enemy->ship_shield < 17) {
                  enemy->ship_hull -= arandom(6L, 15L);
                  if (enemy->ship_hull < 0) {
                     enemy->ship_hull = 0;
                  }
               }

/*
   If the ship the active ship was attacking has little or no hull left,
   call the routine which removes it from the universe.
*/

               if (enemy->ship_hull < 17) {
                  remove_attacked_ship(attacked_ships[scan2]);
                  return;
               }
               else if (enemy->ship_hull < 100) {

                  c_out(LIGHTRED, "Ship %s hull is at %d%%\n\r",
                     hold[attacked_ships[scan2]]->names, enemy->ship_hull);
               }

/*
   See if the ship the active ship is attacking should redistribute
   power from engines to shields.
*/

            if (enemy->ship_shield < 1000 && enemy->ship_power > 2000) {
               total_enemy_power = enemy->ship_shield + enemy->ship_power;
               enemy->ship_shield = enemy->ship_power = (total_enemy_power / 2);

               c_out(LIGHTGREEN, "Ship %s redistributes power\n\r",
                  hold[attacked_ships[scan2]]->names);
            }

/*
   Store the status of the ship the active ship was attacking.
*/

               write_enemy(attacked_ships[scan2]);
               counter_assist(attacked_ships[scan2], m_run->ship);
            }
         }
      }
   }
}

/* **********************************************************************
   * If active ship attacks any other ship, defend the ship it is	*
   * attacking.								*
   *									*
   ********************************************************************** */

void command_defend(void)
{
   short the_dir;
   long looper;

   if (! ship_attack(NIL)) return;

/*
   Compute the direction to the active ship rather than the ship that
   was being attacked. In this way, if the attacked ship trys to run,
   we will vector to the active ship.
*/

   if (m_run == (struct command_options *)NULL) {
      log_error(109);
      m_run = m_first;
      return;
   }

   if (m_run->ship == user_number) {
      log_error(142);
      return;
   }

   the_dir = compute_direction(m_run->ship, user_number,
      X_WARP_WITHIN, Y_WARP_WITHIN);

   if (the_dir == (short)NIL) {
      return;
   }

   if (the_dir != 5) {
      sysop_message(hold[m_run->ship]->names, the_dir, "defend");
      warp_ship(m_run->ship, the_dir, Y_WARP_WITHIN, 1, 1);
      return;
   }

/*
   The defending ship has arised. Cause it to fire back phasers
   at the currently active ship.
*/

   read_enemy(m_run->ship);

   for (looper = 0; looper < arandom(1L, 8L); looper++) {
       if (enemy->ship_power > 1000) {
           fire_back_phaser(m_run->ship, m_run->phaser);
       }
   }

   if (enemy->ship_torp > 0) {
      fire_back_torp(m_run->ship);
   }

   write_enemy(m_run->ship);
}

/* **********************************************************************
   * Warp over and attack the active ship.				*
   * (Subject to bail-outs, of course).					*
   *									*
   * Note that when attacking, the distance permitted is the enemy ship	*
   * warp capacity divided by 2. This is done by passing the amount of	*
   * limiting factor to function warp__ship(). It is usually 1 which is	*
   * used to divide into the ships warp capacity.			*
   *									*
   ********************************************************************** */

void command_attack(void)
{
   short the_dir;
   char looper;

   if (m_run == (struct command_options *)NULL) {
      log_error(110);
      m_run = m_first;
      return;
   }

   if (m_run->ship == user_number) {
      log_error(142);
      return;
   }

   the_dir = compute_direction(m_run->ship, user_number,
      X_WARP_WITHIN, Y_WARP_WITHIN);

/*
   If the direction was not computeable, we simple return and ignore
   the attack declairative.
*/

   if (the_dir == (short)NIL) {
      return;
   }

/*
   If the direction is other than 5, then we are not in the area yet.
   Warp in the direction we computed.
*/

   if (the_dir != 5) {
      sysop_message(hold[m_run->ship]->names, the_dir, "attack");
      warp_ship(m_run->ship, the_dir, Y_WARP_WITHIN, 2, 4);
      return;
   }

/*
   The ship has warped over to the active ship.
   Simply fire at the thing!
*/

   read_enemy(m_run->ship);

/*
   For version 1.61, we check to see if the ship has been imported. If
   it has, see if the ship it's about to fire on has signed onto the
   program less than 6 times. If so, don't fire.

   In version 2.2, we don't care if it's imported or not.
*/

   if (ships->log_count < 6)
      return;

   if (enemy->ship_torp > 0) {
      fire_back_torp(m_run->ship);
   }

   for (looper = 0; looper < arandom(1L, 8L); looper++) {
       if (enemy->ship_power > 1000) {
           fire_back_phaser(m_run->ship, m_run->phaser);
       }
   }

   if (enemy->ship_torp > 0) {
      fire_back_torp(m_run->ship);
   }

   write_enemy(m_run->ship);
}

/* **********************************************************************
   * Check to see if the active ship is near. If so, warp in a random	*
   * direction for maximum distance.					*
   *									*
   ********************************************************************** */

void command_run_from(void)
{
   short the_dir, the_times, stat;

   if (m_run == (struct command_options *)NULL) {
      log_error(111);
      m_run = m_first;
      return;
   }

   if (m_run->ship == user_number) {
      log_error(142);
      return;
   }

   the_dir = compute_direction(m_run->ship, user_number,
      X_WARP_WITHIN, Y_WARP_WITHIN);

   if (the_dir != 5) {
      return;
   }

   while (the_dir == 5) {
      the_dir = arandom(1L, 9L);
   }

   c_out(WHITE, "Sensors detect %s is running away at warp speed!\n\r",
      hold[m_run->ship]->names);

   the_times = arandom(14L, 25L);

   stat = warp_ship(m_run->ship, the_dir, 50 * the_times, 3, 1);

   if (stat != (short)NIL) {
      sysop_message(hold[m_run->ship]->names, the_dir, "run");
   }
   else {
      for (; the_times > 0; the_times--) {
         sysop_message(hold[m_run->ship]->names, the_dir, "run");
         warp_ship(m_run->ship, the_dir, 50, 3, 1);
      }
   }
}

/* **********************************************************************
   * Here is where we perform a bail-out.				*
   *									*
   * If a decoy is to be left behind, then call the routine that puts	*
   * a shuttle into space to use as a decoy.				*
   *									*
   * We jump anywhere from 10 to 23 times the maximum distance allowed	*
   * by the warp class in a random direction unless:			*
   *									*
   * We run out of power or						*
   * We run into the edge of the universe				*
   *									*
   * After we do that, we remove the ship from tyhe linked list		*
   * structure and resume the remaining tasks.				*
   *									*
   ********************************************************************** */

void perform_bail_out(void)
{
   short the_dir, the_times, wloop, stat;

   if (m_run == (struct command_options *)NULL) {
      log_error(113);
      m_run = m_first;
      return;
   }

   if (m_run->decoy == 1) {
      command_decoy();
   }

   the_dir = 5;
   the_times = 0;

   while (the_dir == 5) {
      the_dir = arandom(1L, 9L);
   }

   the_times = arandom(15L, 25L);

/*
   We know what direction and we know how many times. Warp in
   that direction unless we don't have power enough or we run into
   the edge of the universe. Use a 'run-from' priority in the
   warp ship function, (3).

   To make it faster, we now divide the number of jumps by two
   and warp the ships maximum times two. We do this easilly by
   randomizing from 5 to 11 (rather than 10 to 23), and sending
   the warp routine the enemy ships wary value times 2.
*/

   bail_out = FALSE;

   stat = warp_ship(m_run->ship,
      the_dir,
      enemy->ship_warp * 2 * the_times,
      3, 1);

   if (stat != (short)NIL) {
      bail_out = TRUE;
      sysop_message(hold[m_run->ship]->names, the_dir, "bail");
   }
   else {
      for (wloop = 0; !bail_out && wloop < the_times; wloop++) {
         sysop_message(hold[m_run->ship]->names, the_dir, "bail");
         stat = warp_ship(m_run->ship, the_dir, enemy->ship_warp * 2, 3, 1);
         if (stat == (short)NIL) {
            bail_out = TRUE;
         }
      }
   }

/*
   The ship has hopefully warped away in a single direction.
   Now take it out of the linked list.

   In version 1.41, we found that if the m_run value was the block in
   the link removed, Mayhem wouldn't change the m_run value to the
   next or the first element in the linked list. Because of this bug,
   Mayhem would 'run' the element removed from memory.
*/

   m_test = m_first;
   m_previous = (struct command_options *)NULL;
   bail_out = FALSE;

   while (! bail_out) {
      if (! m_test->next) {
         bail_out = TRUE;
      }

      if (m_test->ship == m_run->ship) {
         remove_link();
         m_run = m_first;       /* A Bug! Version 1.41 fixed this! */
         return;
      }

      m_previous = m_test;      /* A bug! This wasn't here before 1.41! */
      m_test = m_test->next;
   }
}

/* **********************************************************************
   * Warp the given ship number to the given xpostion and yposition.	*
   * Use the maximum warp possible and be sure to deduct power from	*
   * systems.  Don't check  to see if dewarp though objects.		*
   *									*
   * We use the same algorythem to determine direction and distance as	*
   * we do with the 'long' command.					*
   *									*
   * We return a -1 if the ship has already been destroyed.		*
   *									*
   * If the warp value of the ship moving into the area is greater than *
   * Y_WARP_WITHIN, we set it to _WARP_WITHIN. This is to allow the	*
   * active ship to leave the battle for awhile and the automated ship	*
   * will need to catch up. It also allows the automated ship to keep	*
   * from overshooting the active ship as was seen in initial testing.	*
   *									*
   * We want to prioritize ships movements.				*
   *									*
   * o If a ship is warping into aid a base:				*
   *   run-from should take presidence if in command file		*
   *   assist should be ignored						*
   *   defend should be ignored						*
   *   attack should be ignored						*
   *									*
   * o If a ship is NOT warping into assist the base, only one of the	*
   *   other warp reasons may take place. 				*
   *									*
   * We are going to need to know when a ship's base is under attack	*
   * and when it's not. This is done by the 'priority' flag that's set	*
   * when ever a warp function is called.				*
   *									*
   * The 'the_limit' is a divide by value which describes how fast the	*
   * ships may move. Normally, ships will be allowed to move at full	*
   * warp capacity. In an attack mode, however, it's only 1/4'th of the	*
   * ships capacity.							*
   *									*
   ********************************************************************** */

short warp_ship(short ship_number,
   short the_dir,
   short the_vel,
   short priority,
   short the_limit)
{
   unsigned short hold_index, the_remove, the_warp, stest, rtest;
   long xpos, ypos;
   long oldx, oldy;
   UC oldz;

/*
   If we are not running and
   if we are not assisting base and
   we _were_ assisting base before,
   then return and don't move this time.
*/

   if (priority != 3) {
      if (priority != 100) {
	 if (last_priority == 100) {
            return((short)NIL);
	 }
      }
   }

   last_priority = priority;
   read_enemy(ship_number);

   if (enemy->ship_warp < 1) {
      the_warp = 1;
   }
   else {
      the_warp = enemy->ship_warp / the_limit;
   }

   if (the_warp > the_vel) {
      the_warp = the_vel;
   }

   the_remove = 0;
   find_power_needed(the_warp);

   if (the_remove > enemy->ship_power) {
      return((short)NIL);
   }

   oldx = xpos = hold[ship_number]->sxpos;
   oldy = ypos = hold[ship_number]->sypos;
   oldz = zpos = hold[ship_number]->szpos;
   enemy->ship_power -= the_remove;
   hold_index = ((the_dir - 1) * 2);
   xpos += (long) the_warp * directions[hold_index];
   ypos += (long) the_warp * directions[++hold_index];

   if (xpos < 0 || xpos > xsize - 1 || ypos < 0 || ypos > ysize - 1) {
      return((short)NIL);
   }

   hold[ship_number]->sxpos = xpos;
   hold[ship_number]->sypos = ypos;
   enemy->ship_xpos = xpos;
   enemy->ship_ypos = ypos;
   write_enemy(ship_number);

/*
   See if the automated ship has a remote attached to it or if it has
   warped off of one during it's travels. If so, update the position
   of the owners record. Also update ram.
*/

   for (stest = 1; stest < players; stest++) {  /* No cops */
      if (Good_Hold(stest)) {
	 for (rtest = 0; rtest < 10; rtest++) {
	    if (hold[stest]->xremotes[rtest] == oldx &&
	       hold[stest]->yremotes[rtest] == oldy &&
                   hold[stest]->remote_universe[rtest] == oldz) {
  	       hold[stest]->xremotes[rtest] = xpos;
	       hold[stest]->yremotes[rtest] = ypos;
	       read_enemy(stest);
               enemy->rem_xpos[rtest] = xpos;
               enemy->rem_ypos[rtest] = ypos;
	       write_enemy(stest);
	    }
	 }
      }
   }

   read_enemy(ship_number);

   if (swarm_sensor_scan(xpos, ypos)) {
      swarm_attack(xpos, ypos, ship_number);
   }

   return(the_dir);
}

/* **********************************************************************
   * Compute the direction of the active ship from the automated ships  *
   * location. We could get VERY accurate within one sector of the 	*
   * active ship if we wanted to, simply by setting the constant 	*
   * X_WARP_WITHIN to the number of sectors we want to warp with to 	*
   * along the xcord and Y_WARP_WITHIN to the number of sectors to warp	*
   * along the ycord.							*
   *									*
   ********************************************************************** */

short compute_direction(short ship_number, short user_number, short x_bounds, short y_bounds)
{
   short p[4], the_dir;
   long xpos, ypos, txpos, typos;
   UC tzpos, zpos;

   txpos = hold[user_number]->sxpos;
   typos = hold[user_number]->sypos;
   tzpos = hold[user_number]->szpos;
   xpos  = hold[ship_number]->sxpos;
   ypos  = hold[ship_number]->sypos;
   zpos  = hold[ship_number]->szpos;

/*
   Make sure the ship is in the same universe!
*/

   if (txpos < 0 || typos < 0 || zpos != tzpos) {
      return((short)NIL);
   }

   p[1] = 0;

   if (txpos < (xpos - x_bounds) && p[1] == 0) {
      p[1] = 1; p[2] = 2; p[3] = 3;
   }

   if (txpos > (xpos + x_bounds) && p[1] == 0) {
      p[1] = 7; p[2] = 8; p[3] = 9;
   }

   if (p[1] == 0) {
      p[1] = 4; p[2] = 5; p[3] = 6;
   }

   if (typos < (ypos - y_bounds))
      the_dir = p[1];
   else if (typos > (ypos + y_bounds))
      the_dir = p[3];
   else the_dir = p[2];

   return(the_dir);
}

/* **********************************************************************
   * Check the data in the command file to see if the captain has 	*
   * something strange like doing anything with his own ship name in 	*
   * the file.								*
   *									*
   * Also see if a ship name occures more than once in the command file	*
   * which may confuse the operating system.				*
   *									*
   ********************************************************************** */

unsigned short examine_command_data(void)
{
   short test1, test2;

   for (test1 = 0; test1 < MAX_COMMAND; test1++) {
      if (! strcmp(attack[test1], hold[user_number]->names)) {
	 c_out(WHITE, "Attack yourself? New command file not written!\n\r");
	 return(FALSE);
      }
      if (! strcmp(defend[test1], hold[user_number]->names)) {
	 c_out(WHITE, "Defend yourself? New command file not written!\n\r");
	 return(FALSE);
      }
      if (! strcmp(assist[test1], hold[user_number]->names)) {
	 c_out(WHITE, "Assist yourself? New command file not written!\n\r");
	 return(FALSE);
      }
      if (! strcmp(run_from[test1], hold[user_number]->names)) {
	 c_out(WHITE, "Run-From yourself? New command file not written!\n\r");
	 return(FALSE);
      }
   }

/*
   That's easy. Now perform the following tasks:
      o If attacking a ship, you may not assist, defend, run-from
      o If defending a ship, you may not attack, run-from
      o If assisting a ship, you may not attack, run-from
      o If running from a ship, you may not attack, defend, assist
*/

   for (test1 = 0; test1 < MAX_COMMAND; test1++) {
      if (strlen(attack[test1]) > 0) {
	 for (test2 = 0; test2 < MAX_COMMAND; test2++) {
            if (strlen(defend[test2]) > 0) {               /* Attack/Defend */
	       if (! strcmp(attack[test1], defend[test2])) {
		  cant_do_it(0, 1);
		  return(FALSE);
	       }
	    }
            if (strlen(assist[test2]) > 0) {               /* Attack/Assist */
	       if (! strcmp(attack[test1], assist[test2])) {
		  cant_do_it(0, 2);
		  return(FALSE);
	       }
	    }
            if (strlen(run_from[test2]) > 0) {             /* Attack/Run-From */
	       if (! strcmp(attack[test1], run_from[test2])) {
		  cant_do_it(0, 3);
		  return(FALSE);
	       }
	    }
	 }
      }
   }

   for (test1 = 0; test1 < MAX_COMMAND; test1++) {
      if (strlen(defend[test1]) > 0) {
	 for (test2 = 0; test2 < MAX_COMMAND; test2++) {
            if (strlen(run_from[test2]) > 0) {             /* Defend/Run-From */
	       if (! strcmp(defend[test1], run_from[test2])) {
		  cant_do_it(1, 3);
		  return(FALSE);
	       }
	    }
         }
      }
   }

   for (test1 = 0; test1 < MAX_COMMAND; test1++) {
      if (strlen(assist[test1]) > 0) {
	 for (test2 = 0; test2 < MAX_COMMAND; test2++) {
            if (strlen(run_from[test2]) > 0) {             /* Assist/Run-From */
	       if (! strcmp(assist[test1], run_from[test2])) {
		  cant_do_it(2, 3);
		  return(FALSE);
	       }
	    }
	 }
      }
   }

/*
   One other thing we want to check is to see if we are trying to
   set up an attack on a ship we have an alliance with. This we should
   not allow and should return a FALSE.
*/

   for (test1 = 0; test1 < 5; test1++) {
      for (test2 = 0; test2 < MAX_COMMAND; test2++) {
         if (! strcmp(ships->allies[test1], attack[test2])) {

            c_out(WHITE,
               "\n\rYou have an alliance with %s and can't attack it!\n\r",
               ships->allies[test1]);

            c_out(WHITE, "Command file not written.\n\r");
	    return(FALSE);
	 }
      }
   }

   return(TRUE);
}


