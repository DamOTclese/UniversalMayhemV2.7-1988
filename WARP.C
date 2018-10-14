
/* **********************************************************************
   * warp.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * 19/Sep/88 - Attached remotes fall off when ship warps. This has 	*
   *   been fixed.							*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "universe.h"
#include "planets.h"
#include "comets.h"
#include "holder.h"
#include "goal.h"
#include "leash.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"
#include "string.h"
#include "stdlib.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long xsize, ysize;
   extern long xpos, ypos, txpos, typos;
   extern UC zpos, tzpos;
   extern long count;
   extern unsigned short the_percent;
   extern unsigned short docked, base_docked;
   extern unsigned short comet_count;
   extern unsigned short close_comets[40];
   extern unsigned short close_goals[10];
   extern char *record;
   extern char the_name[31];
   extern char goals_count;
   extern char pirate;
   extern char total_pirate_count;
   extern char the_plague;
   extern char start_warp;
   extern char swarm_test;
   extern short players;
   extern short bail_out;
   extern short user_number;
   extern short rpt_loop;
   extern short the_tech;
   extern short cost_energy, cost_cargo, cost_shuttle, cost_warp, cost_hull;
   extern short cost_cloak, cost_crew, cost_sensor, cost_torp, cost_remotes;
   extern short cost_sled;
   extern short infect_count;
   extern short the_rnd;
   extern short the_remove;
   extern short pxpos, pypos;
   extern char drag_ship;
   extern char shield_repair;
   extern char planet_hit;
   extern char shoved_flag;

   short attached_flag = 0;
   short oldx, oldy;
   static UC oldz;

/* **********************************************************************
   * For an explanation of this array, if you cant immediatly see whats *
   * happening and how it is used to determine symbolic movement within *
   * the grid, read FidoNews article "Two and Three Dimentional         *
   * Numerical Movements Within a Symbolic Model".                      *
   *                                                                    *
   ********************************************************************** */

    short directions[18] = {
      -1, -1, -1, 0, -1, 1,
       0, -1,  0, 0,  0, 1,
       1, -1,  1, 0,  1, 1
   } ;

/* **********************************************************************
   * Here is the gateway access map. Consider:				*
   *									*
   * You wish to leave universe 3 and go into universe 5. We store	*
   * the following information:						*
   *									*
   * o Each universe has four corners.					*
   *									*
   *   o Upper left is corner 1						*
   *   o Upper right is corner 2					*
   *   o Lower left is corner 3						*
   *   o Lower right is corner 4					*
   *									*
   * o Universe 3 has 5, 4, 0, 2, 99, 99, 8, and 1 stored in its	*
   *   array. This information consists of two pieces of information	*
   *   for each corner:							*
   *									*
   *   o The universe that corner leads to				*
   *   o The corner number of that universe the current corner leads to	*
   *									*
   * If the universe is 99, then that corner has no outbound/inbound	*
   * gateway access.							*
   *									*
   ********************************************************************** */

   UC gateway_access[13][4][2] = {
       1,  3,  2,  2,  3,  1,  4,  3,		/* Universe 0	*/
      99, 99,  6,  2,  5,  1,  0,  0,		/* Universe 1	*/
       6,  3, 99, 99,  0,  1,  7,  0,		/* Universe 2	*/
       5,  3,  0,  1, 99, 99,  8,  0,		/* Universe 3	*/
       0,  3,  7,  2,  8,  1, 99, 99,		/* Universe 4	*/
      99, 99,  1,  2, 99, 99,  3,  0,		/* Universe 5	*/
      99, 99, 10,  2,  1,  1,  2,  0,		/* Universe 6	*/
       2,  3, 99, 99,  4,  1, 99, 99,		/* Universe 7	*/
       3,  3,  4,  2,  9,  1, 99, 99,		/* Universe 8	*/
      99, 99,  8,  2, 11,  1, 99, 99,		/* Universe 9	*/
      99, 99, 12,  2,  6,  1, 99, 99,		/* Universe 10	*/
      99, 99,  9,  2, 99, 99, 99, 99,		/* Universe 11	*/
      99, 99, 99, 99, 10,  1, 99, 99		/* Universe 12	*/
   } ;

/* **********************************************************************
   * After hitting a planet, (docking to the thing), check to see if it	*
   * "belongs" to someone. If so, set the "taxed_here" value to the	*
   * constant we defined in module "defines.h".				*
   *									*
   * o NOT_OWNED if the planet is not owned				*
   *									*
   * o IS_OWNED if the planet is owned by some other player		*
   *   If this value is set to the variable, then we also set the other	*
   *   variable "owned_who" to the ship number that ownes the planet.	*
   *									*
   * o PLAYER_OWNED if the planet is owned by the current player	*
   *									*
   ********************************************************************** */

   unsigned char taxed_here;
   unsigned short owned_who;

/* **********************************************************************
   * If there is the nameing of planets, to_save will be the length of	*
   * the name string. If planets are not to be named, then the value of	*
   * this number is 0.							*
   *									*
   * This was an easy way to "hack off" a third or more of the bytes	*
   * being stored on the drive by oliminating the planet names.		*
   *									*
   ********************************************************************** */

   extern char to_save;

/* **********************************************************************
   * Perform a warp. If the dewarp lands on an object, we want to do	*
   * something about it. We must deduct from the energy available and	*
   * make certain that the shield is capable of handling the warp. We	*
   * also must limit the warp to the amount the ship can perform.	*
   *									*
   * If the cops have been called, then we need to keep them with us.	*
   * If the cops have been called but the warp speed is greater than	*
   * 10, then the cops are left behind.					*
   *									*
   ********************************************************************** */

char perform_warp(void)
{
   int the_dir, the_vel, hold_index;
   char *cpointer, corner, other_corner;
   char afloop, leashed_ship;

   if (start_warp == 1) {
      goto just_started;
   }

   cpointer = record;
   cpointer++;

   if (*cpointer == (char)NULL) {
      c_out(WHITE, "WARP command syntax is missing direction: %s\n\r", record);
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   the_dir = (*cpointer++) - 0x30;

   if (the_dir < 1 || the_dir > 9) {
      c_out(WHITE, "Invalid WARP direction: %d\n\r", the_dir);
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   if (the_dir == 5) {
      return(TRUE);
   }

   if (*cpointer == (char)NULL) {
      c_out(WHITE, "WARP command syntax is missing velocity: %s\n\r", record);
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   the_vel = atoi(cpointer);

   read_user();

   if (the_vel > ships->ship_warp) {
      c_out(WHITE,
         "Unable to attain that warp! Maximum is %d\n\r", ships->ship_warp);
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

   if (the_vel < 1) {
      c_out(WHITE, "Warp value must be greater than 0\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

/*
    Validate the leash. You never know. The failure could happen as
    the result of a leashed ship attacking another assisted ship or
    something really unusual.
*/

   if (drag_ship != (char)NIL || drag_ship == user_number) {
        if (! Good_Hold(drag_ship)) {
            drag_ship = (char)NIL;
            c_out(LIGHTRED, "Drag leash has been broken!\n\r");
        }
    }
    if (ships->leashed_to != (char)NIL || ships->leashed_to == user_number) {
        if (! Good_Hold(ships->leashed_to)) {
            ships->leashed_to = (char)NIL;
            c_out(LIGHTRED, "Leash has been broken!\n\r");
        }
    }

   if (drag_ship != (char)NIL || ships->leashed_to != (char)NIL) {
        if (the_vel * 50000 > ships->ship_power) {
            c_out(WHITE, "Can't attain that speed with a ship leashed!\n\r");
            c_out(WHITE, "Not enough power is available!\n\r");
            bail_out = 1;
            rpt_loop = 0;
            return(FALSE);
        }
        else {
            if (drag_ship != (char)NIL) {
                leashed_ship = drag_ship;
            }
            else {
                leashed_ship = ships->leashed_to;
            }

            if (drag_ship != (char)NIL) {
                c_out(WHITE, "Leashed ship is dragged along! Leash broken!\n\r");
            }
            else {
                c_out(WHITE, "Tight leash of %s maintained.\n\r",
                    hold[leashed_ship]->names);
            }
            ships->ship_power -= (50000 * the_vel);
            read_enemy(leashed_ship);
            return_phaser(leashed_ship);
            return_torp(leashed_ship);
            return_phaser(leashed_ship);
            return_torp(leashed_ship);
            ship_attacking(leashed_ship);
        }
    }
    else {
        find_power_needed(the_vel);

        if (the_remove > ships->ship_power && the_vel > 1) {

            c_out(WHITE,
                "Unable to attain warp! Ship needs %d units of energy\n\r",
                the_remove);

            bail_out = 1;
            rpt_loop = 0;
            return(FALSE);
        }

        if (the_vel > 1) {
            ships->ship_power -= the_remove;
        }
    }

/*
   Ship moved so zero out the planet hit count
*/

   if (planet_hit > 0) {
      planet_hit = 0;
      c_out(WHITE, "Planets damage has been repaired!\n\r");
   }

/*
   Compute new position
*/

   hold_index = ((the_dir - 1) * 2);
   txpos = xpos + (long) the_vel * directions[hold_index];
   typos = ypos + (long) the_vel * directions[++hold_index];

/*
   If the warp is sucessfull, compute what universe, if any, the
   ship will end up in. Place the result into tzpos.
*/

   if (txpos == 0 && typos == 0) corner = 0;
   else if (txpos == 0 && typos == ysize - 1) corner = 1;
   else if (txpos == xsize - 1 && typos == 0) corner = 2;
   else if (txpos == xsize - 1 && typos == ysize - 1) corner = 3;
   else {
      corner = (char)NIL;
      tzpos = zpos;
   }

   if (corner != (char)NIL) {
      tzpos = gateway_access[zpos][corner][0];
      if (tzpos != 99) {
	 other_corner = gateway_access[tzpos][corner][1];
	 if (other_corner == 0) { txpos = 2; typos = 2; }
	 else if (other_corner == 1) { txpos = 2; typos = ysize - 3; }
	 else if (other_corner == 2) { txpos = xsize - 3; typos = 2; }
	 else { txpos = xsize - 3; typos = ysize - 3; }
	 c_out(LIGHTRED, "Left universe %d to universe %d\n\r", zpos, tzpos);
      }
      else {
         other_corner = (char)NIL;
	 tzpos = zpos;
      }
   }

   if (txpos < 0 || txpos > xsize - 1 || typos < 0 || typos > ysize - 1) {
      c_out(LIGHTRED, "Attempted to leave the universe!\n\r");
      bail_out = 1;
      rpt_loop = 0;
      return(FALSE);
   }

/*
   See if it's a warp onto the center and don't allow it.
*/

   if (txpos == xsize / 2 && typos == ysize / 2) {
      c_out(LIGHTRED, "Bounced off of the universe gravitational node!\n\r");
      txpos++;
      typos++;
   }

   oldx = xpos;
   oldy = ypos;
   oldz = zpos;
   xpos = txpos;
   ypos = typos;
   zpos = tzpos;
   hold[user_number]->sxpos = xpos;
   hold[user_number]->sypos = ypos;
   hold[user_number]->szpos = zpos;
   ships->ship_xpos = xpos;
   ships->ship_ypos = ypos;
   ships->ship_universe = zpos;
   docked = base_docked = 0;
   swarm_test = 1;
   pirate = 0;
   pxpos = pypos = (short)NIL;
   total_pirate_count = 0;

/*
   If there are any remotes attached, by all means, bring them along.
*/

   check_attached_remotes(oldx, oldy, oldz);

/*
   If there is an odd number of remotes attached, cause error if
   the time is right. The error is in the x-axis and is one sector.
*/

   if (attached_flag / 2 != attached_flag) {
      if (arandom(1L, 20L) == 10) {
         if (xpos > xsize - 3) {
            xpos--;
         }
         else {
            xpos++;
         }
      }

      hold[user_number]->sxpos = xpos;
      ships->ship_xpos = xpos;
   }

   c_out(WHITE, "(%ld-%ld)\n\r", xpos, ypos);

just_started:
   if (xpos == ships->base_xpos &&
      ypos == ships->base_ypos &&
      zpos == ships->base_universe) {
      if (xpos > 0 && ypos > 0) {
         base_docked = 1;
	 c_out(LIGHTGREEN, "You are docked with your base. Greetings sir!\n\r");

         if (shield_repair == 1) {
            c_out(LIGHTRED, "Defense shield has been repaired!\n\r");
            c_out(LIGHTGREEN, "Base commander wants to know what the hell happened!\n\r");
            shield_repair = 0;
         }

         return(TRUE);
      }
   }

   if (start_warp == 0) {
      write_user();
   }
   else {
      txpos = xpos;
      typos = ypos;
      tzpos = zpos;
   }

/*
   See if we dragged an enemy ship with us!
*/

   if (drag_ship != (char)NIL || ships->leashed_to != (char)NIL) {
        read_enemy(leashed_ship);

        if (! shoved_flag) {
            mail_leash(leashed_ship,
                enemy->ship_xpos, enemy->ship_ypos,
                txpos, typos + 1, 0);

            shoved_flag = TRUE;
        }

        enemy->ship_xpos = txpos;
        enemy->ship_ypos = typos + 1;
        enemy->ship_universe = tzpos;
        hold[leashed_ship]->sxpos = txpos;
        hold[leashed_ship]->sypos = typos + 1;
        hold[leashed_ship]->szpos = tzpos;
        write_enemy(leashed_ship);
        if (drag_ship != (char)NIL) {
            drag_ship = (char)NIL;
        }
    }

/*
   Let's see if we ran into anything stellar.
*/

   read_universe(xpos);

/*
   See if we are at a planet. This is done first because the possibility
   that the ship is docking due to commerce looping is higher than all
   the other possibilities.
*/

   if (find_specific_planet(xpos, ypos) == 1) {
      docked = 1;
      we_docked();
      return(TRUE);
   }

/*
   If we ran into a star, we want to create some damage.
*/

   if (ypos == universe.star) {
      dewarp_star();
      return(TRUE);
   }

/*
   If we ran into a mine, we want to make damage also.
*/

   if (ypos == universe.mine) {
      dewarp_mine();
      return(TRUE);
   }

/*
   If we entered a black hole, we should come out in another part of
   space. Some percentage of the black holes are linked to other black
   holes. Reentry into the linked hole will cause a return back to
   the starting place.
*/

   if (ypos == universe.black_hole) {
      dewarp_hole();
      write_user();
      return(TRUE);
   }

/*
   A rather low possibility: see if the ship hit a comet. If so, the
   comet will destroy the ship.

   A bug fixed in version 1.41: We did not perform a plug of close
   objects at this point. This will slow things down a little more
   now that we check for close objects. We needed a way to make
   certain that at this point, we only look for comets and slaver
   death parts.
*/

   plug_close_objects(CLOSE_WARP);

   for (count = 0; count < comet_count; count++) {
      if (comets[close_comets[count]] != (struct comets_file *)NULL) {
         if (xpos == comets[close_comets[count]]->location[0] &&
            ypos == comets[close_comets[count]]->location[1]) {
            hit_comet(count);
         }
      }
   }

/*
   See if we dewarped onto a slaver death device part.
*/

   for (afloop = 0; afloop < goals_count; afloop++) {
      if (goal_item[close_goals[afloop]] != (struct goal_elements *)NULL) {
         if (goal_item[close_goals[afloop]]->goal_xpos == (short)xpos &&
            goal_item[close_goals[afloop]]->goal_ypos == (short)ypos &&
            goal_item[close_goals[afloop]]->goal_on_ship == (char)NIL) {
            dewarp_slaver_device(close_goals[afloop]);
            return(TRUE);
         }
      }
   }

/*
   If we dewarped onto nothing, simply return.
*/

   return(TRUE);
}

/* **********************************************************************
   * Determine how much power to take away from the ships available	*
   * power to perform the warp jump.					*
   *									*
   ********************************************************************** */

void find_power_needed(short the_vel)
{
   if (the_vel < 10)
      the_remove = 1;
   else if (the_vel < 20)
      the_remove = 10;
   else if (the_vel < 30)
      the_remove = 50;
   else if (the_vel < 40)
      the_remove = 100;
   else the_remove = 150;
}

/* **********************************************************************
   * If you hit a comet, you are destroyed.				*
   *									*
   ********************************************************************** */

void hit_comet(short cnumber)
{
   if (comets[cnumber] == (struct comets_file *)NULL) return;

   c_out(YELLOW, "You have hit a comet!\n\r");

   if (comets[cnumber]->flag == FALSE) {
      c_out(YELLOW, "This is a previously unmapped comet.\n\r");
   }
   else {
      c_out(YELLOW, "Comet %s has been mapped by %s! Pay attention!\n\r",
         comets[cnumber]->name, comets[cnumber]->ship);
   }

/*
   The ship was destroyed by stupidity, so take all of the planets that
   may belong to it and hand them back to the universe. This is done by
   using the routine in SYSOP.C, "hand_over_planets()". We pass it a NIL
   as the number of the ship record to take possession, causing them to
   revert back to NONE's.
*/

   hand_over_planets(user_number, (short)NIL);
   return_slaver_parts(user_number, (char)NIL);
   remove_command_file(ships->ship_name, hold[user_number]->names);
   make_zero_record();
   write_user();
   perform_quit(0);
}

/* **********************************************************************
   * After dewarping, the ship hit a star. Cause some damage to the	*
   * ship. Make sure that destruction of the ship is possible.		*
   *									*
   ********************************************************************** */

void dewarp_star(void)
{
   c_out(LIGHTRED, "You have dewarped through a star!\n\r");
   bounce_off_object();
   the_percent = 40;
   make_some_damage(user_number, (short)NIL);
}

/* **********************************************************************
   * We have hit a mine! Cause some damae and just as when you hit a	*
   * star, make sure that it's possible to get destroyed.		*
   *									*
   ********************************************************************** */

void dewarp_mine(void)
{
   c_out(LIGHTRED, "You have dewarped through a mine!\n\r");
   bounce_off_object();
   the_percent = 25;
   make_some_damage(user_number, (short)NIL);
}

/* **********************************************************************
   * The ship has sustained some damage. The amout is dependent on the	*
   * value in 'the_percent'. The damage can be caused by dewarping onto	*
   * a star or a mine. It can also be caused by defending enemy ships	*
   * and automated ships->                                               *
   *									*
   ********************************************************************** */

void make_some_damage(short on_ship, short from_ship)
{
   float fpercent;
   short crew_started;

   fpercent = (int) arandom(1L, (long) the_percent);
   c_out(LIGHTRED, "All systems reduced %f percent!\n\r", fpercent);

   read_user();
   crew_started = ships->ship_crew;
   ships->ship_power  -= (ships->ship_power  * (fpercent / 100));
   ships->ship_shield -= (ships->ship_shield * (fpercent / 100));
   ships->ship_crew   -= (ships->ship_crew   * (fpercent / 100));
   ships->ship_hull   -= (ships->ship_hull   * (fpercent / 100));
   crew_started -= ships->ship_crew;
   ships->sick_bay += arandom(1L, (long) crew_started);

   if (ships->ship_morale > 10) {
      ships->ship_morale--;
   }

   c_out(LIGHTRED, "Sickbay reporting: %d dead, %d in sick bay\n\r",
      crew_started, ships->sick_bay);

   if (ships->ship_power < 1) ships->ship_power = 0;
   if (ships->ship_shield < 1) ships->ship_shield = 0;
   if (ships->ship_crew < 1) ships->ship_crew = 0;
   write_user();

   if (ships->ship_power < 3) {
      c_out(CYAN, "You are adrift in the universe! You have just enough power\n\r");
      c_out(CYAN, "to send a subspace message asking someone to come kill you!\n\r");
      perform_quit(0);
   }

   if (ships->ship_shield == 0)
      c_out(CYAN, "Your shields are down!\n\r");

   if (ships->ship_crew < 10) {
      c_out(CYAN, "You don't have enough crew members to run this ship! You\n\r");
      c_out(CYAN, "need to ask someone to come and kill you!\n\r");
      perform_quit(0);
   }
   else if (ships->ship_crew < 50) {
      c_out(CYAN, "Your crew members are running out! Soon there will no one\n\r");
      c_out(CYAN, "left to run your ship!\n\r");
   }

/*
   If the ship is destroyed, hand the planets that may be owned over
   to the universe by setting the ownership to NONE if the from_ship
   is NIL, otherwise, hand them over to the owner of the sled group
   that destroyed it.
*/

   if (ships->ship_hull < 27) {
      c_out(CYAN, "Ships hull has been blown out! Your ship is destroyed!\n\r");
      hand_over_planets(on_ship, from_ship);
      return_slaver_parts(on_ship, (char)NIL);
      destroy_user();
   }
   else if (ships->ship_hull < 40) {
      c_out(CYAN, "Your ship is trailing frozen air. It can be tracked!\n\r");
   }
   else if (ships->ship_hull < 50) {
      c_out(CYAN, "Hull integrety is down! Crew members dispatched to maintain\n\r");
      c_out(CYAN, "life support!\n\r");
   }
   else if (ships->ship_hull < 100) {
      c_out(CYAN, "Hull breach deck %d! Automatic seals slamming into posistion!\n\r",
         (short)arandom(1L, 25L));
   }
}

/* **********************************************************************
   * Hit a star or a mine. Blow the ship off one space.			*
   *									*
   * If we can't back off one space for reason, we randomize.		*
   *									*
   ********************************************************************** */

void bounce_off_object(void)
{
   long newx, newy;

   oldx = ships->ship_xpos;
   oldy = ships->ship_ypos;

try_it_again:
   the_rnd = (int) arandom(1L, 9L);
   newx = oldx + directions[(the_rnd - 1) * 2];
   newy = oldy + directions[((the_rnd - 1) * 2) + 1];

   if (newx < 1 || newx > xsize - 1 || newy < 1 || newy > ysize - 1) {
      goto try_it_again;
   }

   xpos = newx;
   ypos = newy;
   ships->ship_xpos = xpos;
   ships->ship_ypos = ypos;
   hold[user_number]->sxpos = xpos;
   hold[user_number]->sypos = ypos;
   write_user();

   if (attached_flag > 0) {
      check_attached_remotes(oldx, oldy, oldz);
   }
}

/* **********************************************************************
   * The ships has been destroyed! Zero it out and bounce the player	*
   * out of the game.							*
   *									*
   ********************************************************************** */

void destroy_user(void)
{
   remove_command_file(ships->ship_name, hold[user_number]->names);
   make_zero_record();
   write_user();
   clever_remarks_two();
   perform_quit(0);
}

/* **********************************************************************
   * We have entered a black hole! 					*
   *									*
   * See if the hold has a white hole to exit from. If not, randomize	*
   * the exit point, else find the yposition of the white hold and make	*
   * our exit point out of the white hole.				*
   *									*
   * We have a 1 on 20 chance of changing the universe number.          *
   *                                                                    *
   ********************************************************************** */

void dewarp_hole(void)
{
   c_out(CYAN, "Time and space twist for the %s\n\r", ships->ship_name);
   oldx = xpos;
   oldy = ypos;
   oldz = zpos;

   if (universe.white_hole == 0) {
      xpos = arandom(1L, (long) xsize - 1);
      ypos = arandom(1L, (long) ysize - 1);
   }
   else {
      xpos = universe.white_hole;
      read_universe(xpos);
      ypos = universe.black_hole - 1;
   }

   hold[user_number]->sxpos = xpos;
   hold[user_number]->sypos = ypos;
   ships->ship_xpos = xpos;
   ships->ship_ypos = ypos;

   if (arandom(1L, 20L) == 10) {
      zpos = ships->ship_universe = arandom(1L, 12L);
      c_out(LIGHTRED, "Hyperspace transport to universe %d!\n\r", zpos);
   }

   if (attached_flag > 0) {
      check_attached_remotes(oldx, oldy, oldz);
   }
}

/* **********************************************************************
   * We docked with a planet. Compute the record number of the planet	*
   * and pull the information we need from it and shove it into the	*
   * variables we allocated to hold them. Next display the name of the	*
   * planet, how visited it last, who protected it if it was, and then	*
   * update the last visited by field.					*
   *									*
   ********************************************************************** */

void we_docked(void)
{
   short loop, test_tax, a_the_rnd;
   char the_last, the_protected;

   for (loop = 0; loop < players; loop++) {
      if (Good_Hold(loop) && loop != user_number) {
         if (hold[loop]->sxpos == xpos &&
             hold[loop]->sypos == ypos &&
             hold[loop]->szpos == zpos) {
            encountered_warp_field();
            return;
         }
      }
   }

   read_universe(txpos);

   if (find_specific_planet(txpos, typos) == 0) {
      c_out(LIGHTGREEN, "\n\rThis is a dead planet!\n\r");
      cost_energy = cost_cargo   = cost_shuttle = cost_warp = 0;
      cost_cloak  = cost_crew    = cost_hull    = cost_sensor = 0;
      cost_torp   = cost_remotes = cost_sled = 0;
      return;
   }

   read_planets(xpos);
   the_rnd = planets.cost;
   the_tech = planets.technology;

   if (the_tech == 0) {
      if (Good_Hold(planets.visited)) {
         c_out(LIGHTGREEN, "Docked with a planet destroyed by %s\n\r",
            hold[planets.visited]->names);
      }
      else {
         log_error(114);
      }
      cost_energy = cost_cargo   = cost_shuttle = cost_warp = 0;
      cost_cloak  = cost_crew    = cost_hull    = cost_sensor = 0;
      cost_torp   = cost_remotes = cost_sled = 0;
      return;
   }

   the_last = planets.visited;
   the_protected = planets.protected;
   the_plague = planets.plagued;
   strcpy(the_name, planets.named);

/*
   Take a one-in-10 chance of the plague being taken off of the planet.
*/

   if (the_plague != (char)NIL) {
      a_the_rnd = arandom(1L, 10L);
      if (a_the_rnd == 5) {
         the_plague = (char)NIL;
         planets.plagued = (char)NIL;
      }
   }

/*
   If there is still plague, bring it aboard by all means!
*/

   if (the_plague != (char)NIL) {
      infect_count = arandom(4L, 20L);
   }

/*
   The constants here must conform to the constants used in other
   modules such as 'info.c' and 'nav.c'.
*/

   cost_remotes = (int) (111 * the_rnd) + 10104;
   cost_energy =  (int) (1   * the_rnd) + 15;
   cost_cargo =   (int) (15  * the_rnd) + 100;
   cost_shuttle = (int) (127 * the_rnd) + 1000;
   cost_warp =    (int) (191 * the_rnd) + 1000;
   cost_hull =    (int) (101 * the_rnd) + 100;
   cost_cloak =   (int) (178 * the_rnd) + 121;
   cost_crew =    (int) (13  * the_rnd) + 10;
   cost_sensor =  (int) (132 * the_rnd) + 120;
   cost_torp =    (int) (17  * the_rnd) + 11;
   cost_sled =    (int) (211 * the_rnd) + 2010;

   if (strcmp(the_name, "NONE")) {
      if (to_save == 0) {
	 c_out(LIGHTGREEN, "Welcome to: %s\n\r", the_name);
      }
   }
   else {
      c_out(LIGHTGREEN, "You have reached docking facilities.\n\r");
   }

   if (the_last == (char)NIL) {
      c_out(LIGHTGREEN, "Currently unvisited\n\r");
   }
   else if (Good_Hold(the_last)) {
      c_out(LIGHTGREEN, "Last visited by %s\n\r", hold[the_last]->names);
   }
   else {
      log_error(114);
   }

   if (the_protected != (char)NIL) {
      if (Good_Hold(the_protected)) {
         c_out(LIGHTGREEN, "Protected by %s\n\r", hold[the_protected]->names);
      }
      else {
         log_error(114);
      }
   }

   planets.visited = user_number;
   write_planets(xpos);

/*
   If we have people in sick bay, return them to active service.
*/

   sick_bay();

/*
   Determine if the planet is owned, not owned, or owned by the player.
   Set the status value into "taxed_here" and return to calling routine.
*/

   if (the_protected == (char)NIL) {
      taxed_here = NOT_OWNED;
   }
   else if (the_protected == user_number) {
      taxed_here = PLAYER_OWNED;
   }
   else {
      taxed_here = IS_OWNED;
      for (test_tax = 0; test_tax < players; test_tax++) {
	 if (the_protected == test_tax) {
	    owned_who = test_tax;
	    return;
	 }
      }
   }

/*
    See what morale looks like. Demand some time off!
*/

   if (ships->ship_morale <= 80 && ships->ship_morale > 70) {
      c_out(LIGHTRED, "The crew DEMANDS shore leave!\n\r");
   }
   else if (ships->ship_morale <= 70 && ships->ship_morale > 60) {
      c_out(LIGHTRED, "The crew is fighting on all decks!\n\r");
   }
   else if (ships->ship_morale <= 60) {
      c_out(LIGHTRED, "Many crew members are not staying at stations!\n\r");
   }

/*
   Repair the shield if needed
*/

   if (shield_repair == 1) {
      c_out(LIGHTRED, "Defense shield has been repaired!\n\r");
      shield_repair = 0;
   }
}

/* **********************************************************************
   * A ship hopeing to dock has run into a warp field; another ship	*
   * already in orbit. Bounce it off a random position.			*
   *									*
   ********************************************************************** */

void encountered_warp_field(void)
{
   c_out(LIGHTRED, "Ship encountered a warp field at (%04ld - %04ld)!\n\r", xpos, ypos);
   xpos = arandom(1L, xsize - 1);
   ypos = arandom(1L, ysize - 1);
   ships->ship_xpos = xpos;
   ships->ship_ypos = ypos;
   hold[user_number]->sxpos = xpos;
   hold[user_number]->sypos = ypos;
   write_user();

   if (attached_flag > 0) {
      check_attached_remotes(oldx, oldy, oldz);
   }
}

/* **********************************************************************
   * See if there are remotes attached. If so, move them.		*
   *									*
   ********************************************************************** */

void check_attached_remotes(short oldx, short oldy, UC oldz)
{
   short s_count, r_count;

   attached_flag = 0;

   for (s_count = 0; s_count < players; s_count++) {
      if (Good_Hold(s_count) && s_count != user_number) {
         for (r_count = 0; r_count < 10; r_count++) {
            if (hold[s_count]->xremotes[r_count] == oldx &&
               hold[s_count]->yremotes[r_count] == oldy &&
               hold[s_count]->remote_universe[r_count] == oldz) {
               attached_flag++;
               read_enemy(s_count);
               enemy->rem_xpos[r_count] = xpos;
               enemy->rem_ypos[r_count] = ypos;
               enemy->rem_universe[r_count] = zpos;
               write_enemy(s_count);
               hold[s_count]->xremotes[r_count] = xpos;
               hold[s_count]->yremotes[r_count] = ypos;
               hold[s_count]->remote_universe[r_count] = zpos;
            }
	 }
      }
   }
}

/* **********************************************************************
   * If we have people in sick bay and we have docked, return them to 	*
   * active duty.							*
   *									*
   ********************************************************************** */

void sick_bay(void)
{
   if (ships->sick_bay == 0) {
      return;
   }

   c_out(YELLOW, "Sick bay returning %d crew members back to active service\n\r",
      ships->sick_bay);

   ships->ship_crew += ships->sick_bay;
   ships->sick_bay = 0;
   write_user();
}

/* **********************************************************************
  * We have dewarped onto the slaver device part. Put it aboard simply	*
  * by changing it's location to NIL and it's ownership to the ship.	*
  *									*
  *********************************************************************** */

void dewarp_slaver_device(short afloop)
{
   if (goal_item[afloop] != (struct goal_elements *)NULL) {
      read_goals((long)afloop);
      goal_item[afloop]->goal_xpos = (short)NIL;
      goal_item[afloop]->goal_ypos = (short)NIL;
      goal_item[afloop]->goal_on_ship = user_number;
      write_goals((long)afloop);
      c_out(LIGHTRED, "You have picked up a slaver death weapon part!\n\r");
      total_up_slaver_parts();
      mail_slaver(afloop);
   }
}

