
/* **********************************************************************
   * auction.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow the bidding or buying of a ship up for auction.		*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "stdlib.h"
#include "ctype.h"
#include "string.h"
#include "conio.h"

/* **********************************************************************
   * The 'to_limits[]' array contains the number of units that are	*
   * allowed to be carried on board the ship. Anything that might	*
   * cause the number of units of an item to exceed these limits are	*
   * spaced or thrown away.						*
   *									*
   ********************************************************************** */

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long to_limits[12];
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern long count;
   extern char *record;
   extern short players;
   extern short user_number;

void perform_auction(void)
{
   short auction_flag;

   auction_flag = 0;

   if (ships->outstanding_bid != (char)NIL) {
      read_enemy(ships->outstanding_bid);
      if (enemy->ship_xpos == 0 && enemy->ship_ypos == 0) {
	 c_out(LIGHTRED, "The ship you bid on was sold to a higer bidder or was ");
	 c_out(LIGHTRED, "destroyed!\n\r");
         ships->outstanding_bid = (char)NIL;
         ships->bid_amount = 0;
	 write_user();
	 return;
      }
      already_bidding();
      return;
   }

   if (strstr(ships->ship_date, "Fri") != (char *)NULL) {
      c_out(LIGHTBLUE, "You are not allowed to place a bid on ships on Friday!\n\r");
      return;
   }

/*
   The active ship does not have a bid outstanding on a ship. See if
   there are any ships awaiting auction. If not, issue a notice and
   simply return;
*/

   c_out(WHITE, "\n\r\n\r");

   for (count = 1; count < players; count++) {  /* make sure no cops in it */
      if (Good_Hold(count)) {
	 read_enemy(count);
         if (enemy->tax_warnings == (char)AUCTION_SHIP) {
	    auction_flag++;
	    c_out(WHITE, "%d) Ship %s last used %s",
               (short) count, enemy->ship_name, enemy->ship_date);
         }
      }
   }

   if (auction_flag == 0) {
      c_out(WHITE, "\n\rThere are no ships up for auction.\n\r");
      return;
   }

   if (auction_flag == 1) {
      c_out(WHITE, "Do you want to bid on this? ");
   }
   else {
      c_out(WHITE, "\n\rDo you want to place a bid on any of these? ");
   }

   timed_input(0);

   if (toupper(record[0]) != 'Y') {
      return;
   }

   c_out(WHITE, "Place a bid on which ship NUMBER? ");
   timed_input(0);
   auction_flag = atoi(record);

   if (auction_flag < 1 || auction_flag > players) {
      c_out(LIGHTRED, "\n\rWhat? Get with it! That's silly!\n\r");
      return;
   }

   read_enemy(auction_flag);

   if (enemy->tax_warnings != (char)AUCTION_SHIP) {
      c_out(LIGHTRED, "\n\rThat ship is NOT up for auction!\n\r");
      return;
   }

   c_out(WHITE, "How many credits do you want to bid? ");
   timed_input(0);
   ships->bid_amount = atol(record);

   if (ships->bid_amount < 1000 || ships->bid_amount > ships->ship_credits) {
      c_out(WHITE, "That bid is not acceptable!\n\r");
      ships->bid_amount = 0;
      return;
   }

   ships->outstanding_bid = auction_flag;

   write_user();
   c_out(WHITE, "The bid has been logged with the Galactic Police. Perform\n\r");
   c_out(WHITE, "AUCTION on FRIDAY to see if you can pick up a ship!\n\r");
}

/* **********************************************************************
   * The ship has a bid outstanding. See if its Friday. If not, then	*
   * send them back with a nottice. Otherwise, see if there are any	*
   * other ships with bids on the same ship. If so, take the highest	*
   * bidder.								*
   *									*
   ********************************************************************** */

void already_bidding(void)
{
   long highest_bid;
   short a_loop;

   if (strstr(ships->ship_date, "Fri") == (char *)NULL) {
      if (Good_Hold(ships->outstanding_bid)) {
         c_out(WHITE, "You have a bid outstanding on ship %s\n\r",
            hold[ships->outstanding_bid]->names);
      }
      else {
         log_error(114);
      }
      c_out(WHITE, "\n\rYou must check on FRIDAY to see if your bid won.\n\r");
      return;
   }

   read_enemy(ships->outstanding_bid);

/*
   Go through the other ships bid fields and see if anyone else has
   made a bid on the ship. If so, then see if the big is greater than
   the ships-> If so, then inform this ship captain that the bid was
   too low.

   If the big was the highest or equal, then give this ship everything
   in the auctioned ship and then destroy the auctioned ship.

   If two players have made a bid on a ship of equal credit amount,
   then the first person who claims the bid get it.
*/

   for (a_loop = highest_bid = 0; a_loop < players; a_loop++) {
      if (Good_Hold(a_loop)) {
         read_enemy(a_loop);
         if (enemy->outstanding_bid == ships->outstanding_bid) {
            if (enemy->bid_amount > highest_bid) {
               highest_bid = enemy->bid_amount;
   	    }
         }
      }
   }

   if (ships->bid_amount == highest_bid) {
      was_highest_bid();
      return;
   }

   c_out(WHITE, "\n\rYour bid was not the highest! Sorry.\n\r");
   ships->outstanding_bid = (char)NIL;
   ships->bid_amount = 0;
   write_user();
}

/* **********************************************************************
   * The current ship was the highest bid. Make sure that the ship was	*
   * not destroyed and transfer everything over. After that, destroy	*
   * the ship that was auctioned.					*
   *									*
   * Also make sure that the current ship has enough cash. If not, then	*
   * put the remainder amount of cash needed into taxes due.		*
   *									*
   * If we exceed the limits for the ship, we space those items!	*
   *									*
   ********************************************************************** */

void was_highest_bid(void)
{
   short hold_ship, enemy_ship;

   read_enemy(ships->outstanding_bid);
   enemy_ship = ships->outstanding_bid;

   if (ships->ship_power + enemy->ship_power > to_limits[1]) {
      c_out(LIGHTRED, "Bleeding excess power to space!\n\r");
      ships->ship_power = to_limits[1];
   }
   else {
      ships->ship_power += enemy->ship_power;
   }

   c_out(LIGHTGREEN, "Auctioned ship power:  %ld\n\r", enemy->ship_power);

   if (ships->ship_warp + enemy->ship_warp > to_limits[4]) {
      c_out(LIGHTRED, "Tossing unworkable warp drive!\n\r");
      ships->ship_warp = to_limits[4];
   }
   else {
      ships->ship_warp += enemy->ship_warp;
   }

   c_out(LIGHTGREEN, "Auctioned warp drive:  %d\n\r", enemy->ship_warp);

   if (ships->ship_cargo + enemy->ship_cargo > to_limits[2]) {
      c_out(LIGHTRED, "Dumping excess cargo!\n\r");
      ships->ship_cargo = to_limits[2];
   }
   else {
      ships->ship_cargo += enemy->ship_cargo;
   }

   c_out(LIGHTGREEN, "Auctioned cargo:       %d\n\r", enemy->ship_cargo);

   if (ships->ship_shuttle + enemy->ship_shuttle > to_limits[5]) {
      c_out(LIGHTRED, "Tossing unworkable shuttle craft technology!\n\r");
      ships->ship_shuttle = to_limits[5];
   }
   else {
      ships->ship_shuttle += enemy->ship_shuttle;
   }

   c_out(LIGHTGREEN, "Auctioned shuttle:     %d\n\r", enemy->ship_shuttle);

   if (ships->ship_hull + enemy->ship_hull > to_limits[6]) {
      c_out(LIGHTRED, "Excess hull patches trashed!\n\r");
      ships->ship_hull = to_limits[6];
   }
   else {
      ships->ship_hull += enemy->ship_hull;
   }

   c_out(LIGHTGREEN, "Auctioned ship hull:   %d\n\r", enemy->ship_hull);

   if (ships->ship_cloak + enemy->ship_cloak > to_limits[8]) {
      c_out(LIGHTRED, "Dumping unworkable cloaking device technology!\n\r");
      ships->ship_cloak = to_limits[8];
   }
   else {
      ships->ship_cloak += enemy->ship_cloak;
   }

   c_out(LIGHTGREEN, "Auctioned ship cloak:  %d\n\r", enemy->ship_cloak);

   if (ships->ship_sensor + enemy->ship_sensor > to_limits[7]) {
      c_out(LIGHTRED, "Can't use unworkable sensor array technology!\n\r");
      ships->ship_sensor = to_limits[7];
   }
   else {
      ships->ship_sensor += enemy->ship_sensor;
   }

   c_out(LIGHTGREEN, "Auctioned sensor:      %d\n\r", enemy->ship_sensor);

   if (ships->ship_torp + enemy->ship_torp > to_limits[9]) {
      c_out(LIGHTRED, "Excess torpedos on board are a health hazard!\n\r");
      ships->ship_torp = to_limits[9];
   }
   else {
      ships->ship_torp += enemy->ship_torp;
   }

   c_out(LIGHTGREEN, "Auctioned ship torps:  %d\n\r", enemy->ship_torp);

   if (ships->ship_credits < ships->bid_amount) {
      ships->bid_amount -= ships->ship_credits;
      ships->ship_credits = 0;
      ships->taxes += ships->bid_amount;
      c_out(LIGHTRED,
	 "\n\r* * * Warning! You don't have enough cash! Balance TAXED! ");
      c_out(LIGHTRED, "* * *\n\r");
   }
   else {
      ships->ship_credits -= ships->bid_amount;
   }

   ships->bid_amount = 0;
   ships->outstanding_bid = (char)NIL;
   write_user();

   make_zero_record();
   hold_ship = user_number;
   user_number = enemy_ship;
   write_user();
   user_number = hold_ship;
   read_user();
   memory_freed((UL)sizeof(struct holder));
   if_any_bounce_it(enemy_ship);
   farfree(hold[enemy_ship]);
   hold[enemy_ship] = (struct holder *)NULL;
}

/* **********************************************************************
   * Take the ship and make it available for auction. All of the	*
   * money in the ship, if any, should be transfered to the Galactic	*
   * Police ship.							*
   *									*
   * We make the ship available for auction by setting the number of	*
   * warnings to -1.							*
   *									*
   * If the ship owned planets, make them available for general use by	*
   * marking them as protected by NIL.					*
   *									*
   * We also don't want the ship to be automated while its in auction.	*
   * We get rid of this by calling the routine that removed the command	*
   * file for the ship if it finds one.					*
   *									*
   ********************************************************************** */

void impound_ship(void)
{
   hand_over_planets(user_number, (short)NIL);
   read_enemy(0);
   enemy->ship_credits += ships->ship_credits;
   remove_command_file(ships->ship_name, (char *)NULL);
   ships->ship_credits = 0;
   ships->tax_warnings = (char)AUCTION_SHIP;
   ships->taxes = 0;
   ships->ship_xpos = xsize / 2;
   ships->ship_ypos = ysize / 2;
   ships->ship_universe = 0;
   write_user();
   write_enemy(0);

   c_out(LIGHTRED,
      "\n\rYour ship has been impounded by the Galactic Police for\n\r");

   c_out(LIGHTRED,
      "failure to pay your taxes! You were warned about it four times!\n\r");

   perform_quit(0);
}


