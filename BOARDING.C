
/* **********************************************************************
   * Boarding.C - Universal Mayhem, Version 2.7, Copyright Fredric L.   *
   * Rice, 1991. All rights reserved.                                   *
   *                                                                    *
   ********************************************************************** */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "function.h"

/* **********************************************************************
   * The ship lay-out.                                                  *
   *                                                                    *
   ********************************************************************** */

    static char ship_matrix[36][4] = {
         0,  2,  7,  0,                 /* Level 1 */
         0,  0,  8,  1,
         0,  0,  9,  0,
         0,  5,  0,  0,
         0,  6, 11,  4,
         0,  0,  0,  5,
         1,  0, 13,  0,                 /* Level 2 */
         2,  9,  0,  0,
         3, 10, 15,  8,
         0,  0,  0,  9,
         5, 12, 17,  0,
         0,  0, 18, 11,
         7, 14,  0,  0,                 /* Level 3 */
         0, 15,  0, 13,
         9,  0, 21, 14,
         0, 17, 22,  0,
        11,  0,  0, 16,
        12,  0, 24,  0,
         0,  0, 25,  0,                 /* Level 4 */
         0, 21, 26,  0,
        15,  0,  0, 20,
        16,  0,  0,  0,
         0, 24, 29,  0,
        18,  0, 30, 23,
        19, 26, 31,  0,                 /* Level 5 */
        20,  0,  0, 25,
        0,  28, 33,  0,
        0,   0, 34, 29,
        23,  0,  0,  0,
        24,  0, 36,  0,
        25, 32,  0,  0,                 /* Level 6 */
        0,  33,  0, 31,
        29,  0,  0, 32,
        28, 35,  0,  0,
        0,  36,  0, 34,
        30,  0,  0, 35
    } ;

    static char *room_descriptions[36] = {
        "cargo",                        /* Level 1 */
        "",
        "cargo",
        "",
        "crew",
        "",
        "",                             /* Level 2 */
        "",
        "cargo",
        "",
        "weapons",
        "crew",
        "",                             /* Level 3 */
        "gally-1",
        "",
        "",
        "gally-2",
        "",
        "crew",                         /* Level 4 */
        "crew",
        "crew",
        "crew",
        "crew",
        "crew",
        "",                             /* Level 5 */
        "",
        "environment",
        "",
        "transport",
        "",
        "",                             /* Level 6 */
        "tractor",
        "",
        "engineering",
        ""
        "bridge"
    } ;


/* **********************************************************************
   * We are going to execute the extended boarding program.             *
   *                                                                    *
   * We pass what it is we are boarding and which enemy player it is    *
   * we are boarding.                                                   *
   *                                                                    *
   * ship_or_base is TRUE if it's a ship, else it's FALSE if a base.    *
   *                                                                    *
   ********************************************************************** */

void boarding_party(char ship_or_base, short which)
{
    if (which == ship_or_base) return;

}


