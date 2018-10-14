
/* **********************************************************************
   * fileio.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Todo: The routines would run faster if the dpoint values were	*
   * removed and the seeks were made by offering the formula rather	*
   * than a variable that contains the formulas result.			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "universe.h"
#include "comets.h"
#include "goal.h"
#include "stdio.h"
#include "planets.h"
#include "function.h"
#include "process.h"
#include "conio.h"
#include "alloc.h"
#include "string.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char to_save;
   extern FILE *config, *aplanets, *auniverse, *aship, *help_file;
   extern FILE *acomet, *command_file, *agoals, *ship_std;
   extern FILE *error_log, *ship_log, *subspace_messages;
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern long dloop, count;
   extern long dpoint;
   extern char *record;
   extern char the_date[27];
   extern short user_number;
   extern short time_remaining;
   extern short total_comets;

/* **********************************************************************
   * Update the user_number record in the ships file. This routine is   *
   * used whenever the file is to be written to.                        *
   *                                                                    *
   ********************************************************************** */

void write_user(void)
{
   dpoint = ((long) user_number * sizeof(struct ship_file));

   if (aship == (FILE *)NULL) {
      log_error(41);
      return;
   }

   if (fseek(aship, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to your record in the ship file.");
      c_out(LIGHTRED, "\n\rTried to point to: %ld, record %d",
         dpoint, user_number);
      log_error(5);
      perform_quit(10);
   }

   if ((mayhem_fwrite(ships, sizeof(struct ship_file), 1, aship)) != 1) {
      c_out(LIGHTRED, "\n\rError occured when writing new SHIP.DAT record.\n\r");
      c_out(LIGHTRED, "\n\rTried to write to: %ld, record %d",
         dpoint, user_number);
      log_error(6);
      perform_quit(10);
   }
}

/* **********************************************************************
   * Read in the users ship record. This function is used by all of the	*
   * other functions.							*
   * 									*
   ********************************************************************** */

void read_user(void)
{
   dpoint = ((long) user_number * sizeof(struct ship_file));

   if (aship == (FILE *)NULL) {
      log_error(42);
      return;
   }

   if (fseek(aship, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to your record in the ship file.");
      c_out(LIGHTRED, "\n\rTried to point to: %ld, record %d",
         dpoint, user_number);
      mayhem_fcloseall();
      log_error(7);
      perform_quit(10);
   }

   if ((mayhem_fread(ships, sizeof(struct ship_file), 1, aship)) != 1) {
      c_out(LIGHTRED, "\n\rError occured when reading your ships record.\n\r");
      c_out(LIGHTRED, "\n\rTried to read from: %ld, record %d",
         dpoint, user_number);
      log_error(8);
      perform_quit(10);
   }
}

/* **********************************************************************
   * Read in a record from the universe file.				*
   *									*
   ********************************************************************** */

void read_universe(unsigned int uni_record)
{
   dpoint = ((long) uni_record * sizeof(struct universe_file));

   if (auniverse == (FILE *)NULL) {
      log_error(43);
      return;
   }

   if (fseek(auniverse, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to universe record.");
      c_out(LIGHTRED, "\n\rTried to point to: %ld, record %d\n\r",
         dpoint, uni_record);
      log_error(9);
      perform_quit(10);
   }

   if ((mayhem_fread(&universe, sizeof(struct universe_file), 1, auniverse)) != 1) {
      c_out(LIGHTRED, "\n\rError occured when reading universe record.\n\r");
      c_out(LIGHTRED, "Address %ld, record number %d\n\r", dpoint, uni_record);
      log_error(10);
      perform_quit(10);
   }
}

/* **********************************************************************
   * Read in the enemy ship record. This function is used by all of the	*
   * other functions.							*
   * 									*
   ********************************************************************** */

void read_enemy(unsigned int the_record)
{
   dpoint = ((long) the_record * sizeof(struct ship_file));

   if (aship == (FILE *)NULL) {
      log_error(44);
      return;
   }

   if (fseek(aship, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to enemy record in the ship file.");
      c_out(LIGHTRED, "\n\rTried to point to: %ld, record %d\n\r",
         dpoint, the_record);
      log_error(11);
      perform_quit(10);
    }

   if ((mayhem_fread(enemy, sizeof(struct ship_file), 1, aship)) != 1) {
      c_out(LIGHTRED, "\n\rError occured when reading enemy ships record.\n\r");
      c_out(LIGHTRED, "Tried to point to: %ld, record %d",
         dpoint, the_record);
      log_error(12);
      perform_quit(10);
    }
}

/* **********************************************************************
   * Write out the enemy ships record.					*
   *									*
   ********************************************************************** */

void write_enemy(unsigned int the_record)
{
   dpoint = ((long) the_record * sizeof(struct ship_file));

   if (aship == (FILE *)NULL) {
      log_error(45);
      return;
   }

   if (fseek(aship, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to enemy record in the ship file.");
      c_out(LIGHTRED, "\n\rTried to point to: %ld, record %d\n\r",
         dpoint, the_record);
      log_error(13);
      perform_quit(10);
   }

   if ((mayhem_fwrite(enemy, sizeof(struct ship_file), 1, aship)) != 1) {
      c_out(LIGHTRED, "\n\rError occured when writeing enemy ships record.\n\r");
      c_out(LIGHTRED, "Point to %ld, Record: %d", dpoint, the_record);
      log_error(14);
      perform_quit(10);
   }
}  

/* **********************************************************************
   * Read the universe record at 'count' from the universe file. Then	*
   * see what number planet we are at, 1 through 4. Then compute the	*
   * planets record number to read.					*
   *									*
   * The number is returned in 'dloop'.					*
   *									*
   ********************************************************************** */

short find_specific_planet(long from_record, long specific_record)
{
   if (from_record < 0 || from_record > xsize - 1) {
      return(0);
   }

   for (dloop = 0; dloop < 4; dloop++) {
      if (universe.planets[dloop] == specific_record) {
         return(1);
      }
   }
   return(0);
}

/* **********************************************************************
   * The information for the planet we wish to read is in dloop. Read	*
   * the required record from the planets file.				*
   *									*
   ********************************************************************** */

void read_planets(long in_it)
{
   dpoint = (long) ((in_it * 4) + dloop) *
      (sizeof(struct planets_file) - to_save);

   if (aplanets == (FILE *)NULL) {
      log_error(46);
      return;
   }

   if (fseek(aplanets, dpoint, 0) != 0) {
      c_out(LIGHTRED,
         "\n\rUnable to point to planets record at %ld, record %ld.\n\r",
         dpoint, in_it);
      log_error(15);
      perform_quit(10);
   }

   if (mayhem_fread(&planets, sizeof(struct planets_file) - to_save,
	 1, aplanets) != 1) {
      c_out(LIGHTRED,
         "\n\rUnable to read planet rec %ld @ %ld. dloop %d, save %d\n\r",
         in_it, dpoint, dloop, to_save);
      log_error(16);
      perform_quit(10);
   }
}

/* **********************************************************************
   * Write out the record being pointed to with dloop and in_it to the	*
   * planets file.							*
   *									*
   ********************************************************************** */

void write_planets(long in_it)
{
   dpoint = (long) ((in_it * 4) + dloop) *
      (sizeof(struct planets_file) - to_save);

   if (aplanets == (FILE *)NULL) {
      log_error(47);
      return;
   }

   if (fseek(aplanets, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to planets record %ld at %ld.\n\r",
         in_it, dpoint);
      log_error(17);
      perform_quit(10);
   }

   if (mayhem_fwrite(&planets, sizeof(struct planets_file) - to_save,
	 1, aplanets) != 1) {
      c_out(LIGHTRED,
         "\n\rUnable to write planets record at %ld, record %ld.\n\r",
         dpoint, in_it);
      log_error(18);
      perform_quit(10);
   }
}

/* **********************************************************************
   * Write out the entire comets structure.				*
   *									*
   ********************************************************************** */

void write_comets(void)
{
   if ((acomet = mayhem_fopen("COMETS.DAT", "wb", acomet)) == (FILE *)NULL) {
      c_out(LIGHTRED, "\n\rError occured reopening COMETS.DAT file\n\r");
      total_comets = 0;
      return;
   }

   for (count = 0; count < total_comets; count++) {
      dpoint = ((long) count * sizeof(struct comets_file));

      if (fseek(acomet, dpoint, 0) != 0) {
         c_out(LIGHTRED, "\n\rUnable to point to comets record in the comets file.");
         c_out(LIGHTRED, "\n\rTried to point to: %ld", dpoint);
         total_comets = 0;
         mayhem_fclose(&acomet);
         return;
      }

      if (comets[count] != (struct comets_file *)NULL) {
         if ((mayhem_fwrite(comets[count], sizeof(struct comets_file), 1,
            acomet)) != 1) {
            c_out(LIGHTRED, "\n\rError occured when writing new COMETS.DAT record.\n\r");
            total_comets = 0;
            mayhem_fclose(&acomet);
            return;
         }
      }
   }

   mayhem_fclose(&acomet);
}

/* **********************************************************************
   * Make a routine which will simply zero out the structures elements	*
   * or set them to various start-up values. This is done as a funcion	*
   * because there are more than 1 function which requires it.		*
   *									*
   ********************************************************************** */

void make_zero_record(void)
{
   short counta;

   ships->ship_xpos = xsize / 2;
   ships->ship_ypos = ysize / 2;
   ships->ship_universe = 0;
   ships->ship_power = 50000L;
   ships->ship_shield = 10000;
   ships->ship_credits = 10000;
   ships->ship_cargo = 300;
   ships->ship_crew = 500;
   ships->ship_shuttle = 1;
   ships->ship_hull = 100;
   ships->ship_cloak = 10;
   ships->ship_sensor = 10;
   STRNCPY(ships->ship_name, "NONE", 4);
   STRNCPY(ships->ship_pass, "NONE", 4);
   STRNCPY(ships->ship_date, the_date, 26);
   ships->ship_warp = 10;
   ships->ship_torp = 200;
   ships->base_xpos = 0;
   ships->base_ypos = 0;
   ships->base_universe = 0;
   ships->base_credits = 0;
   ships->base_cargo = 0;
   ships->base_crew = 0;
   ships->base_cloak = 0;
   ships->base_shield = 0;
   strcpy(ships->ship_person, "No name");

   for (counta = 0; counta < 10; counta++) {
      ships->rem_xpos[counta] = 0L;
      ships->rem_ypos[counta] = 0L;
      ships->rem_universe[counta] = 0;
   }

   ships->total_kills = 0;
   ships->time_remaining = time_remaining;
   ships->planets_owned = 0;
   ships->plague_flag = 0;
   ships->ship_infected = 0;
   ships->ship_morale = 100;
   ships->taxes = 0.0;
   ships->attack_sleds = 0;

   for (counta = 0; counta < 5; counta++) {
      strcpy(ships->allies[counta], "NONE");
   }

   for (counta = 0; counta < 15; counta++) {
      ships->sled_xpos[counta] = (short)NIL;
      ships->sled_ypos[counta] = (short)NIL;
      ships->sled_swarm[counta] = (short)NIL;
      ships->sled_power[counta] = 0;
      ships->sled_universe[counta] = 0;
   }

   strcpy(ships->last_at_by, "NONE");
   strcpy(ships->last_at_who, "NONE");
   ships->last_at_status = 0;
   ships->log_count = 0;
   strcpy(ships->base_death, "NONE");
   ships->base_hit_count = 0;

   for (counta = 0; counta < 10; counta++) {
      ships->x_planet_info[counta] = 0;
      ships->y_planet_info[counta] = 0;
      ships->point_tag[counta][0] = '-';
   }

   ships->outstanding_bid = (char)NIL;
   ships->bid_amount = 0l;
   strcpy(ships->base_boarded, "NONE");

   for (counta = 0; counta < 10; counta++) {
      ships->planet_taxes[counta] = 0.0;
      ships->tax_xpos[counta] = (long)NIL;
      ships->tax_ypos[counta] = (long)NIL;
      strcpy(ships->slot_owned[counta], "NONE");
   }

   for (counta = 0; counta < OWNABLE; counta++) {
      ships->owned_planets[counta] = (short)NIL;
   }

   ships->sick_bay = 0;
   ships->bounty = 0L;
   strcpy(ships->who_destroyed, "NONE");

   for (counta = 0; counta < 10; counta++) {
      ships->scout_xpos[counta] = (short)NIL;
      ships->scout_ypos[counta] = (short)NIL;
      ships->scout_to_x[counta] = (short)NIL;
      ships->scout_to_y[counta] = (short)NIL;
      ships->scout_direction[counta] = (char)NIL;
      ships->scout_universe[counta] = 0;
      ships->stay_on_station[counta] = TRUE;
   }

   strcpy(ships->pirate_code, "NONE");
   ships->leashed_by = (char)NIL;
   ships->leashed_to = (char)NIL;
   ships->local = 0;

   for (counta = 0; counta < MACROS; counta++)
      ships->macro[counta][0] = (char)NULL;
}

/* **********************************************************************
   * by an enemy ship. Let's issue a message and make sure that the     *
   * captain is allowed to have his ship back again.                    *
   *                                                                    *
   ********************************************************************** */

void ship_destroyed(void)
{
   char who_did_it;

   who_did_it = !strncmp(ships->who_destroyed, "NONE", 4);

   c_out(LIGHTRED,
      "\n\rYour ship has been destroyed by %s\n\r",
      who_did_it ? "an unknown enemy!" : ships->who_destroyed);

   clever_remarks_three();

ask_about_restarting:
   c_out(WHITE, "\n\rDo you want to start all over again? ");
   timed_input(0);
   ucase(record);

   if (record[0] != 'Y' && record[0] != 'N') {
      goto ask_about_restarting;
   }

   if (record[0] == 'N') {
      c_out(LIGHTRED, "\n\rA GOOD captain would hunt down");
      c_out(LIGHTRED, " and destroy %s!\n\r", ships->who_destroyed);
      c_out(LIGHTRED, "Please come back and continue the carnage!\n\r\n\r");
      make_zero_record();
      write_user();
      perform_quit(0);
   }

/*
   Basically, we perform a make zero record, however we want to keep
   the ships name, password, captains name, and anything else we want to
   keep. After we write this information out, we also need to update the
   memory values. We do this by asking the 'plug_this_ship' routine
   to do it.

   Also notice that the base, if any, is not touched. This is so that the
   restarted ship may slowly get to its base where it might be able to
   supply itself.

   IF ANYTHING IS ADDED TO SHIP.H, the player structure, or FILEIO.C,
   then it needs to be changed here as well.
*/

   ships->ship_xpos = xsize / 2;
   ships->ship_ypos = ysize / 2;
   ships->ship_universe = 0;
   ships->ship_power = 50000L;
   ships->ship_shield = 10000;
   ships->ship_credits = 10000;
   ships->ship_cargo = 300;
   ships->ship_crew = 500;
   ships->ship_shuttle = 1;
   ships->ship_hull = 100;
   ships->ship_cloak = 10;
   ships->ship_sensor = 10;
   ships->ship_warp = 10;
   ships->ship_torp = 200;

/*
   Password is _not_ changed!
*/

   for (count = 0; count < 10; count++) {
      ships->rem_xpos[count] = 0L;
      ships->rem_ypos[count] = 0L;
      ships->rem_universe[count] = 0;
   }

   ships->total_kills = 0;
   ships->time_remaining = time_remaining;
   ships->planets_owned = 0;
   ships->plague_flag = 0;
   ships->ship_infected = 0;
   ships->ship_morale = 100;
   ships->taxes = 0.0;
   ships->attack_sleds = 0;

   for (count = 0; count < 5; count++) {
      (void)strcpy(ships->allies[count], "NONE");
   }

   for (count = 0; count < 15; count++) {
      ships->sled_xpos[count] = (short)NIL;
      ships->sled_ypos[count] = (short)NIL;
      ships->sled_swarm[count] = (short)NIL;
      ships->sled_power[count] = 0;
      ships->sled_universe[count] = 0;
   }

   (void)strcpy(ships->last_at_by, "NONE");
   (void)strcpy(ships->last_at_who, "NONE");
   ships->last_at_status = 0;
   ships->log_count = 0;
   (void)strcpy(ships->base_death, "NONE");
   ships->base_hit_count = 0;

/*
   Tag value:

        - Empty spot
        + Planet
        [anything else is a tag name]
*/

   for (count = 0; count < 10; count++) {
      ships->x_planet_info[count] = 0;
      ships->y_planet_info[count] = 0;
      ships->point_tag[count][0] = '-';
   }

   ships->outstanding_bid = (char)NIL;
   ships->bid_amount = 0L;
   (void)strcpy(ships->base_boarded, "NONE");

   for (count = 0; count < 10; count++) {
      ships->planet_taxes[count] = 0.0;
      ships->tax_xpos[count] = (long)NIL;
      ships->tax_ypos[count] = (long)NIL;
      (void)strcpy(ships->slot_owned[count], "NONE");
   }

   for (count = 0; count < OWNABLE; count++) {
      ships->owned_planets[count] = (short)NIL;
   }

   ships->sick_bay = 0;
   ships->bounty = 0L;
   (void)strcpy(ships->who_destroyed, "NONE");

   for (count = 0; count < 10; count++) {
      ships->scout_xpos[count] = (short)NIL;
      ships->scout_ypos[count] = (short)NIL;
      ships->scout_to_x[count] = (short)NIL;
      ships->scout_to_y[count] = (short)NIL;
      ships->scout_direction[count] = (char)NIL;
      ships->scout_universe[count] = 0;
      ships->stay_on_station[count] = TRUE;
   }

/*
   pirate_code is NOT CHANGED!
*/

   ships->leashed_by = (char)NIL;
   ships->leashed_to = (char)NIL;

/*
   Version 2.3 addition
*/

   for (count = 0; count < MACROS; count++)
      ships->macro[count][0] = (char)NULL;

   write_user();
   plug_this_ship(user_number, FALSE);
}

/* **********************************************************************
   * Write out the goals being pointed to with dloop and in_it to the	*
   * goals file.							*
   *									*
   ********************************************************************** */

void write_goals(long in_it)
{
   if (goal_item[in_it] == (struct goal_elements *)NULL) return;

   dpoint = (long)in_it * (long)sizeof(struct goal_elements);

   if (agoals == (FILE *)NULL) {
      log_error(48);
      return;
   }

   if (fseek(agoals, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to goals record.\n\r");
      log_error(36);
      perform_quit(10);
   }

   if (mayhem_fwrite(goal_item[in_it], sizeof(struct goal_elements), 1, agoals) != 1) {
      c_out(LIGHTRED, "\n\rUnable to write goals record.\n\r");
      log_error(37);
      perform_quit(10);
   }
}

/* **********************************************************************
   * The information for the goal we wish to read is in dloop. Read	*
   * the required record from the goals file.				*
   *									*
   ********************************************************************** */

void read_goals(long in_it)
{
   if (goal_item[in_it] == (struct goal_elements *)NULL) return;

   dpoint = (long)in_it * (long)sizeof(struct goal_elements);

   if (agoals == (FILE *)NULL) {
      log_error(49);
      return;
   }

   if (fseek(agoals, dpoint, 0) != 0) {
      c_out(LIGHTRED, "\n\rUnable to point to goals record.\n\r");
      log_error(38);
      perform_quit(10);
   }

   if (mayhem_fread(goal_item[in_it], sizeof(struct goal_elements), 1, agoals) != 1) {
      c_out(LIGHTRED, "\n\rUnable to read goals record.\n\r");
      log_error(39);
      perform_quit(10);
   }
}

/* **********************************************************************
   * Check to see if the file is already open. If so, report a log and	*
   * return the existing pointer.					*
   *									*
   ********************************************************************** */

FILE *mayhem_fopen(char *fname, char *types, FILE *fpoint)
{
   if (fpoint != (FILE *)NULL) {
      log_error(103);
      return(fpoint);
   }

   return(fopen(fname, types));
}

/* **********************************************************************
   * Check to see if the file is already closed. If so, return an       *
   * end of file condition.                                             *
   *									*
   * If the file is closed properly, set the pointer to NULL.           *
   *                                                                    *
   ********************************************************************** */

int mayhem_fclose(FILE **fname)
{
   char result;
        
   if (*fname == (FILE *)NULL) return(EOF);

   if ((result = fclose(*fname)) == 0) {
      *fname = (FILE *)NULL;
   }

   return(result);
}

/* **********************************************************************
   * Close all files. Return stdio's return value.			*
   *									*
   ********************************************************************** */

int mayhem_fcloseall(void)
{
   mayhem_fclose(&config);
   mayhem_fclose(&aplanets);
   mayhem_fclose(&auniverse);
   mayhem_fclose(&aship);
   mayhem_fclose(&ship_std);
   mayhem_fclose(&acomet);
   mayhem_fclose(&command_file);
   mayhem_fclose(&agoals);
   mayhem_fclose(&subspace_messages);
   mayhem_fclose(&error_log);
   mayhem_fclose(&ship_log);
   mayhem_fclose(&help_file);
   return(0);
}

/* **********************************************************************
   * Make a read. Before this is done, however, make sure that the file	*
   * has been opened. If not, report a failure and return 0 bytes read.	*
   *									*
   ********************************************************************** */

int mayhem_fread(void *inbuf, size_t tsize, size_t tcount, FILE *fname)
{
   if (fname == (FILE *)NULL) {
      log_error(101);
      return(0);
   }

   return(fread(inbuf, tsize, tcount, fname));
}

/* **********************************************************************
   * Make a write. Before this is done, however, make sure that the 	*
   * file has been opened. If not, report an error and then return 0 as	*
   * the number of bytes written.					*
   *									*
   ********************************************************************** */

int mayhem_fwrite(void *outbuf, size_t tsize, size_t tcount, FILE *fname)
{
   if (fname == (FILE *)NULL) {
      log_error(102);
      return(0);
   }

   return(fwrite(outbuf, tsize, tcount, fname));
}

