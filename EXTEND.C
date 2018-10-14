
/* **********************************************************************
   * extend.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Perform an extended help function.					*
   * Open up SHIP.HLP and read the file until the 'nth * is found as	*
   * the first character of a line. The value offered with the HLPn is	*
   * used to find the proper record.					*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "stdio.h"
#include "function.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern FILE *help_file;
   extern char *record;

void perform_extended(char use_number, char item_number)
{
   char buffer[201], *tpoint;
   short int help_item;

   if (! use_number) {
      tpoint = record;
      tpoint += 3;
      help_item = (*tpoint) - 0x30;
      tpoint++;

      if (*tpoint != (char)NULL) {
         help_item = (help_item * 10) + (*tpoint) - 0x30;
      }

      if (help_item < 1 || help_item > HIGHEST_HELP) {
         c_out(WHITE, "That help item is not valid!\n\r");
         return;
      }
   }
   else {
      help_item = item_number;
   }

   if ((help_file = mayhem_fopen("SHIP.HLP", "r", help_file)) == (FILE *)NULL) {
      c_out(WHITE, "Extended help is not available on this system.\n\r");
      c_out(WHITE, "Look for a file called: SHIP.HLP somewhere in");
      c_out(WHITE, " the files area.\n\r");
      return;
   }

   while (!feof(help_file)) {
      fgets(buffer, 200, help_file);
      if (buffer[0] == '*') {
         help_item--;
         if (help_item == 0) {
	    c_out(WHITE, "%s\r", buffer);
            dump_extended_help(&help_file);
            return;
         }
      }
   }
   c_out(WHITE, "Extended help is not fully implimented on this system.\n\r");
   c_out(WHITE, "Ask your SYSOP to load: SHIP.HLP.\n\r");
   mayhem_fclose(&help_file);
}

/* **********************************************************************
   * The start has been found, continue to dump the file out line by	*
   * line until another * is found or the end of file is encountered.	*
   *									*
   ********************************************************************** */

void dump_extended_help(FILE **help_file)
{
   char buffer[201];
   char lc = 0;

   do {
      fgets(buffer, 200, *help_file);
      if (buffer[0] == '*') {
         mayhem_fclose(help_file);
         return;
      }
      c_out(WHITE, "%s\r", buffer);
      lc++;
      if (lc == 20) {
         c_out(LIGHTRED, "Hit [ENTER] to continue: ");
         timed_input(FALSE);
         lc = 0;
      }
   } while (!feof(*help_file));

   mayhem_fclose(help_file);
}

/* **********************************************************************
   * Offer the extended help.                                           *
   *                                                                    *
   ********************************************************************** */

void extend_question(char this_element)
{
    if (this_element == 0) {
        c_out(LIGHTRED,
            "\n\rThere is no extended help for this command.\n\r");
        return;
    }

    perform_extended(TRUE, this_element);
}


