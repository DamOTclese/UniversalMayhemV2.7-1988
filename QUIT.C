
/* **********************************************************************
   * quit.c								*
   *									*
   * Copyright 1988, 1989, 1990, 1991.                                  *
   * Fredric L. Rice. All rights reserved.                              *
   *                                                                    *
   * Update the users time remaining and then exit the game back to     *
   * its caller, Mayhem.Exe, or from the DOS prompt.			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdio.h"
#include "command.h"
#include "comets.h"
#include "scout.h"
#include "function.h"
#include "alloc.h"
#include "process.h"
#include "dos.h"
#include "conio.h"
#include "string.h"

/* **********************************************************************
   * If the did_mail value is set to TRUE, then we exit with an		*
   * errorlevel that may be used to process network mail.		*
   *									*
   ********************************************************************** */

   extern char did_mail;
   extern long count;
   extern char the_name[31];
   extern short players;
   extern short time_remaining;
   extern short control_break;
   extern short game_time_remaining;
   extern short total_comets;
   extern char crash_reset;
   extern char is_redirected;

/* **********************************************************************
   * If we have revectored the 0x1c interrupt, set it back.             *
   *                                                                    *
   ********************************************************************** */

   extern void interrupt (*old_interrupt)(void);

void perform_quit(short exit_value)
{
   char sloop;
          
   if (control_break == 1) {
      setcbrk(1);
   }

   if (exit_value == 0) {
      if (strcmp(ships->ship_name, "NONE")) {
         ships->time_remaining = game_time_remaining;
         for (sloop = 0; sloop < 10; sloop++) {
            ships->scout_xpos[sloop] = scouts[sloop]->scout_xpos;
            ships->scout_ypos[sloop] = scouts[sloop]->scout_ypos;
            ships->scout_direction[sloop] = scouts[sloop]->scout_direction;
         }
      }
      write_user();
   }

/*
   Deallocate any memory that may have been allocated for the storage
   of the ships information we needed to access quickly.
*/

   for (count = 0; count < players; count++) {
      if (Good_Hold(count)) {
         memory_freed((UL)sizeof(struct holder));
         farfree(hold[count]);
      }
   }

/*
   Deallocate any structure information allocated for the automation
   of ships through the command files.
*/

   while (m_first) {
      m_run = m_first->next;
      memory_freed((UL)sizeof(struct command_options));
      farfree(m_first);
      m_first = m_run;
   }

/*
   Return the goals memory back to the system
*/

   remove_goal_data();

/*
   Return the comets memory to the system
*/

   for (count = 0; count < total_comets; count++) {
      if (comets[count] != (struct comets_file *)NULL) {
         memory_freed((UL)sizeof(struct comets_file));
         farfree(comets[count]);
      }
   }

/*
   Return the scout ships memory to the system
*/

   for (count = 0; count < 10; count++) {
      if (scouts[count] != (struct scout_info *)NULL) {
         memory_freed((UL)sizeof(struct scout_info));
         farfree(scouts[count]);
      }
   }

   if (exit_value != 999) {
      if (strcmp(ships->ship_name, "NONE")) {
         c_out(WHITE, "\n\r%s has signed out. Call back soon!\n\r", ships->ship_name);
      }

      c_out(WHITE, "Returning to BBS software\n\r");
   }

   if (crash_reset)
      setvect(0x1c, old_interrupt);

   if (is_redirected == 1)
       sleep(4);    /* Delay time to empty output buffer */

   io_close();
   mayhem_fcloseall();

/*
   If mail was done and needs to be processed, we exit with an error level
   of 42, otherwise with an error level of what was passed to us.
*/

   if (did_mail && exit_value == 0) {
      exit(42);
   }

   if (exit_value != 999)
      exit(exit_value);
}


