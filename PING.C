
/* **********************************************************************
   * ping.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * This function will send a radar pulse to the enemy ships in the	*
   * area and return a strength, detailing the total accumulative enemy	*
   * ships available power / one million. 				*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long count;
   extern short ship_count;
   extern short pxpos, pypos;

void perform_ping(void)
{
   unsigned long quadrent_summation;
   short scount;
   char ping_add = 0;

   plug_close_objects(CLOSE_NO_ION);

   if (pxpos != (short)NIL) {
      c_out(LIGHTRED, "Pirates are in the area!\n\r");
      ping_add = 1;
   }
   else if (ship_count == 0) {
      c_out(WHITE, "Weapons officer reports no enemy ships in this area.\n\r");
      return;
   }

   c_out(WHITE, "Pinging...");

   for (scount = 0, quadrent_summation = 0; scount < ship_count; scount++) {
      quadrent_summation +=
         (enemy->ship_power / 1000000L) +
         (enemy->ship_shield / 1000000L) +
         (enemy->base_shield / 1000000L);
      quadrent_summation++;
   }

   if (ping_add == 1) {
      quadrent_summation += 27;
   }

   c_out(WHITE, " Ping return strength %ld pico-rems\n\r", quadrent_summation);
}

