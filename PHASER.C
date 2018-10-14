
/* **********************************************************************
   * phaser.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * On 19/Feb/89, phasers were modified to cause damage to the enemy	*
   * ship after lower its hull. In the previous releases, phasers would	*
   * lower the enemy ships shields but cause no further damage.		*
   *									*
   * Phasers won't cause much damage but will cause some. Inversly,	*
   * the defending ship must attack better that it used to.		*
   *									*
   * Also, enemy ships would not return fire. This has been changed so	*
   * that the enemy ship will return fire when phasered. Interestingly	*
   * enough, the enemy ship will perform evasives that it didn't used	*
   * to do. When phasered, the distance of the evasive is a random	*
   * number from 1 to 5. When torped, it's only 1.			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdio.h"
#include "stdlib.h"
#include "function.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern *hit_unshielded_base[];
   extern long xpos, ypos;
   extern UC zpos;
   extern long count;
   extern unsigned short close_ship[TOTAL_PLAYERS];
   extern unsigned short close_base[TOTAL_PLAYERS];
   extern char *record;
   extern short ship_count;
   extern short base_count;
   extern short bail_out;
   extern short user_number;
   extern short rpt_loop;
   extern short the_rnd;
   extern char pirate;
   extern short pxpos, pypos;
   extern char total_pirate_count;

/* **********************************************************************
   * Define the local function prototypes.                              *
   *                                                                    *
   ********************************************************************** */

   static void phaser_pirate(UL hit_value);

/* **********************************************************************
   * Fire phasers at the enemy ship and/or base.			*
   *									*
   * We do a couple of things here. First off, we accept the amount to	*
   * fire after the 'fire' string in the variable 'record'. We must see	*
   * if the attacking ship has enough. Next, we check to see if we are	*
   * attacking a weak or weakened ship with phasers. If so, then the	*
   * Galactic police must come in and offer a warning shot. 		*
   *									*
   * Then, we divide the amount of power being fired by the number of	*
   * enemy ships in the area. This is done after deducting some random	*
   * value, makeing sure we don't go negative. The hit value is removed	*
   * from the defending ship. If the shields are negative, we set them	*
   * to 0. Further hits will cause no damage and torps must be used.	*
   *									*
   * The offending ship will not fire back but WILL redistribute power	*
   * and evade EVERY time, for up to four sectors.			*
   *									*
   * In future, the ship will fight back and allias who have a command	*
   * file or have declaired alliance will come to offer assistance.	*
   *									*
   ********************************************************************** */

void perform_fire(void)
{
   char *tpoint;
   UL hit_value;
   long the_rnd;

   if (strlen(record) < 5) {
      c_out(WHITE, "You must offer a value of energy to use in your phasers.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   tpoint = record;
   tpoint += 4;
   hit_value = atol(tpoint);

   if (strlen(tpoint) > 9 || hit_value < 1 || hit_value > 100000000L) {
      c_out(WHITE, "Invalid value for phasers: from 1 to 100000000.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->ship_power < hit_value) {
      c_out(WHITE, "You don't have that much power.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (ships->ship_power < 55) {
      c_out(WHITE, "Not enough power to energize phasers!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

/*
   After the validation of the phasers energy, check to see what's in
   the area. Call the routine that does so.
*/

   plug_close_objects(CLOSE_NO_ION);

   if (ship_count == 0 && base_count == 0 && pirate == 0) {
      c_out(WHITE,
         "Fire at what? There are no enemy ships, bases or pirates in the area!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   the_rnd = 0;
   ships->ship_power -= hit_value;
   write_user();

   if (hit_value < 55) {
      the_rnd = 0;
   }
   else {
      the_rnd = arandom(50L, (long)(hit_value / (ship_count + base_count + pirate)));
   }

   hit_value = (hit_value / (ship_count + base_count + pirate)) - the_rnd;

   if (ships->ship_morale < 20) {
       c_out(LIGHTRED, "Weapons systems Crew is not at station!\n\r");
       hit_value = 0;
   }

   for (the_rnd = 0; the_rnd < ship_count; the_rnd++) {
      if (! assisting_ship(close_ship[the_rnd])) {
         phaser_ship(close_ship[the_rnd], hit_value);
      }
   }

   for (the_rnd = 0; the_rnd < base_count; the_rnd++) {
      if (close_base[the_rnd] != user_number) {
         if (! assisting_ship(close_ship[the_rnd])) {
            phaser_base(close_base[the_rnd], hit_value);
         }
      }
   }

   if (pirate == 1) {
      phaser_pirate(hit_value);
   }

   if (ships->ship_morale < 20) {
      c_out(LIGHTRED, "Your crew DEMANDS shore leave!\n\r");
   }
}

/* **********************************************************************
   * Bring in the enemy record. Take the deductions to the enemy shield *
   * and display a message offering the status. 			*
   *									*
   ********************************************************************** */

void phaser_ship(unsigned int the_rnd, long hit_value)
{
   char i;
   short hold_count;

   count = hold_count = the_rnd;
   ship_attacking(hold_count);
   read_enemy(hold_count);
   enemy->ship_shield -= hit_value;

/*
   If the ship we are phasering was already fired on by this ship,
   add the number of millions of units of power used.

   If this ship is under attack by this ship for the first time,
   store the active ships name in the last-attacked-by field and
   set the phaser hit count to the number of millions of units used.
*/

   if (! strcmp(enemy->last_at_by, hold[user_number]->names)) {
      enemy->last_phaser_count += (float) hit_value / 1000000L;
   }
   else {
      strcpy(enemy->last_at_by, hold[user_number]->names);
      enemy->last_phaser_count = (float) hit_value / 1000000L;
   }

   strcpy(ships->last_at_who, hold[hold_count]->names);

   if (enemy->ship_shield < 1) {
      enemy->ship_shield = 0;
      ships->last_at_status = 2;
   }

   c_out(WHITE, "%ld unit hit on %s at [%ld-%ld] shields remaining: (%ld)\n\r",
      hit_value, enemy->ship_name, enemy->ship_xpos,
      enemy->ship_ypos, enemy->ship_shield);

   write_enemy(hold_count);

   for (i = 0; i < 5; i++) {
      if (! strcmp(ships->allies[i], hold[hold_count]->names)) {
         fire_at_friend(hold_count, i);
      }
   }

   return_torp((short) hold_count);
   return_phaser((short) hold_count);
}

/* **********************************************************************
   * Here is where we remove the power from an enemy base shield.	*
   *									*
   * If the ship has been destroyed yet the base is active, then we	*
   * want to make sure that the message offered is a good one.		*
   *									*
   ********************************************************************** */

void phaser_base(unsigned int the_rnd, long int hit_value)
{
   short angle_in, result;

   count = the_rnd;
   read_enemy(count);
   enemy->base_shield -= hit_value;

   if (enemy->base_shield < 1) {
      enemy->base_shield = 0;
      c_out(WHITE, "\n\r* * * %s * * *\n\r",
         hit_unshielded_base[enemy->base_hit_count++]);
   }

   c_out(WHITE, "%ld unit hit on base %s at [%ld-%ld] shield remaining: (%ld)\n\r",
      hit_value, enemy->ship_name, enemy->base_xpos,
      enemy->base_ypos, enemy->base_shield);

/*
   Ship _should_ get credit for base destruction
*/

   if (enemy->base_shield == 0 && enemy->base_hit_count == 10) {
      hold[count]->bxpos = hold[count]->bypos = hold[count]->bzpos = 0;
      enemy->base_xpos = enemy->base_ypos = 0;
      strcpy(enemy->base_death, hold[user_number]->names);
      enemy->base_shield = 0;
      enemy->base_credits = 0;
      enemy->base_cargo = 0;
      enemy->base_crew = 0;
      enemy->base_cloak = 0;
      enemy->base_hit_count = 0;
      write_enemy(count);
      read_user();
      ships->total_kills++;
      hold[user_number]->kills++;
      write_user();
      inform_kill(hold[user_number]->names, hold[count]->names, FALSE, 2);
      perform_stand(TRUE);
      return;
   }

   write_enemy(count);

/*
   The base is under attack. See if the ship is in the area. If so,
   then fire back at the ship attacking the base. If the ship is not
   in the area, move it into this area.
*/

   for (angle_in = 0; angle_in < ship_count; angle_in++) {
      if (close_ship[angle_in] == count) {
         if (enemy->ship_torp > 0) {
	    fire_back_torp(count);
	 }
         else if (enemy->ship_power > 100) {
            fire_back_phaser(count, (UL)0);
	 }
	 else {
	    c_out(LIGHTRED, "Enemy ship can't assist base. Out of power and torps!\n\r");
	 }
	 return;
      }
   }

   if (enemy->ship_xpos == 0 && enemy->ship_ypos == 0) {
      c_out(LIGHTRED, "Base mother ship is overdue and presumed dead.\n\r");
      return;
   }

   angle_in =
      compute_direction(count, user_number, X_WARP_WITHIN, Y_WARP_WITHIN);

   if (angle_in == (short)NIL) {
      return;
   }

   if (enemy->ship_warp < 2) {
      c_out(LIGHTRED, "Enemy ships captain can't warp in to assist base!\n\r");
      return;
   }

   result = warp_ship(count, angle_in, Y_WARP_WITHIN, 100, 1);

   if (result == (short)NIL) {
      c_out(LIGHTRED, "Enemy ships captain doesn't have enough power to warp in!\n\r");
      return;
   }

   c_out(LIGHTGREEN, "Enemy ship warping towards your area to assist its base!\n\r");
}

static void phaser_pirate(UL hit_value)
{
   int kill_it;
   kill_it = (int)arandom(1L, 10L);

   c_out(LIGHTRED,
      "\n\r* %ld unit hit on pirate ship at {%d-%d} *\n\r",
      hit_value, pxpos, pypos);

   if (hit_value > 10000l && kill_it == 5) {
      c_out(LIGHTRED, "Pirate ship destroyed! Bounty awarded!\n\r");
      pirate = 0;
      pxpos = pypos = (short)NIL;
      total_pirate_count = 0;
      read_user();
      hold[user_number]->kills = ++ships->total_kills;
      ships->ship_credits += 300000;
      write_user();
      return;
   }

   c_out(LIGHTRED, "---> Phaser energy ineffective! Incoming fire!\n\r");
   pirates_fight_back(TRUE);
   pirates_fight_back(TRUE);
}


