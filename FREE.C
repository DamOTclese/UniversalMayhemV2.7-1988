
/* **********************************************************************
   * free.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   ********************************************************************** */

#include "defines.h"
#include <alloc.h>
#include <stdio.h>
#include "function.h"
#include "ship.h"
#include <io.h>
#include <stdarg.h>
#include <conio.h>
#include "goal.h"
#include "command.h"
#include "holder.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char to_save;
   extern char allow_mail[4];
   extern char *subspace_mail;
   extern UL total_allocated, smallest_allocated, largest_allocated;
   extern char is_redirected;
   extern char *record;
   extern char crash_reset;
   extern char interrupted_serial;
   extern char ham_version;
   extern unsigned long buffered;
   extern char interrupt_enable;
   extern unsigned long recursed, over_run;
   extern unsigned long transmit_overflow;
   extern unsigned long missed_tx;
   extern char echo_board_number;
   extern unsigned int ra_lowest, ra_highest, ra_total;
   extern unsigned int block_count;

   extern void offer_data(void);

/* **********************************************************************
   * An undocumended command is the FREE command. It tells the operator *
   * and the programmer how much free memory there is.			*
   *									*
   ********************************************************************** */

void perform_free(void)
{
   unsigned long amount_left;
   char *atpoint;
   char g_count, count;

   amount_left = (unsigned long) farcoreleft();

   c_out(WHITE, "\n\rFree number of bytes left in heap: %ld\n\r",
      (unsigned long) amount_left);

/*
    What kind of mail do we do?
*/

   c_out(WHITE, "Subspace mail is %s\n\r", allow_mail);

   c_out(WHITE, "Subspace mail goes to %s\n\r", subspace_mail);

   if (echo_board_number == 0) {
      c_out(WHITE, "Running *.MSG format echo mail\n\r");
   }
   else {
      c_out(WHITE,
         "Running Remote Access/Quick BBS echo mail. Echo board number %d\n\r",
         echo_board_number);

      c_out(WHITE, "Remote Access has message %d to %d, total: %d, block %d\n\r",
         ra_lowest, ra_highest, ra_total, block_count);
   }

   if (to_save == 0) {
      c_out(WHITE, "Running with planets nameable\n\r");
   }
   else {
      c_out(WHITE, "Planets are not nameable\n\r");
   }

   c_out(WHITE, "There is %ld bytes of allocated memory in use.\n\r",
      memory_used());

   if (crash_reset)
       c_out(LIGHTRED,
           "Keyboard reset if Mayhem crashes\n\r");

   if (ham_version) {
      c_out(LIGHTBLUE,
          "Running HAM Radio Version!\n\r");
   }

   if (is_redirected == 1) {
       c_out(LIGHTGREEN, "Interrupted serial I/O status: ");
       if (interrupted_serial) {
          c_out(WHITE, "ENABLED\n\r");

          c_out(WHITE,
              "%ld buff %ld recurs %ld runs %ld suspend, miss %ld\n\r",
              buffered, recursed, over_run, transmit_overflow, missed_tx);
       }
       else {
          if (interrupt_enable) {
             c_out(WHITE, "DISABLED\n\r");
          }
          else {
             c_out(WHITE, "TURNED OFF\n\r");
          }
       }
   }

   atpoint = record;
   atpoint += 4;
   skipspace(atpoint);

   if (*atpoint == '^') {
      for (g_count = 0; g_count < 10; g_count++) {
         if (goal_item[g_count] != (struct goal_elements *)NULL) {
            c_out(LIGHTGREEN, "Item %02d) {%d-%d} Universe [%d]",
               g_count,
               goal_item[g_count]->goal_xpos,
               goal_item[g_count]->goal_ypos,
               goal_item[g_count]->goal_universe);
            if (goal_item[g_count]->goal_on_ship == (char)NIL) {
               c_out(LIGHTGREEN, "\n\r");
            }
            else {
               c_out(LIGHTRED, "On ship %d\n\r", goal_item[g_count]->goal_on_ship);
            }
         }
      }
   }
   else if (toupper(*atpoint) == 'C') {
      recursed = over_run = transmit_overflow = missed_tx = 0l;
   }

    if (farheapcheck() == _HEAPCORRUPT) {
        c_out(LIGHTRED, "HEAP IS CORRUPTED!\n\r");
    }
    else {
        c_out(LIGHTGREEN, "Heap is good.\n\r");
    }

    m_point = m_first;
    while (m_point) {
        c_out(LIGHTGREEN,
            "\n\rTask %p: Ship %s ass? %s def? %s att? %s, run? %s\n\r",
            m_point,
            hold[m_point->ship]->names,
            m_point->assist == 1 ? "Yes" : "No",
            m_point->defend == 1 ? "Yes" : "No",
            m_point->attack == 1 ? "Yes" : "No",
            m_point->run_from == 1 ? "Yes" : "No");

        c_out(LIGHTGREEN,
            "   dec %d bo1? %s bo2? %s bo3? %s deffight? %s nxt %p\n\r",
            m_point->decoy,
            m_point->bail_1_out == 1 ? "Yes" : "No",
            m_point->bail_2_out == 1 ? "Yes" : "No",
            m_point->bail_3_out == 1 ? "Yes" : "No",
            m_point->bail_4_out == 1 ? "Yes" : "No",
            m_point->default_fight == 1 ? "Yes" : "No",
            m_point->next);

        m_point = m_point->next;
    }
}

/* **********************************************************************
   * Add the amount of memory allocated to the total and check to see   *
   * if the block is the smallest one yet, or the largest one yet.      *
   *                                                                    *
   ********************************************************************** */

void memory_allocated(UL this_amount)
{
   total_allocated += this_amount;
   if (this_amount < smallest_allocated) smallest_allocated = this_amount;
   if (this_amount > largest_allocated) largest_allocated = this_amount;
}

/* **********************************************************************
   * Subtract the amount of memory from the total.                      *
   *                                                                    *
   ********************************************************************** */

void memory_freed(UL this_amount)
{
   total_allocated -= this_amount;
}

/* **********************************************************************
   * Return the amount of memory still allocated.                       *
   *                                                                    *
   ********************************************************************** */

UL memory_used(void)
{
   return(total_allocated);
}


