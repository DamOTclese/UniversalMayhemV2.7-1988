
/* **********************************************************************
   * leave.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991                                    *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Allow shore leave for the crew. This brings morale up after being	*
   * in space for a long time. On some occasions, plague may have been	*
   * left by an enemy ship. On others, a "social" infestation may be a	*
   * problem.								*
   *									*
   * If 'the_plague' is anything other than 'NIL', then the planet	*
   * shore leave was ordered on is infected.				*
   *									*
   * If the planet is infected, we set the 'infect_count' variable to	*
   * some number from 4 to 20. This value is used to determine when to	*
   * start haveing fatalities. Every time a command is offered from the	*
   * command prompt we subtract 1 from this variable. When it hits 1,	*
   * we kill off some people. We have a 1 in 10 chance of setting this	*
   * variable again to a number from 4 to 2, killing more people.	*
   *									*
   * While a ship is infected and it visits other planets, it will	*
   * infect those planets. Free of charge!				*
   *									*
   ********************************************************************** */

#include <conio.h>
#include <stdio.h>
#include "defines.h"
#include "ship.h"
#include "planets.h"
#include "function.h"
#include "landing.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern unsigned short docked;
   extern char the_plague;
   extern short infect_count;
   extern char shield_repair;
   extern char extended_leave;
   extern char *record;
   extern short user_number;

void perform_leave(void)
{
    char forced_landing;

    if (docked == 0) {
        c_out(LIGHTRED, "You are not docked with a planet.\n\r");
        return;
    }

    if (planets.technology == 0) {
        if (ships->ship_crew > 40) {

            c_out(LIGHTRED,
                "Shore leave party are all dead! Planet was destroyed!\n\r");

            ships->ship_crew = 40;
            write_user();
            return;
        }
        else {
            c_out(LIGHTRED, "Not enough crew members to operate the ship!\n\r");
            return;
        }
    }

    c_out(WHITE, "Are you planning to take over the planet by force? ");
    timed_input(FALSE); /* Allow echo */
    ucase(record);
    if (record[0] == 'Y') {
        forced_landing = TRUE;
    }
    else {
        forced_landing = FALSE;
    }

    if (! extended_leave && forced_landing) {
        c_out(LIGHTRED, "Attack force landing isn't enabled on this system.\n");
        return;
    }

    if (! forced_landing) {
        c_out(WHITE, "Shore leave has been issued for all crew members.\n\r");
        ships->ship_morale = 100;

        if (the_plague == (char)NIL) {
            write_user();
            return;
        }

        infect_count = arandom(4L, 20L);
    }

    c_out(LIGHTRED, "Landing party ready. Weapons checked and stored.\n");

    c_out(LIGHTRED,
        "Do you really wish to leave your ship and head the attack party? ");

    timed_input(FALSE);
    ucase(record);

    if (record[0] != 'Y') {
        c_out(LIGHTGREEN, "Attack Landing party has received stand-down orders\n");
        return;
    }

    landing_party();
}

/* **********************************************************************
   * What's the morale level aboard ship? Should something be done?     *
   *                                                                    *
   * The whole idea of allowing a crew shore leave is to make sure they *
   * don't start to do bad things like drop a torp they're supposed to  *
   * be loading up into the tubes.                                      *
   *                                                                    *
   * o Revolt -                                                         *
   *   To put down the revolution, from 90 to 95 percent of the crew    *
   *   is killed by the loyal officers.                                 *
   *                                                                    *
   *   Main power depleated to maintain force shields around critical   *
   *   engineering areas and command areas.                             *
   *                                                                    *
   *   Main defense shield damage beyond remaining crews ability to     *
   *   repair. Docking at a planet is needed for repair.                *
   *                                                                    *
   ********************************************************************** */

void test_morale_level(void)
{
    unsigned long r_number;
    unsigned long the_total;

    read_user();

    if (ships->ship_morale == 10) {
        ships->ship_morale--;

        c_out(LIGHTRED,
            "CREW IS REVOLTING! They demanded shore leave and didn't get it!\n\r");

        c_out(LIGHTRED,
            "The battle to put down the revolt didn't go so well:\n\r");

        r_number = arandom(90l, 95l);
        the_total = (ships->ship_crew * (r_number / 100));
        ships->ship_crew -= the_total;

        c_out(LIGHTRED, "   - %ld crew members killed, %d loyal remaining\n\r",
            the_total, ships->ship_crew);

        r_number = arandom(91l, 97l);
        the_total = (ships->ship_power * (r_number / 100));
        ships->ship_power -= the_total;

        c_out(LIGHTRED,
            "   - %ld unit power depleation for force shields to\n\r",
            the_total);

        c_out(LIGHTRED,
            "     protect engineering and command areas, %ld remain\n\r",
            ships->ship_power);

        ships->ship_shield = 0l;

        c_out(LIGHTRED,
            "   - Defense shield not repairable. Need a planet for repairs\n\r");

        shield_repair = 1;
        ships->ship_morale = 101;
        write_user();
    }
}


