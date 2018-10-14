
/* **********************************************************************
   * destruct.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow a ship to destroy itself. You must get a password, however.	*
   *									*
   * After some discussions with some engineers at work, it was desided	*
   * that if there are ships in the area, they should be damaged or	*
   * destroyed by the explosion that ensues when a ship self-destructs.	*
   *									*
   * o If the ship is docked, fragments should rain down on the planet	*
   *									*
   * o If the ship is at the base, fragments should pepper the base	*
   *									*
   * o If the ship is boarding another ship, the ship being boarded	*
   *   should be destroyed. The name of the ship that caused the	*
   *   destruction should be credited with the kill.			*
   *									*
   * o Depending on the strength of the shields of the ships in the	*
   *   area, cause damage to the ships or destroy them.			*
   *									*
   * AN IMPORTANT update was added around 26/Dec/88: When a ship kills	*
   * itself, all of the planets that are owned by it, (if any), are	*
   * set back to unprotected mode.					*
   *									*
   * In addition, if the self destruct destroys another ship, all the	*
   * planets that belong to it are likewise returned to unprotected.	*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdio.h"
#include "alloc.h"
#include "function.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long xpos, ypos;
   extern UC zpos;
   extern long count;
   extern unsigned short docked, base_docked;
   extern unsigned short close_ship[TOTAL_PLAYERS];
   extern char *record;
   extern short ship_count;
   extern short user_number;

void perform_destruct(void)
{
   read_user();
   c_out(LIGHTRED, "Self destruct sequence activated.\n\r\n\r");
   c_out(LIGHTRED, "Awaiting your ships password to detonate: ");
   timed_input(1);
   ucase(record);

   if (strcmp(record, ships->ship_pass)) {
      c_out(WHITE, "Incorrect password for destruct sequence...\n\r");
      return;
   }

   hand_over_planets(user_number, (short)NIL);
   return_slaver_parts(user_number, (char)NIL);
   remove_command_file(ships->ship_name, hold[user_number]->names);
   make_zero_record();
   write_user();
   plug_close_objects(CLOSE_NO_ION);
   c_out(LIGHTRED, "Gertzits! Blampo - Ship destroyed!\n\r\n\r");

   if (docked == 1) {
      c_out(LIGHTRED, "Fragments are falling on the planet below!\n\r");
   }

   if (base_docked == 1) {
      c_out(LIGHTRED, "Base station crew members watch mother ships destruction!\n\r");
   }

   if (ship_count == 0) {
      perform_quit(1);
   }

/*
   There are other ships within the scannable range. See if we are
   boarding an enemy ship. If so, destroy that enemy ship.
*/

   for (count = 0; count < ship_count; count++) {
      if (hold[close_ship[count]]->sxpos == xpos &&
	 hold[close_ship[count]]->sypos == ypos &&
	 hold[close_ship[count]]->szpos == zpos &&
	 user_number != close_ship[count]) {
	 is_boarding(close_ship[count]);
      }
      else if (user_number != close_ship[count]) {
	 check_enemy_strength(close_ship[count]);
      }
   }

   perform_quit(1);
}

/* **********************************************************************
   * See if the ship we are on has an intact hull. If so return,	*
   * otherwise destroy the ship we are sitting on.			*
   *									*
   ********************************************************************** */

void is_boarding(short the_ship)
{
   if (enemy->ship_hull > 99 || enemy->ship_shield > 1000000L) {
      return;
   }

   c_out(LIGHTRED, "Boarded ship %s destroyed by explosion!\n\r",
      hold[the_ship]->names);

   read_enemy(the_ship);
   enemy->ship_xpos = enemy->ship_ypos = enemy->ship_universe = 0;
   enemy->leashed_to = enemy->leashed_by = (char)NIL;
   strcpy(enemy->who_destroyed, hold[user_number]->names);
   hand_over_planets(the_ship, (short)NIL);
   return_slaver_parts(the_ship, (char)NIL);
   write_enemy(the_ship);
}

/* **********************************************************************
   * See if the ship in the area has shields greater than 100,000 units	*
   * and if so, report that it's not hurt and return. Otherwise, see if	*
   * the shields are 50,000 or better. If so, simply say that damage 	*
   * from the explosion was repaired. If shields are less than 50,000,	*
   * bring the shields down compleatly.					*
   *									*
   ********************************************************************** */

void check_enemy_strength(short the_ship)
{
   read_enemy(the_ship);
   if (enemy->ship_shield > 100000L) {
      c_out(YELLOW, "Ship %s not hurt by the explosion\n\r", hold[the_ship]->names);
      return;
   }

   if (enemy->ship_shield > 50000L) {
      c_out(YELLOW, "Ship %s repairing minor damage caused by explosion!\n\r",
	 hold[the_ship]->names);
      return;
   }

   c_out(YELLOW, "Ship %s shields are down, status: ", hold[the_ship]->names);
   enemy->ship_shield = 0;
   enemy_shield_down(the_ship);

   if (enemy->ship_hull > 27) {
      write_enemy(the_ship);
      return;
   }

   hand_over_planets(the_ship, (short)NIL);
   return_slaver_parts(the_ship, (char)NIL);
   enemy->ship_xpos = enemy->ship_ypos = enemy->ship_universe = 0;
   enemy->leashed_to = enemy->leashed_by = (char)NIL;
   strcpy(enemy->who_destroyed, hold[user_number]->names);
   write_enemy(the_ship);
}
