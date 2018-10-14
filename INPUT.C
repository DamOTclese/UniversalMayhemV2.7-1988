
/* **********************************************************************
   * input.c								*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   ********************************************************************** */

#include "defines.h"
#include "ship.h"
#include "holder.h"
#include "stdio.h"
#include "function.h"
#include "dos.h"
#include "conio.h"
#include "time.h"
#include "stdlib.h"
#include "bios.h"
#include "alloc.h"
#include "async.h"

#define Warning_Time    90      /* Warning after 1.5 minutes            */
#define Bounce_Time    120      /* Bounce after 2 minutes               */
#define Enemy_Time      15      /* Enemy ships fire every 15 seconds    */
#define Scout_Time       1      /* Move one scout ship every 1 second   */
#define Line_Limit      70      /* The length of auto word-wrap         */

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char port_assignment;
   extern char watch_cd, in_sysop;
   extern unsigned short close_ship[TOTAL_PLAYERS];
   extern unsigned short close_base[TOTAL_PLAYERS];
   extern char *record, *old_record;
   extern char ship_name[5];
   extern char is_redirected;
   extern short ship_count;
   extern short base_count;
   extern short user_number;
   extern short time_remaining;
   extern short game_time_remaining;
   extern char time_warning;
   extern time_t scout_timer;
   extern char in_test_mode;
   extern char chat_mode;
   extern char suspended_real_time;
   extern char tandy;
   extern int ticker;
   extern char crash_reset;
   extern char entering_mail;
   extern short bail_out;
   extern short rpt_loop;
   extern char ham_version;
   extern char interrupt_enable;

/* **********************************************************************
   * If we have revectored the 0x1c interrupt, set it back.             *
   *                                                                    *
   ********************************************************************** */

   extern void interrupt (*old_interrupt)(void);
   extern void empty_transmit_buffer(ComPort ComDev);

   static time_t input_timer, enemy_timer;
   static FILE *regression_test_file;
   static void check_various_timers(void);
   unsigned char check_for_key(void);
   unsigned char last_space;
   char *continue_last;
   char interrupted_serial;
   unsigned long buffered;

/* **********************************************************************
   * Here is what this routine is designed to do:			*
   *									*
   * 1) Supply a universal keyboard input for use everywhere		*
   *									*
   * 2) Scan the keyboard and queue up characters until carriage return *
   *									*
   * 3) If no entry after 3 minutes, issue a warning and beep		*
   *									*
   * 4) If no entry after 1 minute, bounce the player to BBS control	*
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
   * We pass a 0 or a 1 to this routine. If the value is 0, then we 	*
   * will echo the character typed, else if its anything else, we will	*
   * not echo the character typed.					*
   *									*
   * A good way to crash the system was to overflow the input buffer	*
   * 'record' simply by entering more than 200 characters. In version	*
   * 1's release, we check to make sure this can't happen. If there is	*
   * an attempt made, we issue a bell and don't accept it.		*
   *									*
   ********************************************************************** */

void timed_input(short echo_or_not)
{
   char i, byte;

   record[0] = (char)NULL;
   i = time_warning = last_space = 0;

   if (continue_last != (char *)NULL && *continue_last != (char)NULL) {
      strcpy(record, continue_last);
      *continue_last = (char)NULL;
      i = strlen(record);
      c_out(WHITE, record);
   }

/*
   We maintain three time points here. Time one and two have to deal with
   the keyboard time out. That value of time where a warning should be
   issued to the player because the player hasn't done anything in awhile
   and when the player should be bounced.

   Added late into the game development is the other time point. It has to
   deal with if an enemy ship should attack if there is an unfriendly in
   the area and the player has waited too long. If this happens, the
   unfriendly ships will attack every THIRD_TIMER seconds.
*/

   time(&input_timer);          /* Input timer          */
   time(&enemy_timer);          /* Enemy fires back     */

   if (in_test_mode) {
      fgets(record, 200, regression_test_file);
      if (! feof(regression_test_file)) {
         record[strlen(record) - 1] = (char)NULL;
         check_various_timers();
         return;
      }
      else {
         mayhem_fclose(&regression_test_file);
         in_test_mode = FALSE;
         c_out(LIGHTRED, "End of file. Test mode deactivated!\n\r");
      }
   }

/*
    If we are running a HAM radio packet, then send the packet and wait
    for data to be entered. This is needed to send the packet so that
    the data can be read. Otherwise the pending data would restrict the
    received data.

    We give a line feed and THEN carriage return.
*/

    if (ham_version && (is_redirected == 1)) {
        c_out(LIGHTBLUE, "%c\r", 0x0a);
    }

    if (is_redirected && interrupted_serial) {
        empty_transmit_buffer(port_assignment);
    }

/*
   Main entry loop is here
*/

   while (TRUE) {
      if (is_redirected != 0) {
         if (! check_carrier_detect(FALSE)) {
            lost_carrier();
            exit(90);
         }
      }
      check_various_timers();
      if (check_for_key()) {
         byte = return_character();
         if (byte > 0) {
            if (i >= 190 && byte != BACKSPACE && byte != 0x0d) {
                send_character(0x07);
            }
            else
            {
               time_warning = 0;
               if (echo_or_not == 0) {
                  if (byte != BACKSPACE) {
                     send_character(byte);
                     if (byte == ' ') {
                        last_space = i;
                     }
                  }
               }
               else {
                  if (byte != BACKSPACE) {
                     send_character('.');
                     if (byte == ' ') {
                        last_space = i;
                     }
                  }
               }
               if (byte == 0x0d) {
                  record[i++] = (char)NULL;
                  c_out(WHITE, "\n\r");
                  return;
               }
               else if (byte == BACKSPACE) {
                  if (i > 0) {
                     if (i == last_space) {
                        last_space = 0;
                     }
                     record[i--] = (char)NULL;
                     send_character(BACKSPACE);
                     send_character(SPACE);
                     send_character(BACKSPACE);
                  }
               }
               else if (byte == CONTROL_X) {
                  for (i++; i > 0; i--) {
                     send_character(BACKSPACE);
                     send_character(SPACE);
                     send_character(BACKSPACE);
                  }
                  record[0] = (char)NULL;
                  last_space = 0;
               }
               else {
                  time(&input_timer);
                  record[i++] = byte;
               }
            }
         }
         if (chat_mode || entering_mail) {
            char *atpoint;
            if (continue_last != (char *)NULL) {
                if (last_space == Line_Limit) {
                    return;
                }
                if (i == Line_Limit && last_space > 0) {
                    for ( ; i != last_space; i--) {
                        send_character(BACKSPACE);
                        send_character(SPACE);
                        send_character(BACKSPACE);
                    }
                    atpoint = record;
                    atpoint += last_space;
                    STRNCPY(continue_last, atpoint + 1, (Line_Limit - last_space) - 1);
                    record[last_space] = (char)NULL;
                    return;
                }
            }
         }
      }
   }
}

/* **********************************************************************
   * Scan the com port to see if there is a character waiting. If not,	*
   * return false. If so, then get the character and see if it's an	*
   * escape. If so, return false else return true. In any event, the	*
   * character waiting is ignored.					*
   *									*
   * An addition for version 2.3:                                       *
   *                                                                    *
   * If Mayhem crashes, the computer will reset after the reset timer   *
   * expires. This function cleans the timer back up to its setting and *
   * the 18.2 clock tick decriments it.                                 *
   *                                                                    *
   ********************************************************************** */

unsigned char check_for_key(void)
{
   long line_status_register;
   char anything_waiting;

   ticker = Reset_Timer * 60 * 18.2;

   if (check_sysop_key()) {
      return(FALSE);
   }

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

    if (interrupted_serial)
        return(anything_waiting || ComInReady(port_assignment));

   return(((inport(line_status_register) & 1) == 1) || anything_waiting);
}

/* **********************************************************************
   * Who is the user?                                                   *
   *                                                                    *
   ********************************************************************** */

static void who_is_user(void)
{
    sprintf(record,
        "\n\r[%s], captain: [%s], played [%d] times, [%d] minutes remain\n\r",
        ships->ship_name,
        ships->ship_person,
        ships->log_count,
        game_time_remaining);

    fputs(record, stderr);
    sleep(1);
}

/* **********************************************************************
   * See if sysop keys has been pressed. If so, perform some function.  *
   *									*
   * These commands will only work if the program is redirected.        *
   *                                                                    *
   ********************************************************************** */

char check_sysop_key(void)
{
    char the_key;
         
    if (user_number == 0 || is_redirected == 0)
       return(FALSE);

/*
    Control Bounce. Must be redirected to work!
*/

    if ((the_key = bioskey(1)) == 2) {
        the_key = bioskey(0);   /* Get key out of buffer */
        perform_quit(41);
    }

/*
    Control User
*/

    else if (the_key == 21) {
        the_key = bioskey(0);   /* Get key out of buffer */
        who_is_user();
        return(TRUE);
    }

/*
    Control Talk
*/

    else if (the_key == 20) {
        chat_mode = TRUE;
        the_key = bioskey(0);   /* Get key out of buffer */
        perform_talk();
        return(TRUE);
    }

/*
    Control Exit talk.
*/

    else if (the_key == 5) {
        if (! chat_mode) {
            c_out(WHITE, "You are not in chat mode right now!\n\r");
        }
        else {
            the_key = bioskey(0);   /* Get key out of buffer */
            chat_mode = FALSE;
            c_out(WHITE, "\n\r---> Chat mode terminated <---\n\r");
            c_out(WHITE, "---> Hit [ENTER] to resume Mayhem <---\n\r\n\r");
        }

        return(TRUE);
    }

    return(FALSE);
}

/* **********************************************************************
   * If we are not supposed to care about carrier detect, then simply	*
   * return saying that carrier detect was present, even if it wasn't.	*
   * 									*
   * If we do care about carrier detect, read the com ports status and	*
   * return the result as a true/false value.				*
   *									*
   ********************************************************************** */

char check_carrier_detect(char direction_test)
{
   int status;
   char return_value;

   status = bioscom(3, 0, port_assignment);
   return_value = (status & 0x0080) == 0x0080;
   if (direction_test) return(return_value);
   if (! watch_cd) return(TRUE);
   return(return_value);
}

/* **********************************************************************
   * Return the character waiting.                                      *
   *                                                                    *
   * If there is a keyboard character waiting, service it.              *
   *                                                                    *
   * Return what's found at the com port.                               *
   *                                                                    *
   ********************************************************************** */

char return_character(void)
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

    if (is_redirected == 0) return(FALSE);

    if (interrupted_serial) {
        ComIn(port_assignment, &byte, NoEcho);
        return(byte);
    }

    return(bioscom(2, 0, port_assignment));
}

/* **********************************************************************
   * Send the character waiting to get out.                             *
   *                                                                    *
   * At the same time, give it to the console.                          *
   *                                                                    *
   ********************************************************************** */

void send_character(char this_byte)
{
   unsigned int tx_status;
   char sent_char, got_echo;
   char return_byte;
   char wait_for_cts;
   short retry;

   if (is_redirected == 1) {
      if (interrupted_serial) {
         while (ham_version && !ComOutReady(port_assignment)) {
             if (! check_carrier_detect(FALSE)) {
                 lost_carrier();
             }
         }
         ComOut(port_assignment, this_byte);
      }
      else {
          bioscom(1, this_byte, port_assignment);
      }
   }

   putchar(this_byte);
}

/* **********************************************************************
   * Send an entire string.                                             *
   *									*
   ********************************************************************** */

void send_string(char *this_one)
{
    while (*this_one) {
        send_character(*this_one);
        this_one++;
    }
}

/* **********************************************************************
   * When carrier is lost, we come here.				*
   *									*
   ********************************************************************** */

void lost_carrier(void)
{
   mayhem_fcloseall();

   c_out(LIGHTRED, "\n\rCarrier lost!\n\r");

   if (crash_reset) {
      setvect(0x1c, old_interrupt);
   }

   if (interrupted_serial) {
      ComClose(port_assignment);
   }

   exit(90);
}

/* **********************************************************************
   * Check to see if there is a timer that needs to be serviced.        *
   *                                                                    *
   ********************************************************************** */

static void check_various_timers(void)
{
   char s_test;
   time_t time_test;
   char some_hits;

   if (user_number == 0) return;
   time(&time_test);
   some_hits = FALSE;

/*
   System timed out. Bounce everything back to the stone age
*/

   if (! ham_version) {
      if (difftime(time_test, input_timer) > Bounce_Time) {
         mayhem_fcloseall();
         c_out(WHITE, "%c%c%c%c%c%c%c%c%c%c", 7, 0, 0, 7, 0, 0, 7, 0, 0, 7);
         c_out(LIGHTRED,
            "\n\rKeyboard entry timed out... Returning to BBS system.\n\r");
         chat_mode = FALSE;
         perform_quit(TIMED_OUT);
      }
   }

/*
   See if a time-out warning is needed
*/

   if (! ham_version) {
      if (difftime(time_test, input_timer) > Warning_Time && time_warning == 0) {
         c_out(WHITE, "%c%c%c%c%c%c%c%c%c%c", 7, 0, 0, 7, 0, 0, 7, 0, 0, 7);
         c_out(LIGHTRED, "\n\rWarning! Soon you are going to be bounced off!!!\n\r--> ");
         time_warning = 1;
         chat_mode = FALSE;
      }
   }

/*
   At this point, no characters have been entered for THIRD_TIME seconds,
   (initially set to 10 seonds in the first release with 60 seconds until
   the first warning or so).

   See if there are any unfriendly ships or bases in the area and if
   so, have them fire phasers and torpedos. This should keep the player
   from sitting around to long with unfriendlies around.

   Note that we reset the third timer point so that this routine will
   be triggered again.

   Oh, a bug! on't allow your own ship or base to fire at you!

   If the player is in the SysOp functions, don't allow ships to
   fire on the SysOp.
*/

   if (difftime(time_test, enemy_timer) > Enemy_Time && !in_sysop) {
      plug_close_objects(FALSE);
      time(&enemy_timer);
      for (s_test = 0; s_test < ship_count; s_test++) {
         if (close_ship[s_test] != user_number) {
            if (! hold[close_ship[s_test]]->is_friendly) {
               return_phaser(close_ship[s_test]);
               return_torp(close_ship[s_test]);
               some_hits = TRUE;
            }
         }
      }
      for (s_test = 0; s_test < base_count; s_test++) {
         if (close_base[s_test] != user_number) {
            if (! hold[close_base[s_test]]->is_friendly) {
               return_phaser(close_base[s_test]);
               return_torp(close_base[s_test]);
               some_hits = TRUE;
            }
         }
      }
      if (some_hits) c_out(WHITE, "<continue> ");
      some_hits = FALSE;
   }

/*
   Check to see if a scout ship should be moved
*/

   if (!chat_mode && !suspended_real_time) {
      if (difftime(time_test, scout_timer) > Scout_Time) {
         automate_scout_ships();
         time(&scout_timer);
      }
   }
}

/* **********************************************************************
   * Place the Universal Mayhem program into test mode.                 *
   *                                                                    *
   ********************************************************************** */

void start_test_mode(void)
{
   regression_test_file = (FILE *)NULL;
   regression_test_file = mayhem_fopen("SHIP.MSG", "rt", regression_test_file);

   if (regression_test_file == (FILE *)NULL) {
      c_out(LIGHTRED, "Regression file is missing! Program halted!\n\r");
      perform_quit(0);
   }

   c_out(LIGHTRED, "Test mode activated!\n\r");
   in_test_mode = TRUE;
}

/* **********************************************************************
   * Get rid of everything in the com port.                             *
   *                                                                    *
   ********************************************************************** */

void throw_com_port_away(void)
{
    ComInFlush(port_assignment);
}

/* **********************************************************************
   * Initialize the interrupt driver serial I/O process.                *
   *                                                                    *
   ********************************************************************** */

void io_init(void)
{
    continue_last = (char *)farmalloc(201);
    memory_allocated(201);
    interrupted_serial = FALSE;
    buffered = 0;

    if (interrupt_enable) {
        if (is_redirected == 1) {
            ComOpen(port_assignment, MedSpeed, EvenParity);
            interrupted_serial = TRUE;
        }
    }

    if (continue_last == (char *)NULL) {
        c_out(LIGHTRED, "!!! Unable to implement wrap-around feature!!!\n\r");
        log_error(143);
    }
    else {
        continue_last[0] = (char)NULL;
    }

    last_space = 0;
}

/* **********************************************************************
   * Close the interrupted serial stuff.                                *
   *                                                                    *
   ********************************************************************** */

void io_close(void)
{
    if (is_redirected == 0) return;

    if (interrupted_serial) {
        ComClose(port_assignment);
        interrupted_serial = FALSE;
    }
}

/* **********************************************************************
   * Describe the macro syntax.                                         *
   *                                                                    *
   ********************************************************************** */

static void give_syntax(void)
{
    c_out(LIGHTRED, "Syntax:\n\r");

    c_out(LIGHTRED,
        "#[enter]                       for a list of keyboard macros defined\n\r");

    c_out(LIGHTRED,
        "#[0 through %d][enter]          to execute a previously stored macro\n\r",
        MACROS);

    c_out(LIGHTRED,
        "#[0 through %d][string][enter]  to store the macro within[string]\n\r",
        MACROS);

    c_out(LIGHTGREEN, "\n\rExamples:\n\r");
    c_out(WHITE, "         #2w910;w910;w910;find;go;protect;b11000;rpt30\n\r");
    c_out(WHITE, "\n\rThis would store the macro. To execute:\n\r");
    c_out(WHITE, "         #2\n\r\n\r");
}

/* **********************************************************************
   * # [enter]                  will list the keyboard macros.          *
   * #n (n from 0 to MACROS)    will execute the macro.                 *
   * #n {command string}        will store the macro.                   *
   *                                                                    *
   ********************************************************************** */

char perform_macro(void)
{
    char count;
    char *atpoint, *testing;

    if (strlen(record) == 1) {
        c_out(LIGHTGREEN, "List of keyboard macros:\n\r");
        c_out(LIGHTBLUE, "------------------------\n\r");
        for (count = 0; count < MACROS; count++) {
            c_out(LIGHTGREEN, "Macro %d: '%s'\n\r\n\r",
                count, ships->macro[count]);
        }

        bail_out = 1;
        rpt_loop = 0;
        return(FALSE);
    }

    atpoint = old_record;
    atpoint++;
    skipspace(atpoint);
    count = atoi(atpoint);

    if (count == 0 && *atpoint != '0') {
        give_syntax();
        bail_out = 1;
        rpt_loop = 0;
        return(FALSE);
    }

    if (count >= MACROS) {
        give_syntax();
        bail_out = 1;
        rpt_loop = 0;
        return(FALSE);
    }

/*
   See if we hit the end of the string. Point past the macro number.
*/

    atpoint++;
    if (count > 9) atpoint++;
    if (count > 99) atpoint++;
    skipspace(atpoint);

/*
   If it's the end, get the macro and return FALSE.
*/

    if (*atpoint == (char)NULL) {
        strcpy(record, ships->macro[count]);
        strcpy(old_record, record);
        return(TRUE);
    }

/*
   Store the macro and then execute it.
*/

    testing = atpoint;

    while (*testing) {
        if (*testing == '#') {
            c_out(LIGHTRED,
                "Nesting or chaining of Keyboard Macros is not allowed!\n\r");
            bail_out = 1;
            rpt_loop = 0;
            return(FALSE);
        }

        testing++;
    }

    STRNCPY(ships->macro[count], atpoint, 200);
    write_user();
    old_record[0] = record[0] = (char)NULL;
    return(TRUE);
}


