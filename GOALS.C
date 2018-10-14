
/* **********************************************************************
   * goals.c								*
   *									*
   * Copyright 1988, 1989, 1990, 1991.                                  *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * This module will create and maintain the goal items that mayhem	*
   * uses to determine if someone has actually won the highest possible	*
   * "winning" that can be won... Sorry.		`		*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "goal.h"
#include "stdio.h"
#include "functions.h"
#include "alloc.h"
#include "io.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern FILE *agoals;
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern UC zpos;
   extern long aloop, count;
   extern short players;
   extern short user_number;

/* **********************************************************************
   * There are various items that must be found in order to put 	*
   * together the slaver death weapon. A description of the individual	*
   * parts follows:							*
   *									*
   ********************************************************************** */

   char *goal_item_description[10] = {
      "dual frequency bipolar power pack",
      "semi-transparrent liquid teramodulator",
      "spherical wave guide director",
      "magnetic field grip",
      "fully integrated infrared site",
      "monodirectional bounce/return auditator",
      "status/display information screen",
      "back-blast heat shield",
      "self deploying and locking mounting grid",
      "auto breach extending muzzle distender"
   } ;

/* **********************************************************************
   * Create the goals file.						*
   *									*
   ********************************************************************** */

void create_goal_file(void)
{
   char goal_count;

   if ((agoals = mayhem_fopen("GOALS.DAT", "wb", agoals)) == (FILE *)NULL) {
      c_out(LIGHTRED, "Unable to create goals file. No goals are active.\n\r");
      return;
   }

   for (goal_count = 0; goal_count < 10; goal_count++) {
      goal_item[goal_count] =
         (struct goal_elements *)farmalloc(sizeof(struct goal_elements));

      if (goal_item[goal_count] != (struct goal_elements *)NULL) {
         memory_allocated((UL)sizeof(struct goal_elements));
         goal_item[goal_count]->goal_xpos = arandom(1L, xsize);
         goal_item[goal_count]->goal_ypos = arandom(1L, ysize);
         goal_item[goal_count]->goal_universe = arandom(0L, 4L);
         goal_item[goal_count]->goal_on_ship = (char)NIL;
         write_goals((long)goal_count);
         memory_freed((UL)sizeof(struct goal_elements));
         farfree(goal_item[goal_count]);
      }
   }
   mayhem_fclose(&agoals);
}

/* **********************************************************************
   * Go through the goals file and read out the information into some	*
   * structures.							*
   *									*
   ********************************************************************** */

void plug_goal_data(void)
{
   char goal_count;

   if ((agoals = mayhem_fopen("GOALS.DAT", "r+b", agoals)) == (FILE *)NULL) {
      c_out(LIGHTRED, "Unable to open goals file. No goals are active.\n\r");
      return;
   }

   for (goal_count = 0; goal_count < 10; goal_count++) {
      goal_item[goal_count] =
         (struct goal_elements *)farmalloc(sizeof(struct goal_elements));

      if (goal_item[goal_count] != (struct goal_elements *)NULL) {
         memory_allocated((UL)sizeof(struct goal_elements));
         read_goals((long)goal_count);
      }
   }
}

/* **********************************************************************
   * When the program quits, we want to free up some memory.		*
   *									*
   ********************************************************************** */

void remove_goal_data(void)
{
   char goal_count;

   for (goal_count = 0; goal_count < 10; goal_count++) {
      if (goal_item[goal_count] != (struct goal_elements *)NULL) {
         memory_freed((UL)sizeof(struct goal_elements));
         farfree(goal_item[goal_count]);
      }
   }
}

/* **********************************************************************
   * Return the slaver device parts to the universe in a random place.	*
   *									*
   * If the 'because_of_status' is NIL, then the ship was destroyed by	*
   * stupidity or by an automated process. If this is so, the finnal	*
   * location of the devices should be redistributed about. If the	*
   * value is not NIL, then the ship was taken by another active 	*
   * player. If this is so, then the parts should revert to the 	*
   * location around the active ship.					*
   *									*
   ********************************************************************** */

void return_slaver_parts(short from_this_ship, char because_of_status)
{
   char afloop;

   for (afloop = 0; afloop < 10; afloop++) {
      if (goal_item[afloop] != (struct goal_elements *)NULL) {
         if (goal_item[afloop]->goal_on_ship == from_this_ship) {
            if (because_of_status == (char)NIL) {
               goal_item[afloop]->goal_xpos = arandom(1L, xsize - 1);
               goal_item[afloop]->goal_ypos = arandom(1L, ysize - 1);
               goal_item[afloop]->goal_universe = 0;
               goal_item[afloop]->goal_on_ship = (char)NIL;
            }
            else {
               goal_item[afloop]->goal_xpos = xpos;
               if (afloop < 6) {
                  goal_item[afloop]->goal_ypos = ypos - afloop;
               }
               else {
                  goal_item[afloop]->goal_ypos = ypos + (afloop - 5);
               }
               if (goal_item[afloop]->goal_ypos < 1 ||
                  goal_item[afloop]->goal_ypos > ysize - 1) {
                  goal_item[afloop]->goal_ypos = ypos;
               }
            }
            write_goals(afloop);
         }
      }
   }
}

/* **********************************************************************
   * Trigger the Slaver Death Device:					*
   *									*
   * o Redistribute the slaver parts throughout the universe		*
   *									*
   * o Destroy all enemy ships reguardless of alliances such that the	*
   *   owners will be informed that the ship triggering the device was	*
   *   the one that did it.						*
   *									*
   * o Accumulate ships kills.						*
   *									*
   ********************************************************************** */

void trigger_slaver_death_weapon(void)
{
   c_out(LIGHTRED, "Slaver Death Device triggered!\n\r\n\r");
   mayhem_fclose(&agoals);
   create_goal_file();
   read_user();

   for (aloop = 1; aloop < players; aloop++) {
      if (Good_Hold(aloop) && aloop != user_number) {
	 read_enemy(aloop);
         enemy->ship_xpos = enemy->ship_ypos = enemy->ship_universe = 0;
         enemy->leashed_to = enemy->leashed_by = (char)NIL;
         strcpy(enemy->who_destroyed, hold[user_number]->names);
	 hand_over_planets(aloop, user_number);
	 write_enemy(aloop);
         remove_command_file(enemy->ship_name, hold[user_number]->names);
         ships->total_kills++;
	 hold[user_number]->kills++;

	 c_out(YELLOW,
	    "Ship [%s] destroyed at [%05ld-%05ld] (Universe %d)\n\r",
	    hold[aloop]->names,
	    hold[aloop]->sxpos,
	    hold[aloop]->sypos,
	    hold[aloop]->szpos);

         inform_kill(hold[user_number]->names, hold[aloop]->names, TRUE, 0);
         memory_freed((UL)sizeof(struct holder));
         if_any_bounce_it(aloop);
         farfree(hold[aloop]);
	 hold[aloop] = (struct holder *)NULL;
      }
   }
   write_user();
   perform_stand(TRUE);
}

