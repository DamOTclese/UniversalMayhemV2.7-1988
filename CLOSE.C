
/* **********************************************************************
   * close.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Go through the general area of the scanable range and find those	*
   * objects which are attackable. Plug the x and y positions of those	*
   * objects into integer arrays.					*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "comets.h"
#include "holder.h"
#include "goal.h"
#include "scout.h"
#include "stdio.h"
#include "function.h"
#include "mem.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long xsize, ysize;
   extern long xpos, ypos;
   extern UC zpos;
   extern unsigned short ion_trail;
   extern unsigned short comet_count;
   extern unsigned short close_ship[TOTAL_PLAYERS];
   extern unsigned short close_base[TOTAL_PLAYERS];
   extern unsigned short close_remotes[TOTAL_PLAYERS];
   extern unsigned short close_swarms[TOTAL_PLAYERS];
   extern unsigned short close_comets[40];
   extern unsigned short close_goals[10];
   extern unsigned short close_scouts[10];
   extern char goals_count;
   extern char swarm_test;
   extern short players;
   extern short ship_count;
   extern short base_count;
   extern short remote_count;
   extern short user_number;
   extern short total_comets;
   extern short swarm_count;
   extern char scout_count;

   static char count, dloop;

void plug_close_objects(char test_ion_state)
{
   ship_count = base_count = remote_count = 0;
   comet_count = swarm_count = goals_count = scout_count = 0;

/*
   Interestingly enough, these following memset's do in fact work.

   void	*_Cdecl memset	(void *s, int c, size_t n);

   is the prototype. Notice that par 's' points to anything. Normally,
   memset would only accept pointers to a character array. I don't know
   if this is a workable solution to trying to use an assembly language
   function for speed but that was its reason for useage.
*/

   if (test_ion_state == CLOSE_WARP) {
      memset(close_comets,  '\0', 40);
      memset(close_goals,   '\0', 10);
      plug_close_comets();
      plug_close_goals();
   }

   memset(close_ship,    '\0', players);
   memset(close_base,    '\0', players);
   memset(close_remotes, '\0', players);
   memset(close_swarms,  '\0', players);
   memset(close_comets,  '\0', 40);
   memset(close_goals,   '\0', 10);
   memset(close_scouts,  '\0', 10);

/*
   See if any ships are in the area.
*/

   for (count = 0; count < players; count++) {
      if (Good_Hold(count)) {
	 if (hold[count]->sxpos != 0 && hold[count]->sypos != 0) {
	    if (hold[count]->sxpos != xsize / 2 ||
	       hold[count]->sypos != ysize / 2) {
               plug_ship_close(test_ion_state == CLOSE_NORMAL);
	    }
	 }

/*
   See if any bases are in the area.
*/

	 if (hold[count]->bxpos != 0 && hold[count]->bypos != 0) {
	    if (hold[count]->bxpos != xsize / 2 ||
	       hold[count]->bypos != ysize / 2) {
	       plug_base_close();
	    }
	 }

/*
   See if any remotes or swarms are in the area.
*/

         plug_remotes_close();
         plug_close_swarms();
      }
   }

/*
   Find the location of the comets and goal items in this area.
*/

   plug_close_comets();
   plug_close_goals();
   plug_close_scouts();
}

/* **********************************************************************
   * See if there is a ship within the scanning range and if so, put it	*
   * into the array. This array will only be plugged with the record 	*
   * number of the enemy ship. The enemy ships cloak is checked against	*
   * the active ships sensors. If the cloak is greater, the enemy ship	*
   * is not appended to the array.					*
   *									*
   ********************************************************************** */

void plug_ship_close(char test_ion)
{
   if (hold[count]->sxpos > xpos - 6 && hold[count]->sxpos < xpos + 6 &&
      hold[count]->sypos > ypos - 9 && hold[count]->sypos < ypos + 9 &&
      hold[count]->szpos == zpos && count != user_number) {
         check_sensor_enemy(test_ion);
   }
}

/* **********************************************************************
   * Compare cloak against sensor.					*
   *									*
   * If the ship scanners are greater than the ships cloak, we set the	*
   * ship number into the close array, otherwise we set the ion_trail	*
   * flag to indicate that a ship is in the area.			*
   *									*
   ********************************************************************** */

void check_sensor_enemy(char test_ion)
{
   read_enemy(count);
   if (ships->ship_sensor < enemy->ship_cloak && test_ion) {
      ion_trail = TRUE;
   }
   else {
      close_ship[ship_count++] = count;
   }
}

/* **********************************************************************
   * See if there are any bases in the area. If there are, append the	*
   * ships record to the end of the array.				*
   *									*
   * In future versions, bases might be cloaked. It is here that the	*
   * base cloak would be check against the ships sensors.		*
   *									*
   ********************************************************************** */

void plug_base_close(void)
{
   if (hold[count]->bxpos > xpos - 5 && hold[count]->bxpos < xpos + 5 &&
      hold[count]->bypos > ypos - 9 && hold[count]->bypos < ypos + 9 &&
      hold[count]->bzpos == zpos) {
         close_base[base_count++] = count;
   }
}

/* **********************************************************************
   * Make a list of those comets which are in the area.			*
   *									*
   ********************************************************************** */

void plug_close_comets(void)
{
   unsigned char count;

   for (count = 0; count < total_comets; count++) {
      if (comets[count] != (struct comets_file *)NULL) {
         if (comets[count]->location[0] > xpos - 5 &&
            comets[count]->location[0] < xpos + 5 &&
            comets[count]->location[1] > ypos - 9 &&
            comets[count]->location[1] < ypos + 9) {
              close_comets[comet_count++] = count;
         }
      }
   }
}

/* **********************************************************************
   * Make a list of those scouts which are in the area.                 *
   *                                                                    *
   ********************************************************************** */

void plug_close_scouts(void)
{
   unsigned char sloop;
        
   for (sloop = 0; sloop < 10; sloop++) {
      if (scouts[sloop]->scout_direction != (char)NIL &&
         scouts[sloop]->scout_direction != SCOUT_DESTROYED) {
         if (scouts[sloop]->scout_xpos > xpos - 5 &&
            scouts[sloop]->scout_xpos < xpos + 5 &&
            scouts[sloop]->scout_ypos > ypos - 9 &&
            scouts[sloop]->scout_ypos < ypos + 9) {
              close_scouts[scout_count++] = sloop;
         }
      }
   }
}

/* **********************************************************************
   * See if there are any close slaver death weapon parts in the area.	*
   *									*
   ********************************************************************** */

void plug_close_goals(void)
{
   unsigned char count;

   for (count = 0; count < 10; count++) {
      if (goal_item[count] != (struct goal_elements *)NULL) {
         if (goal_item[count]->goal_xpos > xpos - 5 &&
            goal_item[count]->goal_xpos < xpos + 5 &&
            goal_item[count]->goal_ypos > ypos - 9 &&
            goal_item[count]->goal_ypos < ypos + 9 &&
            goal_item[count]->goal_on_ship == (char)NIL &&
            goal_item[count]->goal_universe == zpos) {
               close_goals[goals_count++] = count;
         }
      }
   }
}

/* **********************************************************************
   * If there are any remotes in the area, plug the enemy ships record	*
   * number to the end of the array.					*
   *									*
   ********************************************************************** */

void plug_remotes_close(void)
{
   unsigned char dloop;

   for (dloop = 0; dloop < 10; dloop++) {
      if (hold[count]->xremotes[dloop] != 0 &&
            hold[count]->yremotes[dloop] != 0) {
	 if (hold[count]->xremotes[dloop] > xpos - 5 &&
	    hold[count]->xremotes[dloop] < xpos + 5 &&
	    hold[count]->yremotes[dloop] > ypos - 9 &&
	    hold[count]->yremotes[dloop] < ypos + 9 &&
	    hold[count]->xremotes[dloop] != ONBOARD &&
	    hold[count]->remote_universe[dloop] == zpos) {
	    close_remotes[remote_count++] = count;
	 }
      }
   }
}

/* **********************************************************************
   * An important function of the Universal Mayhem game is the attack	*
   * swarms. In the first BASIC version release, we didn't have them.	*
   *									*
   * See if there are any in the area and if so, append the ship number	*
   * to the close array.						*
   *									*
   ********************************************************************** */

void plug_close_swarms(void)
{
   unsigned char dloop;

   for (dloop = 0; dloop < 15; dloop++) {
      if (hold[count]->xswarm[dloop] != 0 && hold[count]->yswarm[dloop] != 0) {
         if (hold[count]->xswarm[dloop] != (short)NIL &&
	    hold[count]->xswarm[dloop] > xpos - 5 &&
	    hold[count]->xswarm[dloop] < xpos + 5 &&
	    hold[count]->yswarm[dloop] > ypos - 9 &&
	    hold[count]->yswarm[dloop] < ypos + 9 &&
	    hold[count]->swarm_universe[dloop] == zpos) {
	    close_swarms[swarm_count++] = count;
	 }
      }
   }
}

/* **********************************************************************
   * A fast way of seeing if there are attack sleds in the area was	*
   * needed. This is the result. Hopefully, the speed is in the nested	*
   * if's. None of the non-primary if's will be executed if a pre-	*
   * ceeding level if fails.						*
   *									*
   * The biggest aspect of this function is the testing of swarm_test.	*
   * It will be 0 if the ship has not moved and this function does not	*
   * need to be executed. It will be 1 if the warp was performed or the	*
   * program has just be executed.					*
   *									*
   ********************************************************************** */

unsigned short swarm_sensor_scan(long thex, long they)
{
   unsigned char s_loop, t_loop;

   if (swarm_test == 0) {
      if (swarm_count == 0) {
	 return(FALSE);
      }
      else {
         return(TRUE);
      }
   }

   swarm_count = 0;

   for (s_loop = 0; s_loop < players; s_loop++) {
      if (Good_Hold(s_loop) && s_loop != user_number) {
         for (t_loop = 0; t_loop < 15; t_loop++) {
            if (hold[s_loop]->xswarm[t_loop] != (short)NIL &&
               hold[s_loop]->xswarm[t_loop] > thex - 5 &&
               hold[s_loop]->xswarm[t_loop] < thex + 5 &&
               hold[s_loop]->yswarm[t_loop] > they - 9 &&
               hold[s_loop]->yswarm[t_loop] < they + 9 &&
               hold[s_loop]->swarm_universe[t_loop] == zpos) {
               close_swarms[swarm_count++] = s_loop;
            }
	 }
      }
   }

   swarm_test = 0;

   if (swarm_count == 0) {
      return(FALSE);
   }
   else {
      return(TRUE);
   }
}

