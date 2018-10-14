
/* **********************************************************************
   * mayhem.c	(Main module calling ship.c).				*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * This module is called by the FidoNet software by using the         *
   * "Outside command within a batch file. It is intended to let people *
   * play the game, find game information,or go back to the FidoNet     *
   * software.                                                          *
   *                                                                    *
   * Started: 18/Apr/88                                                 *
   * Ended:   21/Apr/88                                                 *
   * Author:  Fredric L. Rice (1:102/901)                               *
   *                                                                    *
   ********************************************************************** */

#include "stdio.h"
#include "string.h"
#include "process.h"
#include "conio.h"
#include "time.h"
#include "io.h"
#include "bios.h"
#include "stdlib.h"
#include "dos.h"
#include "signal.h"

/* **********************************************************************
   * Define some defines to use through this module.			*
   *									*
   ********************************************************************** */

#define THE_VERSION  "2.6b"
#define FIRST_TIMER  60
#define SECOND_TIMER FIRST_TIMER + 30
#define BACKSPACE    0x08
#define SPACE        0x20
#define CONTROL_X    0x18
#define ESC          0x1b
#define TIMED_OUT    50
#define TRUE         1
#define FALSE        0

/* **********************************************************************
   * Define the prototypes for the functions so that we get better	*
   * error checking and get rid of annoying warnings.			*
   *									*
   ********************************************************************** */

   static void run_ship(char **argv);
   static void find_docs(void);
   static unsigned short newline(void);
   static void missing_file(void);
   static void ucase(void);
   static void timed_input(void);
   static char check_carrier_detect(void);
   static void lost_carrier(void);
   static void give_run(void);
   static void give_update(void);
   static void give_standings(void);
   static void give_statistics(void);
   static FILE *mayhem_fopen(char *fname, char *types, FILE *fpoint);
   static int mayhem_fclose(FILE **fname);
   static int mayhem_fcloseall(void);
   static void send_string(char *this_string);
   static unsigned char check_for_key(void);
   static char return_character(void);
   static void send_character(char this_byte);
   static char tandy;

/* **********************************************************************
   * Define some data storage.                                          *
   *                                                                    *
   ********************************************************************** */

   static char last_update[201], record[201], watch_cd;
   static char is_redirected;
   static char port_assignment;
   static char out_buffer[201];
   static FILE *updatefile, *screen, *config, *ship_inf, *ship_std, *stats;
   static char control_c_received;
   static char simple_shell;

/* **********************************************************************
   * Trap the control C'ing that might occur.				*
   *									*
   ********************************************************************** */

void Trap_Control_C(void)
{
   (void)signal(SIGINT, Trap_Control_C);
   control_c_received = TRUE;
}

/* **********************************************************************
   * Go through record and see where the first space or tab is until    *
   * the end of string is found. When either is, put an end of string   *
   * marker at that location in the string. Then return the integer of  *
   * the value to the function that called it. Because it is possible   *
   * for someone to put the delimiter ; right after the numeric value   *
   * we want, we check for ; as well. We try to make this as bullet     *
   * proof as possible because we don't know the capability of the      *
   * people who are going to run it.                                    *
   *                                                                    *
   * The comments that are allowed after the parameter stored are quite *
   * usefull for people to remember what the various items are for so   *
   * when a change to the file is performed, nothing gets corrupted.    *
   *                                                                    *
   ********************************************************************** */

int extract_config(void)
{
   char bail_out;
   char count;

   bail_out = FALSE;

   while (! bail_out) {
      if (! feof(config)) {
         (void)fgets(record, 200, config);
	 if (! feof(config)) {
	    if (! feof(config) && record[0] != ';' && strlen(record) > 2) {
	       bail_out = TRUE;
	    }
	 }
	 else {
	    bail_out = TRUE;
	 }
      }
      else {
	 bail_out = TRUE;
      }
   }

   if feof(config) {
      (void)fputs("SHIP.CFG is corrupted!\n\r", stderr);
      exit(0);
   }

   for (count = 0; record[count]; count++) {
      if (record[count] == 0x20 || record[count] == 0x9) {
         break;
      }

      if (record[count] == 0x3b) {
         break;
      }
   }

   record[count] = (char)NULL;
   return (atoi(record));
}

/* **********************************************************************
   * Read out a config string up to the end of string or to the ;.      *
   * Then strip out the trailing spaces and tabs. Return the string.    *
   *                                                                    *
   ********************************************************************** */

void cfg_string(void)
{
   char bail_out;
   char hold_record[201], *hold_point;
   char count;

   bail_out = FALSE;

   while (! bail_out) {
      if (! feof(config)) {
         (void)fgets(hold_record, 200, config);
	 if (! feof(config)) {
	    hold_point = hold_record;
	    while (*hold_point && *hold_point == ' ') hold_point++;
            (void)strcpy(record, hold_point);
	    if (! feof(config) && record[0] != ';' && strlen(record) > 2) {
	       bail_out = TRUE;
	    }
	 }
	 else {
	    bail_out = TRUE;
	 }
      }
      else {
	  bail_out = TRUE;
      }
   }

   if (feof(config)) {
      (void)fputs("SHIP.CFG is corrupted!\n\r", stderr);
      exit(0);
   }

   for (count = 0; record[count]; count++) {
      if (record[count] == 0x3b) {
         record[count] = (char)NULL;
         break;
      }
   }

   while (record[strlen(record) - 1] == ' ' ||
      record[strlen(record) - 1] == 0x09) {
      record[strlen(record) - 1] = (char)NULL;
   }
}

/* **********************************************************************
   * Function main() will display a menu and ask for the option number. *
   * When an option is valid, the routine is called. When the called    *
   * routine is done, and execution resumes in main, main will recurse  *
   * itself so that another option may ne acquired.                     *
   *                                                                    *
   ********************************************************************** */

void main(int argc, char *argv[])
{
   int option, count;
   char byte, bail_out;

   control_c_received = FALSE;
   (void)signal(SIGINT, Trap_Control_C);
   updatefile = screen = config = ship_inf = ship_std = stats = (FILE *)NULL;
   is_redirected = 0;
   tandy = FALSE;

   record[0] = (char)argc;
   simple_shell = FALSE;

   if (argc > 1 && argv[1][0] == '/') {
       if (argv[1][1] == 'S' || argv[1][1] == 's') {
	   simple_shell = TRUE;
       }
   }

   if ((config = mayhem_fopen("SHIP.CFG", "r", config)) == (FILE *)NULL) {
      (void)fputs("File: SHIP.CFG is missing!\n\r", stderr);
      exit(0);
   }

   (void)extract_config();
   (void)extract_config();
   (void)cfg_string();
   (void)extract_config();
   (void)cfg_string();
   (void)cfg_string();
   (void)extract_config();
   (void)cfg_string();
   (void)cfg_string();
   (void)cfg_string();
   (void)cfg_string();
   port_assignment = extract_config();

   if (port_assignment != 0 && port_assignment != 1) {
      (void)fputs("Port assignment in SHIP.CFG is not supported!\n\r", stderr);
      exit(0);
   }

   cfg_string();
   cfg_string();
   cfg_string();
   ucase();

   if (! strncmp(record, "YES", 3)) {
      watch_cd = TRUE;
   }
   else {
      watch_cd = FALSE;
   }

   if (! check_carrier_detect())
      is_redirected = 0;
   else
      is_redirected = 1;

   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();
   cfg_string();

   cfg_string();
   ucase();

   if (! strncmp(record, "YES", 3)) {
       (void)fputs("\n\r- Running on a Tandy Computer -\n\r", stderr);
       tandy = TRUE;
   }

   (void)mayhem_fclose(&config);

/* **********************************************************************
   * Read the last update date out of the last update data file. After	*
   * that, find the carriage return and eliminate it!			*
   *									*
   ********************************************************************** */

   if ((updatefile = mayhem_fopen("lastupdt.shp", "r", updatefile)) != (FILE *)NULL) {
      (void)fgets(last_update, 200, updatefile);
      (void)mayhem_fclose(&updatefile);
   }
   else {
      (void)strcpy(last_update, "None");
   }

   bail_out = FALSE;

   if ((screen = mayhem_fopen("ship.scr", "rb", screen)) != (FILE *)NULL) {
      while (! feof(screen) && !bail_out && !control_c_received) {
	 if (check_for_key()) {
            byte = return_character();
            if (byte == 's' || byte == 'S' || byte == ESC) {
               bail_out = TRUE;
            }
         }
         (void)fgets(record, 200, screen);
         (void)sprintf(out_buffer, "%s", record);
         send_string(out_buffer);
      }
      control_c_received = FALSE;
      (void)newline();
   }

   for (count = strlen(last_update); count > 0; count--) {
      if (last_update[count] == 0x0d || last_update[count] == 0x0a) {
	 last_update[count] = (char)NULL;
         break;
      }
   }

   (void)sprintf(out_buffer, "\n\r\n\rUniversal Mayhem\n\r");
   send_string(out_buffer);

   (void)sprintf(out_buffer,
       "   Copyright Fredric L. Rice. 1988/89/90/91. Version %s %s\n\r",
       THE_VERSION, __DATE__);

   send_string(out_buffer);

   (void)sprintf(out_buffer,
       "      --- Where honorable bloodletting is exceeded only by ");

   send_string(out_buffer);

   (void)sprintf(out_buffer, "broken alliances.\n\r");
   send_string(out_buffer);

give_menu:
   (void)sprintf(out_buffer, "\n\r\n\r1 ... Run Universal Mayhem\n\r");
   send_string(out_buffer);
   (void)sprintf(out_buffer, "2 ... Acquire Run-Down of the game\n\r");
   send_string(out_buffer);
   (void)sprintf(out_buffer, "3 ... Where can I get the documentation?\n\r");
   send_string(out_buffer);
   (void)sprintf(out_buffer, "4 ... View latest updates (As of %s)\n\r", last_update);
   send_string(out_buffer);
   (void)sprintf(out_buffer, "5 ... Display top ten players ships\n\r");
   send_string(out_buffer);
   (void)sprintf(out_buffer, "6 ... VIEW OTHER SYSTEMS PLAYER STATISTICS!\n\r");
   send_string(out_buffer);
   (void)sprintf(out_buffer, "7 ... Return to BBS software\n\r");
   send_string(out_buffer);

   do {
      (void)sprintf(out_buffer, "\n\rEnter your selection: (7 to quit): ");
      send_string(out_buffer);
      timed_input();
      option = atoi(record);
   } while (option < 1 || option > 7);

   (void)sprintf(out_buffer, "\n\r");
   send_string(out_buffer);

   switch(option) {
      case 1: run_ship(argv); break;
      case 2: give_run(); break;
      case 3: find_docs(); break;
      case 4: give_update(); break;
      case 5: give_standings(); break;
      case 6: give_statistics(); break;
      case 7: exit(0);
   }
   goto give_menu;
}

/* **********************************************************************
   * This module will execute the main program module. Note that when   *
   * the control returns to this program, it is exited and DOS takes    *
   * over. As this program is designed to operate from a batch file,    *
   * control will be passed back to FidoNet software through DOS.       *
   *                                                                    *
   ********************************************************************** */

static void run_ship(char **argv)
{
    if (! simple_shell) {
        (void)execv("ship.exe", argv);
	exit(0);
    }
    (void)system("ship.exe\r");
}

/* **********************************************************************
   * This routine goes into the file SHIP.INF and displays all of the   *
   * data contained within this file. This is usefull when the player   *
   * wishes to acquire a quick run-down of the game. The file SHIP.INF  *
   * may me modified by the Sysop. Note that if the list is terminated  *
   * by the operator, we don't ask them to hit enter again. When normal *
   * EOF, we will ask again.                                            *
   *                                                                    *
   ********************************************************************** */

static void give_run(void)
{
   char byte;
   int ocount;

   ocount = 0;
   byte = 0;

   if ((ship_inf = mayhem_fopen("SHIP.INF", "r", ship_inf)) == (FILE *)NULL) {
      missing_file();
      return;
   }

   (void)sprintf(out_buffer, "\n\r\n\r\n\r");
   send_string(out_buffer);

   while(byte != (char)EOF) {
      send_character(byte = (getc(ship_inf)));
      if (byte == 0x0a) {
	 send_character(0x0d);
	 if ((ocount++) == 18) {
            ocount = 0;
            if (newline() == 1) {
	       break;
	    }
         }
      }
   }

   (void)mayhem_fclose(&ship_inf);

   if (byte == (char)EOF ) {
      (void)newline();
   }
}

/* **********************************************************************
   * Read out SHIP.INF until a line feed is encountered. When it is,    *
   * return to menu. This routine should also display the line feed.    *
   *                                                                    *
   ********************************************************************** */

static void find_docs(void)
{
   char byte;
   byte = 0;

   if ((ship_inf = mayhem_fopen("SHIP.INF", "r", ship_inf)) == (FILE *)NULL) {
      missing_file();
      return;
   }

   (void)sprintf(out_buffer, "You can find SHIP.DOC in ---> ");
   send_string(out_buffer);

   while(byte != 10) {
      send_character(byte = (getc(ship_inf)));
   }

   (void)sprintf(out_buffer, "\n\r");
   send_string(out_buffer);
   (void)mayhem_fclose(&ship_inf);
}

/* **********************************************************************
   * Perform a wait for any key to be hit routine so that the message   *
   * will not scroll off of the screen when requesting a run-down.      *
   *                                                                    *
   ********************************************************************** */

static unsigned short newline(void)
{
   (void)sprintf(out_buffer, "\n\rHit ENTER to continue (type END to exit): ");
   send_string(out_buffer);
   timed_input();
   ucase();

   if (! strncmp(record, "END", 3)) {
      (void)sprintf(out_buffer, "\n\rAborted\n\r");
      send_string(out_buffer);
      return(1);
   }

   return(0);
}

/* **********************************************************************
   * What to do in case the information file is missing.		*
   *									*
   ********************************************************************** */

static void missing_file(void)
{
   (void)sprintf(out_buffer, "\n\rSHIP.INF file is missing so information is not available.\n\r");
   send_string(out_buffer);
}

/* **********************************************************************
   * This routine will take string array record and convert all of the  *
   * characters within it inbto uppercase that are valid in uppercase.  *
   * This is done so that END may be checked for. Some compilers supply *
   * a method for converting entire strings to upper case so this may   *
   * not be required by other systems. "toupper()" didn't work here so  *
   * we subtract.                                                       *
   *                                                                    *
   ********************************************************************** */

static void ucase(void)
{
   int i;

   for (i = 0; record[i]; i++) {
      if (record[i] > 0x60 && record[i] < 0x7b) {
         record[i] = record[i] - 32;
      }
   }
}

/* **********************************************************************
   * Just like the run-down, however, this time we will be displaying	*
   * another file. We will also check first off to see if the last_	*
   * update variable has a date in it.					*
   *									*
   ********************************************************************** */

static void give_update(void)
{
   char byte;
   int ocount;

   byte = (char)NULL;
   ocount = 0;

   if (! strcmp(last_update, "None")) {
      (void)sprintf(out_buffer, "\n\rThere have been no receint updates.\n\r");
      send_string(out_buffer);
      return;
   }

   if ((updatefile = mayhem_fopen("lastupdt.shp", "r", updatefile)) == (FILE *)NULL) {
      (void)sprintf(out_buffer, "\n\rUnable to find last update file.\n\r");
      send_string(out_buffer);
      return;
   }

   (void)sprintf(out_buffer, "\n\r\n\r\n\r\n\r");
   send_string(out_buffer);

   while(byte != (char)EOF) {
      send_character(byte = (getc(updatefile)));
      if (byte == 0x0a) {
         send_character(0x0d);
         if ((ocount++) ==18) {
            ocount = 0;
            if (newline() == 1) {
	       break;
	    }
         }
      }
   }

   (void)mayhem_fclose(&updatefile);

   if (byte == (char)EOF ) {
      (void)newline();
   }
}

/* **********************************************************************
   * Here is what this routine is designed to do:			*
   *									*
   * 1) Supply a universal keyboard input for use everywhere		*
   *									*
   * 2) Scan the keyboard and queue up characters until carriage return *
   *									*
   * 3) If no entry after 1 minute, issue a warning and beep		*
   *									*
   * 4) If no entry after 2 minutes, bounce the player to Opus control	*
   *									*
   * The specifics and technicals:					*
   *									*
   * String variable 'record' is used. Upon entry, we will null that	*
   * puppy out. We will then set the timer to zero. As a key is		*
   * entered, we zero that timer out. If the timer hits the first	*
   * value, (which will need to be tweaked, we issue a warning. If the	*
   * timer hits the second value, (needs to be tweaked also), we wll	*
   * bounce the player off.						*
   *									*
   * Eventually, this routine will be polling the com port for the	*
   * receied characters. We will want to retain the typeahead aspect of	*
   * the standard keyboard.						*
   *									*
   * Another aspect of this function is to perform psudo-real-time	*
   * operations. very so often, while waiting for input, we will call	*
   * the real-time operations function. This permits the execution of	*
   * command-files during player inactivity areas of time.		*
   *									*
   * We pass a 0 or a 1 to this routine. If the value is 0, then we 	*
   * will echo the character typed, else if its anything else, we will	*
   * not echo the character typed.					*
   *									*
   ********************************************************************** */

static void timed_input(void)
{
   short byte, i, warning;
   time_t time_one, time_two;

   record[0] = (char)NULL;
   i = warning = 0;

   (void)time(&time_one);
   (void)time(&time_two);

/*
   Main entry loop is here
*/

   while (difftime(time_two, time_one) < SECOND_TIMER) {
      if (check_for_key()) {
	 byte = return_character();
	 if (byte > 0) {
	    if (i == 200 && byte != BACKSPACE && byte != 0x0d) {
		send_character(7);
	    }
	    else {
	       send_character(byte);
	       if (byte == 0x0d) {
		  record[i++] = (char)NULL;
                  (void)sprintf(out_buffer, "\n\r");
		  send_string(out_buffer);
		  return;
	       }
	       else if (byte == BACKSPACE) {
		  if (i > 0) {
		     record[i--] = (char)NULL;
		     send_character(SPACE);
		     send_character(BACKSPACE);
		  }
		  else {
		     send_character(SPACE);
		  }
	       }
	       else if (byte == CONTROL_X) {
		  for (i++; i > 0; i--) {
		     send_character(BACKSPACE);
		     send_character(SPACE);
		     send_character(BACKSPACE);
		  }
		  record[0] = (char)NULL;
	       }
	       else {
                  (void)time(&time_one);
		  record[i++] = byte;
	       }
	    }
	 }
      }

/*
   Keys have been processed. See if timer is exceeded.
*/

      if (difftime(time_two, time_one) > FIRST_TIMER && warning == 0) {
         (void)sprintf(out_buffer,
            "\n\r%cWarning! %cSoon you %care going to %cbe bounced %coff!!!\n\r",
            7, 7, 7, 7, 7);
         send_string(out_buffer);
         warning = 1;
      }

      if (is_redirected != 0) {
         if (!check_carrier_detect()) {
            lost_carrier();
            exit(90);
         }
      }

      (void)time(&time_two);
   }

/*
   System timed out. Bounce everything back to the stone age
*/

   (void)mayhem_fcloseall();

   (void)sprintf(out_buffer,
      "\n\r%cKeyboard entry %ctimed out... %cReturning to %cBBS system.%c\n\r",
      7, 7, 7, 7, 7);

   send_string(out_buffer);
   exit(TIMED_OUT);
}

/* **********************************************************************
   * If we are not supposed to care about carrier detect, then simply	*
   * return saying that carrier detect was present, even if it wasn't.	*
   * 									*
   * If we do care about carrier detect, read the com ports status and	*
   * return the result as a true/false value.				*
   *									*
   ********************************************************************** */

static char check_carrier_detect(void)
{
   if (! watch_cd) return(TRUE);
   return((bioscom(3, 0, port_assignment) & 0x80) == 0x80);
}

/* **********************************************************************
   * When we do loose carrier detect and we care, we come here. First	*
   * close any open files and then issue a short message. After that,	*
   * return to the operating system with an error level of 90.		*
   *									*
   ********************************************************************** */

static void lost_carrier(void)
{
   mayhem_fcloseall();
   (void)sprintf(out_buffer, "\n\rCarrier lost!\n\r");
   (void)send_string(out_buffer);
   exit(90);
}

/* **********************************************************************
   * Give the standings file if there is one.				*
   *									*
   ********************************************************************** */

static void give_standings(void)
{
   char byte, ocount;

   if ((ship_std = mayhem_fopen("SHIP.STD", "r", ship_std)) == (FILE *)NULL) {
      (void)sprintf(out_buffer, "Standings file: SHIP.STD is not available.\n\r\n\r");
      send_string(out_buffer);
      return;
   }

   (void)sprintf(out_buffer, "\n\r\n\r\n\r\n\r");
   send_string(out_buffer);
   byte = (char)NULL;
   ocount = 0;

   while(byte != (char)EOF) {
      send_character(byte = (getc(ship_std)));
      if (byte == 0x0a) {
         send_character(0x0d);
         if ((ocount++) ==18) {
            ocount = 0;
            if (newline() == 1) {
	       break;
	    }
         }
      }
   }

   (void)mayhem_fclose(&ship_std);

   if (byte == (char)EOF ) {
      (void)newline();
   }
}

/* **********************************************************************
   * Check to see if the file is already open. If so, report a log and	*
   * return the existing pointer.					*
   *									*
   ********************************************************************** */

static FILE *mayhem_fopen(char *fname, char *types, FILE *fpoint)
{
   if (fpoint != (FILE *)NULL) {
      return(fpoint);
   }

   return(fopen(fname, types));
}

/* **********************************************************************
   * Check to see if the file is already closed. If so, report an error	*
   * and return an end of file condition.				*
   *									*
   ********************************************************************** */

static int mayhem_fclose(FILE **fname)
{
   int result;

   if (*fname == (FILE *)NULL) {
      return(EOF);
   }

   if ((result = fclose(*fname)) == 0) {
      *fname = (FILE *)NULL;
   }

   return(result);
}

/* **********************************************************************
   * Close all files. Return stdio's return value.			*
   *									*
   ********************************************************************** */

static int mayhem_fcloseall(void)
{
   (void)mayhem_fclose(&updatefile);
   (void)mayhem_fclose(&screen);
   (void)mayhem_fclose(&config);
   (void)mayhem_fclose(&ship_inf);
   (void)mayhem_fclose(&ship_std);
   return(0);
}

/* **********************************************************************
   * Send the character waiting to get out.                             *
   *                                                                    *
   * At the same time, give it to the console.                          *
   *                                                                    *
   ********************************************************************** */

static void send_character(char this_byte)
{
   if (is_redirected == 1)
      (void)bioscom(1, this_byte, port_assignment);

   (void)putchar(this_byte);
}

/* **********************************************************************
   * Send the string out the com port.                                  *
   *                                                                    *
   ********************************************************************** */

static void send_string(char *this_string)
{
   while (*this_string)
      send_character(*this_string++);
}

/* **********************************************************************
   * Scan the com port to see if there is a character waiting. If not,  *
   * return false. If so, then get the character and see if it's an     *
   * escape. If so, return false else return true. In any event, the    *
   * character waiting is ignored.                                      *
   *                                                                    *
   ********************************************************************** */

static unsigned char check_for_key(void)
{
   long line_status_register;
   char anything_waiting;

   if (! tandy) {
       anything_waiting = (kbhit() != 0);
   }
   else {
       anything_waiting = (bioskey(1) != 0);
   }

   if (port_assignment == 0)
      line_status_register = 0x3fd;
   else
      line_status_register = 0x2fd;

   return(((inport(line_status_register) & 1) == 1) || anything_waiting);
}

/* **********************************************************************
   * Return the character waiting.                                      *
   *                                                                    *
   * If there is a keyboard character waiting, service it.              *
   *                                                                    *
   * Return what's found at the com port.                               *
   *                                                                    *
   ********************************************************************** */

static char return_character(void)
{
    char byte;

    if (! tandy) {
	if (kbhit() != 0) {
	    byte = getch();
	    return(byte);
	}
    }
    else {
	if (bioskey(1) != 0) {
	    return(bioskey(0));
	}
    }

    return(bioscom(2, 0, port_assignment));
}

/* **********************************************************************
   * If there is a statistics file, offer it.                           *
   *                                                                    *
   ********************************************************************** */

static void give_statistics(void)
{
   char bail_out;
   char byte;

   bail_out = FALSE;

   if ((stats = mayhem_fopen("stats.shp", "rb", stats)) != (FILE *)NULL) {
      while (! feof(stats) && !bail_out && !control_c_received) {
	 if (check_for_key()) {
            byte = return_character();
            if (byte == 's' || byte == 'S' || byte == ESC) {
               bail_out = TRUE;
            }
         }
         (void)fgets(record, 200, stats);
         (void)sprintf(out_buffer, "%s", record);
         send_string(out_buffer);
      }
      control_c_received = FALSE;
      (void)newline();
   }
   else {
      send_string("\n\rNo player statistics from other Mayhem Sites is\n\r");
      send_string("available on this system. STATS.SHP file isn't found!\n\r");
   }
}


