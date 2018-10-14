
/* **********************************************************************
   * nav.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow the storage/retrieval information on up to ten planets.      *
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "planets.h"
#include "stdio.h"
#include "function.h"
#include "stdlib.h"
#include "ctype.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern long xpos, ypos;
   extern UC zpos;
   extern long count;
   extern unsigned short docked;
   extern char *record, *go_string;
   extern short the_tech;
   extern short cost_energy, cost_cargo, cost_shuttle, cost_warp, cost_hull;
   extern short cost_cloak, cost_crew, cost_sensor, cost_torp, cost_remotes;
   extern short cost_sled;
   extern short the_rnd;
   static void store_current_posistion(void);

void perform_nav(void)
{
   short nav_bank, the_option;
   long target_x, target_y;
   char appending[10], hold_go[10];
   int test_warp, small_jump;
   char data_banked;
   char *patpoint;
   char go_test[201];
   char to_remote;

   data_banked = FALSE;

/*
    See if it's also offering an option number
*/

   patpoint = record;
   patpoint += 4;
   skipspace(patpoint);
   the_option = nav_bank = 99;

   if (*patpoint >= '0' && *patpoint <= '6') {
      nav_bank = (*patpoint) - '0';
      patpoint++;
      skipspace(patpoint);
      if (*patpoint >= '0' && *patpoint <= '9') {
         the_option = atoi(patpoint);
         if (the_option < 0 || the_option > 9) {
            return;
         }
         while (*patpoint >= '0' && *patpoint <= '9') {
            patpoint++;
         }
      }
      else {
         nav_bank = 99;
      }
   }

   for (count = 0; count < 10; count++) {
      if (ships->x_planet_info[count] == 0) {
         if (nav_bank == 99 || the_option == 99) {
            c_out(WHITE, "Planet data bank %ld is empty\n\r", count);
         }
      }
      else {
         if (ships->point_tag[count][0] == '+') {
            if (nav_bank == 99 || the_option == 99) {

               c_out(WHITE, "Planet data bank %d for planet at [%ld - %ld]",
                  (short) count,
                  ships->x_planet_info[count],
                  ships->y_planet_info[count]);
            }

            if (xpos == ships->x_planet_info[count] &&  ypos == ships->y_planet_info[count]) {
               data_banked = TRUE;
               if (nav_bank == 99 || the_option == 99) {
                  c_out(WHITE, " ---> Current");
               }
            }

            if (nav_bank == 99 || the_option == 99) {
               c_out(WHITE, "\n\r");
            }
         }
         else {
            if (nav_bank == 99 || the_option == 99) {

               c_out(WHITE, "Data bank %d: Posistion tag [%ld - %ld]: '%s'",
                   (short) count,
                  ships->x_planet_info[count],
                  ships->y_planet_info[count],
                  ships->point_tag[count]);
            }

            if (xpos == ships->x_planet_info[count] &&
                ypos == ships->y_planet_info[count]) {
               if (nav_bank == 99 || the_option == 99) {
                  c_out(WHITE, " ---> Current");
               }
            }
         
            if (nav_bank == 99 || the_option == 99) {
               c_out(WHITE, "\n\r");
            }
         }
      }
   }

   c_out(WHITE, "\n\r");

   if (docked == 1 && !data_banked) {
      c_out(WHITE, "You are docked.\n\r");
   }

   if (nav_bank == 99) {
      c_out(WHITE, "1 ... EMPTY a data bank of planet information\n\r");
      c_out(WHITE, "2 ... VIEW a planets commerce information\n\r");
      c_out(WHITE, "3 ... Try to WARP to a specified PLANET or TAG\n\r");
      c_out(WHITE, "4 ... Store current posistion and tag it with a name\n\r");
      c_out(WHITE, "5 ... Store current planets posistion into nav banks\n\r");
      c_out(WHITE, "6 ... Try to WARP to a specific REMOTES location\n\r");
      c_out(WHITE, "\n\r     Which operation do you want to perform? ");
      timed_input(0);
      nav_bank = atoi(record);
      if (nav_bank < 1 || nav_bank > 6) return;
   }

/*
   If it's option 1...

   Here we want to be able to empty a data bank.
*/

   if (nav_bank == 1) {
      if (the_option == 99) {
         c_out(WHITE, "Empty which bank? (0 to 9): ");
         timed_input(0);
         nav_bank = atoi(record);
         if (nav_bank < 0 || nav_bank > 9 || record[0] == (char)NULL) {
            return;
         }
      }
      else {
         nav_bank = the_option;
      }
      if (ships->x_planet_info[nav_bank] == 0) {
	 c_out(WHITE, "That data bank was already empty. Nothing done.\n\r");
	 return;
      }
      ships->x_planet_info[nav_bank] = 0;
      ships->y_planet_info[nav_bank] = 0;
      c_out(WHITE, "Nav bank %d dumped\n\r", nav_bank);
      write_user();
      return;
   }

/*
   If it's option 2...

   Here we want to be able to view a data bank.
*/

   if (nav_bank == 2) {
      if (the_option == 99) {
         c_out(WHITE, "View commerce information from which nav bank? (0 to 9): ");
         timed_input(0);
         nav_bank = atoi(record);

         if (nav_bank < 0 || nav_bank > 9 || record[0] == (char)NULL) {
            return;
         }
      }
      else {
         nav_bank = the_option;
      }

      if (ships->x_planet_info[nav_bank] == 0) {
         c_out(WHITE, "Planet data bank %d is empty!\n\r", nav_bank);
         return;
      }

      read_universe(ships->x_planet_info[nav_bank]);

      if (find_specific_planet(ships->x_planet_info[nav_bank],
         ships->y_planet_info[nav_bank]) == 0) {
         c_out(WHITE, "\n\rThis is a dead planet!\n\r");
         return;
      }

      read_planets(ships->x_planet_info[nav_bank]);
      the_rnd = planets.cost;
      the_tech = planets.technology;

      cost_remotes = (int) (111 * the_rnd) + 10104;
      cost_energy =  (int) (1 * the_rnd)   + 15;
      cost_cargo =   (int) (15 * the_rnd)  + 100;
      cost_shuttle = (int) (127 * the_rnd) + 1000;
      cost_warp =    (int) (191 * the_rnd) + 1000;
      cost_hull =    (int) (101 * the_rnd) + 100;
      cost_cloak =   (int) (178 * the_rnd) + 121;
      cost_crew =    (int) (13 * the_rnd)  + 10;
      cost_sensor =  (int) (132 * the_rnd) + 120;
      cost_torp =    (int) (17 * the_rnd)  + 11;
      cost_sled =    (int) (211 * the_rnd)  + 2010;
      perform_info(
         ships->x_planet_info[nav_bank], ships->y_planet_info[nav_bank]);

      return;
   }

   if (nav_bank == 4) {
      store_current_posistion();
      return;
   }

/*
   Could be option 5
*/

   if (nav_bank == 5) {
      if (docked == 1 && !data_banked) {
         if (the_option == 99) {
            c_out(WHITE, "Store into which data bank? (0 to 9): ");
            timed_input(0);
            nav_bank = atoi(record);
            if (nav_bank < 0 || nav_bank > 9 || record[0] == (char)NULL) {
               return;
            }
         }
         else {
            nav_bank = the_option;
         }
         ships->x_planet_info[nav_bank] = xpos;
         ships->y_planet_info[nav_bank] = ypos;
         ships->point_tag[nav_bank][0] = '+';
         c_out(WHITE, "Computer nav data bank updated with current planet.\n\r");
         write_user();
         return;
      }
      else {
         c_out(LIGHTRED, "There is no planet here!\n\r");
      }
   }

/*
   If it's option three... Or six, added later.

   Here we want to try to warp to the nav planet. I have noticed that it
   was possible to warp to the currently docked planet. Now we need to see
   if the planet we are on, if any, is the planet requested from the nav
   bank.

   Added 18/Mar/91 was the NAV to a remotes location if option 6.
*/

   if (nav_bank == 3) {
      to_remote = FALSE;
   }
   else {
      to_remote = TRUE;
   }

   if (the_option == 99) {
      go_test[0] = (char)NULL;
      if (! to_remote) {
         c_out(WHITE, "Warp to planet in which nav bank? (0 to 9): ");
      }
      else {
         c_out(WHITE, "Enter remote number to warp to? (0 to 9): ");
      }
      timed_input(0);
      nav_bank = atoi(record);

      if (nav_bank < 0 || nav_bank > 9 || record[0] == (char)NULL) {
         return;
      }
   }
   else {
      nav_bank = the_option;
   }

   if (to_remote) {
      if (ships->rem_xpos[nav_bank] == 0 && ships->rem_ypos[nav_bank] == 0) {
         c_out(WHITE, "That remote is not bought yet!\n\r");
         return;
      }

      if (ships->rem_xpos[nav_bank] == ONBOARD && ships->rem_ypos[nav_bank] == ONBOARD) {
         c_out(WHITE, "That remote has not been deployed yet!\n\r");
         return;
      }

      if (ships->rem_universe[nav_bank] != zpos) {
         c_out(WHITE, "That remote is in another universe!\n\r");
         return;
      }

      target_x = ships->rem_xpos[nav_bank];
      target_y = ships->rem_ypos[nav_bank];
   }
   else {
      if (ships->x_planet_info[nav_bank] == 0) {
         c_out(WHITE, "Planet data bank %d is empty!\n\r", nav_bank);
         return;
      }

      target_x = ships->x_planet_info[nav_bank];
      target_y = ships->y_planet_info[nav_bank];
   }

   if (xpos == target_x && ypos == target_y) {
      c_out(WHITE, "You are already at the destination sector!\n\r");
      return;
   }

   if (xpos == target_x) {
      goto skip_xpos;
   }

   if (xpos - target_x < 1) {
      strcpy(go_test, "W8");
      strcpy(appending, ";W8");
   }
   else {
      strcpy(go_test, "W2");
      strcpy(appending, ";W2");
   }

   test_warp = (short) abs(xpos - target_x);
   small_jump = test_warp;

   while (small_jump > ships->ship_warp) {
      sprintf(hold_go, "%d", ships->ship_warp);
      strcat(go_test, hold_go);
      if (strlen(go_test) > 190) {
	 c_out(WHITE, "\n\rDistance is just too great to warp direct!\n\r");
	 return;
      }
      small_jump -= ships->ship_warp;
      if (small_jump > 0) {
	 strcat(go_test, appending);
      }
   }

   if (small_jump > 0) {
      sprintf(hold_go, "%d", small_jump);
      strcat(go_test, hold_go);
   }

skip_xpos:
   if (xpos != target_x) {
      strncat(go_test, ";", 1);
   }

   if (ypos == target_y) {
      goto skip_ypos;
   }

   if (ypos - target_y < 1) {
      strcat(go_test, "W6");
      strcpy(appending, ";W6");
   }
   else {
      strcat(go_test, "W4");
      strcpy(appending, ";W4");
   }

   small_jump = (short) abs(ypos - target_y);
   test_warp += small_jump;

   while (small_jump > ships->ship_warp) {
      sprintf(hold_go, "%d", ships->ship_warp);
      strcat(go_test, hold_go);
      if (strlen(go_test) > 190) {
	 c_out(WHITE, "\n\rDistance is just too great to warp direct!\n\r");
	 return;
      }
      small_jump -= ships->ship_warp;
      if (small_jump > 0) {
	 strcat(go_test, appending);
      }
   }

   if (small_jump > 0) {
      sprintf(hold_go, "%d", small_jump);
      strcat(go_test, hold_go);
   }

skip_ypos:
   strcpy(go_string, go_test);
   perform_go();
}

/* **********************************************************************
   * Allow the storage of current posistion and give it a name.         *
   *                                                                    *
   ********************************************************************** */

static void store_current_posistion(void)
{
   char look_point, empty;

   empty = 99;

   for (look_point = 0; look_point < 10; look_point++) {
      if (xpos == ships->x_planet_info[look_point] &&
             ypos == ships->y_planet_info[look_point]) {

         if (ships->point_tag[look_point][0] != '-') {

            if (ships->point_tag[look_point][0] == '+') {
                c_out(LIGHTRED, "Already tagged as a planet...\n\r");
            }
            else {
                c_out(LIGHTRED, "Location already tagged as '%s'\n\r",
                    ships->point_tag[look_point]);
            }

            c_out(WHITE, "Do you wish to rename tag? ");
            timed_input(FALSE);
            ucase(record);

            if (record[0] != 'Y') {
                return;
            }

            empty = look_point;
         }
      }
      if (ships->x_planet_info[look_point] == 0 &&
          ships->y_planet_info[look_point] == 0) {
         if (empty == 99) {
            empty = look_point;
         }
      }
   }

   if (empty == 99 || empty == 10) {
      c_out(LIGHTRED, "No room to store current ships location\n\r");
      return;
   }

   c_out(LIGHTGREEN, "\n\rEnter tag name: (20 characters maximum): ");
   timed_input(0);

   ships->x_planet_info[empty] = xpos;
   ships->y_planet_info[empty] = ypos;
   STRNCPY(ships->point_tag[empty], record, 20);

   c_out(WHITE, "Nav bank %d updated with [%ld-%ld] as '%s'.\n\r",
       empty, xpos, ypos, ships->point_tag[empty]);

   write_user();
   return;
}


