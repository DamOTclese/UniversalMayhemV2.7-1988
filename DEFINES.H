
#ifndef __TRUTHS
#define __TRUTHS
#define TRUE            1
#define FALSE           0
#endif

#define LIMITS          2
#define COST            3
#define TOTAL_PLAYERS   250
#define THE_VERSION     "2.7"
#define ONBOARD         -9999
#define UL              unsigned long
#define UC              unsigned char
#define TAXES           .0003
#define TAXES_DUE       1000
#define TIMED_OUT       50
#define BACKSPACE       0x08
#define SPACE           0x20
#define ESC             0x1b
#define CONTROL_X       0x18
#define AUCTION_SHIP    -1
#define ION_COUNT       50
#define X_WARP_WITHIN   5
#define Y_WARP_WITHIN   8
#define MAX_COMMAND     10
#define NIL             -1
#define HIGHEST_HELP    46
#define NOT_OWNED       99
#define IS_OWNED        NOT_OWNED + 1
#define PLAYER_OWNED    IS_OWNED + 1
#define OWNABLE         100

/*
   Scout ship status
*/

#define SCOUT_INBOUND   0
#define SCOUT_OUTBOUND  1
#define SCOUT_STATION   2
#define SCOUT_DESTROYED 3

/*
   Plugging of close objects status
*/

#define CLOSE_NORMAL    0   /* Test for ION storms                      */
#define CLOSE_NO_ION    1   /* Don't test for ION storms                */
#define CLOSE_WARP      2   /* Only test comets and slaver death parts  */

#define skipspace(x)    while (*x == 0x20 || *x == 0x09) { x++; }
#define Good_Hold(x)    (hold[(x)] != (struct holder *)NULL)
#define Reset_Timer     1   /* How long of inactivity before keyboard reset */
#define MACROS          5   /* How many macros do we have?              */

#define STRNCPY(x, y, z) { strncpy(x, y, z); x[z] = (char)NULL; }

