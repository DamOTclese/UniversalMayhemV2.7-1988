
/* **********************************************************************
   * stat.c                                                             *
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "goal.h"
#include "scout.h"
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

   extern char allow_mail[4];
   extern char *goal_item_description[10];
   extern long aloop, dloop, count;
   extern long xpos, ypos;
   extern short user_number;
   extern char valid_command_file;
   extern UC zpos;

   static char line_count = 0;

/* **********************************************************************
   * Wait for enter to be hit if we have displayed 20 lines.            *
   *                                                                    *
   ********************************************************************** */

void wait_for_enter(void)
{
    line_count++;
    if (line_count < 20) {
        return;
    }

    c_out(LIGHTRED, "\n\rHit [ENTER] to continue: ");
    timed_input(TRUE);
    line_count = 0;
}


/* **********************************************************************
   *									*
   * Do a status of the ships functions. Display everything the ship 	*
   * can hold and then the base. If the ship has remotes on board or	*
   * have deployed some, give the status of those as well.		*
   *									*
   * If the remotes posistion is ONBOARD, then it is on board		*
   * If the remotes posistion is 0, then it has not been bought		*
   * If the remotes posistion is +, then the remote is deployed		*
   * If the remotes posistion is -, then it was destroyed		*
   *									*
   ********************************************************************** */

void perform_stat(char from_sysop)
{
   short aloop;
   float total_taxes;
   char oc, tc;
   char sin, sout, sstal, ssta, sboard;
   unsigned long sled_tally;

   total_taxes = 0.0;
   sin = sout = ssta = sstal = sboard = 0;
   line_count = 0;

   if (! from_sysop)
      read_user();

   c_out(YELLOW, "Ship: (%ld-%ld) Universe %d   ",
      ships->ship_xpos, ships->ship_ypos, ships->ship_universe);

   if (ships->base_xpos > 0) {
      c_out(YELLOW, "Base: [%ld-%ld] Universe %d",
         ships->base_xpos, ships->base_ypos, ships->base_universe);
   }

   c_out(WHITE, "\n\r");
   wait_for_enter();

   c_out(LIGHTRED, "\n\r%d Total Kills -- Player #%d",
       ships->total_kills, user_number);

   wait_for_enter();

   c_out(YELLOW, "\n\rPower:        %10ld", ships->ship_power);
   c_out(YELLOW, "  Shield:        %10ld", ships->ship_shield);
   wait_for_enter();

   c_out(YELLOW, "\n\rCredits:      %10ld", ships->ship_credits);
   c_out(YELLOW, "  Cargo:         %10d", ships->ship_cargo);
   wait_for_enter();

   c_out(YELLOW, "\n\rCrew:         %10d", ships->ship_crew);
   c_out(YELLOW, "  Class Shuttle: %10d", ships->ship_shuttle);
   wait_for_enter();

   c_out(YELLOW, "\n\rClass Hull:   %10d", ships->ship_hull);
   c_out(YELLOW, "  Class Cloak:   %10d", ships->ship_cloak);
   wait_for_enter();

   c_out(YELLOW, "\n\rClass Sensor: %10d", ships->ship_sensor);
   c_out(YELLOW, "  Class Warp:    %10d", ships->ship_warp);
   wait_for_enter();

   c_out(YELLOW, "\n\rTorps:        %10d", ships->ship_torp);
   c_out(YELLOW, "  Attack sleds:  %10d", ships->attack_sleds);
   wait_for_enter();

   if (ships->sick_bay > 0) {
      c_out(LIGHTRED, "\n\rSick Bay reports %d crew members are out of action",
         ships->sick_bay);

      wait_for_enter();
   }

   if (ships->plague_flag == 0) {
      c_out(YELLOW, "\n\r -- No biological weapons on board --");
      wait_for_enter();
   }
   else {
      c_out(YELLOW, "\n\r -- Class [%d] biological weapon on board --",
         ships->plague_flag);

      wait_for_enter();
   }

   if (! from_sysop) {
      for (count = 0; count < 10; count++) {
         switch(scouts[count]->scout_direction) {
            case SCOUT_INBOUND:
               if (scouts[count]->scout_universe == zpos)
                  sin++;
               else
                  sstal++;
               break;
            case SCOUT_OUTBOUND: sout++; break;
            case SCOUT_STATION: ssta++; break;
            case (char)NIL: sboard++; break;
         }
      }

      c_out(LIGHTGREEN,
         "\n\r -- Scouts: %d inbound, %d stalled, %d outbound, %d on station, %d on board",
         sin, sstal, sout, ssta, sboard);

      wait_for_enter();
   }


   if (ships->taxes > TAXES_DUE) {
      c_out(YELLOW, "\n\r -- Taxes in the amount of %10.2f are due the cops --",
         ships->taxes);

      wait_for_enter();
   }
   else if (ships->taxes > 10) {
      c_out(YELLOW, "\n\r -- Taxes outstanding for the cops: %10.2f credits --",
         ships->taxes);

      wait_for_enter();
   }

   for (aloop = 0; aloop < 10; aloop++) {
      if (ships->planet_taxes[aloop] > 0.0) {
         total_taxes += ships->planet_taxes[aloop];
      }
   }

   if (total_taxes > 0.0) {
      c_out(YELLOW, "\n\r -- Taxes in the amount of %10.2f owed to other players --",
	 total_taxes);

      wait_for_enter();
   }

   if (strcmp(ships->last_at_by, "NONE")) {
      c_out(LIGHTRED, "\n\rLast attacked by %s, [%d] torps used, [%3.1f] phaser units",
         ships->last_at_by, ships->last_torp_count,
         ships->last_phaser_count * 1000000L);

      wait_for_enter();
   }

   if (ships->bounty > 0) {
      c_out(LIGHTRED,
         "\n\rThere is a bounty on your head for %ld credits!", ships->bounty);

      wait_for_enter();
   }

   if (valid_command_file) {
      c_out(LIGHTRED, "\n\r --> You have an active COMMAND file.");

      wait_for_enter();
   }

   c_out(LIGHTGREEN, "\n\rCrew morale factor: %d%%", ships->ship_morale);
   wait_for_enter();

/*
    Find out how many attack sled are deployed
*/

   for (aloop = sled_tally = 0; aloop < 15; aloop++) {
      if (ships->sled_swarm[aloop] != (short)NIL) {
         sled_tally += ships->sled_swarm[aloop];
      }
   }

   if (sled_tally > 0) {
      c_out(LIGHTBLUE, "\n\rThere are %ld attack sleds deployed and on patrol",
         sled_tally);

      wait_for_enter();
   }

   if (strcmp(ships->last_at_who, "NONE")) {
      c_out(LIGHTRED, "\n\rYou last attacked %s and ", ships->last_at_who);
      wait_for_enter();

      switch (ships->last_at_status) {
       case 0: c_out(LIGHTRED, "you did no damage!");
	  break;
       case 1: c_out(LIGHTRED, "you damaged it!");
	  break;
       case 2: c_out(YELLOW, "you dropped its shields!");
	  break;
       case 3: c_out(YELLOW, "left it drifting!");
	  break;
       case 4: c_out(LIGHTRED, "you destroyed it!");
	  break;
       case 5: c_out(LIGHTRED, "you boarded it and left it drifting!");
	  break;
       }
    }

   if (ships->base_xpos != 0 && ships->base_ypos != 0) {
      c_out(YELLOW, "\n\rBase credits: %10ld", ships->base_credits);
      c_out(YELLOW, "   Base Cargo:   %10ld", ships->base_cargo);
      wait_for_enter();

      c_out(YELLOW, "\n\rBase Crew:    %10ld", ships->base_crew);
      c_out(YELLOW, "   Base Cloak:   %10d", ships->base_cloak);
      wait_for_enter();

      c_out(YELLOW, "\n\rBase shield:  %10ld", ships->base_shield);
      c_out(YELLOW, "   Base last boarded by: %s", ships->base_boarded);
      wait_for_enter();

      if (ships->base_hit_count != 0) {
         c_out(LIGHTRED, "\n\r   Base Damage:  %10d", ships->base_hit_count);
         wait_for_enter();
      }
      else {
	 c_out(YELLOW, "\n\r   Base not damaged");
         wait_for_enter();
      }

      c_out(WHITE, "\n\r");
      wait_for_enter();
   }

   if (strcmp(ships->base_death, "NONE")) {
      c_out(LIGHTRED, "\n\r--- Your base was destroyed by %s ---", ships->base_death);
      wait_for_enter();
   }

   for (dloop = oc = tc = 0; dloop < 10; dloop++) {
      if (ships->rem_xpos[dloop] == ONBOARD &&
         ships->rem_ypos[dloop] == ONBOARD) {
         if (tc == 0) {
            c_out(YELLOW, "\n\r");
            wait_for_enter();
         }
         c_out(YELLOW, "Remote %02d {on board}   ", (short)dloop);
         oc++;
         tc++;
         if (oc == 3) {
            c_out(YELLOW, "\n\r");
            wait_for_enter();
            oc = 0;
         }
      }
   }

   if (! from_sysop) {
      for (oc = tc = 0, dloop = 0; dloop < 10; dloop++) {
         if (hold[user_number]->xremotes[dloop] > 0 &&
            hold[user_number]->yremotes[dloop] > 0 &&
            ships->rem_xpos[dloop] != ONBOARD) {
            if (tc == 0) {
                c_out(YELLOW, "\n\r");
                wait_for_enter();
            }
            c_out(YELLOW, "Remote %02d {%04d-%04d} {Universe %02d}   ", (short)dloop,
               hold[user_number]->xremotes[dloop],
               hold[user_number]->yremotes[dloop],
               hold[user_number]->remote_universe[dloop]);
            oc++;
            tc++;
            if (oc == 2) {
               c_out(YELLOW, "\n\r");
               wait_for_enter();
               oc = 0;
            }
         }
      }
   }
   else {
      c_out(YELLOW,
         "\n\r--- Active Remote information is unavailable from SYSOP command\n\r");

      wait_for_enter();
   }

   for (dloop = 0; dloop < 10; dloop++) {
      if (ships->rem_xpos[dloop] < 0 && ships->rem_ypos[dloop] < 0 &&
         ships->rem_xpos[dloop] != ONBOARD) {
         if (Good_Hold(abs(ships->rem_xpos[dloop]))) {

            c_out(LIGHTRED, "\n\rRemote %02d destroyed by %s", (short)dloop,
               hold[abs(ships->rem_xpos[dloop])]->names);

            wait_for_enter();
         }
         else {
            log_error(114);
         }
      }
   }

   c_out(WHITE, "\n\r");
   wait_for_enter();

/*
   Does the ship have any of the slaver death weapon aboard? If
   so, display the description of the part.
*/

   if (! from_sysop) {
      for (dloop = aloop = 0; dloop < 10; dloop++) {
         if (goal_item[dloop] != (struct goal_elements *)NULL) {
            if (goal_item[dloop]->goal_on_ship == user_number) {
               if (aloop == 0) {
                  c_out(LIGHTRED, "Parts of the slaver death weapon on board:\n\r");
                  wait_for_enter();
               }
               c_out(LIGHTRED, "   %s\n\r", goal_item_description[dloop]);
               wait_for_enter();
               aloop++;
            }
         }
      }
      if (aloop > 0) {
         c_out(LIGHTRED, "   You have %d parts out of 10\n\r", aloop);
         wait_for_enter();
      }
      if (aloop == 10) {
         c_out(LIGHTRED, "---> You can now use the TRIGGER command <---\n\r");
         wait_for_enter();
      }
   }

   if (from_sysop) {
       c_out(LIGHTRED, "Hit [ENTER] to continue: ");
       timed_input(TRUE);
   }
}


