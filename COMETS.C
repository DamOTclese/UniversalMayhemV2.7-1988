
/* **********************************************************************
   * comets.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * For those comets which has been scanned, display the information	*
   * about the various comets.						*
   *									*
   ********************************************************************** */

#include "defines.h"
#include "comets.h"
#include "stdio.h"
#include "function.h"
#include "io.h"
#include "alloc.h"
#include "conio.h"

/* **********************************************************************
   * Define the external and local data that this module needs to       *
   * access.                                                            *
   *                                                                    *
   ********************************************************************** */

   extern FILE *acomet;
   extern long xsize, ysize;
   extern long xpos, ypos;
   extern long count;
   extern long ion_field[ION_COUNT][2];
   extern long dpoint;
   extern short total_comets;

   unsigned short close_comets[40];

void perform_comets(void)
{
   for (count = 0; count < total_comets; count++) {
      if (comets[count] != (struct comets_file *)NULL) {
         if (comets[count]->flag == TRUE) {
            c_out(YELLOW, "Commet %s named by %s at {%ld-%ld}\n\r",
               comets[count]->name, comets[count]->ship,
               comets[count]->location[0], comets[count]->location[1]);
         }
      }
   }

   c_out(YELLOW, "\n\r--- Ion field detected around :%ld,%ld: ---\n\r",
      (long) ion_field[0][0], (long) ion_field[0][1]);
}

/* **********************************************************************
   * Create a comets file. If for some reason the file can not be	*
   * created, don't allow it to cause the failure of the program. We	*
   * can run without it if needed.					*
   *									*
   ********************************************************************** */

void create_comets_file(void)
{
   for (count = 0; count < total_comets; count++) {
      if (comets[count] != (struct comets_file *)NULL) {
         xpos = (long)arandom(1L, (long) xsize - 1);
         ypos = (long)arandom(1L, (long) ysize - 1);
         comets[count]->location[0] = xpos;
         comets[count]->location[1] = ypos;
         comets[count]->flag = FALSE;
         comets[count]->direction = 0;
         strcpy(comets[count]->name, "NONE");
      }
   }

   if (comets[0] != (struct comets_file *)NULL) {
      comets[0]->flag = TRUE;
      comets[0]->direction = arandom(1L, 10L);
      strcpy(comets[0]->ship, "<GP>");
      strcpy(comets[0]->name, "The Sentinel!");
   }

   write_comets();
}

/* **********************************************************************
   * Simply read out the comets file and plug it into the structure.    *
   *									*
   ********************************************************************** */

void plug_comets_values(void)
{
   char cloop;

/*
   Allocate the memory for the comets
*/

   for (cloop = 0; cloop < total_comets; cloop++) {
      comets[cloop] =
          (struct comets_file *)farmalloc(sizeof(struct comets_file));

      if (comets[cloop] != (struct comets_file *)NULL) {
         memory_allocated((UL)sizeof(struct comets_file));
      }

   }
      
/*
   See if we need to create the comets file
*/

   if ((acomet = mayhem_fopen("COMETS.DAT", "rb", acomet)) == (FILE *)NULL) {
      create_comets_file();
      return;
   }

/*
   Read out the comets file
*/
 
   for (count = 0; count < total_comets; count++) {
      dpoint = ((long) count * sizeof(struct comets_file));

      if (fseek(acomet, dpoint, 0) != 0) {
         c_out(LIGHTRED, "\n\rUnable to point to comets record in the comets file.");
         c_out(LIGHTRED, "\n\rTried to point to: %ld", dpoint);
         total_comets = 0;
         return;
      }

      if (comets[count] != (struct comets_file *)NULL) {
         if ((mayhem_fread(comets[count], sizeof(struct comets_file), 1, acomet)) != 1) {
            c_out(LIGHTRED, "\n\rError occured when reading comets record.\n\r");
            total_comets = 0;
            return;
         }
      }
   }

   mayhem_fclose(&acomet);
}


