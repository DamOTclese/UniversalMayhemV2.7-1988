
/* **********************************************************************
   * info.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991,                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Display price information for the planet we are currently docked	*
   * with. If we are not docked, offer an error message.		*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char to_save;
   extern long xpos;
   extern unsigned short docked;
   extern char the_name[31];
   extern short the_tech;
   extern short cost_energy, cost_cargo, cost_shuttle, cost_warp, cost_hull;
   extern short cost_cloak, cost_crew, cost_sensor, cost_torp, cost_remotes;
   extern short cost_sled;

void perform_info(long thex, long they)
{
   if (docked == 0 && thex == xpos && they == they) {
      c_out(LIGHTRED, "You are not docked with a planet!\n\r");
      return;
   }

   if (strcmp(the_name, "NONE") && to_save == 0) {
      c_out(LIGHTGREEN, "%s", the_name);
   }
   else {
      c_out(LIGHTGREEN, "Planet");
   }

   c_out(LIGHTGREEN, " at (%ld-%ld) technology level: -[%d]-\n\r", thex, they, the_tech);
   c_out(LIGHTGREEN, "------------------------------------------------\n\r");

   if (the_tech == 0) {
      return;
   }

   c_out(LIGHTGREEN, "Remote Robots:  %d\n\r", cost_remotes);
   c_out(LIGHTGREEN, "Energy:         %d\n\r", cost_energy);
   c_out(LIGHTGREEN, "Cargo:          %d\n\r", cost_cargo);
   c_out(LIGHTGREEN, "Shuttles:       %d\n\r", cost_shuttle);
   c_out(LIGHTGREEN, "Warp Drives:    %d\n\r", cost_warp);
   c_out(LIGHTGREEN, "Hull Patches:   %d\n\r", cost_hull);
   c_out(LIGHTGREEN, "Cloaks:         %d\n\r", cost_cloak);
   c_out(LIGHTGREEN, "Crew Members:   %d\n\r", cost_crew);
   c_out(LIGHTGREEN, "Sensors:        %d\n\r", cost_sensor);
   c_out(LIGHTGREEN, "Torpedos:       %d\n\r", cost_torp);
   c_out(LIGHTGREEN, "Attack sleds:   %d\n\r", cost_sled);
}

