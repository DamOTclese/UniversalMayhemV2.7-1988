
/* **********************************************************************
   * rpt.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * The command is a RPTn. The old command string contains commands	*
   * which look something like this:					*
   *									*
   * w28;scan;rpt10							*
   *									*
   * We scan the 'old_record' string and find the RPT command. We take	*
   * the value after the RPT and deduct one from it. If it is zero, we	*
   * return to get a new command string, else we copy the resulting	*
   * string into 'record' so that it may be executed again.		*
   *									*
   * We will limit the number of iterations to 30.			*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"
#include "dos.h"
#include "async.h"

/* **********************************************************************
   * In order to determine if we need to check the com port for an	*
   * escape or the keyboard, we import the 'is_redirected' value.       *
   *									*
   ********************************************************************** */

   extern char is_redirected;
   extern char *record, *old_record;
   extern short bail_out;
   extern short rpt_loop;
   extern char interrupted_serial;
   extern char port_assignment;

   extern void empty_transmit_buffer(ComPort ComDev);

void perform_rpt(void)
{
   char *thepoint;
   int msb, lsb, total;

   if (check_for_key()) {
      if (return_character() == ESC) {
	 bail_out = 1;
	 rpt_loop = 0;
	 return;
      }
   }

   if (is_redirected && interrupted_serial) {
      empty_transmit_buffer(port_assignment);
   }

   strcpy(record, old_record);
   thepoint = record;

   for (; *thepoint++; ) {
      if (! strncmp(thepoint, "RPT", 3)) {
         break;
      }
   }

   thepoint++; thepoint++;
   msb = (*++thepoint) - 0x30;

   if (*++thepoint < '0' || *thepoint > '9') {
      lsb = msb;
      msb = 0;
   }
   else {
      lsb = (*thepoint) - 0x30;
   }

   total = (msb * 10) + lsb;
   *++thepoint = (char)NULL;

   if (total > 30 || total < 1) {
      bail_out = 1;
      rpt_loop = 0;
      return;
   }
   else {
      rpt_loop = 1;
   }

   msb = (int) --total / 10;
   lsb = total - (msb * 10);
   *--thepoint = lsb + 0x30;
   *--thepoint = msb + 0x30;

   strcpy(old_record, record);
   return;
}

/* **********************************************************************
   * Check to see if there is a RPT in the command. If so, then make	*
   * sure there is nothing following its parameter by setting a NULL in	*
   * place.								*
   *									*
   ********************************************************************** */

void check_after_rpt(void)
{
   char *thepoint;
   int nulcnt;

   thepoint = record;
   nulcnt = 0;

   for (; *thepoint++; nulcnt++) {
      if (! strncmp(thepoint, "RPT", 3)) {
         break;
      }
   }

   thepoint++; thepoint++; thepoint++; thepoint++; thepoint++;
   *thepoint++ = (char)NULL;
   nulcnt = nulcnt + 5;

   if (record[nulcnt] == ';') {
      record[nulcnt] = (char)NULL;
   }
}

