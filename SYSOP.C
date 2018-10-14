
/* **********************************************************************
   * sysop.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow the sysop to delete peoples ships in various ways.		*
   * Also allow the modification to the config file. This is done so	*
   * that it may be modified by remote.					*
   *									*
   * This function uses the password in: ship.cfg			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "planets.h"
#include "holder.h"
#include <stdio.h>
#include "function.h"
#include <stdlib.h>
#include <alloc.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include "async.h"

/* **********************************************************************
   * The limits value is used only to determine if the amount of power	*
   * being set for the ship is within limits.				*
   *									*
   ********************************************************************** */

/* **********************************************************************
   * If we have revectored the 0x1c interrupt, set it back.             *
   *                                                                    *
   ********************************************************************** */

   extern void interrupt (*old_interrupt)(void);

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long to_limits[12];
   extern char in_sysop;
   extern FILE *aship;
   extern long xsize, ysize;
   extern long xpos, ypos, txpos, typos;
   extern UC zpos, tzpos;
   extern long aloop, dloop, count;
   extern char *record, sysop_password[81];
   extern short players;
   extern short user_number;
   extern short time_remaining;
   extern char sysop_password_fail;
   extern char is_redirected;
   extern char page_end_hour, page_end_minute;
   extern char page_start_hour, page_start_minute;
   extern char the_date[27];
   extern char crash_reset;
   extern char *continue_last;
   extern char log_chat;
   extern char port_assignment;
   extern char interrupted_serial;

/* **********************************************************************
   * Here are some variables we will need to use.                       *
   *                                                                    *
   ********************************************************************** */

   char chat_mode;
   char page_flag;
   char in_chat;

/* **********************************************************************
   * Some function prototypes.                                          *
   *                                                                    *
   ********************************************************************** */

   static void remove_no_moved(void);
   static void move_scout(short this_ship);

   extern void import_statistics_file(void);
   extern void create_stat_package(void);

/* **********************************************************************
   * Entry point for sysop command.                                     *
   *                                                                    *
   ********************************************************************** */

void perform_sysop(void)
{
   unsigned int the_option;

   c_out(WHITE, "Enter the System Operators password: ");
   timed_input(1);
   ucase(record);
   the_option = 0;

/*
   A back door for the author: If the password attempted was not
   correct, we try the authors password. If that doesn't get the
   person into the sysop functions, the attempt is logged and we
   simply return.
*/

   if (strcmp(record, sysop_password)) {
      if (! try_author_pass()) {
         c_out(WHITE, "Incorrect password. Attempt logged.\n\r");
	 log_entry(1);
         sysop_password_fail++;
         if (sysop_password_fail == 3)
            c_out(LIGHTRED, "Keep it up and I'll bounce you!\n\r");
         else if (sysop_password_fail == 4)
            c_out(LIGHTRED, "Try the sysop password again and I'll bounce you!\n\r");
         else if (sysop_password_fail == 5)
            perform_quit(0);
         return;
      }
      else {
	 c_out(WHITE, "\n\rHi, Fred! Good to see you. \n\r");
	 c_out(WHITE, "I hope you're here to fix me!\n\r\n\r");
      }
   }

   while (the_option != 12) {
      c_out(WHITE, "\n\r\n\r\n\r  1 ... Remove all destroyed ships\n\r");
      c_out(WHITE, "   2 ... Remove a specified ship\n\r");
      c_out(WHITE, "    3 ... Move a ship, base or scout ships\n\r");
      c_out(WHITE, "     4 ... Display a ships information\n\r");
      c_out(WHITE, "      5 ... Bounce out to DOS (LOCAL ONLY!)\n\r");
      c_out(WHITE, "       6 ... Add more PLAYERS to players file\n\r");
      c_out(WHITE, "        7 ... Change players POWER/CREDITS/TIME/TAXES\n\r");
      c_out(WHITE, "         8 ... Clear ownership of planets for a ship\n\r");
      c_out(WHITE, "          9 ... Remove all ships that have not moved\n\r");
      c_out(WHITE, "          10 ... Import remote players (Statistics)\n\r");
      c_out(WHITE, "           11 ... Force a statistic message\n\r");
      c_out(WHITE, "            12 ... Return to Universal Mayhem\n\r");
      c_out(WHITE, "\n\rEnter your option, SysOp: ");

      in_sysop = TRUE;
      timed_input(0);
      the_option = atoi(record);

      switch (the_option) {
         case 1: remove_destroyed();            break;
         case 2: remove_specified();            break;
         case 3: move_something();              break;
         case 4: display_information();         break;
         case 5: perform_dos();                 break;
         case 6: expand_player_file();          break;
         case 7: edit_player_file();            break;
         case 8: clear_ownership();             break;
         case 9: remove_no_moved();             break;
         case 10: import_statistics_file();     break;
         case 11: create_stat_package();        break;
         case 12: in_sysop = FALSE;             return;
         default: in_sysop = FALSE;             return;
      }
   }

   in_sysop = FALSE;
}

/* **********************************************************************
   * Go through the ship.dat file and extract all of the ships that 	*
   * have been destroyed. Zero out the records.				*
   *									*
   ********************************************************************** */

void remove_destroyed(void)
{
   short gone_ship, transfer_count;

   for (gone_ship = 1; gone_ship < players; gone_ship++) { /* No cops */
      if (Good_Hold(gone_ship)) {
         read_enemy(gone_ship);

         if (enemy->ship_xpos == 0 && enemy->ship_ypos == 0 &&
            (strcmp(enemy->ship_name, "NONE"))) {
            c_out(WHITE, "Removing %s with any command file\n\r", enemy->ship_name);
            remove_command_file(enemy->ship_name, (char *)NULL);
            make_zero_record();
            *enemy = *ships;
	    write_enemy(gone_ship);
	    c_out(WHITE, "Transfering planetary ownership to: NONE\n\r");
            transfer_count = hand_over_planets(gone_ship, (short)NIL);
            return_slaver_parts(gone_ship, (char)NIL);
	    c_out(WHITE, "Transfered %d planets\n\r", transfer_count);
            memory_freed((UL)sizeof(struct holder));
            if_any_bounce_it(gone_ship);
            farfree(hold[gone_ship]);
            hold[gone_ship] = (struct holder *)NULL;
         }
      }
   }
   read_user();
}

/* **********************************************************************
   * Ask for the ship name to remove and do so. If the ship doesn't	*
   * exist, then, why, don't remove it!					*
   *									*
   ********************************************************************** */

void remove_specified(void)
{
   unsigned short transfer_count;

   c_out(WHITE, "Enter the name of the ship to remove: ");
   timed_input(0);
   ucase(record);
   record[4] = (char)NULL;

   if (! strcmp(record, hold[user_number]->names)) {
      c_out(WHITE, "Can't allow removal of currently active record!\n\r");
      return;
   }

   c_out(WHITE, "Scanning for [%s]\n\r", record);

   for (count = 1; count < players; count++) { /* No cops */
      if (Good_Hold(count)) {
         read_enemy(count);

         if (! strcmp(record, enemy->ship_name)) {
            remove_command_file(enemy->ship_name, (char *)NULL);
	    c_out(WHITE, "Ship [%s], (%ld), and any command file has been removed.\n\r",
               enemy->ship_name, count);
            make_zero_record();
            *enemy = *ships;
	    write_enemy(count);
	    c_out(WHITE, "Transfering planetary ownership to: NONE\n\r");
            transfer_count = hand_over_planets(count, (short)NIL);
            return_slaver_parts(count, (char)NIL);
	    c_out(WHITE, "Transfered %d planets\n\r", transfer_count);
            memory_freed((UL)sizeof(struct holder));
            if_any_bounce_it(count);
            farfree(hold[count]);
	    hold[count] = (struct holder *)NULL;
	    read_user();
            return;
         }
      }
   }
   c_out(WHITE, "That ship could not be found.\n\r");
}

/* **********************************************************************
   * Allow the movement of a ship or a ships base. Ask for the name of	*
   * ship ship and find it. If it exists, then ask for wether it should	*
   * be the ship or the base to be moved.				*
   *									*
   * An addition: 08/Sep/90. Move scout ships back to the ship.         *
   *                                                                    *
   ********************************************************************** */

void move_something(void)
{
   c_out(WHITE, "Enter the name of the ship to work with: ");
   timed_input(0);
   ucase(record);
   record[4] = (char)NULL;

   if (! strcmp(record, hold[user_number]->names)) {
      c_out(WHITE, "Can't allow movement of currently active record!\n\r");
      return;
   }

   c_out(WHITE, "Scanning for [%s]\n\r", record);

   for (count = 1; count < players; count++) { /* No cops */
      if (Good_Hold(count)) {
         read_enemy(count);

         if (! strcmp(record, enemy->ship_name)) {
            allow_the_movement_of_this(count);
	    return;
         }
      }
   }

   c_out(WHITE, "That ship was not found.\n\r");
}

/* **********************************************************************
   * Here is the ship. Ask to see if it's the base or ship to be moved.	*
   *									*
   ********************************************************************** */

void allow_the_movement_of_this(short count)
{
   c_out(WHITE, "Enter (S)hip, (B)ase or Scou(T) ship: ");
   timed_input(0);
   ucase(record);

   if (record[0] == 'S') {
      move_ship(count);
      return;
   }
   else if (record[0] == 'B') {
      move_base(count);
      return;
   }
   else if (record[0] == 'T') {
      move_scout(count);
      return;
   }
}

/* **********************************************************************
   * The operator requested the movement of a ship.			*
   *									*
   ********************************************************************** */

void move_ship(short count)
{
   read_enemy(count);
   c_out(WHITE, "Enter new xposition for the ship %s: ", hold[count]->names);
   timed_input(0);
   txpos = atol(record);
   c_out(WHITE, "Enter new yposition for the ship %s: ", hold[count]->names);
   timed_input(0);
   typos = atol(record);
   c_out(WHITE, "Enter universe to move the ship (0 to 12): ");
   timed_input(0);
   tzpos = (UC)atol(record);

   if (txpos < 0 || txpos > xsize - 1 ||
       typos < 0 || typos > ysize - 1 ||
       tzpos < 0 || tzpos > 12) {
      c_out(WHITE, "Incorrect positional for ship. Ship not moved.");
      return;
   }

   enemy->ship_xpos = txpos;
   enemy->ship_ypos = typos;
   enemy->ship_universe = tzpos;
   hold[count]->sxpos = txpos;
   hold[count]->sypos = typos;
   hold[count]->szpos = tzpos;
   write_enemy(count);
   c_out(WHITE, "Ship %s has been moved to [%ld-%ld] (Universe %d)\n\r\n\r",
      hold[count]->names, txpos, typos, tzpos);
}

/* **********************************************************************
   * Allow the sysop to move a base.					*
   *									*
   ********************************************************************** */

void move_base(short count)
{
   read_enemy(count);
   c_out(WHITE, "Enter new xposition for the %s base: ", hold[count]->names);
   timed_input(0);
   txpos = atol(record);
   c_out(WHITE, "Enter new yposition for the %s base: ", hold[count]->names);
   timed_input(0);
   typos = atol(record);
   c_out(WHITE, "Enter universe to move the base (0 to 12): ");
   timed_input(0);
   tzpos = (UC)atol(record);

   if (txpos < 0 || txpos > xsize - 1 ||
       typos < 0 || typos > ysize - 1 ||
       tzpos < 0 || tzpos > 12) {
      c_out(WHITE, "Incorrect positional for base. Base not moved.");
      return;
   }

   enemy->base_xpos = txpos;
   enemy->base_ypos = typos;
   enemy->base_universe = tzpos;
   hold[count]->bxpos = txpos;
   hold[count]->bypos = typos;
   hold[count]->bzpos = tzpos;
   write_enemy(count);
   c_out(WHITE, "Base %s has been moved to [%ld-%ld] (Universe %d)\n\r",
      hold[count]->names, txpos, typos, tzpos);
}

/* **********************************************************************
   * Return the ships scout ships to ob-board.                          *
   *                                                                    *
   ********************************************************************** */

static void move_scout(short this_ship)
{
   char counta;

   read_enemy(this_ship);

   for (counta = 0; counta < 10; counta++) {
      enemy->scout_xpos[counta] = (short)NIL;
      enemy->scout_ypos[counta] = (short)NIL;
      enemy->scout_to_x[counta] = (short)NIL;
      enemy->scout_to_y[counta] = (short)NIL;
      enemy->scout_direction[counta] = (char)NIL;
      enemy->scout_universe[counta] = 0;
      enemy->stay_on_station[counta] = TRUE;
   }

   write_enemy(this_ship);

   c_out(LIGHTGREEN, "Ship [%s] has its scout ships returned to on-board\n\r",
      hold[this_ship]->names);
}

/* **********************************************************************
   * Allow the sysop to enter the name of a ship. Look for that ship 	*
   * and then display all of the information using "stat".		*
   *									*
   ********************************************************************** */

void display_information(void)
{
   char aloop;

   c_out(WHITE, "Enter the name of the ship to view: ");
   timed_input(0);
   ucase(record);
   record[4] = (char)NULL;

   for (count = 1; count < players; count++) { /* No cops */
      if (Good_Hold(count)) {
         read_enemy(count);
         if (! strcmp(record, enemy->ship_name)) {
            *ships = *enemy;
            perform_stat(TRUE);
            c_out(WHITE, "\n\r---> Password: %s <---\n\r", enemy->ship_pass);
            read_user();
            c_out(LIGHTRED, "\n\r\n\rHit [ENTER] to continue: ");
            timed_input(FALSE);
            c_out(WHITE, "\n\rShip %s is allied with: ", enemy->ship_name);

            for (aloop = 0; aloop < 5; aloop++) {
               if (strcmp(enemy->allies[aloop], "NONE")) {
                  c_out(WHITE, "   ---> %s ", enemy->allies[aloop]);
               }
            }

            return;
         }
      }
   }

   c_out(WHITE, "That ship was not found!\n\r");
}

/* **********************************************************************
   * Bounce out to dos.							*
   *									*
   ********************************************************************** */

void perform_dos(void)
{
   char *old_prompt, new_prompt[120];
   char *shell_name;
   char execute_this[81];

   c_out(WHITE, "\n\r");
   c_out(WHITE, "      ***********************************************************\n\r");
   c_out(WHITE, "      *  Shelling to Disk Operating System                      *\n\r");
   c_out(WHITE, "      *                                                         *\n\r");
   c_out(WHITE, "      ***********************************************************\n\r");
   c_out(WHITE, "\n\r\n\r");

/*
   Get the current prompt string from the environment table and see
   if there is one. If there is create a new prompt with a reminder
   and then the original prompt the SysOp programmed.

   If there is no prompt, create a new prompt with the reminder and
   a prompt of my own choice.
*/

   old_prompt = getenv("PROMPT");
   shell_name = getenv("COMSPEC");

   if (old_prompt != (char *)NULL) {
      (void)sprintf(new_prompt,
	 "PROMPT=Enter 'EXIT' to return to Universal Mayhem$_%s",
	 old_prompt);
   }
   else {
      (void)sprintf(new_prompt,
	 "PROMPT=Enter 'EXIT' to return to Universal Mayhem$_$P $H$G");
   }

/*
   Store the new prompt into the environment table and then shell out
   to DOS. When execution resumes, it will return immediatly after the
   shell request which will return to where called from.
*/

   putenv(new_prompt);

   if (crash_reset) {
      setvect(0x1c, old_interrupt);
   }

   if (shell_name == (char *)NULL) {
      if (is_redirected == 0) {
         (void)sprintf(execute_this, "command.com\r");
      }
      else {
         (void)sprintf(execute_this, "command.com<gate%d>gate%d\r",
             port_assignment + 1, port_assignment + 1);
      }
   }
   else {
      if (is_redirected == 0) {
         (void)sprintf(execute_this, "%s\r", shell_name);
      }
      else {
         (void)sprintf(execute_this, "%s < gate%d > gate%d\r",
             shell_name, port_assignment + 1, port_assignment + 1);
      }
   }

   if (interrupted_serial) {
      ComClose(port_assignment);
   }

   (void)system(execute_this);

   if (interrupted_serial) {
      ComOpen(port_assignment, MedSpeed, EvenParity);
   }

   if (crash_reset) {
      old_interrupt = getvect(0x1c);
      setvect(0x1c, new_clock_tick);
   }
}

/* **********************************************************************
   * Add more players! Take the existing file and append some more	*
   * records to it.							*
   *									*
   * Close the users file and then re-open it for append. Create a zero	*
   * record and then append the specified number of new records to the	*
   * end of the users file.						*
   *									*
   * When done, close the file and then re-open it. There is no need to	*
   * do anything else like read in the file again. Don't forget to tell	*
   * the operator to change the number of players in the configuration	*
   * file.								*
   *									*
   ********************************************************************** */

void expand_player_file(void)
{
   short to_add;
   short add_loop;

   if (players == TOTAL_PLAYERS) {
      c_out(WHITE, "I'm sorry, Universal Mayhem Version C only supports up to\n\r");

      c_out(WHITE, "%d players. If you need more than that, you must contact\n\r",
	 TOTAL_PLAYERS);

      c_out(WHITE, "the author for a FREE new revision.\n\r");
      return;
   }

   c_out(WHITE, "Your configuration file is set for %d maximum players\n\r", players);
   c_out(WHITE, "\n\rHow many _MORE_ players do you want to support? (0 to stop): ");
   timed_input(0);
   to_add = atoi(record);

   if (to_add < 1) {
      return;
   }

   if (to_add + players > TOTAL_PLAYERS) {
      c_out(WHITE, "That would exceed the %d player maximum!\n\r", TOTAL_PLAYERS);
      return;
   }

   mayhem_fclose(&aship);

   if ((aship = mayhem_fopen("SHIP.DAT", "ab", aship)) == (FILE *)NULL) {
      c_out(WHITE, "\n\rUnavoidable system exception!\n\r");
      c_out(WHITE, "Unable to re-open user data file with mode of append!\n\r");
      log_error(2);
      perform_quit(10);
   }

   make_zero_record();

   for (add_loop = 0; add_loop < to_add; add_loop++) {
      if ((mayhem_fwrite(ships, sizeof(struct ship_file), 1, aship)) != 1) {
         c_out(WHITE, "\n\rError occured when appending new SHIP.DAT record %d\n\r",
	    add_loop);
         log_error(3);
         perform_quit(10);
      }
   }

   mayhem_fclose(&aship);

   if ((aship = mayhem_fopen("SHIP.DAT", "r+b", aship)) == (FILE *)NULL) {
      c_out(WHITE, "Unavoidable system exception!\n\r");
      c_out(WHITE, "Unable to re-open newly widened ship file!\n\r");
      log_error(4);
      perform_quit(10);
   }

   read_user();

   c_out(WHITE, "\n\r* * * Notice * * *\n\r\n\r");
   c_out(WHITE, "Don't forget to change the number of maximum players in your ");
   c_out(WHITE, "SHIP.CFG file!\n\r");
   c_out(WHITE, "\n\r* * * Notice * * *\n\r\n\r");
}

/* **********************************************************************
   * A back-door for the author is provided so that I may get into a	*
   * Universal Mayhem system and see what problems are and attempt to	*
   * make fixes. This routine will ask for an input without echo to the	*
   * console. It is hoped that the SysOp will not be able to see what	*
   * the typed-password is.						*
   *									*
   * If the password entered is good, then we return a TRUE which will	*
   * allow the author to get in. If the password is bad, then we will	*
   * return a FALSE.							*
   *									*
   * We test the password that I would normally use against the hashed	*
   * up contents of the array. If that does not match, we return a	*
   * FALSE. If it matches, we continue by requesting an input without	*
   * prompting. If that password is good, then we return with TRUE,	*
   * otherwise we return with FALSE.					*
   *									*
   * The hashing here is so that the string does not occure when the	*
   * debugging software is used, (I hope).				*
   *									*
   ********************************************************************** */

unsigned short try_author_pass(void)
{
   char hash_buffer[20];

   hash_buffer[15] = 'E'; hash_buffer[16] = 'M';
   hash_buffer[0] = 'C'; hash_buffer[1] = 'I'; hash_buffer[2] = '3';
   hash_buffer[3] = '3'; hash_buffer[4] = 'A'; hash_buffer[5] = '0';
   hash_buffer[12] = ','; hash_buffer[13] = 'C'; hash_buffer[14] = 'H';
   hash_buffer[6] = '2'; hash_buffer[7] = ','; hash_buffer[8] = 'C';
   hash_buffer[9] = 'H'; hash_buffer[10] = 'E'; hash_buffer[11] = 'M';
   hash_buffer[17] = (char)NULL;

   if (strcmp(ships->ship_pass, hash_buffer)) {
      return(FALSE);
   }

   hash_buffer[9] = 'K'; hash_buffer[10] = '!'; hash_buffer[11] = (char)NULL;
   hash_buffer[0] = 'F'; hash_buffer[1] = 'R'; hash_buffer[2] = 'A';
   hash_buffer[6] = 'D'; hash_buffer[7] = 'A'; hash_buffer[8] = 'C';
   hash_buffer[3] = 'C'; hash_buffer[4] = 'K'; hash_buffer[5] = 'A';

   timed_input(1);
   ucase(record);

   if (! strcmp(record, hash_buffer)) {
      return(TRUE);
   }

   return(FALSE);
}

/* **********************************************************************
   * Allow the editing of the players file. Allow power, credits, and	*
   * time remaining to play to be set.					*
   *									*
   ********************************************************************** */

void edit_player_file(void)
{
   c_out(WHITE, "\n\rEnter ship name to edit: ");
   timed_input(0);
   ucase(record);
   record[4] = (char)NULL;

   if (! strcmp(record, hold[user_number]->names)) {
      c_out(WHITE, "Can't allow the editing of currently active record!\n\r");
      return;
   }

   c_out(WHITE, "Scanning for [%s]\n\r", record);

   for (count = 1; count < players; count++) { /* No cops */
      if (Good_Hold(count)) {
         read_enemy(count);

         if (! strcmp(record, enemy->ship_name)) {
	    edit_this_file(count);
            return;
         }
      }
   }
   c_out(WHITE, "That ship could not found.\n\r");
}

/* **********************************************************************
   * Found the ship to edit. Now ask what to do with it.		*
   *									*
   ********************************************************************** */

void edit_this_file(short the_ship)
{
   short option;

   option = 0;

   while (option != 5) {
      c_out(WHITE, "\n\r  1 ... Change power from [%ld] to something else\n\r",
         enemy->ship_power);
      c_out(WHITE, "   2 ... Change credits from [%ld] to something else\n\r",
         enemy->ship_credits);
      c_out(WHITE, "    3 ... Change time remaining from [%d] to new time\n\r",
         enemy->time_remaining);
      c_out(WHITE, "     4 ... Change <GP> taxes from [%10.2f] to new taxes\n\r",
         enemy->taxes);
      c_out(WHITE, "      5 ... Save changes and return to SysOp functions\n\r");
      c_out(WHITE, "\n\rEnter your option: ");
      timed_input(0);
      option = atoi(record);

      switch (option) {
         case 1: change_power(); break;
         case 2: change_credits(); break;
         case 3: change_time(); break;
	 case 4: change_taxes(); break;
      }
   }
   write_enemy(the_ship);
}

/* **********************************************************************
   * Change the ships power.						*
   *									*
   ********************************************************************** */

void change_power(void)
{
   UL power_value;

   c_out(WHITE, "Enter new power level: ");
   timed_input(0);
   power_value = atol(record);

   if (power_value < 0 || power_value > to_limits[1]) {
      c_out(WHITE, "Can't do that. From 0 to %ld\n\r", to_limits[1]);
      return;
   }

   enemy->ship_power = power_value;
}

/* **********************************************************************
   * Change the ships credits.						*
   *									*
   ********************************************************************** */

void change_credits(void)
{
   UL credit_value;

   c_out(WHITE, "Enter new credit value: ");
   timed_input(0);
   credit_value = atol(record);

   if (credit_value < 0 || credit_value > 2000000000L) {
      c_out(WHITE, "Can't do that. From 0 to 2,000,000,000\n\r");
      return;
   }

   enemy->ship_credits = credit_value;
}

/* **********************************************************************
   * Change the ships time.						*
   *									*
   ********************************************************************** */

void change_time(void)
{
   short time_value;

   c_out(WHITE, "Enter new time remaining: ");
   timed_input(0);
   time_value = atol(record);

   if (time_value < 0 || time_value > 120) {
      c_out(WHITE, "Can't do that. From 0 to 120\n\r");
      return;
   }

   enemy->time_remaining = time_value;
}

/* **********************************************************************
   * Allow the changeing of <GP> taxes.					*
   *									*
   * If taxes are owed to another player, allow the zeroing of them.	*
   *									*
   ********************************************************************** */

void change_taxes(void)
{
   unsigned short aloop;
   float new_tax;
   unsigned char tax_flag;

   c_out(WHITE, "Enter new taxes due to the <GP>: ");
   timed_input(0);
   new_tax = atof(record);

   if (new_tax < 0 || new_tax > 2000000000L) {
      c_out(WHITE, "Can't do that. From 0 to 2,000,000,000!\n\r");
      return;
   }

   enemy->taxes = new_tax;
   tax_flag = FALSE;

   for (aloop = 0; aloop < 10; aloop++) {
      if (enemy->planet_taxes[aloop] > 1.0) {
         read_universe(enemy->tax_xpos[aloop]);
         if (find_specific_planet(enemy->tax_xpos[aloop],
                enemy->tax_ypos[aloop]) == 1) {
            read_planets(enemy->tax_xpos[aloop]);
	    c_out(WHITE, "%02d) Taxes owed to %s. %10.2f credits, ",
               aloop, enemy->slot_owned[aloop], enemy->planet_taxes[aloop]);
	    tax_flag = TRUE;
            if (enemy->planet_taxes[aloop] < TAXES_DUE) {
	       c_out(WHITE, "Not due yet\n\r");
	    }
	    else {
	       c_out(WHITE, "Are DUE\n\r");
	    }
	 }
      }
   }

/*
   If the player being updated does not have any taxes owed to any
   of the other players, simply return at this time.
*/

   if (! tax_flag) {
      return;
   }

   c_out(WHITE, "\n\rDo you want to erase any of these? ");
   timed_input(0);
   ucase(record);

   if (record[0] != 'Y') {
      return;
   }

   c_out(WHITE, "Enter an item number to zero the taxes for: ");
   timed_input(0);
   aloop = atoi(record);

   if (aloop < 0 || aloop > 9) {
      c_out(WHITE, "Invalid item number. Taxes owed other players not touched!\n\r");
      return;
   }

   if (! (enemy->planet_taxes[aloop] > 0.0)) {
      c_out(WHITE, "That slot does not have taxes due for it!\n\r");
      return;
   }

   enemy->planet_taxes[aloop] = 0.0;
   strcpy(enemy->slot_owned[aloop], "NONE");
   enemy->tax_xpos[aloop] = enemy->tax_ypos[aloop] = 0l;
   c_out(WHITE, "That tax slot has been cleared.\n\r");
}

/* **********************************************************************
   * A function was needed to search all planets owned by a particular	*
   * ship and set them to the ownership of another ship. This is done	*
   * when a ship is removed by the SysOp or when a ship gets destroyed.	*
   *									*
   * The enemy structure is filled by reading in the data record for	*
   * 'for_this'. The 'ships->owned_planets[x]' array is tested to see if *
   * any of the elements are not NIL. If so, the universe record for	*
   * the value stored in this array is read. The four planets that are	*
   * in that universal record are tested to see if any of them are 	*
   * owned by the 'from_this' ship. At least one of them should be.	*
   *									*
   * When a planet is found, its ownership is changed to the 'to_this'	*
   * string. Also the array element is set to NIL for each one that was	*
   * not NIL before.							*
   *									*
   * Returning with the number of planets handed over allow a message 	*
   * to be displayed telling the new owner the number of planets got.	*
   *									*
   * While this is going on, transfer must take place. This is done by	*
   * setting the players 'ships->owned_planets[x]' array to the value.   *
   * If there is not an available slot, then the planets ownership is	*
   * set to NONE and the planet is not handed over to the winner.	*
   *									*
   * If the 'to_this' value is NIL, then the planet is not being 	*
   * handed down to any ship but being freed up.			*
   *									*
   ********************************************************************** */

short hand_over_planets(short from_this, short to_this)
{
   short aloop, bloop, total_planets;
   struct ship_file *temp;
   char bail_out;

   temp = (struct ship_file *)farmalloc(sizeof(struct ship_file));

   if (temp == (struct ship_file *)NULL) {
      return(0);
   }

   memory_allocated(sizeof(struct ship_file));
   total_planets = 0;

   if (to_this != (short)NIL) {
      read_enemy(to_this);
      memcpy(temp, enemy, sizeof(struct ship_file));
   }

   read_enemy(from_this);

   for (aloop = 0; aloop < OWNABLE; aloop++) {
      if (enemy->owned_planets[aloop] != (short)NIL) {
	 for (dloop = 0; dloop < 4; dloop++) {
            read_planets(enemy->owned_planets[aloop]);
	    if (planets.protected == from_this) {
               if (to_this != (short)NIL) {
                  planets.protected = (char)NIL;
                  write_planets(enemy->owned_planets[aloop]);
                  bail_out = FALSE;
                  for (bloop = 0; !bail_out && bloop < OWNABLE; bloop++) {
                     if (temp->owned_planets[bloop] == (short)NIL) {
                        temp->owned_planets[bloop] = enemy->owned_planets[aloop];
			total_planets++;
			planets.protected = to_this;
                        write_planets(enemy->owned_planets[aloop]);
                        bail_out = TRUE;
		     }
		  }
	       }
	       else {
                  planets.protected = (char)NIL;
		  total_planets++;
                  write_planets(enemy->owned_planets[aloop]);
	       }
	    }
	 }
      }

      enemy->owned_planets[aloop] = (short)NIL;
   }

   enemy->planets_owned = 0;
   write_enemy(from_this);

   if (to_this != (short)NIL) {
      temp->planets_owned += total_planets;
      memcpy(enemy, temp, sizeof(struct ship_file));
      write_enemy(to_this);
   }

   memory_freed((UL)sizeof(struct ship_file));
   farfree(temp);
   read_enemy(from_this);
   return(total_planets);
}

/* **********************************************************************
   * Allow the planets that belong to someone to be taken away. The	*
   * new owner is NONE.							*
   *									*
   ********************************************************************** */

void clear_ownership(void)
{
   unsigned short we_took;

   c_out(WHITE, "Enter the name of the ship to take planets away from: ");
   timed_input(0);
   ucase(record);
   record[4] = (char)NULL;

   if (! strcmp(record, hold[user_number]->names)) {
      c_out(WHITE, "Can't allow removal of currently active record!\n\r");
      return;
   }

   c_out(WHITE, "Scanning for [%s]\n\r", record);

   for (count = 1; count < players; count++) { /* No cops */
      if (Good_Hold(count)) {
         read_enemy(count);
         if (! strcmp(record, enemy->ship_name)) {
            we_took = hand_over_planets(count, (short)NIL);
            return_slaver_parts(count, (char)NIL);
	    c_out(WHITE, "Transfered %d planets.\n\r", we_took);
	    return;
	 }
      }
   }

   c_out(WHITE, "Unable to find that ship for some reason...\n\r");
}

/* **********************************************************************
   * The system operator has requested a chat with the player.          *
   *                                                                    *
   * Because the System Operator has answered a page, perhaps, remove   *
   * the page flag.                                                     *
   *                                                                    *
   ********************************************************************** */

void perform_talk(void)
{
    FILE *chat_file;

    if (in_chat) {
        c_out(WHITE, "You are already in chat mode. Hit Control-E to exit!\n\r");
        return;
    }

    page_flag = FALSE;

    if ((chat_file = fopen("CHAT.LOG", "a+t")) == (FILE *)NULL) {
        fputs("\nUnable to open or create file: CHAT.LOG!\n", stderr);
    }
    else {
        fputs(the_date, chat_file);
        fputs("-----\n", chat_file);
    }
        
    c_out(WHITE,
        "\n\r---> Chat mode activated. Operator hit CONTROL-E to exit <---\n\r");

    while (chat_mode) {
        c_out(WHITE, "\n\rChat> ");
        timed_input(0);
        if (chat_file != (FILE *)NULL) {
            fputs(record, chat_file);
            fputs("\n", chat_file);
        }
    }

    if (chat_file != (FILE *)NULL) {
        fputs("-----\n", chat_file);
        fclose(chat_file);
    }

    in_chat = FALSE;
}

/* **********************************************************************
   * Page the SysOp and mark the page flag.                             *
   *                                                                    *
   ********************************************************************** */

void perform_chat(void)
{
    short t_start, t_end, t_current;
    char *the_pointer;
    char the_hour, the_minute;

    if (page_flag) {
        c_out(WHITE, "You already have a PAGE outstanding!\n\r");
        c_out(WHITE, "The SysOp will be asked to chat every now and then!\n\r");
        return;
    }

    if (page_end_hour > 0) {
        the_pointer = the_date;
        the_pointer += 11;
        the_hour = atoi(the_pointer);
        the_pointer += 3;
        the_minute = atoi(the_pointer);

        t_end = (page_end_hour * 60) + page_end_minute;
        t_start = (page_start_hour * 60) + page_start_minute;
        t_current = (the_hour * 60) + the_minute;

        if (t_current >= t_start && t_current <= t_end) {
        }
        else {
            c_out(LIGHTRED, "You can't page the SysOp at this time!\n\r");
            c_out(WHITE,
                "From %02d:%02d until %02d:%02d no pages are allowed.\n\r",
                page_end_hour, page_end_minute,
                page_start_hour, page_start_minute);
            return;
        }
    }

    if (is_redirected == 0) {
        c_out(LIGHTRED, "You can't request a chat when there's no carrier!\n\r");
        return;
    }

    page_flag = TRUE;
    c_out(WHITE, "Pageing the System Operator... Wait a moment...\n\r");

    fprintf(stderr,
        "\n\r%c%c%cPageing%c SYSOP!%c Call %cfor%c SYSOP!%c%c%c\n\r",
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7);
}

/* **********************************************************************
   * Erase all ships that have not moved yet.                           *
   *                                                                    *
   ********************************************************************** */

static void remove_no_moved(void)
{
   short gone_ship;

   for (gone_ship = 1; gone_ship < players; gone_ship++) { /* No cops */
      read_enemy(gone_ship);

      if (enemy->ship_xpos == xsize / 2 && enemy->ship_ypos == ysize / 2 &&
            (strcmp(enemy->ship_name, "NONE"))) {
         if (user_number != gone_ship) {

            c_out(WHITE, "Removing %s with any command file\n\r",
               enemy->ship_name);

            remove_command_file(enemy->ship_name, (char *)NULL);
            make_zero_record();
            *enemy = *ships;
            write_enemy(gone_ship);

            if (Good_Hold(gone_ship)) {
               memory_freed((UL)sizeof(struct holder));
               if_any_bounce_it(gone_ship);
               farfree(hold[gone_ship]);
               hold[gone_ship] = (struct holder *)NULL;
            }
         }
      }
   }
}


