
/* **********************************************************************
   * power.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow the player to redistribute power between engines and shields	*
   * and make sure that the player doesn't outstep the bounds.		*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "stdio.h"
#include "stdlib.h"
#include "function.h"
#include "conio.h"

   extern char *record;
   extern char shield_repair;

void perform_power(void)
{
   unsigned long total, formove;

   if (shield_repair == 1) {
      c_out(LIGHTRED, "Defense Shield was destroyed beyond repair by crew!\n\r");
      c_out(LIGHTRED, "You can't move power around safely!\n\r");
      return;
   }

   read_user();
   total = (unsigned long) ships->ship_power + ships->ship_shield;
   c_out(WHITE, "Total power available: %ld units.\n\r", (unsigned long) total);
   c_out(WHITE, "---------------------------------------\n\r");
   c_out(WHITE, "Enter the number of units to place into shield: ");
   timed_input(0);

   formove = (unsigned long) atol(record);

   if (formove > total || formove < 1) {
      c_out(LIGHTRED, "Redistribution of power did not take place.\n\r");
      return;
   }

   if (formove > 2000000000L) {
      c_out(LIGHTRED, "Shield can't hold that much power!\n\r");
      return;
   }

   ships->ship_shield = formove;
   ships->ship_power = total - formove;
   write_user();
}

