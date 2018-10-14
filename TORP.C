
/* **********************************************************************
   * torp.c                                                             *
   *                                                                    *
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *                                                                    *
   * When the active player destroyed some attack sleds, give some      *
   * message about the ways the swarm group commander handels the       *
   * problem.                                                           *
   *                                                                    *
   * AN IMPORTANT update was made around 26/Dec/88: When a ship gets    *
   * destroyed, all of the planets that belong to it are transfered to  *
   * the ship that destroyed it.                                        *
   *                                                                    *
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "universe.h"
#include "planets.h"
#include "holder.h"
#include "command.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * We need some messages for display when a sled gets fired into.	*
   *									*
   ********************************************************************** */

   char *destroyed_sleds[] = {
      "Now they're really mad!",
      "Swarm Group Commander is giving you the intergalatic hand sign!",
      "Swarm Commander was nearly hit by that last blast!\n\r",
      "Some of those fighter pilots had wives!",
      "They never knew what hit them!",
      "Those pilots will become heros of their mother ship!",
      "Those guys won't have to wory about running out of fuel!",
      "What a mess! You should SEE the body parts fly!",
      "I never saw such blood!",
      "Look at those suckers burn!",
      (char *)NULL
   } ;

/* **********************************************************************
   * When a ship fires at a base, we offer these messages starting from	*
   * the first to the last, letting the description of the damage get	*
   * worse as the damage gets worse.					*
   *									*
   ********************************************************************** */

   char *hit_unshielded_base[] = {
      "Enemy base shields are down!",
      "Minor damage to enemy base electrical subsystems!",
      "Electrical subsystems are out on enemy base!",
      "Massive damage to enemy base environmental systems!",
      "Emergency bulkheads slamming down all over enemy base!",
      "Explosions in enemy base armory!",
      "Computer system explosion aboard enemy base!",
      "Enemy base hull integrity impaired. Atmosphere spewing into space!",
      "Half of the enemy base is dripping metal!",
      "Enemy base destroyed!",
      (char *)NULL
   } ;

/* **********************************************************************
   * When the active ship destroys an enemy ship, one of these clever	*
   * remarks will be offered. There should be quite a few of these to	*
   * keep it interesting...						*
   *									*
   ********************************************************************** */

   char *destroyed_enemy_ship[] = {
      "That'll teach that sucker!",
      "Wow! What a bummer! That captain will surely come looking for you!",
      "It looks like you made an enemy!",
      "I think you might want to run and hide!",
      "Keep an eye on your back... There may be some revenge!",
      "Good heavens... Did I do that?",
      "Dead men tell no tails. That sucker should keep quiet for a while!",
      "That's the biggest explosion I have ever seen!",
      "Wow! Twisted metal all over the place!",
      "Your ship is peppered with enemy ship fragments. It's worth it!",
      "That ship's now a radioactive hazard to navagation!",
      "Sent them bastards right to hell!",
      "That captain won't have to worry about his paper work any more!",
      "That captains' commanding officer is probably pissed!",
      "Strike off another poor slob!",
      (char *)NULL
   } ;

/* **********************************************************************
   * When a planet is fired on                                          *
   *                                                                    *
   ********************************************************************** */

   static char *planet_fire[] = {
      "Massive shock waves scanned in planets atmosphere!",         /* 0 */
      "Planets polar ice caps have melted!",                        /* 1 */
      "Massive ground quakes on a global scale are being scanned!", /* 2 */
      "One fourth of the planets atmosphere has peeled away!",      /* 3 */
      "Most of the planets biosphere has been destroyed!",          /* 4 */
      "The planets seas are boiling!",                              /* 5 */
      "Two thirds of the planets atmoshpere has been blown away!",  /* 6 */
      "Mountains are melting all over the planet!",                 /* 7 */
      "There is no life left on the planet!",                       /* 8 */
      "Planet has been cracked wide open!"                          /* 9 */
   } ;

/* **********************************************************************
   * Part of the command files: When a ship bails out of an automation,	*
   * it may leave a decoy behind. These values determine if a decoy is	*
   * to be hit or not.							*
   *									*
   ********************************************************************** */

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern short decoy_xpos, decoy_ypos, decoy_value;
   extern long xsize, ysize;
   extern long xpos, ypos, txpos, typos;
   extern UC zpos;
   extern long tloop, count;
   extern unsigned short the_percent;
   extern unsigned short docked, base_docked;
   extern unsigned short close_ship[TOTAL_PLAYERS];
   extern unsigned short close_base[TOTAL_PLAYERS];
   extern unsigned short close_remotes[TOTAL_PLAYERS];
   extern unsigned short close_swarms[TOTAL_PLAYERS];
   extern char *record;
   extern char pirate;
   extern char total_pirate_count;
   extern short players;
   extern short ship_count;
   extern short base_count;
   extern short remote_count;
   extern short user_number;
   extern short the_rnd;
   extern short swarm_count;
   extern short directions[18];
   extern short bail_out;
   extern short rpt_loop;
   extern short pxpos, pypos;
   extern char torped_morale;
   extern char planet_hit;
   extern char drag_ship;

/* **********************************************************************
   * Define the functions that are used only in this module.            *
   *                                                                    *
   ********************************************************************** */

   static void hit_pirate(void);
   static char d_loop;

/* **********************************************************************
   * Torpedo something. You may torp anything that is reachable in the	*
   * current scan. This is planets, holes, stars, ships, bases, and	*
   * remote robots.							*
   *									*
   ********************************************************************** */

void perform_torp(void)
{
   int the_dir, the_vel, hit_value;
   char *cpointer;
   int rloop;

   read_user();

   if (ships->ship_torp < 1) {
      c_out(LIGHTRED, "You have no torpedos to use.\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (strlen(record) < 3 || strlen(record) > 4) {
      c_out(WHITE, "TORP command syntax is bad: %s\n\r", record);
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   if (docked == 1) {
      c_out(LIGHTRED, "Planitary government won't let you fire while in orbit!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return;
   }

   plug_close_objects(CLOSE_NO_ION);

   cpointer = record;
   cpointer++;

   the_dir = (*cpointer++) - 0x30;
   the_vel = (*cpointer++) - 0x30;

   if (strlen(record) == 4) {
      the_vel = (the_vel * 10) + (*cpointer++) - 0x30;
   }

/*
   If the moral of the crew is quite low, there will be problems!
*/

   if (ships->ship_morale <= 20 && !torped_morale) {
      the_dir = 5;
   }

   if (the_dir == 5) {
      if (ships->ship_morale <= 20) {
         c_out(LIGHTRED, "Torpedo dropped in the control room while being loaded!\n\r");
         the_percent = 10;
         torped_morale = TRUE;
      }
      else {
         c_out(LIGHTRED, "Explosion in torpedo control room!\n\r");
         the_percent = 80;
      }

      make_some_damage(user_number, (short)NIL);

      if (the_percent == 10) {
         c_out(LIGHTRED, "Next time issue your crew some SHORE LEAVE%!!!\n\r");
      }

      if (docked == 1) {
	 c_out(YELLOW, "Fragments raining down on the planet below!\n\r");
      }
      if (base_docked == 1) {
	 c_out(YELLOW, "Fragments peppering your base!\n\r");
      }

      return;
   }

   c_out(WHITE, "Torpedo away... Wait for progress...\n\r");
   hit_value = 755;
   ships->ship_torp--;
   write_user();
   txpos = xpos;
   typos = ypos;

   for (tloop = 0; tloop < the_vel; tloop++) {
      hit_value = hit_value - 50;
      txpos = txpos + directions[(the_dir - 1) * 2];
      typos = typos + directions[((the_dir - 1) * 2) + 1];

      if (txpos < 0 || txpos > xsize - 1 ||
         typos < 0 || typos > ysize - 1) {
         c_out(WHITE, "Torpedo has left the universe.\n\r");
         return;
      }

      c_out(WHITE, "(%ld-%ld)", txpos, typos);
      read_universe(txpos);

/*
   See if the torp hit a star
*/

      if (universe.star == typos) {
         hit_star(the_vel);
         return;
      }

/*
   See if the torp hit a mine
*/

      if (universe.mine == typos) {
         hit_mine(the_vel);
         return;
      }

/*
   See if the torpedo has entered a black hole. Notice that we dont
   return to the calling routine. This is so that we may cause the
   torp to continue on its way after it leaves the exit point of the
   black hole.
*/

      if (universe.black_hole == typos) {
         hit_hole();
         return;
      }

/*
   See if it hit an enemy ship. We scan for a ship first so that we can
   hit it if it's on a planet, rather than destroying the planet.
*/

      for (d_loop = 0; d_loop < ship_count; d_loop++) {
         if (hold[close_ship[d_loop]]->sxpos == txpos &&
            hold[close_ship[d_loop]]->sypos == typos &&
            hold[close_ship[d_loop]]->szpos == zpos) {
            hit_enemy(close_ship[d_loop], hit_value);
            return;
         }
      }

/*
   See if it hit a planet
*/

      for (d_loop = 0; d_loop < 4; d_loop++) {
         if (universe.planets[d_loop] == typos) {
	    hit_planet();
            return;
         }
      }

/*
   See if it hit a base
*/

      for (d_loop = 0; d_loop < base_count; d_loop++) {
         if (hold[close_base[d_loop]]->bxpos == txpos &&
            hold[close_base[d_loop]]->bypos == typos &&
            hold[close_base[d_loop]]->bzpos == zpos) {
            hit_base(hit_value);
            return;
         }
      }

/*
   See if it hit a remote robot sensor
*/

      for (d_loop = 0; d_loop < remote_count; d_loop++) {
	 for (rloop = 0; rloop < 10; rloop++) {
            if (hold[close_remotes[d_loop]]->xremotes[rloop] == txpos &&
               hold[close_remotes[d_loop]]->yremotes[rloop] == typos &&
               hold[close_remotes[d_loop]]->remote_universe[rloop] == zpos) {
               hit_remote(rloop);
               return;
            }
         }
      }

/*
   See if it hit a swarm attack sled
*/

      for (d_loop = 0; d_loop < swarm_count; d_loop++) {
         for (rloop = 0; rloop < 15; rloop++) {
            if (hold[close_swarms[d_loop]]->xswarm[rloop] == txpos &&
               hold[close_swarms[d_loop]]->yswarm[rloop] == typos &&
               hold[close_swarms[d_loop]]->swarm_universe[rloop] == zpos) {
	       hit_swarm(rloop);
	       return;
	    }
	 }
      }

/*
   Another possibility is that it fired on a decoy left by a
   bailing-out ship. If so, then decriment the class decoy used,
   (blow a hole in it), and fake a hit.

   If the shuttle is destroyed, then no decoy is left so remove it.
*/

      if (decoy_xpos == txpos && decoy_ypos == typos) {
	 if (hit_decoy()) {
  	    return;
	 }
      }

/*
   Another possibility: it fired on a pirate!
*/

      if (pxpos == txpos && pypos == typos) {
         hit_pirate();
         return;
      }
   }

   c_out(WHITE, "\n\rTorpedo lost\n\r");
}

/* **********************************************************************
   * The torp hit a star. This should cause the star to throw off a lot	*
   * of hydrogen and helium, (antimater torps). Cause some damage to	*
   * the ship doing the fireing.					*
   *									*
   ********************************************************************** */

void hit_star(char the_velocity)
{
   c_out(WHITE, "\n\rTorpedo enters star...");

   if (arandom(1L, 8000L) > 4000) {
      c_out(WHITE, " nothing happens...\n\r");
      return;
   }

   c_out(LIGHTRED, " star going nova!\n\r");

   if (the_velocity < 10) {
      the_percent = 10;
      make_some_damage(user_number, (short)NIL);
   }
   else {
      c_out(WHITE, " no damage to ships systems\n\r");
   }
}

/* **********************************************************************
   * Simply make some damage. The ship hit a mine.			*
   *									*
   ********************************************************************** */

void hit_mine(char the_velocity)
{
   c_out(LIGHTRED, "\n\rTorpedo had hit a mine!\n\r");

   if (the_velocity < 10) {
      the_percent = 10;
      make_some_damage(user_number, (short)NIL);
   }
   else {
      c_out(WHITE, "No damage to ships systems\n\r");
   }
}

/* **********************************************************************
   * The torpedo entered a black hole. If the black hole has a white 	*
   * hole as an exit point, use it, otherwise randomize the output.	*
   *									*
   ********************************************************************** */

void hit_hole(void)
{
   c_out(WHITE, "\n\rTorpedo enters black hole! ");
   if (universe.white_hole == 0) {
      txpos = arandom(1L, (long) xsize - 1);
      typos = arandom(1L, (long) ysize - 1);
   }
   else {
      txpos = universe.white_hole;
      read_universe(txpos);
      typos = universe.black_hole - 1;
   }
   c_out(WHITE, "(%ld-%ld) exits\n\r", txpos, typos);
}

/* **********************************************************************
   * Find the planet and see if it is protected. If not, destroy it!    *
   *                                                                    *
   ********************************************************************** */

void hit_planet(void)
{
   if (find_specific_planet(txpos, typos) == 0) {
      c_out(WHITE, "\n\rYou fired on a dead planet!\n\r");
      return;
   }

   read_planets(txpos);

   if (planets.protected != (char)NIL) {
      c_out(LIGHTGREEN, "\n\rTorpedo bounces off planetary defense shields.\n\r");
      ypos = ships->ship_ypos;
      return;
   }

   if (planets.technology == 0) {
      if (Good_Hold(planets.visited)) {
         c_out(YELLOW, "\n\rPlanet already destroyed by: %s\n\r",
            hold[planets.visited]->names);
      }
      else {
         log_error(114);
      }
      ypos = ships->ship_ypos;
      return;
   }

   planet_hit++;

   if (planet_hit == 10) {
      clever_remarks_one();
      ships->bounty += (long) 5000L * (long) planets.technology;
      planets.technology = 0;
      planets.visited = user_number;
      write_planets(txpos);
      ypos = ships->ship_ypos;

      c_out(LIGHTRED,
         "You now have a bounty on your head worth %ld credits!\n\r",
         ships->bounty);

      planet_hit = 0;
      write_user();
   }
   else {
      c_out(LIGHTRED, "\n\r%s\n\r", planet_fire[planet_hit - 1]);
   }
}

/* **********************************************************************
   * The ship has hit an enemy ship. Deduct the hit value from the	*
   * enemy ships shields. If the shields go down, make sure they are	*
   * set to a positive 0 then see if some damage needs to be done.	*
   *									*
   * Make sure that both ships get deducted as things go on...		*
   *									*
   ********************************************************************** */

void hit_enemy(int the_ship, int hit_value)
{
   char i;
   short planet_transfer;

   hit_value -= arandom(10L, 200L);
   read_enemy(the_ship);
   ship_attacking(the_ship);
   enemy->ship_shield -= hit_value;

/*
   If the ship being attacked has been fired at by the active ship
   the last time, incriment the number of torpedos used.

   If the ship attacking is not the last to attack the ship under
   fire, write the last-attacked-by value and set the torp count to 1.
*/

   if (! strcmp(enemy->last_at_by, hold[user_number]->names)) {
      enemy->last_torp_count++;
   }
   else {
      strcpy(enemy->last_at_by, hold[user_number]->names);
      enemy->last_torp_count = 1;
   }

   strcpy(ships->last_at_who, hold[the_ship]->names);

   if (enemy->ship_shield < 1) {
      enemy->ship_shield = 0;
   }

   c_out(LIGHTRED, "\n\r%d unit hit on %s by %s, [%ld] remaining\n\r",
      hit_value,
      hold[the_ship]->names,
      hold[user_number]->names,
      (long) enemy->ship_shield);

   for (i = 0; i < 5; i++) {
      if (strcmp(ships->allies[i], hold[the_ship]->names) == 0) {
	 fire_at_friend(the_ship, i);
      }
   }

/* **********************************************************************
   * If the enemy shield is down, cause some damage. Notice that we do	*
   * not return to the calling routine yet. We allow the ship to fight	*
   * back if it is still able to.					*
   *									*
   * Because the ships shields are down, perform an evasive if the hull	*
   * is still intact. If not intact, the ship was destroyed. Update the	*
   * enemy ships data file and return.					*
   *									*
   * DESTROYED flag							*
   ********************************************************************** */

   if (enemy->ship_shield < 1) {
      enemy_shield_down(the_ship);
      write_user();

      if (enemy->ship_hull < 27) {
         enemy->ship_xpos = enemy->ship_ypos = enemy->ship_universe = 0;
         enemy->leashed_to = enemy->leashed_by = (char)NIL;
         strcpy(enemy->who_destroyed, hold[user_number]->names);
	 write_enemy(the_ship);

         c_out(CYAN, "Transfering planetary ownership to you!\n\r");
	 planet_transfer = hand_over_planets(the_ship, user_number);
	 return_slaver_parts(the_ship, 0);
	 c_out(CYAN, "You gained %d planets!\n\r", planet_transfer);

         if (enemy->bounty > 0) {

	    c_out(CYAN,
               "Bounty worth %ld has been collected\n\r", enemy->bounty);

            ships->ship_credits += enemy->bounty;
            enemy->bounty = 0;
	 }

         if (the_ship > 0) {
            memory_freed((UL)sizeof(struct holder));
            if_any_bounce_it(the_ship);
            farfree(hold[the_ship]);
            hold[the_ship] = (struct holder *)NULL;
         }

	 write_enemy(the_ship);
         remove_command_file(enemy->ship_name, hold[user_number]->names);

         if (! strcmp(enemy->ship_name, "<GP>")) {
            ships->bounty += 1500000L;;
            c_out(CYAN, "You killed a cop! There is a HUGE bounty on your head!\n\r");
            c_out(CYAN, "The cops command file has been activated!\n\r");
         }

         if (drag_ship != (char)NIL || ships->leashed_to != (char)NIL) {
            if (the_ship == drag_ship || the_ship == ships->leashed_to) {
                if (drag_ship != (char)NIL) {
                    drag_ship = (char)NIL;
                    c_out(LIGHTRED, "Drag has been broken!\n\r");
                }
                if (ships->leashed_to != (char)NIL) {
                    ships->leashed_to = (char)NIL;
                    c_out(LIGHTRED, "Leash has been broken!\n\r");
                }
            }
         }

         ships->last_at_status = 4;
         write_user();

         clever_remarks_five();
         return;
      }
      else {
	 write_enemy(the_ship);
      }

      perform_evasive((short) the_ship, 1);
   }

/* **********************************************************************
   * If the enemy shields are low and there is still enough power left	*
   * to rechannel power, do so. Split the available power in two and	*
   * set the shield and power equally.					*
   *									*
   ********************************************************************** */

   if (enemy->ship_shield < 1000 && enemy->ship_power > 1000) {
      c_out(LIGHTGREEN, "* * * Enemy ship rechanneling power * * *\n\r");
      enemy->ship_power += enemy->ship_shield;
      enemy->ship_power /= 2;
      enemy->ship_shield = enemy->ship_power;
      ships->last_at_status = 1;
   }

   return_torp(the_ship);
   return_phaser(the_ship);
}

/* **********************************************************************
   * If there is enough power, and if there are enough people to use	*
   * the equipment, fire the phasers.					*
   *									*
   * See if we should perform evasive here as well.			*
   *									*
   ********************************************************************** */

void return_phaser(short the_ship)
{
   if (enemy->ship_power > 1000) {
      fire_back_phaser(the_ship, (UL)0);
      perform_evasive(the_ship, 5);
      write_enemy(the_ship);
      return;
   }
}

/* **********************************************************************
   * If there are torpedos left, and if there are enough peple left to	*
   * use them, allow the defending ship to fire back.			*
   *									*
   * After sending a torpedo back at the attacking ship, call the 	*
   * routine which randomly determined wether or not to warp around.	*
   *									*
   ********************************************************************** */

void return_torp(short the_ship)
{
   if (enemy->ship_torp > 1 && enemy->ship_crew > 10) {
      fire_back_torp(the_ship);
      perform_evasive(the_ship, 1);
      write_enemy(the_ship);
      return;
   }
}

/* **********************************************************************
   * Send a random number, (from 1 to 4), torps back in the general	*
   * direction of the attacking ship.					*
   *									*
   ********************************************************************** */

void fire_back_torp(short the_ship)
{
   int the_rnd, hit_times;

   if (enemy->ship_torp == 0) {
      return;
   }

   for (hit_times = (int) arandom(1L, 4L); hit_times > 0; hit_times--) {
      if (enemy->ship_torp-- == 0) {
         return;
      }

      the_rnd = (int) arandom(500L, 1000L);

      read_user();
      ships->ship_shield -= the_rnd;
      if (ships->ship_shield < 0) ships->ship_shield = 0;
      write_user();

      c_out(LIGHTRED,
	 "* * * Torpedo from %s - %d unit hit on %s, [%ld] remaining\n\r",
	 hold[the_ship]->names,
         the_rnd,
	 hold[user_number]->names,
         (long) ships->ship_shield);

      if (ships->ship_shield < 1) {
	 the_percent = 10;
	 make_some_damage(user_number, the_ship);
         return;
      }
   }
}

/* **********************************************************************
   * Fire with phasers. The value may be any random value up half of	*
   * the amount remaining in the ship.					*
   * 									*
   *									*
   ********************************************************************** */

void fire_back_phaser(short the_ship, UL how_much)
{
   long hit_value;

   if (how_much != (UL)0 && enemy->ship_power > how_much) {
      hit_value = arandom(how_much / 2, how_much);
   }
   else {
      hit_value = arandom(1000L, (long)enemy->ship_power / 2);
   }

   if (enemy->ship_power < hit_value) {
      return;
   }

   enemy->ship_power -= hit_value;

   read_user();
   ships->ship_shield -= hit_value;
   if (ships->ship_shield < 1) ships->ship_shield = 0;
   write_user();

   c_out(LIGHTRED, "* * * Phaser from %s - %ld unit hit on %s, [%ld] remaining\n\r",
      hold[the_ship]->names,
      hit_value,
      hold[user_number]->names,
      ships->ship_shield);

   if (ships->ship_shield == 0) {
      the_percent = 10;
      make_some_damage(user_number, the_ship);
      return;
   }
}

/* **********************************************************************
   * The enemy ship shield is down. Make a percentage deduction and a	*
   * hard reduction in hull and crew.					*
   *									*
   ********************************************************************** */

void enemy_shield_down(short the_ship)
{
   enemy->ship_shield = 0;
   the_rnd = (int) arandom(10L, 40L);

   enemy->ship_hull -= the_rnd;
   enemy->ship_crew -= the_rnd;
   ships->last_at_status = 2;

   if (enemy->ship_hull < 27) {
      enemy->ship_hull = 0;
      c_out(LIGHTRED, "Enemy ship destroyed!\n\r");
      hold[user_number]->kills++;
      ships->total_kills++;
      inform_kill(hold[user_number]->names, hold[the_ship]->names, TRUE, 1);
      perform_stand(TRUE);
      return;
   }

/*
   Display problems and then allow dropping to the display of the
   enemy ships hull.
*/

   if (enemy->ship_crew < 1) {
      enemy->ship_crew = 0;
      c_out(LIGHTBLUE, "Unliveable environmental conditions on enemy ship! ");
      ships->last_at_status = 3;
   }

   c_out(LIGHTBLUE, "Enemy ship hull %d%%\n\r", enemy->ship_hull);
}

/* **********************************************************************
   * Choose a random number and if it's greater than 25, perform an	*
   * evasive by warping randomly one space in any direction.		*
   *									*
   * Some changes were done on 19/Feb/89 that would allow an evadeing	*
   * ship to warp more than one space if fired on with phasers. The 	*
   * maximum distance is passed to the routine and a random number from	*
   * one to that number is selected by the random number engine.	*
   *									*
   * EVADE flag								*
   *									*
   ********************************************************************** */

void perform_evasive(short the_ship, short the_times)
{
   long newx, newy, oldx, oldy;
   short test_ship, test_remote, the_vel;

   if ((int) arandom(1L, 100L) > 25) {
      return;
   }

   oldx = enemy->ship_xpos;
   oldy = enemy->ship_ypos;

   if (the_times == 1) {
      the_vel = 1;
   }
   else {
      the_vel = arandom(1L, (long) the_times);
   }

try_it_again:
   the_rnd = (int) arandom(1L, 9L);
   newx = enemy->ship_xpos + (long) the_vel * directions[(the_rnd - 1) * 2];
   newy = enemy->ship_ypos + (long) the_vel * directions[((the_rnd - 1) * 2) + 1];

   if (newx < 1 || newx > xsize - 1 || newy < 1 || newy > ysize - 1) {
      goto try_it_again;
   }

   enemy->ship_xpos = newx;
   enemy->ship_ypos = newy;
   hold[the_ship]->sxpos = enemy->ship_xpos;
   hold[the_ship]->sypos = enemy->ship_ypos;
   write_enemy(the_ship);

/*
   Now see if we have moved off of a remote robot sensor. If so, put
   that remote robot sensor back to where the automated ship is now and
   update the memory and disk records.

   'the_ship' is the ship that moved.
   'test_ship' is the ship that has a remote attached to it.
*/

   for (test_ship = 1; test_ship < players; test_ship++) { /* No cops */
      if (Good_Hold(test_ship)) {
         if (test_ship != the_ship) {
            for (test_remote = 0; test_remote < 10; test_remote++) {
               if (hold[test_ship]->xremotes[test_remote] == oldx &&
                  hold[test_ship]->yremotes[test_remote] == oldy &&
                  hold[test_ship]->remote_universe[test_remote] == zpos) {
                     bring_along_remote(test_ship, test_remote, newx, newy);
               }
	    }
	 }
      }
   }

   read_enemy(the_ship);
}

/* **********************************************************************
   * The evasive ship has warped and there was an attached remote.	*
   * Take the remote back to the ships location.			*
   *									*
   ********************************************************************** */

void bring_along_remote(short test_ship, short the_remote, long newx, long newy)
{
   read_enemy(test_ship);
   ships->rem_xpos[the_remote] = newx;
   ships->rem_ypos[the_remote] = newy;
   write_enemy(test_ship);
   hold[test_ship]->xremotes[the_remote] = newx;
   hold[test_ship]->yremotes[the_remote] = newy;
}

/* **********************************************************************
   * Torpedo hits enemy base.						*
   * 									*
   * Here we will need to drop the base shields before we can allow it	*
   * to be boarded. Once the shields are down, then we down't allow any	*
   * more damage to take place.						*
   *									*
   * If the ship has been destroyed yet the base is active, then we	*
   * want to make sure that the message offered is a good one.		*
   *									*
   ********************************************************************** */

void hit_base(long hit_value)
{
   short angle_in, result, the_base;

   the_base = close_base[d_loop];

   if (the_base == user_number) {
      c_out(WHITE, "\n\rBase commander knows how to demegaphase torpedo burst!\n\r");
      return;
   }

   read_enemy(the_base);
   enemy->base_shield = enemy->base_shield - hit_value;

   if (enemy->base_shield < 1) {
      enemy->base_shield = 0;
      c_out(LIGHTRED, "\n\r* * * %s * * *\n\r",
         hit_unshielded_base[enemy->base_hit_count++]);
   }

   c_out(LIGHTRED, "\n\r%ld unit hit on base %s [%ld] remaining\n\r",
      hit_value, enemy->ship_name, enemy->base_shield);

/*
   The ship _should_ get credited with a kill for the base.
*/

   if (enemy->base_shield == 0 && enemy->base_hit_count == 10) {
      hold[the_base]->bxpos = hold[the_base]->bypos = 0;
      enemy->base_xpos = enemy->base_ypos = enemy->base_universe = 0;
      strcpy(enemy->base_death, hold[user_number]->names);
      enemy->base_shield = 0;
      enemy->base_credits = 0;
      enemy->base_cargo = 0;
      enemy->base_crew = 0;
      enemy->base_cloak = 0;
      enemy->base_hit_count = 0;
      write_enemy(the_base);
      read_user();
      ships->total_kills++;
      hold[user_number]->kills++;
      write_user();
      inform_kill(hold[user_number]->names, hold[the_base]->names, FALSE, 1);
      perform_stand(TRUE);
      return;
   }

   write_enemy(the_base);

/*
   The base is under attack. See if the ship is in the area. If so,
   then fire back at the ship attacking the base. If the ship is not
   in the area, move it into this area.
*/

   for (angle_in = 0; angle_in < ship_count; angle_in++) {
      if (close_ship[angle_in] == the_base) {
         if (enemy->ship_torp > 0) {
	    fire_back_torp(the_base);
	 }
         else if (enemy->ship_power > 100) {
            fire_back_phaser(the_base, (UL)0);
	 }
	 else {
	    c_out(LIGHTRED, "Enemy ship can't assist base. Out of power and torps!\n\r");
	 }
	 return;
      }
   }

   angle_in =
      compute_direction(the_base, user_number, X_WARP_WITHIN, Y_WARP_WITHIN);

   if (angle_in == (short)NIL) {
      return;
   }

   if (enemy->ship_warp < 2) {
      c_out(LIGHTRED, "Enemy ships captain can't warp in to assist base!\n\r");
      return;
   }

   if (enemy->ship_xpos == 0 && enemy->ship_ypos == 0) {
      c_out(LIGHTRED, "Base mother ship is overdue and presumed dead.\n\r");
      return;
   }

   result = warp_ship(the_base, angle_in, Y_WARP_WITHIN, 100, 1);

   if (result == (short)NIL) {
      c_out(LIGHTRED, "Enemy ships captain doesn't have enough power to warp in!\n\r");
      return;
   }
   c_out(LIGHTGREEN, "Enemy ship warping in to assist base!\n\r");
}

/* **********************************************************************
   * Torp hit a remote.							*
   *									*
   * If the remote belongs to the currently active ship, then it should	*
   * not be destroyed.							*
   *									*
   * If the remote belongs to the enemy, however, then it should be	*
   * removed from the possession of the ship. This is done by setting	*
   * the x and y posistions of the remote to 0. We COULD do something	*
   * like set it to a negative based on the user_number. This would	*
   * allow the owner of the remote to know who destroyed it. Let's DO	*
   * it!								*
   *									*
   * Don't forget to update xremotes and yremotes			*
   *									*
   * When we enter this routine, close_remotes[d_loop] is the owner of  *
   * the remote. 'rloop' contains the number of the remote.		*
   *									*
   ********************************************************************** */

void hit_remote(unsigned int rloop)
{
   count = close_remotes[d_loop];

   if (count == user_number) {
      c_out(YELLOW, "\n\rThat's your own remote you tried to destroy!\n\r");
      return;
   }

   read_enemy(count);
   c_out(LIGHTRED, "\n\rTorpedo destroyed %s remote number %d!\n\r",
      hold[count]->names, rloop);

   enemy->rem_xpos[rloop] = -user_number;
   enemy->rem_ypos[rloop] = -user_number;
   enemy->rem_universe[rloop] = 0;
   write_enemy(count);
   hold[count]->xremotes[rloop] = 0;
   hold[count]->yremotes[rloop] = 0;
   hold[count]->remote_universe[rloop] = 0;
   hold[count]->remote_universe[rloop] = 0;
}

void hit_swarm(unsigned int rloop)
{
   short swarm_ships, swarm_power;

   count = close_swarms[d_loop];
   c_out(WHITE, "\n\r");

   if (user_number == count) {
      c_out(LIGHTBLUE, "Message: Swarm commander, group %d\n\r", rloop);
      c_out(LIGHTBLUE, "   What are you fireing at me for?!\n\r");
      return;
   }

/*
   The active ship has fired a torp into an enemy swarm of attack
   sleds. We need to determine what kind of damage has been done and
   we need to do some random messages to reflect some kind of active
   battle.
*/

   read_enemy(count);
   swarm_ships = enemy->sled_swarm[rloop];
   swarm_power = enemy->sled_power[rloop];

   c_out(LIGHTRED, "Swarm %s: proximity detonation. Status: ",
      hold[count]->names);

/*
   If the number of attack sleds is rather small, then see if it
   should be destroyed compleatly or if a miss should take place. We
   ask for a random number from 1 to 10 and check for if it's less
   than 6. If so, we have a 50% chance of hit/mill.
*/

   if (swarm_ships < 25) {
      the_rnd = arandom(1L, 10L);
      if (the_rnd < 6) {
	 c_out(LIGHTRED, "Destroyed!\n\r");
         enemy->sled_swarm[rloop] = 0;
         enemy->sled_xpos[rloop] = (short)NIL;
         enemy->sled_ypos[rloop] = (short)NIL;
         enemy->sled_universe[rloop] = 0;
         hold[count]->xswarm[rloop] = (short)NIL;
         hold[count]->yswarm[rloop] = (short)NIL;
	 hold[count]->swarm_universe[rloop] = 0;
         enemy->sled_power[rloop] = 0;
	 write_enemy(count);
	 return;
      }
      else {
	 c_out(LIGHTGREEN, "Evaded.\n\r");
	 return;
      }
   }

/*
   There are more than 25 attack sleds. Take a random number of
   destroyed ships based on the available power to that swarm. Destroy
   that number of ships->
*/

   if (swarm_power > 100) {
      the_rnd = arandom(1L, 5L);
   }
   else if (swarm_power > 90) {
      the_rnd = arandom(1L, 10L);
   }
   else if (swarm_power > 80) {
      the_rnd = arandom(1L, 15L);
   }
   else if (swarm_power > 50) {
      the_rnd = arandom(1L, 20L);
   }
   else {
      the_rnd = arandom(1L, 25L);
   }

   c_out(LIGHTRED, "%d sleds destroyed!\n\r", (short) the_rnd);
   enemy->sled_swarm[rloop] -= the_rnd;
   write_enemy(count);
   clever_remarks_four();
}

/* **********************************************************************
   * Hit a decoy. Fake a hit on the decoy shuttle and also degrade the	*
   * shuttle class. If the shuttle is too far gone, remove it.		*
   *									*
   * If the shuttle is removed, then return a FALSE so that the torp	*
   * will appear to pass right through the decoy, otherwise return a	*
   * TRUE so that the torp will stop at the decoy.                      *
   *									*
   ********************************************************************** */

unsigned short hit_decoy(void)
{
   short the_removal;

   the_removal = (short)arandom(5L, 10L);

   if (m_run != (struct command_options *)NULL) {
      m_run->decoy_class -= the_removal;

      if (m_run->decoy_class < 1) {
         m_run->decoy_class = 0;
         decoy_xpos = decoy_ypos = decoy_value = (short)NIL;
         m_run->bail_4_out = 0;
         return(FALSE);
      }
   }
   else {
      log_error(106);
      return(FALSE);
   }

/*
   Now "Fake" a hit on the ship that is really a decoy
*/

   if (Good_Hold(decoy_value)) {
      c_out(LIGHTRED, "\n\r%d unit hit on %s [%ld] remaining\n\r",
         (short) arandom(100L, 500L),
         hold[decoy_value]->names,
         (long) arandom(1000L, 2000L));
   }
   else {
      log_error(114);
   }

   return(TRUE);
}

/* **********************************************************************
   * When attack sleds are fired at and there is destruction, give a	*
   * clever remark.							*
   *									*
   ********************************************************************** */

void clever_remarks_four(void)
{
   the_rnd = (int) arandom(1L, 10L);
   c_out(LIGHTRED, "%s\n\r", destroyed_sleds[the_rnd - 1]);
}

/* **********************************************************************
   * When the active ship destroys a ship, give some remark.		*
   *									*
   ********************************************************************** */

void clever_remarks_five(void)
{
   the_rnd = (int) arandom(1L, 15L);
   c_out(LIGHTRED, "%s\n\r", destroyed_enemy_ship[the_rnd - 1]);
}

/* **********************************************************************
   * Have a good chance of killing the pirate. If the pirate is killed, *
   * offer some bounty, otherwise, have the pirate shoot back!          *
   *                                                                    *
   ********************************************************************** */

static void hit_pirate(void)
{
   int kill_it;
   kill_it = (int)arandom(1L, 10L);

   c_out(LIGHTRED,
      "\n\r* Torpedo impact on pirate ship at {%d-%d} *\n\r", pxpos, pypos);

   if (kill_it == 5) {
      c_out(LIGHTRED, "Pirate ship destroyed! Bounty awarded!\n\r");
      pirate = 0;
      pxpos = pypos = (short)NIL;
      total_pirate_count = 0;
      read_user();
      ships->ship_credits += 300000;
      hold[user_number]->kills = ++ships->total_kills;
      write_user();
      return;
   }

   pirates_fight_back(TRUE);
}

