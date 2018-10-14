
/* **********************************************************************
   * taxes.								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991                                    *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * A _major_ change has been created over the Beta Version 1 in favor	*
   * of release version 1.00. This has to do mainly with this module.	*
   *									*
   * OLD FORMAT:							*
   *									*
   * The Galactic police will collect taxes from all of the ships. If   *
   * a ship doesn't have enough credits, they will demand it sometime.	*
   *									*
   * This function allows the ship to pay those taxes. It also tells	*
   * the player what his taxes are when they are due, and if he is paid	*
   * up in his taxes.							*
   *									*
   * When the taxes have been paid, put the money into the Galactic	*
   * polices ships credits. Then write that enemy record out again.	*
   *									*
   * NEW FORMAT:							*
   *									*
   * Each ship may have taxes outstanding to ten planets and the	*
   * Galactic Police. When selling at a planet, the owner, if any,	*
   * are owed the taxes. If no owner, taxes goes to the GP.		*
   *									*
   * These taxes are held in the users record and don't allow the	*
   * selling at a planet that has the maximum taxes  or at a planet	*
   * that is number 11 when ten others have taxes dues.			*
   *									*
   * If taxes is requested, the currently docked planet, if there is	*
   * one, is checked to see if taxes are owed. If so, they are paid.	*
   * If the taxes are too much _OR_ we are not docked at a planet, then	*
   * the player gets to enter a planet number to pay taxes to.		*
   *									*
   * If taxes is requested with a number from 0 to 10, then the planet	*
   * slot is paid. If the value is 0, then the GP are paid.		*
   *									*
   *  float planet_taxes[10];   Taxes owed at ten planets.		*
   *  long tax_xpos[10];	Xpositions of the ten tax due planets   *
   *  long tax_ypos[10];	Ypositions of the ten tax due planets   *
   *  float taxes		Taxes owed to the Galactic Police	*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "planets.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "stdlib.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * "Tax_increase" is the amount that is added to the normal TAXES	*
   * value to determine the percentage of taxes that are required. It	*
   * is incrimented in the buysell.c module when the amount of cash is	*
   * getting high.							*
   *									*
   ********************************************************************** */

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern float tax_increase;
   extern long aloop;
   extern long xpos, ypos;
   extern char *record;
   extern short players;

void perform_taxes(void)
{
   short aloop, the_item, test_slot;
   unsigned char tax_flag;

   tax_flag = FALSE;

   c_out(WHITE, "Taxes are %5.2f credits on every 100 credits\n\r",
      (TAXES + tax_increase) * 100);

   read_user();

   c_out(WHITE, "-------------------------------------------------------\n\r");

   if (ships->taxes > 1.0) {
      c_out(WHITE, "00) Taxes owed to <GP>. %10.2f credits, ", ships->taxes);
      tax_flag = TRUE;
      if (ships->taxes < TAXES_DUE) {
	 c_out(WHITE, "Not due yet\n\r");
      }
      else {
	 c_out(LIGHTRED, "Are DUE\n\r");
      }
   }

/*
   One other "patch" that was put in just a little later: If we are
   unable to find an empty spot, (a slot where there are 0 credits
   due), we then perform a check to see if there are any names that
   are "NONE". If so, reguardless of the amount of taxes due, zero
   out that ballance and return the slot number.

   It's placement here, in this routine, make this a "house cleaning".
*/

   for (test_slot = 0; test_slot < 10; test_slot++) {
      if (! strncmp(ships->slot_owned[test_slot], "NONE", 4)) {
         ships->planet_taxes[test_slot] = 0.0;
      }
   }

/*
   Go through the various "slots" where taxes may be due and display
   information about the planet if there are any taxes which need to
   be paid. Either due now or later.
*/

   for (aloop = 0; aloop < 10; aloop++) {
      if (ships->planet_taxes[aloop] > 1.0) {
         read_universe(ships->tax_xpos[aloop]);
         if (find_specific_planet(ships->tax_xpos[aloop],
                ships->tax_ypos[aloop]) == 1) {

            read_planets(ships->tax_xpos[aloop]);

	    c_out(WHITE, "%02d) Taxes owed to %s. %10.2f credits, ",
               aloop + 1, ships->slot_owned[aloop], ships->planet_taxes[aloop]);

	    tax_flag = TRUE;

            if (ships->planet_taxes[aloop] < TAXES_DUE) {
	       c_out(WHITE, "Not due yet\n\r");
	    }
	    else {
	       c_out(LIGHTRED, "Are DUE\n\r");
	    }
	 }
      }
   }

   if (! tax_flag) {
      c_out(WHITE, "You don't have any taxes outstanding at any planets!\n\r");
      return;
   }

   c_out(WHITE, "-------------------------------------------------------\n\r");

   c_out(WHITE, "Do you want to pay any of these? ");
   timed_input(0);
   ucase(record);

   if (record[0] != 'Y') {
      return;
   }

   c_out(WHITE, "Enter the item number to pay taxes for: ");
   timed_input(0);
   the_item = atoi(record);

   if (the_item < 0 || the_item > 10) {
      c_out(LIGHTRED, "Invalid item number. No taxes were paid!\n\r");
      return;
   }

/*
   See if the player wants to pay taxes to the cops. If so, make sure that
   there are enough credits to pay them all due and make the payment.
*/

   if (the_item == 0) {
      if (ships->ship_credits < ships->taxes) {
	 c_out(LIGHTRED, "You don't have enough to pay those taxes!\n\r");
	 return;
      }

/*
   Let the cops buy some things instead of just twinkies and doughnuts.
*/

      cops_buy_things_too(ships->taxes);
      ships->ship_credits -= ships->taxes;

      c_out(WHITE, "Paying taxes to <GP> of %f leaves %ld credits\n\r",
         ships->taxes, ships->ship_credits);

      ships->tax_warnings = 0;
      ships->taxes = 0;
      write_user();
      return;
   }

/*
   Here we allow the taxes for another players ship to be paid. As
   with the Galactic Police, make sure that the player has enough taxes
   to pay the debt.
*/

   the_item--;

   if (ships->planet_taxes[the_item] < 1) {
      c_out(WHITE, "No taxes are due for that property owner!\n\r");
      return;
   }

   if (ships->planet_taxes[the_item] > ships->ship_credits) {
      c_out(LIGHTRED, "You don't have enough to pay those taxes!\n\r");
      return;
   }

   if (find_specific_planet(ships->tax_xpos[the_item],
          ships->tax_ypos[the_item]) != 1) {

      c_out(LIGHTBLUE, "Non-Fatal problem occured when trying to find the owner\n\r");
      c_out(LIGHTBLUE, "of the planet. No taxes were paid! Sorry.\n\r");
      return;
   }

/*
   We have read in the universe information and found the specific planet.
   Now read in the data on that planet and determine who owns it. After we
   do that, shuffle the credits around. Note that if the property owner
   can't hold all that money, it's credits is set to the maximum.
*/

   read_planets(ships->tax_xpos[the_item]);

   for (aloop = 0; aloop < players; aloop++) {
      if (Good_Hold(aloop)) {
	 if (aloop == planets.protected) {
	    read_enemy(aloop);
            enemy->ship_credits += ships->planet_taxes[the_item];
            ships->ship_credits -= ships->planet_taxes[the_item];

	    c_out(WHITE, "Paying taxes to %s of %f leaves %ld credits\n\r",
               enemy->ship_name,
               ships->planet_taxes[the_item],
               ships->ship_credits);

            if (enemy->ship_credits > 2000000000L) {
               enemy->ship_credits = 2000000000L;
	    }

            ships->tax_warnings = 0;
            ships->planet_taxes[the_item] = 0;
            ships->tax_xpos[the_item] = ships->tax_ypos[the_item] = 0l;
            strcpy(ships->slot_owned[the_item], "NONE");
	    write_user();
	    write_enemy(aloop);
	    return;
	 }
      }
   }

/*
   Something screwy happened and we are unable to find the owner of the
   planets. Because of this, let's simply get rid of the debt and call
   it even. It's more than likely that something strange happened and we
   need to erase this debt anyways. Perhaps taxes owed to someone who
   got killed. Silly of them, wasn't it?
*/

   c_out(LIGHTGREEN, "Ship has been destroyed and its planets are free!\n\r");
   c_out(LIGHTGREEN, "Debt has been erased!\n\r");
   ships->tax_warnings = 0;
   ships->planet_taxes[the_item] = 0;
   write_user();
}


