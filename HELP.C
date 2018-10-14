
/* **********************************************************************
   * help.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Display the commands which are available and offer a short		*
   * description for each. Also display the detailed extended help	*
   * value for each command.						*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "stdio.h"
#include "function.h"
#include "scout.h"
#include "conio.h"

/* **********************************************************************
   * Define the names of the various universes.                         *
   *                                                                    *
   ********************************************************************** */

   extern char ham_version;
   extern char *record;

   static char *universe_name[13] = {
        "Reticulum January psi-Horologium",
        "Puppis Febuary psi-Canopus",
        "Volans March psi-Carina",
        "Carina April psi-Crux",
        "Centarus May psi-Musca",
        "Lupus June psi-Circinus",
        "Ara July Triangulium psi-Australe",
        "Telescopium August psi-Pavo",
        "Indus September psi-Octans",
        "Grus October psi-Tucana",
        "Phoenix November psi-Tucana",
        "Hydrus November psi-Tucana",
        "Eridanus December delta-Horologium"
   } ;

/* **********************************************************************
   * Offer the initial help screen.                                     *
   *                                                                    *
   * If the file: MAYHEM.HLP can't be found, we offer one that's hard   *
   * coded into the program, otherwise we open up the file and display  *
   * its contents.                                                      *
   *                                                                    *
   * This means that we must maintain the information is two different  *
   * places but it allows people in other countries to exit the help    *
   * screen to other languages.                                         *
   *                                                                    *
   ********************************************************************** */

void perform_help(void)
{
    FILE *mayhem_help;

    if ((mayhem_help = fopen("MAYHEM.HLP", "r")) == (FILE *)NULL) {
        c_out(WHITE, "\n\rSCAN  - (HLP1)  sensor scan        PROTECT  - (HLP24) Save a planet");
        c_out(WHITE, "\n\rWdv   - (HLP2)  Warp drive         GO       - (HLP25) Goto a planet");
        c_out(WHITE, "\n\rRPTn  - (HLP3)  Repeat n times     LEAVE    - (HLP26) Shore leave");
        c_out(WHITE, "\n\rQUIT  - (HLP4)  Return to BBS      TAXES    - (HLP27) Pay your taxes");
        c_out(WHITE, "\n\rSTAT  - (HLP5)  Ships status       COMMAND  - (HLP28) Command files");
        c_out(WHITE, "\n\rINFO  - (HLP6)  Prices on planet   SPY      - (HLP29) + Remote operations");
        c_out(WHITE, "\n\rTdv   - (HLP7)  Fire torpedo       PLAGUE   - (HLP30) Bio Weapons");
        c_out(WHITE, "\n\rBOARD - (HLP8)  Board enemy        DESTRUCT - (HLP31) Self Destruct");
        c_out(WHITE, "\n\rPOWER - (HLP9)  Shield Power       COMETS   - (HLP32) Comets/ion info");
        c_out(WHITE, "\n\rSiv   - (HLP10) Sell 'S' -help     PING     - (HLP33) Enemy strength");
        c_out(WHITE, "\n\rBiv   - (HLP11) Buy 'B' -help      SLED     - (HLP34) Attack Sleds");
        c_out(WHITE, "\n\rBASE  - (HLP12) Base functions     AUCTION  - (HLP35) Buy taxed ship");
        c_out(WHITE, "\n\rA     - (HLP13) Do it again        NAV      - (HLP36) Nav planets");
        c_out(WHITE, "\n\rMSG   - (HLP14) Subspace msgs      SYSOP    - (HLP37) SysOp functions");
        c_out(WHITE, "\n\rTIME  - (HLP15) Time remaining     PASS     - (HLP38) Change password");
        c_out(WHITE, "\n\rSTAND - (HLP16) Top ten ships      COLOR    - (HLP39) Toggle colors");
        c_out(WHITE, "\n\rNAMES - (HLP17) Everyones names    BOUNTY   - (HLP40) Bounty Hunter");
        c_out(WHITE, "\n\rNAME  - (HLP18) Name a planet      MAP      - (HLP41) Universe Map");
        c_out(WHITE, "\n\rLONG  - (HLP19) Find a ship        LEASH    - (HLP42) Leash a ship");
        c_out(WHITE, "\n\rPEACE - (HLP20) Declare peace      SCOUT    - (HLP43) Scout ships");
        c_out(WHITE, "\n\rFIRE  - (HLP21) Fire phasers       CHAT     - (HLP44) Page the SysOp");
        c_out(WHITE, "\n\rFIND  - (HLP22) Find planets       OWN      - (HLP45) Where's your planets?");
        c_out(WHITE, "\n\rREMnPr- (HLP23) Do remotes         #        - (HLP46) Keyboard macros");
        c_out(WHITE, "\n\r");
    }
    else {
        while (! feof(mayhem_help)) {
            fgets(record, 200, mayhem_help);
            if (! feof(mayhem_help)) {
                c_out(WHITE, "%s\r", record);
            }
        }

        fclose(mayhem_help);
    }
}

/* **********************************************************************
   * See if a remote is in this universe.                               *
   *                                                                    *
   ********************************************************************** */

static void try_remote(char this_one)
{
   char loopit;
        
   for (loopit = 0; loopit < 10; loopit++) {
      if (ships->rem_xpos[loopit] > 0L &&
          ships->rem_xpos[loopit] != ONBOARD &&
              ships->rem_universe[loopit] == this_one) {
         c_out(YELLOW, " Remote %d", ships->rem_universe[loopit]);
         return;
      }
   }
}

/* **********************************************************************
   * See if a scout is in this universe.                                *
   *                                                                    *
   ********************************************************************** */

static void try_scout(char this_one)
{
   char loopit;
        
   for (loopit = 0; loopit < 10; loopit++) {
      if (scouts[loopit]->scout_direction != (char)NIL &&
         scouts[loopit]->scout_direction != SCOUT_DESTROYED &&
            scouts[loopit]->scout_universe == this_one) {
         c_out(LIGHTRED, " Scout %d", scouts[loopit]->scout_universe);
         return;
      }
   }
}

/* **********************************************************************
   * See if a sled is in this universe.                                 *
   *                                                                    *
   ********************************************************************** */

static void try_sled(char this_one)
{
   char loopit;

   for (loopit = 0; loopit < 15; loopit++) {
      if (ships->sled_xpos[loopit] != (short)NIL &&
          ships->sled_universe[loopit] == this_one) {
         c_out(LIGHTRED, " Sled %d", ships->sled_universe[loopit]);
         return;
      }
   }
}

/* **********************************************************************
   * Check to see if the active ship has any hardware in one of the     *
   * offered universes (or more than one). Display a comment to the     *
   * effect that hardware exists for the various universes.             *
   *                                                                    *
   ********************************************************************** */

void check_map(char gal1, char gal2, char gal3)
{
   char loopit, bail_out;

   if (ships->ship_universe == gal1 ||
        ships->ship_universe == gal2 ||
            ships->ship_universe == gal3)
      c_out(LIGHTGREEN, " Ship %d", ships->ship_universe);

   if (ships->base_xpos != 0 &&
        (ships->base_universe == gal1 ||
            ships->base_universe == gal2 ||
                ships->base_universe == gal3))
      c_out(LIGHTRED, " Base %d", ships->base_universe);

   try_remote(gal1); try_remote(gal2); try_remote(gal3);
   try_scout(gal1); try_scout(gal2); try_scout(gal3);
   try_sled(gal1); try_sled(gal2); try_sled(gal3);
   c_out(WHITE, "\n\r");
}

/* **********************************************************************
   * The Multi-universe map is displayed.                               *
   *                                                                    *
   ********************************************************************** */

void perform_universe_map(void)
{
   if (! ham_version) {
      c_out(WHITE, "   The Multi-Universe looks like this:\n\r");
      c_out(WHITE, "\n\r");
      c_out(WHITE, "                     ----\n\r");
      c_out(WHITE, "                    | 12 |"); check_map(12, NIL, NIL);
      c_out(WHITE, "                ---- ----\n\r");
      c_out(WHITE, "               | 10 |"); check_map(10, NIL, NIL);
      c_out(WHITE, "           ---- ----\n\r");
      c_out(WHITE, "          | 06 |"); check_map(6, NIL, NIL);
      c_out(WHITE, "      ---- ---- ----\n\r");
      c_out(WHITE, "     | 01 |    | 02 |"); check_map(1, 2, NIL);
      c_out(WHITE, " ---- ---- ---- ---- ----\n\r");
      c_out(WHITE, "| 05 |    | 00 |    | 07 |"); check_map(5, 0, 7);
      c_out(WHITE, " ---- ---- ---- ---- ----\n\r");
      c_out(WHITE, "     | 03 |    | 04 |"); check_map(3, 4, NIL);
      c_out(WHITE, "      ---- ---- ----\n\r");
      c_out(WHITE, "          | 08 |"); check_map(8, NIL, NIL);
      c_out(WHITE, "      ---- ----\n\r");
      c_out(WHITE, "     | 09 |"); check_map(9, NIL, NIL);
      c_out(WHITE, " ---- ----\n\r");
      c_out(WHITE, "| 11 |"); check_map(11, NIL, NIL);
      c_out(WHITE, " ----\n\r");
   }
   else {
      c_out(WHITE, "   The Multi-Universe looks like this:\n\r");
      c_out(WHITE, "\n\r");
      c_out(WHITE, "                     ----\n\r");
      c_out(WHITE, "                    : 12 :"); check_map(12, NIL, NIL);
      c_out(WHITE, "                ---- ----\n\r");
      c_out(WHITE, "               : 10 :"); check_map(10, NIL, NIL);
      c_out(WHITE, "           ---- ----\n\r");
      c_out(WHITE, "          : 06 :"); check_map(6, NIL, NIL);
      c_out(WHITE, "      ---- ---- ----\n\r");
      c_out(WHITE, "     : 01 :    : 02 :"); check_map(1, 2, NIL);
      c_out(WHITE, " ---- ---- ---- ---- ----\n\r");
      c_out(WHITE, ": 05 :    : 00 :    : 07 :"); check_map(5, 0, 7);
      c_out(WHITE, " ---- ---- ---- ---- ----\n\r");
      c_out(WHITE, "     : 03 :    : 04 :"); check_map(3, 4, NIL);
      c_out(WHITE, "      ---- ---- ----\n\r");
      c_out(WHITE, "          : 08 :"); check_map(8, NIL, NIL);
      c_out(WHITE, "      ---- ----\n\r");
      c_out(WHITE, "     : 09 :"); check_map(9, NIL, NIL);
      c_out(WHITE, " ---- ----\n\r");
      c_out(WHITE, ": 11 :"); check_map(11, NIL, NIL);
      c_out(WHITE, " ----\n\r");
   }
}

/* **********************************************************************
   * When the name of the universes is needed, we call this function to *
   * have it return a pointer to the universes name.                    *
   *                                                                    *
   ********************************************************************** */

char *what_universe_name(char this_universe)
{
   if (this_universe < 0 || this_universe > 12)
      return("No known universe");

   return(universe_name[this_universe]);
}

