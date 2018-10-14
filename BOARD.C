
/* **********************************************************************
   * board.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   ********************************************************************** */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "function.h"
#include "boarding.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long to_limits[12];
   extern long xpos, ypos;
   extern UC zpos;
   extern long count;
   extern short players;
   extern short user_number;
   extern char extended_board;

   static short check_friend_base();
   static short is_a_friend();
   static short is_a_mutual_friend();

/* **********************************************************************
   * Check to see if the current ship is on an enemy ship or an enemy   *
   * base. We don't check to see if there are more than one ship or	*
   * base at this location. This could be done, however, simply by	*
   * setting a flag to 1, andother to the ships nummber, and then	*
   * continueing with the loop. As ships are found, increment the flag.	*
   * At the end of the loop, we would know how many ships are present	*
   * and who is the first one to be boarded.				*
   *									*
   ********************************************************************** */

void perform_board(void)
{
   for (count = 0; count < players; count++) {
      if (Good_Hold(count) && count != user_number) {
         if (hold[count]->sxpos == xpos && hold[count]->sypos == ypos &&
            hold[count]->szpos == zpos) {
            board_ship();
            return;
         }
      }
   }

   for (count = 0; count < players; count++) {
      if (Good_Hold(count) && count != user_number) {
         if (hold[count]->bxpos == xpos && hold[count]->bypos == ypos &&
            hold[count]->bzpos == zpos) {
            board_base();
            return;
         }
      }
   }

   c_out(WHITE, "Board what? There isn't anything under you!\n\r");
   return;
}

/* **********************************************************************
   * Board the ship. Check to see if the shields are down. If not, then	*
   * the ship may not be boarded. After that, check to see if there is	*
   * at least a little hole in the ships hull. If not, it can not be	*
   * boarded. 								*
   *									*
   * If the ship is boardable, take everything that isn't nailed down.	*
   * In the old version, shuttles and warp drives were thrown away if	*
   * the class was less than or equal to the current ships-> This is to  *
   * change in this version. The item taken will ADD to the current	*
   * ships values and classes.						*
   *									*
   ********************************************************************** */

void board_ship(void)
{
   UL took_power, took_credits, took_cargo, took_torps;

   read_enemy(count);
   if (enemy->ship_shield > 0) {
      c_out(WHITE, "Enemy ships shields are not fully down!\n\r");
      return;
   }

   if (enemy->ship_hull > 99) {
      c_out(WHITE, "You need to make a hole in the enemy ships hull before you\n\r");
      c_out(WHITE, "can attach a shuttle craft to the ship.  Send over a torp!\n\r");
      return;
   }

   if (ships->ship_shuttle == 0) {
      c_out(WHITE, "Board the enemy ship with what? You don't have shuttle!\n\r");
      return;
   }

   if (ships->ship_crew < 10) {
      c_out(WHITE, "Board the enemy ship with what crew? You need at least\n\r");
      c_out(WHITE, "ten loyal crew members.\n\r");
      return;
   }

/*
   Now that the active ship is boarding the disabled ship, let's
   make sure that the ships capacity is not overloaded. We do this by
   checking against maximum values and spaceing everything we don't
   want. In early testing, it was shown that power and credits also
   needed to be tested. We test credits by using the same value for
   the power!
*/

   if (extended_board) {
      boarding_party(TRUE, count);
   }

   c_out(LIGHTBLUE, "Boarding enemy ship: %s\n\r", enemy->ship_name);
   c_out(LIGHTRED, "------------------------\n\r");

   took_power = enemy->ship_power;

   if (to_limits[1] < ships->ship_power + enemy->ship_power) {
      c_out(LIGHTRED, "Excess power drained to space...\n\r");
      ships->ship_power = to_limits[1];
   }
   else {
      ships->ship_power += enemy->ship_power;
      c_out(LIGHTBLUE, "Enemy ship power:  %ld\n\r", enemy->ship_power);
   }

   took_credits = enemy->ship_credits;

   if (to_limits[1] < ships->ship_credits + enemy->ship_credits) {
      c_out(LIGHTRED, "Excess credits went to the poor...\n\r");
      ships->ship_credits = to_limits[1];
   }
   else {
      ships->ship_credits += enemy->ship_credits;
      c_out(LIGHTBLUE, "Enemy credits:     %ld\n\r", enemy->ship_credits);
   }

   if (to_limits[4] < ships->ship_warp + enemy->ship_warp) {
      c_out(LIGHTRED, "Spaceing unworkable warp drive...\n\r");
      ships->ship_warp = to_limits[4];
   }
   else {
      ships->ship_warp += enemy->ship_warp;
      c_out(LIGHTBLUE, "Enemy warp drive:  %d\n\r", enemy->ship_warp);
   }

   took_cargo = enemy->ship_cargo;

   if (to_limits[2] < ships->ship_cargo + enemy->ship_cargo) {
      c_out(LIGHTRED, "Dumping excess cargo...\n\r");
      ships->ship_cargo = to_limits[2];
   }
   else {
      ships->ship_cargo += enemy->ship_cargo;
      c_out(LIGHTBLUE, "Enemy cargo:       %d\n\r", enemy->ship_cargo);
   }

/*
   A bug that I don't really want to search for occures here. When
   boarding a ship with 32000 units of cargo and having 32000 units
   of cargo on board, the ships cargo goes negative.
*/

   if (ships->ship_cargo < 1) {
      ships->ship_cargo = to_limits[2];
   }

   if (to_limits[5] < ships->ship_shuttle + enemy->ship_shuttle) {
      c_out(LIGHTRED, "Discarding unworkable shuttle craft...\n\r");
      ships->ship_shuttle = to_limits[5];
   }
   else {
      ships->ship_shuttle += enemy->ship_shuttle;
      c_out(LIGHTBLUE, "Enemy shuttle:     %d\n\r", enemy->ship_shuttle);
   }

   if (to_limits[6] < ships->ship_hull + enemy->ship_hull) {
      c_out(LIGHTRED, "Throwing away excess hull patches...\n\r");
      ships->ship_hull = to_limits[6];
   }
   else {
      ships->ship_hull += enemy->ship_hull;
      c_out(LIGHTBLUE, "Enemy ship hull:   %d\n\r", enemy->ship_hull);
   }

   if (to_limits[8] < ships->ship_cloak + enemy->ship_cloak) {
      c_out(LIGHTRED, "Spaceing unworkable cloaking device...\n\r");
      ships->ship_cloak = to_limits[8];
   }
   else {
      ships->ship_cloak += enemy->ship_cloak;
      c_out(LIGHTBLUE, "Enemy ship cloak:  %d\n\r", enemy->ship_cloak);
   }

   if (to_limits[7] < ships->ship_sensor + enemy->ship_sensor) {
      c_out(LIGHTRED, "Trashing unworkable sensor array...\n\r");
      ships->ship_sensor = to_limits[7];
   }
   else {
      ships->ship_sensor += enemy->ship_sensor;
      c_out(LIGHTBLUE, "Enemy sensor:      %d\n\r", enemy->ship_sensor);
   }

   took_torps = enemy->ship_torp;

   if (to_limits[9] < ships->ship_torp + enemy->ship_torp) {
      c_out(LIGHTRED, "Can't find room for all the torpedos...\n\r");
      ships->ship_torp = to_limits[9];
   }
   else {
      ships->ship_torp += enemy->ship_torp;
      c_out(LIGHTBLUE, "Enemy ship torps:  %d\n\r", enemy->ship_torp);
   }

   ships->last_at_status = 5;
   strcpy(ships->last_at_who, enemy->ship_name);
   write_user();

/*
   Make the boarded ship crippled. Give it _something_, however,
   so that it might consider trying to live to fight another day!
*/

   mail_board(count, TRUE, took_power, took_credits, took_cargo, took_torps, FALSE);

   enemy->ship_power = 121;
   enemy->ship_warp = 1;
   enemy->ship_credits = 10;
   enemy->ship_shuttle = 0;
   enemy->ship_hull = 31;
   enemy->ship_cloak = 0;
   enemy->ship_sensor = 0;
   enemy->ship_torp = 1;
   write_enemy((unsigned char) count);
   c_out(LIGHTBLUE, "\n\rEnemy ship is drifting...\n\r");

   if (enemy->base_xpos != 0 && enemy->base_ypos != 0) {
      c_out(YELLOW,
	 "Enemy ship computers yield base location: %ld-%ld {Universe :%d}\n\r",
         enemy->base_xpos, enemy->base_ypos, enemy->base_universe);
   }
}

/* **********************************************************************
   * We are on an enemy base. Board it if the shields are down. There 	*
   * is a fight which must take place between the crew members of the	*
   * attacking ship and those of the defending base.			*
   *									*
   ********************************************************************** */

void board_base(void)
{
   UL took_credits, took_cargo;

   read_enemy(count);

   if (ships->base_xpos == 0 && ships->base_ypos == 0) {
      c_out(WHITE, "Before you board an enemy base, you must make a base\n\r");
      c_out(WHITE, "of your own to hold all of the pirated cloaks and cargo!\n\r");
      return;
   }

   if (ships->ship_crew < 40) {
      c_out(WHITE, "You can't attempt a boarding with less than 40 crew members.\n\r");
      return;
   }

   if (check_friend_base((short) count)) {
      return;
   }

   if (enemy->base_shield > 0) {
      c_out(WHITE, "Enemy base %s shields are not down!\n\r",
         hold[count]->names);
      return;
   }

   if (extended_board) {
      boarding_party(FALSE, count);
   }

   ships->ship_crew -= enemy->base_crew;

   if (ships->ship_crew < 40) {
      ships->ship_crew = 40;
      c_out(WHITE, "Boarding party repelled! 40 survivers return to sick bay.\n\r");
      write_user();
      write_enemy((unsigned char) count);
      return;
   }

   c_out(LIGHTBLUE, "Boarding party successfull: %ld people killed!\n\r", enemy->base_crew);
   c_out(LIGHTBLUE, "%ld Credits from enemy base to your ship.\n\r", enemy->base_credits);
   ships->ship_credits += enemy->base_credits;
   took_credits = enemy->base_credits;

   c_out(LIGHTBLUE, "%ld Cargo units from enemy base to your base.\n\r", enemy->base_cargo);
   ships->base_cargo += enemy->base_cargo;
   took_cargo = enemy->base_cargo;

   c_out(LIGHTBLUE, "%d Cloaks from enemy base to your base.\n\r", enemy->base_cloak);
   ships->base_cloak += enemy->base_cloak;
   write_user();

   enemy->base_credits = 0;
   enemy->base_cargo = 0;
   enemy->base_crew = 0;
   enemy->base_cloak = 0;
   write_enemy((unsigned char) count);
   mail_board(count, FALSE, 0l, took_credits, took_cargo, 0l, FALSE);
}

/* **********************************************************************
   * See if the base that the ship wants to board is a friend. If not,	*
   * return a false, otherwise call the next routine.			*
   *									*
   ********************************************************************** */

static short check_friend_base(short ships_number)
{
   short f_test;

   for (f_test = 0; f_test < 5; f_test++) {
      if (! strcmp(enemy->allies[f_test], hold[user_number]->names)) {
	 return(is_a_friend(ships_number));
      }
   }

   return(FALSE);
}

/* **********************************************************************
   * See if you are a friend to the base you are going to board. If	*
   * not, return false, otherwise call the next routine.		*
   *									*
   ********************************************************************** */

static short is_a_friend(short ships_number)
{
   short f_test;

   for (f_test = 0; f_test < 5; f_test++) {
      if (! strcmp(ships->allies[f_test], hold[ships_number]->names)) {
         return(is_a_mutual_friend(ships_number));
      }
   }

   return(FALSE);
}

/* **********************************************************************
   * Both ships are allies. Take 10% of everything.			*
   *									*
   ********************************************************************** */

static short is_a_mutual_friend(short ships_number)
{
   c_out(YELLOW, "You have boarded an allied ships base.\n\r");

   if (ships->base_xpos == 0 || ships->base_ypos == 0) {
      c_out(WHITE, "Before you board an allied base, you must make a base\n\r");
      c_out(WHITE, "of your own to hold all of the transfered cloaks and cargo!\n\r");
      return(TRUE);
   }

   c_out(WHITE, "%f Credits from allied base to your ship.\n\r",
      (float) enemy->base_credits * .1);

   ships->ship_credits += (float) (enemy->base_credits * .1);

   c_out(WHITE, "%f Cargo units from allied base to your base.\n\r",
      (float) enemy->base_cargo * .1);

   ships->base_cargo += (float) (enemy->base_cargo * .1);

   c_out(WHITE, "%f Cloaks from allied base to your base.\n\r",
      (float) enemy->base_cloak * .1);

   ships->base_cloak += (float) (enemy->base_cloak * .1);
   write_user();
   mail_board(count, FALSE, 0l, 0l, 0l, 0l, TRUE);

   enemy->base_credits -= (float) (enemy->base_credits * .1);
   enemy->base_cargo -= (float) (enemy->base_cargo * .1);
   enemy->base_crew -= (float) (enemy->base_crew * .1);
   enemy->base_cloak -= (float) (enemy->base_cloak * .1);
   strcpy(enemy->base_boarded, hold[user_number]->names);
   write_enemy((unsigned char) ships_number);
   return(TRUE);
}


