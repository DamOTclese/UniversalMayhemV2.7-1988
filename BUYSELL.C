
/* **********************************************************************
   * buysell.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Here are the items that can be bought. This array is used as a	*
   * description of things bought and possible to buy.			*
   *									*
   * A relativly major change was performed 25/Dec/88 which had to do	*
   * with who the taxes went to when a ship sells at a planet. The 	*
   * areas of concern for these updates are documentedin the selling	*
   * routines within this module.					*
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
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern float tax_increase;
   extern long dloop;
   extern long xpos, ypos;
   extern unsigned short docked;
   extern char *record;
   extern short bail_out;
   extern short user_number;
   extern short rpt_loop;
   extern short the_tech;
   extern short cost_energy, cost_cargo, cost_shuttle, cost_warp, cost_hull;
   extern short cost_cloak, cost_crew, cost_sensor, cost_torp, cost_remotes;
   extern short cost_sled;
   static unsigned short the_item;
   static UL total_cost;
   static UL the_val;

/* **********************************************************************
   * The "taxed_here" value value is usually set in the perform_warp	*
   * routines. It determines who owns the planet that the player is	*
   * currently docked with. It can be set to one of these constants:	*
   *									*
   * o NOT_OWNED if the planet is not owned. Taxes will be sent to the	*
   *   Galactic Police and will be used by them for equipment.		*
   *									*
   * o IS_OWNED if the planet is owned by another player. Taxes will go	*
   *   to the owner of the planet when performtaxes is executed.	*
   *   If the planet is owned by another player, the value "owned_who"	*
   *   is set to the number of the ship which owns the planet. It is	*
   *   done in the "perform_warp()" function so that multiple sells do	*
   *   not cause the setting of this value every time, slowing down the	*
   *   selling routines.						*
   *									*
   * o PLAYER_OWNED if the currently active player owns the planet. The	*
   *   taxes will go to the Galactic Police but will be reduced to 10	*
   *   percent of the normal tax value.					*
   *									*
   ********************************************************************** */

   extern unsigned char taxed_here;
   extern unsigned short owned_who;

/* **********************************************************************
   * If the taxes at this planet have been exceeded, don't allow the	*
   * items to br sold. This value contains the amount of taxes which	*
   * are needed to sell the items.					*
   *									*
   ********************************************************************** */

   unsigned long need_ships_taxes = 0;
   unsigned char the_slot = 0;

/* **********************************************************************
   * A descriptive title for the items sold and bought are stored here.	*
   *									*
   ********************************************************************** */

   static char *to_buy[] = {
      "remote robots",
      "units of energy",
      "units of cargo",
      "crew members",
      "warp drive units",
      "shuttle crafts",
      "hull patches",
      "sensor devices",
      "cloaking devices",
      "torpedos",
      "attack sleds",
      (char *)NULL } ;

/* **********************************************************************
   * Here are the upper limits to the things a ship can hold. Note	*
   * that the type should be long so that energy and credits can be	*
   * quite high.							*
   *									*
   ********************************************************************** */

   long to_limits[12] = {
      10,		/* the number of remotes allowed 	*/
      2100000000L,	/* the most energy a ship can hold 	*/
      32000L,		/* the most cargo a ship can hold 	*/
      8050,		/* the maximum number of crew members	*/
      50,		/* the maximum warp drive allowed	*/
      200,		/* the highest class shuttle craft	*/
      1000,		/* the highest number of hull patches 	*/
      250,		/* the highest sensor class		*/
      250,		/* the highest cloaking device class	*/
      5000,		/* the maximum number of torpedos	*/
      4000,		/* the maximum number of attack sleds 	*/
      0			/* Make a null entry here		*/
   } ;			/* the end of it all			*/

/* **********************************************************************
   * Sell something.							*
   *									*
   *  0 - Remote robot							*
   *  1 - Energy							*
   *  2 - Cargo								*
   *  3 - Crew								*
   *  4 - Warp drive							*
   *  5 - Shuttle							*
   *  6 - Hull patch							*
   *  7 - Sensor							*
   *  8 - Cloak								*
   *  9 - Tropedo							*
   *  A - Attack sleds							*
   *									*
   ********************************************************************** */

void perform_sell(void)
{
   char *tpoint, ar[25];
   unsigned char sell_result;

   the_item = 0;

   if (docked != 1) {
      c_out(LIGHTRED, "You need to dock at a planet before you can sell things!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (the_tech == 0) {
      c_out(LIGHTRED, "This planet is dead! There is no one to sell to!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (strlen(record) < 3) {
      c_out(LIGHTRED, "Syntax is SiVAL - Sell item i val many units.\n\r");
      rpt_loop = 0;
      while (to_buy[the_item] != (char)NULL) {
	 if (the_item < 10) {
	    c_out(LIGHTRED, "%d - to sell [val] %s\n\r", the_item, to_buy[the_item]);
	 }
	 else {
	    c_out(LIGHTRED, "A - to sell [val] %s\n\r", to_buy[the_item]);
	 }
         the_item++;
      }
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   read_user();

/*
   The amount of taxes increase depending on the amount of property
   that the ship holds.
*/

   tax_increase = (hold[user_number]->standings * .001);

   if (tax_increase > .10) {
      tax_increase = .10;
   }

   tpoint = record;
   tpoint++;

   if (*tpoint == 'A') {
      the_item = 10;
   }
   else {
      the_item = (*tpoint) - 0x30;
   }

   tpoint++;
   strcpy(ar, tpoint);
   the_val = atol(tpoint);

   if (the_val == 0 && record[1] != '0') {
      c_out(LIGHTRED, "Invalid item to sell. Request 'S' for a list.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (strlen(ar) > 9 || the_val < 0 || the_val > 100000000L) {
      c_out(LIGHTRED, "Invalid number of units to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   switch (the_item) {
      case 0: sell_result = sell_remote(); break;
      case 1: sell_result = sell_power(); break;
      case 2: sell_result = sell_cargo(); break;
      case 3: sell_result = sell_crew(); break;
      case 4: sell_result = sell_warp(); break;
      case 5: sell_result = sell_shuttle(); break;
      case 6: sell_result = sell_hull(); break;
      case 7: sell_result = sell_sensor(); break;
      case 8: sell_result = sell_cloak(); break;
      case 9: sell_result = sell_torp(); break;
      case 10: sell_result = sell_sled(); break;
   default:
      c_out(LIGHTRED, "For a list of things to sell, request 'S'.\n\r");
      return;
   }

/*
   If the selling fails for any reason, simply return, otherwise find
   the amount of taxes needed and deduct that amount and update all the
   records.
*/

   if (! sell_result) {
      return;
   }

/*
   If the planet is now owned, put the cash into the Galactic Police
   ship. At the same time, if the taxes are quite high, allow a tax
   break by setting it to two billion credits.
*/

   if (taxed_here == NOT_OWNED) {
      ships->taxes += need_ships_taxes;
      if (ships->taxes > 2000000000L || ships->taxes < 0) {
	 c_out(LIGHTGREEN, "You qualify for a tax break!\n\r");
         ships->taxes = 2000000000L;
      }
   }

/*
   If the player owns the planet, only 10% of the needed taxes go
   to the cops. Note that the same three strings are allocated here.
   With the compile options, I merge duplicate strings so we don't
   waste that data segment space.
*/

   if (taxed_here == PLAYER_OWNED) {
      ships->taxes += (need_ships_taxes * 0.10);
      if (ships->taxes > 2000000000L || ships->taxes < 0) {
	 c_out(LIGHTGREEN, "You qualify for a tax break!\n\r");
         ships->taxes = 2000000000L;
      }
   }

/*
   If the planet is owned by another player, add the taxes to that ships
   tax slot. If the taxes are too high, allow a tax break here as well.
*/

   if (taxed_here == IS_OWNED) {
      ships->planet_taxes[the_slot] += need_ships_taxes;
      ships->tax_xpos[the_slot] = xpos;
      ships->tax_ypos[the_slot] = ypos;
      if (Good_Hold(planets.protected)) {
         strcpy(ships->slot_owned[the_slot], hold[planets.protected]->names);
      }
      else {
         log_error(114);
      }
      if (ships->planet_taxes[the_slot] > 2000000000L ||
          ships->planet_taxes[the_slot] < 0) {
	 c_out(LIGHTGREEN, "You qualify for a tax break!\n\r");
         ships->planet_taxes[the_slot] = 2000000000L;
      }
   }

/*
   If the ship can't hold enough credits, send it to the poor and
   set the value to the upper limit. The taxes due are also divided
   by ten so that the ship gets a BIG tax break.
*/

   if (ships->ship_credits > 2100000000L) {
      c_out(LIGHTRED, "Ship can't hold that much cash! Excess went to the poor.\n\r");
      ships->ship_credits = 2100000000L;
      if (ships->taxes > 2000) {
         ships->taxes /= 10;
      }
   }

   write_user();
}

/* **********************************************************************
   * Buy something.							*
   *									*
   *  0 - Remote robot							*
   *  1 - Energy							*
   *  2 - Cargo								*
   *  3 - Crew								*
   *  4 - Warp drive							*
   *  5 - Shuttle							*
   *  6 - Hull patch							*
   *  7 - Sensor							*
   *  8 - Cloak								*
   *  9 - Tropedo							*
   * 10 - Attack sled							*
   *									*
   ********************************************************************** */

void perform_buy(void)
{
   char *tpoint, ar[25];
   unsigned buy_result;

   the_item = 0;

   if (docked != 1) {
      c_out(LIGHTRED, "You need to dock at a planet before you can buy things!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (the_tech == 0) {
      c_out(LIGHTRED, "This planet is dead! There is no one to buy from!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (strlen(record) < 3) {
      c_out(LIGHTRED, "Syntax is BiVAL - Buy item i val many units.\n\r");
      rpt_loop = 0;
      while (to_buy[the_item] != (char)NULL) {
	 if (the_item < 10) {
	    c_out(LIGHTRED, "%d - to buy [val] %s\n\r", the_item, to_buy[the_item]);
	 }
	 else {
	    c_out(LIGHTRED, "A - to buy [val] %s\n\r", to_buy[the_item]);
	 }
         the_item++;
      }
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

/* **********************************************************************
   * Well! Here is a piece of true, honest to goodness, gold plated	*
   * sh|t which one and all should take a good look at! There has GOT	*
   * to be a better way!						*
   *									*
   * First of all, make a pointer to the command which is Bxyyyyyyy	*
   * Then point to the next character. That character is the item	*
   * number from 0 to 9 to buy. Then point to the next character. All	*
   * that follows is the amount to buy. The best we can do with this	*
   * value is try to convert it into a long.				*
   *									*
   * Notice later that we test to see if the length of this value is	*
   * more than 10 characters. If it is, then we can't do anything with	*
   * the number so we don't allow it.					*
   *									*
   ********************************************************************** */

   tpoint = record;
   tpoint++;

   if (*tpoint == 'A') {
      the_item = 10;
   }
   else {
      the_item = (*tpoint) - 0x30;
   }

   tpoint++;
   strcpy(ar, tpoint);
   the_val = atol(tpoint);

   if (the_val == 0 && record[1] != '0') {
      c_out(LIGHTRED, "Invalid item to buy. Request 'B' for a list.\n\r");
      rpt_loop = 0;
      return;
   }

   if (strlen(ar) > 9 || the_val < 0 || the_val > 100000000L) {
      rpt_loop = 0;
      c_out(LIGHTRED, "Invalid number of units to buy.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   read_user();

   switch (the_item) {
      case 0:
	 buy_result = buy_remote();
	 break;
      case 1:
	 buy_result = buy_power();
	 break;
      case 2:
	 buy_result = buy_cargo();
	 break;
      case 3:
	 buy_result = buy_crew();
	 break;
      case 4:
         if (ships->ship_warp + the_val > planets.technology) {
	    technology_level_bad();
	    return;
	 }
	 buy_result = buy_warp();
	 break;
      case 5:
         if (ships->ship_shuttle + the_val > planets.technology) {
	    technology_level_bad();
	    return;
	 }
	 buy_result = buy_shuttle();
	 break;
      case 6:
	 buy_result = buy_hull();
	 break;
      case 7:
         if (ships->ship_sensor + the_val > planets.technology) {
	    technology_level_bad();
	    return;
	 }
	 buy_result = buy_sensor();
	 break;
      case 8:
         if (ships->ship_cloak + the_val > planets.technology) {
	    technology_level_bad();
	    return;
	 }
	 buy_result = buy_cloak();
	 break;
      case 9:
         buy_result = buy_torp();
	 break;
      case 10:
	 buy_result = buy_sled();
	 break;
   default:
      c_out(LIGHTRED, "For a list of things to buy, request 'B'.\n\r");
      return;
   }

   if (buy_result == FALSE) {
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (buy_result == LIMITS) {
      c_out(LIGHTRED, "Your ship can not carry that much!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

  if (buy_result == COST) {
     c_out(LIGHTRED, "You can't afford to buy that!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   c_out(LIGHTBLUE, "Bought %ld %s\n\r", the_val, to_buy[the_item]);
   ships->ship_credits -= total_cost;
   write_user();
   return;
}

/* **********************************************************************
   * See if the player has enough credits to buy what is wanted. Return	*
   * true or false.							*
   *									*
   ********************************************************************** */

unsigned short have_enough(void)
{
   if (total_cost > ships->ship_credits || total_cost == (UL)NIL) {
      return(FALSE);
   }
   return(TRUE);
}

/* **********************************************************************
   * Here is where we see if there is enough money and we also check to	*
   * see if the ship can hold any more of something.			*
   *									*
   * If there is no room, we say so. Notice that we ignore the_val.	*
   *									*
   ********************************************************************** */

unsigned short buy_remote(void)
{
   if (the_val != 1) {
      c_out(LIGHTRED, "Only one remote may be bought at a time!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      total_cost = (UL)NIL;
      return(FALSE);
   }

   total_cost = cost_remotes;

   if (! have_enough()) {
      return(COST);
    }

   for (dloop = 0; dloop < 10; dloop++) {
      if (ships->rem_xpos[dloop] < 1 &&
         ships->rem_ypos[dloop] < 1 &&
         ships->rem_xpos[dloop] != ONBOARD &&
         ships->rem_ypos[dloop] != ONBOARD) {
         ships->rem_xpos[dloop] = ONBOARD;
         ships->rem_ypos[dloop] = ONBOARD;
         ships->rem_universe[dloop] = 0;
         return(TRUE);
      }
   }

   c_out(LIGHTRED, "You don't have room to put another remote.\n\r");
   bail_out = 1;
   rpt_loop = 0;
   total_cost = (UL)NIL;
   return(LIMITS);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_remote(void)
{
   if (the_val != 1) {
      bail_out = 1;
      rpt_loop = 0;
      c_out(LIGHTRED, "You may sell only one remote at a time!\n\r");
      return(FALSE);
   }

   for (dloop = 0; dloop < 10; dloop++) {
      if (ships->rem_xpos[dloop] == ONBOARD) {
	 need_ships_taxes = cost_remotes * (TAXES + tax_increase);
	 if (test_taxes_due()) {
	    return(FALSE);
	 }
	 c_out(LIGHTBLUE, "Selling remote %ld for %d credits\n\r", dloop, cost_remotes);
         ships->ship_credits += cost_remotes;
         ships->rem_xpos[dloop] = 0;
         ships->rem_ypos[dloop] = 0;
         ships->rem_universe[dloop] = 0;
         hold[user_number]->xremotes[dloop] = 0;
	 hold[user_number]->yremotes[dloop] = 0;
	 hold[user_number]->remote_universe[dloop] = 0;
	 return(TRUE);
      }
   }
   c_out(LIGHTRED, "You have NO remotes on board!\n\r");
   bail_out = 1;
   rpt_loop = 0;
   return(FALSE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_power(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_energy * the_val;

   if (! have_enough()) {
      return(COST);
    }

   if (ships->ship_power + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_power += the_val;
   return(TRUE);
}

/* **********************************************************************
   * It was suggested after version 1.1 release that power not be	*
   * sellable. This update was performed for version 1.2.		*
   *									*
   ********************************************************************** */

unsigned char sell_power(void)
{
   c_out(LIGHTRED, "You can't sell power!\n\r");
   bail_out = 1;
   rpt_loop = 0;
   return(FALSE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_cargo(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_cargo * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_cargo + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_cargo += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_cargo(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_cargo < the_val) {
      c_out(LIGHTRED, "You don't have that many units of cargo to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_cargo) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling %ld units of cargo for %ld credits\n\r",
      the_val, the_val * cost_cargo);

   ships->ship_cargo -= the_val;
   ships->ship_credits += (the_val * cost_cargo);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_crew(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_crew * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_crew + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_crew += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_crew(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_crew < the_val) {
      c_out(LIGHTRED, "You don't have that many crew members to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   if (ships->ship_crew - the_val < 10) {
      c_out(LIGHTRED, "You can't sell that many crew members. No one will run ");
      c_out(LIGHTRED, "the ship!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_crew) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling %ld crew members for %ld credits\n\r",
      the_val, the_val * cost_crew);

   ships->ship_crew -= the_val;
   ships->ship_credits += (the_val * cost_crew);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_warp(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_warp * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_warp + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_warp += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_warp(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_warp < the_val) {
      c_out(LIGHTRED, "You don't have that many warp units to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   if (ships->ship_warp == the_val) {
      c_out(LIGHTRED, "That would leave your ship drifting!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_warp) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling %ld warp units for %ld credits.\n\r",
      the_val, the_val * cost_warp);

   ships->ship_warp -= the_val;
   ships->ship_credits += (the_val * cost_warp);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_shuttle(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_shuttle * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_shuttle + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_shuttle += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_shuttle(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_shuttle < the_val) {
      c_out(LIGHTRED, "You don't have that class shuttle to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_shuttle) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling %ld class shuttle for %ld credits.\n\r",
     the_val, the_val * cost_shuttle);

   ships->ship_shuttle -= the_val;
   ships->ship_credits += (the_val * cost_shuttle);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_hull(void)
{

   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_hull * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_hull + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_hull += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_hull(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_hull < the_val) {
      c_out(LIGHTRED, "You don't have that hull value to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   if (ships->ship_hull - the_val < 40) {
      c_out(LIGHTRED, "That would cause MUCH death aboard your ship!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_hull) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling %ld hull patches for %ld credits\n\r",
      the_val, the_val * cost_hull);

   ships->ship_hull -= the_val;
   ships->ship_credits += (the_val * cost_hull);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_sensor(void)
{

   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_sensor * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_sensor + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_sensor += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_sensor(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_sensor < the_val) {
      c_out(LIGHTRED, "You dont have that class sensor to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_sensor) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling class %ld sensor for %ld credits.\n\r",
      the_val, the_val * cost_sensor);

   ships->ship_sensor -= the_val;
   ships->ship_credits += (the_val * cost_sensor);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_cloak(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_cloak * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_cloak + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_cloak += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_cloak(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_cloak < the_val) {
      c_out(LIGHTRED, "You don't have that class cloaking device to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_cloak) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling class %ld cloaking device for %ld credits\n\r",
      the_val, the_val * cost_cloak);

   ships->ship_cloak -= the_val;
   ships->ship_credits += (the_val * cost_cloak);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_torp(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_torp * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->ship_torp + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->ship_torp += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_torp(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->ship_torp < the_val) {
      c_out(LIGHTRED, "You don't have that many torpedos to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_torp) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling %ld torpedos for %ld credits.\n\r",
      the_val, the_val * cost_torp);

   ships->ship_torp -= the_val;
   ships->ship_credits += (the_val * cost_torp);
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned short buy_sled(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   total_cost = cost_sled * the_val;

   if (! have_enough()) {
      return(COST);
   }

   if (ships->attack_sleds + the_val > to_limits[the_item]) {
      return(LIMITS);
   }

   ships->attack_sleds += the_val;
   return(TRUE);
}

/* **********************************************************************
   *									*
   ********************************************************************** */

unsigned char sell_sled(void)
{
   if (the_val < 1) {
      return(FALSE);
   }

   if (ships->attack_sleds < the_val) {
      c_out(LIGHTRED, "You don't have that many attack sleds to sell.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   need_ships_taxes = (the_val * cost_sled) * (TAXES + tax_increase);

   if (test_taxes_due()) {
      return(FALSE);
   }

   c_out(LIGHTBLUE, "Selling %ld attack sleds for %ld credits.\n\r",
      the_val, the_val * cost_sled);

   ships->attack_sleds -= the_val;
   ships->ship_credits += (the_val * cost_sled);
   return(TRUE);
}

/* **********************************************************************
   * The technology level of the planet could not supply the technology	*
   * level of the device that is trying to be bought.			*
   *									*
   ********************************************************************** */

void technology_level_bad(void)
{
   c_out(LIGHTRED, "This planet can not supply that level of technology!\n\r");
   bail_out = 1;
   rpt_loop = 0;
}

/* **********************************************************************
   * See if the taxes required plus the outstanding taxes are exceeding	*
   * the taxes due value. If so, display a message and return with a	*
   * TRUE, otherwise, return with a FALSE.				*
   *									*
   ********************************************************************** */

unsigned short test_taxes_due(void)
{
   short test_slot;

/*
   If no one owns the planet, return FALSE. This is to say that the
   test did not return a reason why the sell could not continue.

   At the same time, if the planet is owned by the currently active
   player, then allow the selling at the planet by returning FALSE.
*/

   if (taxed_here == NOT_OWNED || taxed_here == PLAYER_OWNED) {
      return(FALSE);
   }

/*
   At this point, the planet is owned and not by the currently active
   player. See if there is an empty slot where taxes could be acrued
   and if not, display a message and return TRUE. Returning a TRUE will
   cause the selling to discontinue.

   If a slot is found, return with a FALSE, allowing the selling but
   before that is done, set the global value "the_slot" to the empty
   slot number.

   It's important to note that we first scan the ten available slots
   to see if there is already taxes due for the owner of this planet. If
   so, we set the "the_slot" value to the slot number and return with
   a FALSE, allowing the sell to continue.
*/

   for (test_slot = 0; test_slot < 10; test_slot++) {
      if (Good_Hold(owned_who)) {
         if (! strcmp(ships->slot_owned[test_slot], hold[owned_who]->names)) {
            the_slot = test_slot;
            return(FALSE);
         }
      }
      else {
         log_error(114);
      }
   }

   for (test_slot = 0; test_slot < 10; test_slot++) {
      if (ships->planet_taxes[test_slot] == 0.0) {
	 the_slot = test_slot;
	 return(FALSE);
      }
   }

   c_out(LIGHTRED, "You already have taxes due at ten planets! Pay up deadbeat!\n\r");
   return(TRUE);
}

