

/* **********************************************************************
   * cops.c								*
   *									*
   * Copyright Fredric L. Rice 1988, 1989, 1990, 1991.                  *
   * All rights reserved.                                               *
   *									*
   * When the cops are created first time, they get some planets. Find	*
   * enough planets and assign them to the ownership of the cops.	*
   *									*
   * We will try to find available 'planets_to_find' times.		*
   * We return the number of planets actualy found.			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "universe.h"
#include "planets.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char is_redirected;
   extern long to_limits[12];
   extern FILE *aship;
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern long dloop, count;
   extern unsigned short the_percent;
   extern short try_count;
   extern short user_number;
   extern short cost_energy, cost_hull, cost_cloak, cost_torp;
   extern short the_rnd;

short find_some_available_planets(short planets_to_find)
{
   short try_count, testx, total_found;

   c_out(WHITE, "Handing over %d planets to the Galactic Police\n\r\n\r",
      planets_to_find);

   for (try_count = total_found = 0; try_count < planets_to_find; try_count++) {
      testx = arandom(1L, xsize);
      dloop = arandom(0L, 3L);
      read_planets(testx);
      if (planets.protected == (char)NIL) {
	 planets.protected = 0;
	 write_planets(testx);
         ships->owned_planets[total_found] = testx;
	 total_found++;
      }
      else if (planets.protected == 0) {
         ships->owned_planets[total_found] = testx;
	 total_found++;
      }
   }
   return(total_found);
}

/* **********************************************************************
   * Make a ship for the Galactic Police.				*
   *									*
   ********************************************************************** */

void create_galactic_police(short the_stat)
{
   unsigned short hold_user, actually_found;

   if (the_stat > 0) {
      hold_user = user_number;
      make_zero_record();
   }

   user_number = 0;
   ships->ship_power = 32000000L;
   ships->ship_shield = 36000000L;
   ships->ship_hull = 2000;
   ships->ship_universe = 0;

/*
   If the cops are being created with the universe, set them in
   the lower right hand side of the default universe, otherwise
   set them randomly in the universe 12.
*/

   if (the_stat == 0) {
      ships->ship_xpos = xsize - 10L;
      ships->ship_ypos = ysize - 10L;
      ships->ship_universe = (UC)12;
   }
   else {
      ships->ship_xpos = arandom(10L, xsize - 10);
      ships->ship_ypos = arandom(10L, ysize - 10);
   }

   ships->ship_credits = 0L;
   ships->ship_warp = 777;
   ships->total_kills = 3;
   ships->attack_sleds = 1000;
   strcpy(ships->ship_name, "<GP>");
   strcpy(ships->ship_pass, "The Galactic Police");
   strcpy(ships->ship_person, "The Galactic Police");

/*
   If the cops come back because they have been destroyed, they don't
   get to own any planets. If the cops are being created along with the
   universe, give them some planets to start out with.
*/

   if (the_stat == 0) {
      ships->planets_owned = xsize / 12;
      while (ships->planets_owned > OWNABLE) {
         ships->planets_owned /= 2;
      }
   }
   else {
      ships->planets_owned = 0;
   }

   if (the_stat == 0) {
      actually_found = find_some_available_planets(ships->planets_owned);
      ships->planets_owned = actually_found;
   }
   else {
      for (count = 0; count < OWNABLE; count++) {
         ships->owned_planets[count] = (short)NIL;
      }
   }

   for (count = 0; count < 5; count++) {
      strcpy(ships->allies[count], "+-+-");
   }

   for (count = 0; count < 15; count++) {
      ships->sled_xpos[count] = (short)NIL;
      ships->sled_ypos[count] = (short)NIL;
      ships->sled_universe[count] = 0;
      ships->sled_swarm[count] = (short)NIL;
      ships->sled_power[count] = 0;
   }

   ships->sick_bay = 0;
   ships->bounty = 0L;
   strcpy(ships->who_destroyed, "NONE");

/*
   If the status is 1, then we need to get the cops in-memory elements
   updated for a data structure.

   The FALSE asks the plug function to _not_ allocate the memory for
   this ship as when it was destroyed we didn't deallocate it!
*/

   if (the_stat == 1) {
      plug_this_ship(0, FALSE);
   }

   write_user();
   rewind(aship);

   if (the_stat > 0) {
      user_number = hold_user;
      read_user();
   }
}

/* **********************************************************************
   * What we need to do is have the cops buy things. In this order:	*
   * 									*
   * 1. Power up to the limit / 2					*
   * 2. Hull patches up to the limit					*
   * 3. Torpedos up to the limit / 2					*
   * 4. Cloaking devices up to the limit				*
   * 5. Money to ships credits						*
   *									*
   * The cost of the various items are determined in these functions	*
   * and have no dealings with the cost that the players have to pay.	*
   *									*
   ********************************************************************** */

void cops_buy_things_too(float the_amount_of_taxes)
{
   short cop_cost_energy, cop_cost_hull, cop_cost_torp, cop_cost_cloak;
   unsigned long total_to_buy;
   char sysop_output[100];

   the_rnd = arandom(10L, 15L);
   cop_cost_energy = (int) (1 * the_rnd) + 15;
   cop_cost_hull = (int) (101 * the_rnd) + 100;
   cop_cost_torp = (int) (17 * the_rnd) + 11;
   cop_cost_cloak = (int) (178 * the_rnd) + 121;

   read_enemy(0);
   enemy->ship_credits += the_amount_of_taxes;

/*
   Find out how many we can buy without going over our available
   credits ad without exceeding the values we want to have. The remainder
   of the cash goes to the credit bank.

   Notice that we recurse if there is still some money left worth
   taking the time to see about buying things.
*/

start_buying_again:
   if (enemy->ship_power < to_limits[1] / (long)2L) {
      total_to_buy = enemy->ship_credits / cop_cost_energy;
      if (total_to_buy > 0) {

         if (total_to_buy + enemy->ship_power > to_limits[1] / (long)2L) {
            total_to_buy = (to_limits[1] / (long)2L) - enemy->ship_power;
	 }

         enemy->ship_power += total_to_buy;
         enemy->ship_credits -= (total_to_buy * cop_cost_energy);
	 the_amount_of_taxes -= (total_to_buy * cop_cost_energy);

	 if (is_redirected == 1) {

            sprintf(sysop_output,
               "SYSOP: Cops bought %ld units of energy\n\r", total_to_buy);

	    fputs(sysop_output, stderr);
	 }
      }
   }

   if (enemy->ship_hull < to_limits[6]) {
      total_to_buy = enemy->ship_credits / cop_cost_hull;

      if (total_to_buy + enemy->ship_hull > to_limits[6]) {
         total_to_buy = to_limits[6] - enemy->ship_hull;
      }

      if (total_to_buy > 0) {
         enemy->ship_hull += total_to_buy;
         enemy->ship_credits -= (total_to_buy * cop_cost_hull);
	 the_amount_of_taxes -= (total_to_buy * cop_cost_hull);

         if (is_redirected == 1) {

            sprintf(sysop_output,
               "SYSOP: Cops bought %ld additional hull patches\n\r", total_to_buy);

	    fputs(sysop_output, stderr);
	 }
      }
   }

   if (enemy->ship_torp < to_limits[9] / (long)2L) {
      total_to_buy = enemy->ship_credits / cop_cost_torp;

      if (total_to_buy + enemy->ship_torp > to_limits[9] / (long)2L) {
         total_to_buy = (to_limits[9] / (long)2L) - enemy->ship_torp;
      }

      if (total_to_buy > 0) {
         enemy->ship_torp += total_to_buy;
         enemy->ship_credits -= (total_to_buy * cop_cost_torp);
	 the_amount_of_taxes -= (total_to_buy * cop_cost_torp);

         if (is_redirected == 1) {
            sprintf(sysop_output,
               "SYSOP: Cops bought %ld additional torpedos\n\r", total_to_buy);

	    fputs(sysop_output, stderr);
	 }
      }
   }

   if (enemy->ship_cloak < to_limits[8] + 10) {
      total_to_buy = enemy->ship_credits / cop_cost_cloak;

      if (total_to_buy + enemy->ship_cloak > to_limits[8]) {
         total_to_buy = to_limits[8] - enemy->ship_cloak;
      }

      if (total_to_buy > 0) {
         enemy->ship_cloak += total_to_buy;
         enemy->ship_credits -= (total_to_buy * cop_cost_cloak);
	 the_amount_of_taxes -= (total_to_buy * cop_cost_cloak);

	 if (is_redirected == 1) {

            sprintf(sysop_output,
               "SYSOP: Cops bought %ld cloaking device upgrades\n\r", total_to_buy);

	    fputs(sysop_output, stderr);
	 }
      }
   }

   write_enemy(0);

   if (enemy->ship_credits > 100000) {
      goto start_buying_again;
   }
}

/* **********************************************************************
   * Every now and then, have the cops move.				*
   *									*
   ********************************************************************** */

void move_the_cops_around(void)
{
   char x_diff, y_diff;
   char sysop_output[80];

   read_enemy(0);

   if (arandom(1L, 10L) < 5) {
      enemy->ship_xpos -= arandom(1L, 10L);
      enemy->ship_ypos -= arandom(1L, 10L);
   }
   else {
      enemy->ship_xpos += arandom(1L, 10L);
      enemy->ship_ypos += arandom(1L, 10L);
   }

   if (enemy->ship_xpos < 1) {
      enemy->ship_xpos = 10;
   }
   else if (enemy->ship_xpos >= xsize) {
      enemy->ship_xpos = xsize - 10;
   }

   if (enemy->ship_ypos < 1) {
      enemy->ship_ypos = 10;
   }
   else if (enemy->ship_ypos >= ysize) {
      enemy->ship_ypos = ysize - 10;
   }

/*
   To give the SysOp something to watch...
*/

   if (is_redirected == 1) {
      sprintf(sysop_output,
         "SYSOP: Cops are on the move from [%ld-%ld] to [%ld-%ld]\n\r",
	 hold[0]->sxpos, hold[0]->sypos,
         enemy->ship_xpos, enemy->ship_ypos);
      fputs(sysop_output, stderr);
   }

   hold[0]->sxpos = enemy->ship_xpos;
   hold[0]->sypos = enemy->ship_ypos;
   write_enemy(0);
}

/* **********************************************************************
   * If the player has a lot of money and has not bought plague from	*
   * them, have the pirates fire at them.				*
   *									*
   ********************************************************************** */

void pirates_fight_back(char first_blood)
{
   long hit_value;
         
   if (ships->plague_flag > 9 || ships->ship_credits < 100000L)
      if (! first_blood)
         return;

   c_out(LIGHTRED, "The pirate ship is %s!\n\r",
      first_blood ? "returning fire" : "attacking");

   hit_value = arandom(10000L, 90000L);
   if (hit_value > ships->ship_shield) hit_value = ships->ship_shield;

   c_out(LIGHTRED, "%ld unit hit on shielding grid %d!",
      hit_value, (short)arandom(1L, 100L));

   ships->ship_shield -= hit_value;
   c_out(LIGHTBLUE, " %ld units remain\n\r", ships->ship_shield);
   write_user();

   if (ships->ship_shield > 0) return;
   the_percent = first_blood ? 20 : 10;
   make_some_damage(user_number, (short)NIL);
   write_user();
}

