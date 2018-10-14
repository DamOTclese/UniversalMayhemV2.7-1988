
/* **********************************************************************
   * color.c								*
   *									*
   * Copyrite 1989, 1990, 1991.                                         *
   * Fredric L. Rice, All rights reserved.                              *
   *									*
   * This module performs the user/console output. It was created on	*
   * 17/Jan/88, rather late in Mayhems development, so that color could	*
   * be added to the game.						*
   *									*
   * o If color is not allowed by the SysOp or				*
   * o Color is not enabled by the players data base element		*
   *  									*
   *     Use vprintf to perform the output				*
   * 									*
   * o If console is redirected						*
   *									*
   *      Send ANSI color sequence to console and then cprintf		*
   *									*
   * o If console is not redirected					*
   *									*
   *      Use colortext() to set color then cprintf			*
   *									*
   *									*
   * This function will need to make use of the variable arguments that	*
   * Turbo C, High C, and Unix C allow. I'm not sure if a portability	*
   * problem may be encountered.					*
   *									*
   * 'color_enable'  == TRUE or FALSE if SysOp allows colors		*
   * 'want_color'    == TRUE or FALSE if player wants colors		*
   * 'is_redirected' == 0 if console is not redirected, > 0 otherwise	*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "stdarg.h"
#include "stdio.h"
#include "conio.h"
#include "string.h"
#include "async.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern char want_color;
   extern char color_enable;
   extern char is_redirected;
   extern char port_assignment;

/* **********************************************************************
   * Allocate an array that I can use to map textcolor() color codes	*
   * to ANSI escape sequence codes.					*
   *									*
   * The way it works: Suppose the program passes a LIGHTGREEN to the	*
   * c_out program. If there is no console redirection, then we use the	*
   * textcolor(the_color) because the_color will be the number 10, (we	*
   * count the items in the list starting from 0). If the console is	*
   * redirected, we index into the array color_map[the_color] and item	*
   * number 10 is the number 32. The escape sequence for a green 	*
   * forground is ESC[32;m which will select green.			*
   *									*
   *                                                                    *
   *	BLACK,                                                          *
   *	BLUE,                                                           *
   *	GREEN,                                                          *
   *	CYAN,                                                           *
   *	RED,                                                            *
   *	MAGENTA,                                                        *
   *	BROWN,                                                          *
   *	LIGHTGRAY,                                                      *
   *	DARKGRAY,                                                       *
   *	LIGHTBLUE,                                                      *
   *	LIGHTGREEN,                                                     *
   *	LIGHTCYAN,                                                      *
   *	LIGHTRED,                                                       *
   *	LIGHTMAGENTA,                                                   *
   *	YELLOW,                                                         *
   *	WHITE                                                           *
   *									*
   * In the comments section beside each of the array elements, the	*
   * items which contain->offer the color that was requested and then	*
   * the color that is given. For instance, if the program wanted Grey,	*
   * we give it White. If the program wants Black anywhere, we give it	*
   * White. This is done because I don't want to offer black letters as	*
   * I don't expect to be changeing background colors.			*
   *									*
   ********************************************************************** */

   char color_map[][3] = {
      "37",	/* Black->White		*/
      "34",	/* Blue			*/
      "32", 	/* Green		*/
      "36",	/* Cyan			*/
      "31",	/* Red			*/
      "35",	/* Magenta		*/
      "33",	/* Brown->Yellow	*/
      "37",	/* Gray->White		*/
      "37",	/* Black->White		*/
      "34",	/* Blue			*/
      "32",	/* Green		*/
      "36",	/* Cyan			*/
      "31",	/* Red			*/
      "35",	/* Magenta		*/
      "33",	/* Brown->Yellow	*/
      "37"	/* Gray->White		*/
   } ;

/* **********************************************************************
   * To keep from sending the secape sequence for colors to the remote	*
   * terminal when console if redirected, we keep track of what the 	*
   * last color was that was sent and only send the secape sequence 	*
   * when needed.							*
   *									*
   ********************************************************************** */

   short last_color = 99;

void c_out(int the_color, char *format, ...)
{
   void *param0, *param1, *param2, *param3, *param4, *param5;
   char out_buffer[200];

   va_list arguments;
   va_start(arguments, format);
   param0 = arguments;
   param1 = va_arg(arguments, void *);
   param2 = va_arg(arguments, void *);
   param3 = va_arg(arguments, void *);
   param4 = va_arg(arguments, void *);
   param5 = va_arg(arguments, void *);

/*
   If the SysOp does not allow color or the player does not
   want color, simply call vprintf() with the format and
   a pointer to any parameters. Then return.
*/

   if (! color_enable || ! want_color) {
      if (is_redirected == 0) {
         vprintf(format, param0);
      }
      else {
         vsprintf(out_buffer, format, param0);
         ComOutStr(port_assignment, out_buffer);
      }
      return;
   }

/*
   The SysOp allows colors and the player wants colors. If the
   console is not redirected, use cprintf after setting the
   colortext to the color desired. Then return.

   If the console _is_ redirected, send the ANSI escape sequence
   that will select the proper forground color and then use
   cprintf to send the data. We always send the '40' to have a background
   color of black. This is usefull when PROCOM PLUS and the Test Drive
   version are used where the background color is always set to blue.
*/

   if (is_redirected == 0) {
      if (the_color == BLACK) {
	 the_color = WHITE;
      }
      textcolor(the_color);
   }
   else {
      if (last_color != the_color) {
         if (is_redirected == 0) {
            printf("%c[1;%s;40m", 27, color_map[the_color]);
         }
         else {
            sprintf(out_buffer, "%c[1;%s;40m", 27, color_map[the_color]);
            ComOutStr(port_assignment, out_buffer);
         }
         last_color = the_color;
      }
      if (is_redirected == 0) {
         vprintf(format, param0);
      }
      else {
         vsprintf(out_buffer, format, param0);
         ComOutStr(port_assignment, out_buffer);
      }
      return;
   }

   if (is_redirected == 0) {
      cprintf(format, param1, param2, param3, param4, param5);
   }
   else {
      sprintf(out_buffer, format, param1, param2, param3, param4, param5);
      ComOutStr(port_assignment, out_buffer);
   }
}

