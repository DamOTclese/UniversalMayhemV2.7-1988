
/* **********************************************************************
   * distrib.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   ********************************************************************** */

#include "defines.h"
#include "goal.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   static user_slaver_count;
   extern long dloop;
   extern long xpos, ypos;
   extern char *record, *again_string;
   extern short bail_out;
   extern short user_number;
   extern short rpt_loop;

/* **********************************************************************
   * Variables we use only here.                                        *
   *                                                                    *
   ********************************************************************** */

   static char bad_count = 0;

/* **********************************************************************
   * Function prototypes we need to know about.                         *
   *                                                                    *
   ********************************************************************** */

   extern void start_test_mode(void);

/* **********************************************************************
   *									*
   * The 'record' string contains a command. Use this routine to call	*
   * the proper function.						*
   *									*
   * What we want to do is copy the command string into a holder so we	*
   * can test to see if there are leading spaces or tabs. We do this	*
   * by shoving the command into a holder, setingg a pointer to that	*
   * holder, then checking the leading characters and poining past any.	*
   *									*
   * The result gets placed back into 'record'.				*
   *									*
   ********************************************************************** */

char distribute_command(void)
{
   char hold_rec[201], *atpoint;
   char question;

   strcpy(hold_rec, record);
   atpoint = hold_rec;

   while (*atpoint == 0x20 || *atpoint == 0x09) {
      atpoint++;
   }

   strcpy(record, atpoint);
   question = FALSE;

   while (*atpoint) {
      if (*atpoint == '?') {
         question = TRUE;
      }
      atpoint++;
   }

   if (! strcmp(record, "PLEASE ACTIVATE TEST MODE")) {
      start_test_mode();
      return(FALSE);
   }

   if (! strncmp(record, "FREE", 4)) {
      if (question) {
         extend_question(0);
      }
      else {
         perform_free();
      }
      return(FALSE);
   }

   if (! strncmp(record, "SCAN", 4)) {
      if (question) {
         extend_question(1);
      }
      else {
         perform_scan();
      }
      return(FALSE);
   }

   if (! strncmp(record, "HELP", 4)) {
      if (question) {
         extend_question(0);
      }
      else {
         perform_help();
      }
      return(FALSE);
   }

   if (! strncmp(record, "QUIT", 4)) {
      if (question) {
         extend_question(4);
      }
      else {
         perform_quit(0);
      }
      return(FALSE);
   }

   if (! strncmp(record, "EXIT", 4)) {
      if (question) {
         extend_question(4);
      }
      else {
         perform_quit(0);
      }
      return(FALSE);
   }

   if (! strncmp(record, "STAT", 4)) {
      if (question) {
         extend_question(5);
      }
      else {
         perform_stat(FALSE);
      }
      return(FALSE);
   }

   if (! strncmp(record, "INFO", 4)) {
      if (question) {
         extend_question(6);
      }
      else {
         perform_info(xpos, ypos);
      }
      return(FALSE);
   }

   if (! strncmp(record, "BOARD", 5)) {
      if (question) {
         extend_question(8);
      }
      else {
         perform_board();
      }
      return(FALSE);
   }

   if (! strncmp(record, "POWER", 5)) {
      if (question) {
         extend_question(9);
      }
      else {
         perform_power();
      }
      return(FALSE);
   }

   if (! strncmp(record, "BASE", 4)) {
      if (question) {
         extend_question(12);
      }
      else {
         perform_base();
      }
      return(FALSE);
   }

   if (! strncmp(record, "PEACE", 5)) {
      if (question) {
         extend_question(20);
      }
      else {
         perform_peace();
      }
      return(FALSE);
   }

   if (! strncmp(record, "PROTECT", 7)) {
      if (question) {
         extend_question(24);
      }
      else {
         perform_protect();
      }
      return(FALSE);
   }

   if (! strncmp(record, "SYSOP", 5)) {
      if (question) {
         extend_question(37);
      }
      else {
         perform_sysop();
      }
      return(FALSE);
   }

   if (! strncmp(record, "TIME", 4)) {
      if (question) {
         extend_question(15);
      }
      else {
         perform_time();
      }
      return(FALSE);
   }

   if (! strncmp(record, "STAND", 5)) {
      if (question) {
         extend_question(16);
      }
      else {
         perform_stand(FALSE);
      }
      return(FALSE);
   }

   if (! strncmp(record, "NAMES", 5)) {
      if (question) {
         extend_question(17);
      }
      else {
         perform_names();
      }
      return(FALSE);
   }

   if (! strncmp(record, "NAME", 4)) {
      if (question) {
         extend_question(18);
      }
      else {
         perform_name();
      }
      return(FALSE);
   }

   if (! strncmp(record, "FIND", 4)) {
      if (question) {
         extend_question(22);
      }
      else {
         perform_find();
      }
      return(FALSE);
   }

   if (! strncmp(record, "GO", 2)) {
      if (question) {
         extend_question(25);
      }
      else {
         perform_go();
      }
      return(FALSE);
   }

   if (! strncmp(record, "LEAVE", 5)) {
      if (question) {
         extend_question(26);
      }
      else {
         perform_leave();
      }
      return(FALSE);
   }

   if (! strncmp(record, "COMMAND", 7)) {
      if (question) {
         extend_question(28);
      }
      else {
         perform_command();
      }
      return(FALSE);
   }

   if (! strncmp(record, "TAXES", 5)) {
      if (question) {
         extend_question(27);
      }
      else {
         perform_taxes();
      }
      return(FALSE);
   }

   if (! strncmp(record, "PLAGUE", 6)) {
      if (question) {
         extend_question(30);
      }
      else {
         perform_plague();
      }
      return(FALSE);
   }

   if (! strncmp(record, "DESTRUCT", 8)) {
      if (question) {
         extend_question(31);
      }
      else {
         perform_destruct();
      }
      return(FALSE);
   }

   if (! strncmp(record, "COMETS", 6)) {
      if (question) {
         extend_question(32);
      }
      else {
         perform_comets();
      }
      return(FALSE);
   }

   if (! strncmp(record, "PING", 4)) {
      if (question) {
         extend_question(33);
      }
      else {
         perform_ping();
      }
      return(FALSE);
   }

   if (! strncmp(record, "SLED", 4)) {
      if (question) {
         extend_question(34);
      }
      else {
         perform_sled();
      }
      return(FALSE);
   }

   if (! strncmp(record, "AUCTION", 7)) {
      if (question) {
         extend_question(35);
      }
      else {
         perform_auction();
      }
      return(FALSE);
   }

   if (! strncmp(record, "NAV", 3)) {
      if (question) {
         extend_question(36);
      }
      else {
         perform_nav();
      }
      return(FALSE);
   }

   if (! strncmp(record, "PASS", 4)) {
      if (question) {
         extend_question(38);
      }
      else {
         perform_password();
      }
      return(FALSE);
   }

   if (! strncmp(record, "COLOR", 5)) {
      if (question) {
         extend_question(39);
      }
      else {
         perform_color();
      }
      return(FALSE);
   }

   if (! strncmp(record, "BOUNTY", 6)) {
      if (question) {
         extend_question(40);
      }
      else {
         perform_bounty();
      }
      return(FALSE);
   }

   if (! strncmp(record, "TRIGGER", 7)) {
      if (question) {
         extend_question(0);
      }
      else {
         perform_trigger();
      }
      return(FALSE);
   }

   if (! strncmp(record, "MAP", 3)) {
      if (question) {
         extend_question(41);
      }
      else {
         perform_universe_map();
      }
      return(FALSE);
   }

   if (! strncmp(record, "CHAT", 4)) {
      if (question) {
         extend_question(44);
      }
      else {
         perform_chat();
      }
      return(FALSE);
   }

   if (! strncmp(record, "LONG", 4)) {
      if (question) {
         extend_question(19);
      }
      else {
         perform_long();
      }
      return(FALSE);
   }

   if (! strncmp(record, "FIRE", 4)) {
      if (question) {
         extend_question(21);
      }
      else {
         perform_fire();
      }
      return(FALSE);
   }

   if (! strncmp(record, "HLP", 3)) {
      if (question) {
         extend_question(0);
      }
      else {
         perform_extended(FALSE, 0);
      }
      return(FALSE);
   }

   if (! strncmp(record, "REM", 3)) {
      if (question) {
         extend_question(23);
      }
      else {
         perform_remote();
      }
      return(FALSE);
   }

   if (! strncmp(record, "RPT", 3)) {
      if (question) {
         extend_question(3);
      }
      else {
         perform_rpt();
      }
      return(FALSE);
   }

   if (! strncmp(record, "SPY", 3)) {
      if (question) {
         extend_question(29);
      }
      else {
         perform_spy();
      }
      return(FALSE);
   }

   if (! strncmp(record, "MSG", 3)) {
      if (question) {
         extend_question(14);
      }
      else {
         perform_msg();
      }
      return(FALSE);
   }

   if (! strncmp(record, "LEASH", 5)) {
      if (question) {
         extend_question(42);
      }
      else {
         perform_leash();
      }
      return(FALSE);
   }

   if (! strncmp(record, "SCOUT", 5)) {
      if (question) {
         extend_question(43);
      }
      else {
         perform_scout();
      }
      return(FALSE);
   }

   if (! strncmp(record, "FREE", 4)) {
      if (question) {
         extend_question(0);
      }
      else {
         perform_free();
      }
      return(FALSE);
   }

   if (! strncmp(record, "OWN", 3)) {
      if (question) {
         extend_question(45);
      }
      else {
         perform_own();
      }
      return(FALSE);
   }

   if (record[0] == '#') {
      if (question) {
         extend_question(46);
      }
      else {
         return(perform_macro());
      }
      return(FALSE);
   }

   if (record[0] == 'W') {
      if (question) {
         extend_question(2);
      }
      else {
         (void)perform_warp();
      }
      return(FALSE);
   }

   if (record[0] == 'T') {
      if (question) {
         extend_question(7);
      }
      else {
         perform_torp();
      }
      return(FALSE);
   }

   if (record[0] == 'S') {
      if (question) {
         extend_question(10);
      }
      else {
         perform_sell();
      }
      return(FALSE);
   }

   if (record[0] == 'B') {
      if (question) {
         extend_question(11);
      }
      else {
         perform_buy();
      }
      return(FALSE);
   }

   if (strlen(record) < 1 || record[0] == '{') {
        return(FALSE);
   }

   if (record[0] == '?') {
        perform_help();
        return(FALSE);
   }

   c_out(LIGHTRED, "\n\rCommand %s unknown\n\r", record);
   again_string[0] = (char )NULL;
   bail_out = 1;
   rpt_loop = 0;

   if (++bad_count == 3) {
      c_out(RED, "Enter HELP for help or HLPn for extended help.\n\r");
      bad_count = 0;
   }

   return(FALSE);
}

/* **********************************************************************
   * So that we may display the "TRIGGER" command we need to know how	*
   * many items this person has.					*
   *									*
   ********************************************************************** */

void total_up_slaver_parts(void)
{
   user_slaver_count = 0;

   for (dloop = 0; dloop < 10; dloop++) {
      if (goal_item[dloop] != (struct goal_elements *)NULL) {
         if (goal_item[dloop]->goal_on_ship == user_number) {
            user_slaver_count++;
         }
      }
   }
}

/* **********************************************************************
   * See if the player has all of the parts and if so, allow the use	*
   * of the trigger command.						*
   *									*
   ********************************************************************** */

void perform_trigger(void)
{
   if (user_slaver_count == 10) {
      trigger_slaver_death_weapon();
   }
   else {
      c_out(LIGHTRED,
	 "You don't have all 10 pieces of the Slaver Death Device!\n\r");
   }
}
