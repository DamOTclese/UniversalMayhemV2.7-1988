
/* **********************************************************************
   * sled.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * This module maintains attack sled deployment and pickups for the	*
   * attack sled swarms.						*
   *									*
   * In the early versions of Universal Mayhem, there were no attack	*
   * sleds. The TradeWars game, which I learned about six or seven	*
   * months after this projects inception, have attack ships that must	*
   * be defeated in order to enter a section of space. I took it a	*
   * different direction and made them defensive in case a ship comes	*
   * into a section of space that needs to be guarded, such as a base	*
   * or planets that the person likes.					*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
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

   extern long xpos, ypos;
   extern UC zpos;
   extern unsigned short the_percent;
   extern unsigned short docked;
   extern unsigned short close_swarms[TOTAL_PLAYERS];
   extern char *record;
   extern short user_number;
   extern short swarm_count;

void perform_sled(void)
{
   short sleds;
   short sled_count, sitting_on, to_add, next_available;

   read_user();

   for (sleds = 0, sled_count = 0, sitting_on = (short)NIL; sleds < 15; sleds++) {
      if (ships->sled_xpos[sleds] != (short)NIL) {
	 sled_count++;
	 c_out(YELLOW,
	    "Swarm %d, [%05d-%05d] {Universe %d} has %03d sleds. Power average %03d\n\r",
            sleds,
            ships->sled_xpos[sleds],
            ships->sled_ypos[sleds],
            ships->sled_universe[sleds],
            ships->sled_swarm[sleds],
            ships->sled_power[sleds]);
         if (ships->sled_xpos[sleds] == xpos &&
             ships->sled_ypos[sleds] == ypos &&
             ships->sled_universe[sleds] == zpos) {
             sitting_on = sleds;
	 }
      }
   }

/*
   If we are not sitting on a swarm point, see what the next available
   swarp group number is.
*/

   if (sitting_on == (short)NIL) {
      next_available = (short)NIL;
      for (sleds = 0; sleds < 15; sleds++) {
         if (ships->sled_xpos[sleds] == (short)NIL && next_available == (short)NIL) {
	    next_available = sleds;
	 }
      }
   }

   if (sled_count == (short)NIL) {
      c_out(WHITE, "There are no deployed attack swarms\n\r\n\r");
   }
   else {
      c_out(WHITE, "--------------------------------------------------------\n\r\n\r");
   }

   c_out(YELLOW, "There are %d attack fighters aboard\n\r\n\r", ships->attack_sleds);

/*
   If there are no attack sleds in the ship or deployed, then simply
   return without doing anything. Don't offer any options.
*/

   if (sled_count == 0 && ships->attack_sleds == 0) {
      return;
   }

/*
   If we are sitting on a swarm point, we ask if player wants to
   deploy additional sleds at this point. We also ask if player wants
   to recall some or all of the attack sleds.

   If the point we are in does not contain any attack sleds, then
   we simply ask how many attack sleds to deploy at this point.
*/

   if (sitting_on == (short)NIL) {
      if (next_available == (short)NIL) {
         c_out(YELLOW, "You already have 15 attack sled swarms deployed\n\r");
	 return;
      }

      if (docked == 1) {
	 c_out(LIGHTRED, "You can not deploy an attack swarm on a planet!\n\r");
	 return;
      }

      c_out(WHITE, "Deploy how many attack sleds? (0 for none): ");
      timed_input(0);
      to_add = atoi(record);
      if (to_add == 0) {
	 return;
      }
      if (to_add > ships->attack_sleds) {
	 c_out(LIGHTRED, "You don't have that many attack sleds to deploy!\n\r");
	 return;
      }
      ships->sled_xpos[next_available] = xpos;
      ships->sled_ypos[next_available] = ypos;
      ships->sled_universe[next_available] = zpos;
      hold[user_number]->xswarm[next_available] = xpos;
      hold[user_number]->yswarm[next_available] = ypos;
      hold[user_number]->swarm_universe[next_available] = zpos;
      ships->sled_swarm[next_available] = to_add;
      ships->sled_power[next_available] = 100;
      ships->attack_sleds -= to_add;
      write_user();
      c_out(WHITE, "Attack sleds for swarp group %d deployed\n\r", next_available);
      return;
   }

   c_out(WHITE, "(D)eploy additional sleds, (R)ecall attack sleds: ");
   timed_input(0);
   ucase(record);

/*
   Allow the deployment of additional attack sleds.
*/

   if (record[0] == 'D') {
      if (ships->attack_sleds == 0) {
	 c_out(LIGHTRED, "You don't have any attack sleds on board!\n\r");
	 return;
      }
      c_out(WHITE, "How many more attack sleds to add to swarm group %d?",
	 sitting_on);

      timed_input(0);
      to_add = atoi(record);

      if (to_add == 0) {
	 return;
      }

      if (to_add > ships->attack_sleds) {
	 c_out(LIGHTRED, "You don't have that many attack sleds on board!\n\r");
	 return;
      }

      if (ships->sled_swarm[sitting_on] + to_add > 8000) {
	 c_out(LIGHTRED, "Swarm can't maintain that many sleds!\n\r");
	 return;
      }

      ships->sled_swarm[sitting_on] += to_add;
      ships->attack_sleds -= to_add;
      ships->sled_power[sitting_on] += 75;
      ships->sled_power[sitting_on] += arandom(20L, 50L);
      write_user();
      c_out(WHITE, "Additional sleds have been deployed in attack group %d\n\r",
	 sitting_on);
      return;
   }

/*
   Allow the recall of all or some of the attack sleds
*/

   if (record[0] != 'R') {
      return;
   }

   c_out(WHITE, "Recall how many of the %d attack sleds? ",
      ships->sled_swarm[sitting_on]);

   timed_input(0);
   to_add = atoi(record);

   if (to_add == 0) {
      return;
   }

   if (to_add > ships->sled_swarm[sitting_on]) {
      c_out(LIGHTRED, "There aren't that many sleds at this sector!\n\r");
      return;
   }

   if (ships->attack_sleds + to_add > 10000) {
      c_out(LIGHTRED, "Ship can not maintain that many attack sleds at once!\n\r");
      return;
   }

   if (to_add == ships->sled_swarm[sitting_on]) {
      c_out(WHITE, "Recalling entire attack swarm group\n\r");
      ships->sled_xpos[sitting_on] = (short)NIL;
      ships->sled_ypos[sitting_on] = (short)NIL;
      ships->sled_universe[sitting_on] = 0;
      hold[user_number]->xswarm[sitting_on] = (short)NIL;
      hold[user_number]->yswarm[sitting_on] = (short)NIL;
      hold[user_number]->swarm_universe[sitting_on] = 0;
      ships->sled_power[sitting_on] = (short)NIL;
      ships->sled_swarm[sitting_on] = 0;
   }
   else {
      c_out(WHITE, "Recalling some of the attack group\n\r");
      ships->sled_power[sitting_on] += 75;
      ships->sled_swarm[sitting_on] -= to_add;
   }

   ships->attack_sleds += to_add;
   write_user();
}

/* **********************************************************************
   * This function is called just before a command is to be entered. It	*
   * also is called only when there are swarm ships in the area.	*
   *									*
   * When attacking, take some three percent of the available power 	*
   * away from a random number of the sleds unless there are less than	*
   * 10 sleds in which case, 10 percent of the power is taken.		*
   *									*
   ********************************************************************** */

void swarm_attack(long thex, long they, short on_ship)
{
   short s_count, the_ship, t_loop;

   for (s_count = 0; s_count < swarm_count; s_count++) {
      the_ship = close_swarms[s_count];
      for (t_loop = 0; t_loop < 15; t_loop++) {
         if (hold[the_ship]->xswarm[t_loop] != (short)NIL &&
	     hold[the_ship]->xswarm[t_loop] > thex - 5 &&
   	     hold[the_ship]->xswarm[t_loop] < thex + 5 &&
	     hold[the_ship]->yswarm[t_loop] > they - 9 &&
	     hold[the_ship]->yswarm[t_loop] < they + 9 &&
             hold[the_ship]->swarm_universe[t_loop] == zpos) {
             fire_from_swarm(the_ship, t_loop, on_ship);
         }
      }
   }
}

/* **********************************************************************
   * Ship 'the_ship' has swarm group 't_loop' in the area and it is	*
   * attacking.								*
   *									*
   * It has been shown that attack sleds _can_ destroy a ship.		*
   *									*
   ********************************************************************** */

void fire_from_swarm(short the_ship, short t_loop, short on_ship)
{
   long total_hits;
   short the_drop;

   if (check_mutual_alliance(the_ship, on_ship)) {
      return;
   }

   read_enemy(the_ship);
   total_hits = (long) enemy->sled_swarm[t_loop] * (long) arandom(200L, 500L);

   if (on_ship != user_number) {
      swarm_on_automated(the_ship, t_loop, on_ship, total_hits);
      return;
   }

   c_out(LIGHTRED, "* * * Swarm group from %s attacking %s, %ld total damage! * * *\n\r",
      hold[the_ship]->names, hold[on_ship]->names, total_hits);

   ships->ship_shield -= total_hits;

/*
   If the ship is damaged enough, it will be destroyed. At that time,
   the 'make_some_damage()' function will hand over any planets.
*/

   if (ships->ship_shield < 0) {
      ships->ship_shield = 0;
      the_percent = 10;
      make_some_damage(user_number, the_ship);
      return;
   }

   write_user();

   if (enemy->sled_swarm[t_loop] < 11) {
      the_drop = 10;
   }
   else {
      the_drop = (short) arandom(5L, 15L);
   }

   enemy->sled_power[t_loop] -= the_drop;

   c_out(LIGHTGREEN, "* * * Attack swarm %s power reserves down %d percent!\n\r",
      hold[the_ship]->names, the_drop);

   if (enemy->sled_power[t_loop] < 15) {
      drop_sled_group(the_ship, t_loop);
   }

   write_enemy(the_ship);
}

/* **********************************************************************
   * The sled should turn on the attacking/automated ship		*
   *                                                                    *
   * 'the_ship' is the owner of the swarm                               *
   * 't_loop' is the swarm group doing the attacking                    *
   * 'on_ship' is the automated ship being attacked                     *
   * 'total_hits' is the amount of hit value                            *
   *									*
   ********************************************************************** */

void swarm_on_automated(short the_ship, short t_loop, short on_ship, long total_hits)
{
   short the_drop;

   if (check_mutual_alliance(the_ship, on_ship)) {
      return;
   }

   read_enemy(on_ship);
   enemy->ship_shield -= total_hits;

   if (enemy->ship_shield < 0) {
      enemy->ship_shield = 0;
   }

   c_out(LIGHTRED, "* * * Attack sleds %s attacking automated ship %s * * *\n\r",
      hold[the_ship]->names, hold[on_ship]->names);

   write_enemy(on_ship);
   read_enemy(the_ship);

   if (enemy->sled_swarm[t_loop] < 110) {
      the_drop = arandom(1L, 3L);
   }
   else {
      the_drop = (short) arandom(3L, 5L);
   }

   enemy->sled_power[t_loop] -= the_drop;

   c_out(LIGHTGREEN, "* * * Attack swarm %s power reserves down %d percent!\n\r",
      hold[the_ship]->names, the_drop);

   if (enemy->sled_power[t_loop] < 5) {
      drop_sled_group(the_ship, t_loop);
   }

   write_enemy(the_ship);
}

/* **********************************************************************
   * The attack sled group swarm has run out of enough power to run.	*
   * Because of this, destroy it and issue a message.			*
   *									*
   ********************************************************************** */

void drop_sled_group(short the_ship, short t_loop)
{
   enemy->sled_xpos[t_loop] = enemy->sled_ypos[t_loop] = (short)NIL;
   enemy->sled_universe[t_loop] = 0;
   enemy->sled_swarm[t_loop] = enemy->sled_power[t_loop] = 0;
   hold[the_ship]->xswarm[t_loop] = hold[the_ship]->yswarm[t_loop] = (short)NIL;
   hold[the_ship]->swarm_universe[t_loop] = 0;

   c_out(LIGHTRED, "* * * Attack swarm %s has run out of power and is adrift!\n\r",
      hold[the_ship]->names);
}

/* **********************************************************************
   * If the ships have mutual alliances, return a TRUE, otherwise FALSE	*
   * is returned. Swarms may not attack mutual alliances.		*
   *									*
   ********************************************************************** */

unsigned char check_mutual_alliance(short the_ship, short on_ship)
{
   short the_test;

   if (! Good_Hold(on_ship)) return(FALSE);
   read_enemy(the_ship);

   for (the_test = 0; the_test < 5; the_test++) {
      if (! strcmp(enemy->allies[the_test], hold[on_ship]->names)) {
         return(check_other_side(the_ship, on_ship));
      }
   }

   return(FALSE);
}

/* **********************************************************************
   * Attacking ship is allied to ship, now check the other ship to see	*
   * if the feeling is mutual.						*
   *									*
   ********************************************************************** */

unsigned char check_other_side(short the_ship, short on_ship)
{
   short the_test;

   read_enemy(on_ship);

   for (the_test = 0; the_test < 5; the_test++) {
      if (! strcmp(enemy->allies[the_test], hold[the_ship]->names)) {
	 return(TRUE);
      }
   }
   return(FALSE);
}
