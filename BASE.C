
/* **********************************************************************
   * base.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdlib.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long to_limits[12];
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern UC zpos;
   extern unsigned short docked, base_docked;
   extern char *record;
   extern short user_number;

/* **********************************************************************
   * Either create a base where we are or transfer items to and from	*
   * the base.								*
   *									*
   ********************************************************************** */

void perform_base(void)
{
   short int the_option;

   read_user();
   the_option = 0;

   if (xpos == xsize / 2 && ypos == ysize / 2) {
      c_out(LIGHTRED, "You can't build your base here!\n\r");
      return;
   }

   if (docked == 1) {
      c_out(LIGHTRED, "You can't build a base on a planet!\n\r");
      return;
   }

   if (ships->base_xpos == 0 && ships->base_ypos == 0) {
      ships->base_xpos = xpos;
      ships->base_ypos = ypos;
      ships->base_universe = zpos;
      write_user();
      c_out(WHITE,
	 "\n\rBuilt a base at [%ld-%ld] {Universe %d}.\n\r", xpos, ypos, zpos);
      hold[user_number]->bxpos = xpos;
      hold[user_number]->bypos = ypos;
      hold[user_number]->bzpos = zpos;
      strcpy(ships->base_death, "NONE");
      base_docked = 1;
   }

   if (base_docked == 0) {
      c_out(LIGHTRED, "You are not docked with your base!\n\r");
      return;
   }

/*
   If the base is disassembled, then we want to return to the command
   entry prompt, otherwise, things can be done to a base that does not
   exist any longer!
*/

   while (the_option != 4 && the_option != 3) {
      c_out(WHITE, "\n\r1 ... Move items from ship to base\n\r");
      c_out(WHITE, "2 ... Move items from base to ship\n\r");
      c_out(WHITE, "3 ... Disassemble the base\n\r");
      c_out(WHITE, "4 ... Leave base and enter ship.\n\r");

      if (ships->base_hit_count > 0) {
	 c_out(WHITE, "5 ... Repair damaged base.\n\r");
      }

      c_out(WHITE, "Enter your option: ");
      timed_input(0);
      the_option = atoi(record);

      switch (the_option) {
         case 1:
	    to_base();
            break;
         case 2:
	    to_ship();
            break;
         case 3:
	    disassemble_base();
	    break;
         case 4:
	    return;
	 case 5:
            repair_base();
            break;
      }
   }

}

/* **********************************************************************
   * Here is where we move things from the ship to the base. 		*
   *									*
   ********************************************************************** */

void to_base(void)
{
   unsigned int the_option;

   if (ships->base_hit_count > 0) {
      c_out(LIGHTRED, "Your base has been damaged. Repair before loading.\n\r");
      return;
   }

   the_option = 9999;

   while (the_option != 6) {
      c_out(WHITE, "1 ... Ship shield  [%10ld]\n\r", ships->ship_shield);
      c_out(WHITE, "2 ... Ship credits [%10ld]\n\r", ships->ship_credits);
      c_out(WHITE, "3 ... Ship cargo   [%10d]\n\r", ships->ship_cargo);
      c_out(WHITE, "4 ... Ship crew    [%10d]\n\r", ships->ship_crew);
      c_out(WHITE, "5 ... Ship cloak   [%10d]\n\r", ships->ship_cloak);
      c_out(WHITE, "6 ... Return to Universal Mayhem\n\r");
      c_out(WHITE, "ALL - Move half of everything over to base\n\r");
      c_out(WHITE, "\n\rEnter your selection: ");
      timed_input(0);
      ucase(record);
      the_option = atoi(record);

      if (! strncmp(record, "ALL", 3)) {
         half_ship();
      }
      else {
         switch (the_option) {
            case 0:
            case 6: return;
            case 1: ship_shield();
                    write_user();
                    break;
            case 2: ship_credits();
                    write_user();
                    break;
            case 3: ship_cargo();
                    write_user();
                    break;
            case 4: ship_crew();
                    write_user();
                    break;
            case 5: ship_cloak();
                    write_user();
                    break;
         }
      }
   }
}

/* **********************************************************************
   * Here is where we move things from the base to the ship.		*
   *									*
   ********************************************************************** */

void to_ship(void)
{
   unsigned int the_option;

   if (ships->base_hit_count > 0) {
      c_out(LIGHTRED, "Your base has been damaged. Repair before offloading.\n\r");
      return;
   }

   the_option = 9999;

   while (the_option != 6) {
      c_out(WHITE, "1 ... Base shield  [%10ld]\n\r", ships->base_shield);
      c_out(WHITE, "2 ... Base credits [%10ld]\n\r", ships->base_credits);
      c_out(WHITE, "3 ... Base cargo   [%10ld]\n\r", ships->base_cargo);
      c_out(WHITE, "4 ... Base crew    [%10ld]\n\r", ships->base_crew);
      c_out(WHITE, "5 ... Base cloak   [%10d]\n\r", ships->base_cloak);
      c_out(WHITE, "ALL - Move half of everything over to ship\n\r");
      c_out(WHITE, "6 ... Return to Universal Mayhem!\n\r");
      c_out(WHITE, "\n\rEnter your request: ");
      timed_input(0);
      ucase(record);
      the_option = atoi(record);

      if (! strncmp(record, "ALL", 3)) {
         half_base();
      }
      else {
         switch (the_option) {
            case 0: return;
            case 1: base_shield();
                    write_user();
                    break;
            case 2: base_credits();
                    write_user();
                    break;
            case 3: base_cargo();
                    write_user();
                    break;
            case 4: base_crew();
                    write_user();
                    break;
            case 5: base_cloak();
                    write_user();
                    break;
         }
      }
   }
}

/* **********************************************************************
   * General routine to ask about the number of units to move.		*
   *									*
   ********************************************************************** */

unsigned long ask_move_value(void)
{
   c_out(WHITE, "Enter the number of units to move: ");
   timed_input(0);
   return((long)atol(record) );
}

/* **********************************************************************
   * Allow building of base shield.					*
   *									*
   ********************************************************************** */

void ship_shield(void)
{
   long int move_value;

   if ((move_value = ask_move_value()) > ships->ship_shield) {
      c_out(WHITE, "You don't have that much shield power in your ship!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->base_shield + move_value > to_limits[1]) {
      c_out(WHITE, "You base can not hold that much power!\n\r");
      return;
   }

   ships->ship_shield -= move_value;
   ships->base_shield += move_value;
   ships->base_hit_count = 0;
}

/* **********************************************************************
   * Allow the offloading of power from base to ship.			*
   *									*
   ********************************************************************** */

void base_shield(void)
{
   long int move_value;

   if ((move_value = ask_move_value()) > ships->base_shield) {
      c_out(WHITE, "You don't have that much shield power in your base!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->ship_shield + move_value > to_limits[1]) {
      c_out(WHITE, "You ship can not hold that much power!\n\r");
      return;
   }

   ships->base_shield -= move_value;
   ships->ship_shield += move_value;
}

/* **********************************************************************
   * Move credits from ship to base.					*
   *									*
   ********************************************************************** */

void ship_credits(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->ship_credits) {
      c_out(WHITE, "You don't have that many credits in your ship!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   ships->ship_credits -= move_value;
   ships->base_credits += move_value;
}

/* **********************************************************************
   * Move credits from base to ship.					*
   *									*
   ********************************************************************** */

void base_credits(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->base_credits) {
      c_out(WHITE, "You don't have many credits in your base!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   ships->base_credits -= move_value;
   ships->ship_credits += move_value;
}

/* **********************************************************************
   * Move cargo from ship to base.					*
   *									*
   ********************************************************************** */

void ship_cargo(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->ship_cargo) {
      c_out(WHITE, "You don't have that much cargo in your ship!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->base_cargo + move_value > to_limits[2]) {
      c_out(WHITE, "You base can not hold that much cargo!\n\r");
      return;
   }

   ships->ship_cargo -= move_value;
   ships->base_cargo += move_value;
}

/* **********************************************************************
   * Move cargo from base to ship.					*
   *									*
   ********************************************************************** */

void base_cargo(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->base_cargo) {
      c_out(WHITE, "You don't have that much cargo in your base!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->ship_cargo + move_value > to_limits[2]) {
      c_out(WHITE, "You ship can not hold that much cargo!\n\r");
      return;
   }

   ships->base_cargo -= move_value;
   ships->ship_cargo += move_value;
}

/* **********************************************************************
   * Move crew from ship to base.					*
   *									*
   ********************************************************************** */

void ship_crew(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->ship_crew) {
      c_out(WHITE, "You don't have that many crew members in your ship!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->base_crew + move_value > 200000000) {
      c_out(WHITE, "You base can not hold that many crew people!\n\r");
      return;
   }

   ships->ship_crew -= move_value;
   ships->base_crew += move_value;
}

/* **********************************************************************
   * Move crew from base to ship.					*
   *									*
   ********************************************************************** */

void base_crew(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->base_crew) {
      c_out(WHITE, "You don't have that many crew members in your base!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->ship_crew + move_value > to_limits[3]) {
      c_out(WHITE, "You ship can not hold that much crew!\n\r");
      return;
   }

   ships->base_crew -= move_value;
   ships->ship_crew += move_value;
}

/* **********************************************************************
   * Move cloak from ship to base.					*
   *									*
   ********************************************************************** */

void ship_cloak(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->ship_cloak) {
      c_out(WHITE, "You don't have that class cloak in your ship!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->base_cloak + move_value > to_limits[8] * 10) {
      c_out(WHITE, "You base can not hold that high class a cloak!\n\r");
      return;
   }

   ships->ship_cloak -= move_value;
   ships->base_cloak += move_value;
}

/* **********************************************************************
   * Move cloak from base to ship.					*
   *									*
   ********************************************************************** */

void base_cloak(void)
{
   UL int move_value;

   if ((move_value = ask_move_value()) > ships->base_cloak) {
      c_out(WHITE, "You don't have that class cloak in your base!\n\r");
      return;
   }

   if (move_value == 0) {
      return;
   }

   if (ships->ship_cloak + move_value > to_limits[8]) {
      c_out(WHITE, "You ship can not hold that high class a cloak!\n\r");
      return;
   }

   ships->base_cloak -= move_value;
   ships->ship_cloak += move_value;
}

/* **********************************************************************
   * Take half of everything in the ship and put it into the base.	*
   *									*
   ********************************************************************** */

void half_ship(void)
{
   read_user();
   ships->base_shield += ships->ship_shield /= 2;
   ships->base_credits += ships->ship_credits /= 2;
   ships->base_cargo += ships->ship_cargo /= 2;
   ships->base_crew += ships->ship_crew /= 2;
   ships->base_cloak += ships->ship_cloak /= 2;
   c_out(WHITE, "Half of everything moved from ship to base.\n\r");
   write_user();
}

/* **********************************************************************
   * Transfer half of everything in base to ship. If the limits of the	*
   * ships capcity are going to be exceeded, don't allow it.		*
   *									*
   ********************************************************************** */

void half_base(void)
{
   read_user();
   ships->ship_shield += ships->base_shield /= 2;
   ships->ship_credits += ships->base_credits /= 2;

   if (ships->ship_cargo + ships->base_cargo / 2 > to_limits[2]) {
      c_out(WHITE, "Can't transfer that much cargo. Cargo NOT transfered!\n\r");
   }
   else {
      ships->ship_cargo += ships->base_cargo /= 2;
   }

   if (ships->ship_crew + ships->base_crew / 2 > to_limits[3]) {
      c_out(WHITE, "Can't transfer that many crew! Crew NOT transfered!\n\r");
   }
   else {
       ships->ship_crew += ships->base_crew /= 2;
   }

   if (ships->ship_cloak + ships->base_cloak / 2 > to_limits[8]) {
      c_out(WHITE, "Cant mount that high a class cloaking device in ship!\n\r");
   }
   else {
      ships->ship_cloak += ships->base_cloak /= 2;
   }

   c_out(WHITE, "Transfers from base to ship compleated.\n\r");
   write_user();
}

/* **********************************************************************
   * Take down the base. Offload everything and eject anything that	*
   * won't fit.								*
   *									*
   ********************************************************************** */

void disassemble_base(void)
{
   ships->ship_shield += ships->base_shield;
   ships->ship_credits += ships->base_credits;

   ships->ship_cargo += ships->base_cargo;

   if (ships->ship_cargo > to_limits[2]) {
      c_out(WHITE, "Had to dump %ld units of cargo into space!\n\r",
         ships->ship_cargo - to_limits[2]);
      ships->ship_cargo = to_limits[2];
   }

   ships->ship_crew += ships->base_crew;

   if (ships->ship_crew > to_limits[3]) {
      c_out(WHITE, "Had to vacuume %ld crew members! No room in ship!\n\r",
         ships->ship_crew - to_limits[3]);
      ships->ship_crew = to_limits[3];
   }


   ships->ship_cloak += ships->base_cloak;

   if (ships->ship_cloak > to_limits[8]) {
      c_out(WHITE, "Had to dump a class %ld cloaking device into space!\n\r",
         ships->ship_cloak - to_limits[8]);
      ships->ship_cloak = to_limits[8];
   }

   ships->base_shield = 0;
   ships->base_credits = 0;
   ships->base_cargo = 0;
   ships->base_crew = 0;
   ships->base_cloak = 0;
   ships->base_xpos = ships->base_ypos = ships->base_universe = 0;
   hold[user_number]->bxpos = 0;
   hold[user_number]->bypos = 0;
   hold[user_number]->bzpos = 0;
   c_out(WHITE, "Base has been disassembled.\n\r");
   write_user();
}

/* **********************************************************************
   * The base has been damaged. Compute how much it's going to take to	*
   * fix it.								*
   *									*
   ********************************************************************** */

void repair_base(void)
{
   long needed_cash;

   if (ships->base_hit_count == 0) {
      c_out(WHITE, "You base is not damaged!\n\r");
      return;
   }

   needed_cash = (long) ships->base_hit_count * 10745;
   c_out(WHITE, "It will cost %ld credits to repair the damage.\n\r", needed_cash);

   if (ships->ship_credits < needed_cash) {
      c_out(WHITE, "You just don't have enough cash!\n\r");
      return;
   }

   ships->base_hit_count = 0;
   ships->ship_credits -= needed_cash;
   c_out(WHITE, "Enemy base repaired! Construction crew recalled.\n\r");
   write_user();
}

