
/* **********************************************************************
   * mail.c								*
   *									*
   * Copyrite 1988, 1989, 1990, 1991.                                   *
   * Fredric L. Rice. All rights reserved.                              *
   *									*
   * Two E-Mail message standards are supported:                        *
   *            Fido *.MSG                                              *
   *            RA/QBBS (5 files)                                       *
   *                                                                    *
   * Current *.MSG KLUDGE flags in the standard are:                    *
   * FTS-0001 - Fido Technical Standards Document                       *
   *                                                                    *
   *   o TOPT <pt no> - destination point address                       *
   *   o FMPT <pt no> - origin point address                            *
   *   o INTL <dest z:n/n> <orig z:n/n> - used for inter-zone address   *
   *                                                                    *
   * I've extended the kludge with the following:                       *
   *   o NOTE <Anything>                                                *
   *                                                                    *
   ********************************************************************** */

#include "defines.h"
#include "holder.h"
#include "ship.h"
#include <stdio.h>
#include "function.h"
#include <dir.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <alloc.h>
#include <io.h>

/* **********************************************************************
   * The string array 'allow_mail' determines if mail is enabled by	*
   * the system operator. Some BBS packages do not allow proper		*
   * subspace mail handleing.						*
   *									*
   ********************************************************************** */

   extern char allow_mail[4];
   extern FILE *mail, *subspace_messages;
   extern FILE *command_file;
   extern long count;
   extern char *record;
   extern char *subspace_mail;
   extern short players;
   extern short active_players;
   extern short user_number;
   extern short from_node_zone, from_node_network, from_node_number;
   extern short from_node_point, to_node_zone, to_node_network;
   extern short to_node_number, to_node_point;
   extern short stat_node_zone, stat_node_network, stat_node_number;
   extern short stat_node_point;
   extern char the_date[27];
   extern char statistics_enabled;
   extern char is_redirected;
   extern long xsize, ysize;
   extern char want_color;
   extern char color_enable;
   extern char *statistics_outbound_directory;
   extern char *network_directory;
   extern char stat_hold;
   extern char *echo_origin;
   extern char *network_origin;
   extern char crash_reset;
   extern char entering_mail;
   extern char *continue_last;
   extern char *goal_item_description[10];
   extern char stat_file_name[100];
   extern char *point;
   extern char mail_actives;
   extern char mail_type;
   extern char echo_board_number;
   extern char shoved_flag;

/* **********************************************************************
   * An interesting update requested by Astro-Nets SysOp: The did_mail	*
   * flag determines what error level to return to the operating	*
   * system in the event of a normal exit. This is so that the batch	*
   * file handeling the Universal Mayhem package may process the mail	*
   * slot only if main had been entered and not waste time if it had	*
   * not been entered.							*
   *									*
   ********************************************************************** */

   extern char did_mail;

#define Network_Mail    0
#define Echo_Mail       1
#define Subspace_Mail   2

/* **********************************************************************
   * The message file format offered here is Fido format which has	*
   * been tested with OPUS and Dutchie. It represents the latest	*
   * format that I know about.						*
   *									*
   ********************************************************************** */

   static struct fido_msg {
      char from[36];		/* Who the message is from		  */
      char to[36];		/* Who the message to to		  */
      char subject[72];		/* The subject of the message.		  */
      char date[20];		/* Message createion date/time		  */
      int times;		/* Number of time the message was read	  */
      int destination_node;	/* Intended destination node		  */
      int originate_node;	/* The originator node of the message	  */
      int cost;			/* Cost to send this message		  */
      int originate_net;	/* The originator net of the message	  */
      int destination_net;	/* Intended destination net number	  */
      int destination_zone;	/* Intended zone for the message	  */
      int originate_zone;	/* The zone of the originating system	  */
      int destination_point;	/* Is there a point to destination?	  */
      int originate_point;	/* The point that originated the message  */
      unsigned reply;		/* Thread to previous reply		  */
      unsigned attribute;	/* Message type				  */
      unsigned upwards_reply;	/* Thread to next message reply		  */
      char a_character_1;	/* Used for AREA:MAYHEM precursor	  */
      char area_name[12];       /* Echo mail name                         */
   } *message;                  /* Something to point to this structure   */

/* **********************************************************************
   * 'Attribute' bit definitions we will use                            *
   *                                                                    *
   ********************************************************************** */

#define Fido_Crash              0x0002
#define Fido_Kill               0x0080
#define Fido_Local              0x0100
#define Fido_File_Attach        0x0010
#define Fido_Hold               0x0200

/* **********************************************************************
   * MSGINFO.BBS File structure                                         *
   *                                                                    *
   * Element 'total_on_board' is an array of words which indicates the  *
   * number of messages in each message area (board). If you wanted to  *
   * find out how many messages were on board 3, for instance, you      *
   * would access total_on_board[2].                                    *
   *                                                                    *
   ********************************************************************** */

    struct Message_Information {
	unsigned int lowest_message;
        unsigned int highest_message;
        unsigned int total_messages;
        unsigned int total_on_board[200];
    } *msg_info;

/* **********************************************************************
   * MSGIDX.BBS File structure                                          *
   *                                                                    *
   ********************************************************************** */

    struct Message_Index {
	unsigned int message_number;
	unsigned char board_number;
    } *msg_index;

/* **********************************************************************
   * MSGTOIDX.BBS File structure                                        *
   *                                                                    *
   * Since the data structure indicates a Pascal convention of storage  *
   * of the string length prior to the actual string, we allocate a     *
   * single byte called 'string_length' and use it to insert a NULL     *
   * into the element 'to_record[]' to make it conform to the C         *
   * convention of a NULL terminated string. We do this for each string *
   * element that happens to occur in the Remote Access message         *
   * subsystem.                                                         *
   *                                                                    *
   ********************************************************************** */

    struct Message_To_Index {
	unsigned char string_length;    /* Length of next field         */
        char to_record[35];             /* Null padded                  */
    } *msg_to;

/* **********************************************************************
   * MSGHDR.BBS File strucrure                                          *
   *                                                                    *
   *  message_number is somewhat redundant yet offers some validation of*
   *     the Remote Access data files.                                  *
   *                                                                    *
   * start_block indicates an index into the message text file:         *
   *    MSGTXT.BBS. Each block in the text file is 255 bytes long and   *
   *    there is some additional overhead for the length of the string  *
   *    that describes the messages. To find the starting point of the  *
   *    text of this message, then, you would multiply the size of the  *
   *    message text structure by the starting block number offered     *
   *    here, and you yield a byte offset that may be used to seek into *
   *    the text file.                                                  *
   *                                                                    *
   * message_attribute - Bits are described below:                      *
   *    0 - Deleted message                                             *
   *    1 - Unmoved outbound network message (Not echo mail)            *
   *    2 - Is a network message                                        *
   *    3 - Private message (not allowed in echo mail)                  *
   *    4 - Received message                                            *
   *    5 - Unmoved outbound echo mail message (Not network mail)       *
   *    6 - Local bit (Message was created here)                        *
   *    7 - Reserved (Use not yet defined)                              *
   *                                                                    *
   * network_attribute - Bits are described below:                      *
   *    0 - Kill message after being sent                               *
   *    1 - Sent message OK                                             *
   *    2 - File attach                                                 *
   *    3 - Crash Priority                                              *
   *    4 - Request Receipt                                             *
   *    5 - Audit request                                               *
   *    6 - Is a return receipt                                         *
   *    7 - Reserved (Use not yet defined)                              *
   *                                                                    *
   * board is somewhat redundant also yet could be used to validate the *
   *    Remote Access data files when used with the message number and  *
   *    the message index file.                                         *
   *                                                                    *
   * date, time, who_to, who_from, subject - These are all not          *
   *    specifically NULL terminated though they may be. Reguardless,   *
   *    the bytes prior to them indicate the strings length.            *
   *                                                                    *
   ********************************************************************** */

    struct Message_Header {
        unsigned int message_number;            /* 2 bytes - 1/2    */
        unsigned int previous_reply;            /* 2 bytes - 3/4    */
        unsigned int next_reply;                /* 2 bytes - 5/6    */
        unsigned int times_read;                /* 2 bytes - 7/8    */
        unsigned int start_block;               /* 2 bytes - 9/10   */
        unsigned int number_blocks;             /* 2 bytes - 11/12  */
        unsigned int destination_network;       /* 2 bytes - 13/14  */
        unsigned int destination_node;          /* 2 bytes - 15/16  */
        unsigned int originating_network;       /* 2 bytes - 17/18  */
        unsigned int originating_node;          /* 2 bytes - 19/20  */
        unsigned char destination_zone;         /* 1 byte  - 21     */
        unsigned char origination_zone;         /* 1 byte  - 22     */
        unsigned int cost;                      /* 2 bytes - 23/24  */
        unsigned char message_attribute;        /* 1 byte  - 25     */
        unsigned char network_attribute;        /* 1 byte  - 26     */
        unsigned char board;                    /* 1 byte  - 27     */
	unsigned char ptlength;         /* Hard-coded to 5              */
	char post_time[5];              /* hh:mm                        */
	unsigned char pdlength;         /* Hard-coded to 8              */
	char post_date[8];              /* mm-dd-yy                     */
	unsigned char wtlength;         /* Length of next field         */
        char who_to[35];                /* Null padded                  */
	unsigned char wflength;         /* Length of next field         */
        char who_from[35];              /* Null padded                  */
	unsigned char slength;          /* Length of next field         */
        char subject[72];               /* Null padded                  */
    } *msg_hdr;

/* **********************************************************************
   * Message attribute bit definitions we will use                      *
   *                                                                    *
   ********************************************************************** */

#define Outbound_Network_Mail   0x02
#define Is_Network_Mail         0x04
#define Outbound_Echo_Mail      0x20
#define RA_Local_Bit            0x40

/* **********************************************************************
   * Network attribute bit definitions we will use                      *
   *                                                                    *
   ********************************************************************** */

#define RA_File_Attach          0x04
#define RA_Crash_Priority       0x08

/* **********************************************************************
   * MSGTXT.BBS File structure                                          *
   *                                                                    *
   * The text of the messages is offered with the first byte indicating *
   * the length of the block that's actually used. It could be that all *
   * of the 255 byte block is used for the message and that the next    *
   * blocks will likewise also be fully used. Good going, Remote Access *
   * guys, this saves a LOT of unused disk space.                       *
   *                                                                    *
   * Remote access places the ^A Kludge lines at the top of the text    *
   * file here. There is a product ID (Kludge PID:), and a message ID,  *
   * (Kludge MSGID:). Both of these Kludge lines are terminated with a  *
   * carriage return.                                                   *
   *                                                                    *
   * The lines of the text occures next. Each line is terminated with a *
   * carriage return.                                                   *
   *                                                                    *
   * A 'tear line' comes after all of the text. This is the ---         *
   * characters which indicates that what follows is a human-readable   *
   * identification which usually offers the origination text line of   *
   * the originating system. The tear line can also be used to indicate *
   * that anything which follows may be discarded as unimportant. For   *
   * most, if not all, of the FidoNet world, information after the tear *
   * line is never discarded. This tear line is terminated with both a  *
   * carriage return _AND_ a line feed.                                 *
   *                                                                    *
   * The originating systems origin line comes next (if there is to be  *
   * an origin line. Network and local mail will probably not have an   *
   * origin line). It is terminated with a carriage return.             *
   *                                                                    *
   * The rest of the 255 byte block (if there is more) is all NULLs.    *
   *                                                                    *
   * If the person who entered the message allowed the entry to         *
   * automatically word wrap, then rather than there being a carriage   *
   * return, there will be a soft carriage return. This is, instead of  *
   * a 0x0d, a 0x8d.                                                    *
   *                                                                    *
   ********************************************************************** */

    struct Message_Text {
	unsigned char trlength;         /* Length of next field         */
        unsigned char text_record[255]; /* CR delimited, NULL padded    */
    } *msg_text;

/* **********************************************************************
   * For Remote Access and Quick BBS, we maintain a linked list of all  *
   * the text that's to be saved in the file in the event the message   *
   * does get saved (doesn't get aborted).                              *
   *                                                                    *
   * When we are certain the message is to be saved, we go through this *
   * linked list and pull out the text, appending it to the end of the  *
   * E-Mail message format for RA/QBBS Text.                            *
   *                                                                    *
   * When all is done, indeed, if aborted, the text within this linked  *
   * list, as well as the linked list itself, gets deallocated.         *
   *                                                                    *
   ********************************************************************** */

    static struct Text_Entry {
        char *text_line;                /* What is the line of text?    */
        char line_length;               /* The number of characters?    */
        struct Text_Entry *next;        /* Where is the first one?      */
    } *te_first, *te_next, *te_point;

/* **********************************************************************
   * Define data storage needed                                         *
   *                                                                    *
   ********************************************************************** */

   struct ffblk file_block;
   char to_file_listing[20];
   static FILE *mail_list;
   static int original_message_number;
   static char hold_message_file_name[81];
   static FILE *msg_file;
   static FILE *MSGINFO, *MSGIDX, *MSGTOIDX, *MSGHDR, *MSGTXT;
   unsigned int ra_lowest, ra_highest, ra_total;
   unsigned int block_count;

/* **********************************************************************
   * Allocate some memory for the message base.                         *
   *                                                                    *
   * mail type designates the type of message system to use:            *
   *                                                                    *
   *    0 - Fido *.MSG format                                           *
   *    1 - Remote Access/QBBS                                          *
   *                                                                    *
   * If we are RA/QBBS Format, open up all the message base files.      *
   *                                                                    *
   ********************************************************************** */

void mail_init(void)
{
    int result;
    char record[81];

    te_first = te_next = te_point = (struct Text_Entry *)NULL;
    msg_file = (FILE *)NULL;
    MSGINFO = MSGIDX = MSGTOIDX = MSGHDR = MSGTXT = (FILE *)NULL;
    block_count = (unsigned int)NIL;

    if (! strncmp(allow_mail, "ON", 2)) {

        if (subspace_mail[strlen(subspace_mail) - 1] != '\\') {
            subspace_mail[strlen(subspace_mail) - 1] = (char)NULL;
            (void)strcat(subspace_mail, "\\");
        }

/*
    If the echo mail board number is 0, we use the *.MSG type format,
    otherwise we consider it RA/QBBS for now. Other formats will be
    supported
*/

        if (echo_board_number == 0) {
            mail_type = 0;
        }
        else {
            mail_type = 1;
        }

        message = (struct fido_msg *)farmalloc(sizeof(struct fido_msg));

        if (message != (struct fido_msg *)NULL) {
            memory_allocated(sizeof(struct fido_msg));
        }
        else {
            (void)strcpy(allow_mail, "OFF"); /* No memory. Turn it off */
        }

        if (mail_type == 1) {
            msg_info = (struct Message_Information *)
                farmalloc(sizeof(struct Message_Information ));

            msg_index = (struct Message_Index *)
                farmalloc(sizeof(struct Message_Index));

            msg_to = (struct Message_To_Index *)
                farmalloc(sizeof(struct Message_To_Index));

            msg_hdr = (struct Message_Header *)
                farmalloc(sizeof(struct Message_Header));

            msg_text = (struct Message_Text *)
                farmalloc(sizeof(struct Message_Text));

            if (msg_info == (struct Message_Information *)NULL ||
                msg_index == (struct Message_Index *)NULL ||
                    msg_to == (struct Message_To_Index *)NULL ||
                        msg_hdr == (struct Message_Header *)NULL ||
                            msg_text == (struct Message_Text *)NULL) {
                (void)strcpy(allow_mail, "OFF");
            }
            else {
                memory_allocated(sizeof(struct Message_Information));
                memory_allocated(sizeof(struct Message_Index));
                memory_allocated(sizeof(struct Message_To_Index));
                memory_allocated(sizeof(struct Message_Header));
                memory_allocated(sizeof(struct Message_Text));

/*
    How many messages are in the Remote Access message system?
    This information is for display only. When the information
    is updated in the file, the structure elements are updated.

    If the information can not be read, we issue an error, mark
    the mail as not working, and then return.

    Open up ALL five of the message files now.
*/

                (void)sprintf(record, "%smsginfo.bbs", subspace_mail);
                if ((MSGINFO = fopen(record, "r+b")) == (FILE *)NULL) {
                    log_error(148);
                    (void)strcpy(allow_mail, "OFF");
                    return;
                }

                (void)sprintf(record, "%smsgidx.bbs", subspace_mail);
                if ((MSGIDX = fopen(record, "a+b")) == (FILE *)NULL) {
                    log_error(148);
                    (void)strcpy(allow_mail, "OFF");
                    return;
                }

                (void)sprintf(record, "%smsgtoidx.bbs", subspace_mail);
                if ((MSGTOIDX = fopen(record, "a+b")) == (FILE *)NULL) {
                    log_error(148);
                    (void)strcpy(allow_mail, "OFF");
                    return;
                }

                (void)sprintf(record, "%smsghdr.bbs", subspace_mail);
                if ((MSGHDR = fopen(record, "a+b")) == (FILE *)NULL) {
                    log_error(148);
                    (void)strcpy(allow_mail, "OFF");
                    return;
                }

                (void)sprintf(record, "%smsgtxt.bbs", subspace_mail);
                if ((MSGTXT = fopen(record, "a+b")) == (FILE *)NULL) {
                    log_error(148);
                    (void)strcpy(allow_mail, "OFF");
                    return;
                }

/*
    Get certain information into the mail system. This file is
    re-wound and re-written when messages are appended to it.
*/

                result = fread(msg_info, sizeof(struct Message_Information),
                    1, MSGINFO);

                if (result != 1) {
                    log_error(148);
                    (void)strcpy(allow_mail, "OFF");
                    return;
                }

                ra_lowest = msg_info->lowest_message;
                ra_highest = msg_info->highest_message;
                ra_total = msg_info->total_messages;
            }
        }
    }
}

/* **********************************************************************
   *									*
   * Create a subspace message in OPUS format. Allow any number of	*
   * lines to be put in, (up to 2K), and terminate on blank line.	*
   *									*
   * This routine will be replaced with a wrap-around message entry 	*
   * which I think can be taken from a program called 'bbs.c'.		*
   *									*
   ********************************************************************** */

void perform_msg(void)
{
   char arecord[201], *atpoint;

   if (strncmp(allow_mail, "ON", 2)) {
      c_out(WHITE, "Subspace mail can't be sent due to heavy noise levels\n\r");
      return;
   }

   (void)strcpy(arecord, record);
   atpoint = arecord;
   atpoint += 3;

   if (strlen(atpoint) > 4) {
      while (*atpoint == 0x20 && *atpoint) {
         atpoint++;
      }
   }

   if (strlen(atpoint) == 4) {
      (void)strcpy(record, atpoint);
      ucase(record);
   }
   else {
      c_out(WHITE, "Enter the name of the ship to send message to (Or ALL): ");
      timed_input(0);
      ucase(record);
      record[4] = (char)NULL;
   }

   if (! strcmp(record, "ALL")) {
      message_to_this((short)NIL, TRUE, "");
      return;
   }

   if (strlen(record) != 4) {
      return;
   }

   if (! strcmp(record, hold[user_number]->names)) {
      c_out(WHITE, "Send mail to yourself? Don't be silly!\n\r");
      return;
   }

/*
    See if the message is going to a ship that's defined in the
    players data records. If so, ask to create a message to it.
*/

   for (count = 0; count < players; count++) {
      if (Good_Hold(count)) {
         if (! strcmp(record, hold[count]->names)) {
            message_to_this(count, TRUE, "");
            return;
         }
      }
   }

/*
    The players name wasn't found... Assume it's not a local
    player and ask the message to be created to a remote player.
*/

   message_to_this(count, FALSE, record);
}

/* **********************************************************************
   * A function is called rather than simply including it where it's    *
   * needed; this is so that and additional conditioning required in    *
   * the future may take place here.                                    *
   *                                                                    *
   ********************************************************************** */

void msg_out(char byte)
{
   char test;
        
   if (mail_type == 0) {
      test = fputc(byte, msg_file);

      if (test != byte) {
         c_out(LIGHTRED, "Unable to finish write of Subspace Message!\n\r");
         log_error(117);
      }
   }
   else if (mail_type == 1) {
      te_point = farmalloc(sizeof(struct Text_Entry));
      te_point->text_line = farmalloc(1);
      *te_point->text_line = byte;
      te_point->line_length = 1;
      te_point->next = (struct Text_Entry *)NULL;
      memory_allocated(sizeof(struct Text_Entry));
      memory_allocated(1);

      if (te_first == (struct Text_Entry *)NULL) {
         te_first = te_next = te_point;
      }
      else {
         te_next->next = te_point;
         te_next = te_point;
      }
   }
   else {
      log_error(149);
      return;
   }
}

/* **********************************************************************
   * Take the data in the_buffer and plug it into the file if it's the  *
   * *.MSG format, else create a linked list entry if it's RA/QBBS.     *
   *                                                                    *
   ********************************************************************** */

void plug_msg(char *the_buffer, int char_count)
{
   char byte;

   if (mail_type == 0) {
      for (count = 0; count < char_count; count++, the_buffer++) {
         byte = fputc(*the_buffer, msg_file);

         if (byte != *the_buffer) {
            c_out(LIGHTRED, "Unable to finish write of Subspace Message!\n\r");
            log_error(116);
            return;
         }
      }
   }
   else if (mail_type == 1) {
      te_point = farmalloc(sizeof(struct Text_Entry));
      te_point->text_line = farmalloc(strlen(the_buffer) + 1);
      (void)strcpy(te_point->text_line, the_buffer);
      te_point->line_length = strlen(the_buffer);
      te_point->next = (struct Text_Entry *)NULL;
      memory_allocated(sizeof(struct Text_Entry));
      memory_allocated(te_point->line_length);

      if (te_first == (struct Text_Entry *)NULL) {
         te_first = te_next = te_point;
      }
      else {
         te_next->next = te_point;
         te_next = te_point;
      }
   }
   else {
      log_error(149);
      return;
   }
}

/* **********************************************************************
   * If we are sending to a point or we are a point, then we need to    *
   * make sure that we insert the proper Kludge lines.                  *
   *                                                                    *
   * If we are an international zone transfer, then we must also make   *
   * the proper Kludge as well.                                         *
   *                                                                    *
   * These are FidoNet Technical Standards Document: FTS-0001.TXT.      *
   * Other kludge lines get suggested and when they are needed to be    *
   * implimented, this one routine is designed to cover it all.         *
   *                                                                    *
   ********************************************************************** */

static void do_kludge(void)
{
   if (to_node_point > 0) {
      (void)sprintf(record, "%cTOPT:%d", 0x01, to_node_point);
      plug_msg(record, strlen(record));
      msg_out(13);
      msg_out(10);
   }

   if (from_node_point > 0) {
      (void)sprintf(record, "%cFMPT:%d", 0x01, from_node_point);
      plug_msg(record, strlen(record));
      msg_out(13);
      msg_out(10);
   }

   if (from_node_zone != to_node_zone) {

      (void)sprintf(record, "%cINTL:%d:%d/%d %d:%d/%d",
         0x01,
         to_node_zone, to_node_network, to_node_number,
         from_node_zone, from_node_network, from_node_number);

      plug_msg(record, strlen(record));
      msg_out(13);
      msg_out(10);
   }
}

/* **********************************************************************
   * The month is offered as text. Return it as the month number.       *
   *                                                                    *
   ********************************************************************** */

static char to_month(char *this_one)
{
    if (! strncmp(this_one, "Jan", 3)) return 1;
    if (! strncmp(this_one, "Feb", 3)) return 2;
    if (! strncmp(this_one, "Mar", 3)) return 3;
    if (! strncmp(this_one, "Apr", 3)) return 4;
    if (! strncmp(this_one, "May", 3)) return 5;
    if (! strncmp(this_one, "Jun", 3)) return 6;
    if (! strncmp(this_one, "Jul", 3)) return 7;
    if (! strncmp(this_one, "Aug", 3)) return 8;
    if (! strncmp(this_one, "Sep", 3)) return 9;
    if (! strncmp(this_one, "Oct", 3)) return 10;
    if (! strncmp(this_one, "Nov", 3)) return 11;
    if (! strncmp(this_one, "Dec", 3)) return 12;
    log_error(150);
    return(1);
}

/* **********************************************************************
   * Go through the RA/QBBS linkjed list and free up all of the         *
   * memory that has been allocated for the message text... If any.     *
   *                                                                    *
   ********************************************************************** */

static void abort_ra_mail(void)
{
    te_point = te_first;

    while (te_point != (struct Text_Entry *)NULL) {
        te_next = te_point->next;

        if (te_point->line_length > 1) {
            memory_freed((UL)te_point->line_length + 1);
        }
        else {
            memory_freed(1);
        }

        farfree(te_point->text_line);
        farfree(te_point);
        memory_freed((UL)sizeof(struct Text_Entry));
        te_point = te_next;
    }

    te_first = te_next = te_point = (struct Text_Entry *)NULL;
}

/* **********************************************************************
   * Send the RA/QBBS message!                                          *
   *                                                                    *
   * When done, free up all of the memory that was used.                *
   *                                                                    *
   * If we don't know where the block count is, find out!               *
   *                                                                    *
   ********************************************************************** */

static void send_ra_mail(char deallocate_message_when_done, char use_this_board)
{
    char record[81];
    int result;
    unsigned char char_count, message_block_count;
    char looper;
    unsigned char byte;
    unsigned long hold_bc;

/*
    This is only done when the block count of the messages isn't known.
    It's done only once and at that, only if mail is ever sent.
*/

    if (block_count == (unsigned int)NIL) {
        hold_bc = (unsigned long)filelength(fileno(MSGTXT)) / 256L;
        block_count = (unsigned short)hold_bc;
    }

/*
    We know the starting block number... Now do the rest

    Update the Remote Access File MSGINFO.BBS
*/

    ra_highest = msg_info->highest_message++;
    ra_total = msg_info->total_messages++;
    msg_info->total_on_board[use_this_board - 1]++;
    rewind(MSGINFO);

    result =
        fwrite(msg_info, sizeof(struct Message_Information), 1, MSGINFO);

    if (result != 1) {
        log_error(151);

        c_out(LIGHTRED,
            "Could not send message... Problem unknown: MSGINFO\n\r");

        return;
    }

/*
    Append to the Remote Access file MSGIDX
*/

    msg_index->message_number = msg_info->highest_message;
    msg_index->board_number = use_this_board;
    
    result =
        fwrite(msg_index, sizeof(struct Message_Index), 1, MSGIDX);

    if (result != 1) {
        log_error(151);

        c_out(LIGHTRED,
            "Could not send message... Problem unknown: MSGIDX\n\r");

        return;
    }

/*
    Append to the Remote Access file MSGTOIDX
*/

    msg_to->string_length = (unsigned char)msg_hdr->wtlength;
    (void)strncpy(msg_to->to_record, msg_hdr->who_to, msg_hdr->wtlength);

    result =
        fwrite(msg_to, sizeof(struct Message_To_Index), 1, MSGTOIDX);

    if (result != 1) {
        log_error(151);

        c_out(LIGHTRED,
            "Could not send message... Problem unknown: MSGTOIDX\n\r");

        return;
    }

/*
    Append to the Remote Access file MSGTXT.

    Here we compute the number of 255 byte blocks in the message
    and the information is retained to append information to the
    header file.
*/

    char_count = 0;
    message_block_count = 0;
    te_point = te_first;

    while (te_point) {
        for (looper = 0; looper < te_point->line_length; looper++) {
            byte = te_point->text_line[looper];
            msg_text->text_record[char_count] = byte;
            if (char_count == 254) {
                msg_text->trlength = 255;
                char_count = 0;

                result =
                    fwrite(msg_text, sizeof(struct Message_Text), 1, MSGTXT);

                if (result != 1) {
                    log_error(151);

                    c_out(LIGHTRED,
                        "Could not send message... Problem unknown: MSGTXT\n\r");

                    return;
                }

                message_block_count++;
            }
            else {
                char_count++;
            }
        }

        te_point = te_point->next;
    }

/*
    Find out what the length of the last message block is and store it
    away. Then append the remaining text field with NULLs.

    If the character count is 0, then it could be an empty message or
    it could have ended exactly on the 255'th byte boundry.
*/

    if (char_count != 0) {
        msg_text->trlength = char_count + 1;

        for (; char_count < 255; char_count++)
            msg_text->text_record[char_count] = 0x00;

        result =
            fwrite(msg_text, sizeof(struct Message_Text), 1, MSGTXT);

        if (result != 1) {
            log_error(151);

            c_out(LIGHTRED,
                "Could not send message... Problem unknown: MSGTXT\n\r");

            return;
        }

        message_block_count++;
    }

/*
    Append to the Remote Access file MSGHDR
*/

    msg_hdr->start_block = block_count;
    msg_hdr->number_blocks = message_block_count;

    result =
        fwrite(msg_hdr, sizeof(struct Message_Header), 1, MSGHDR);

    if (result != 1) {
        log_error(151);

        c_out(LIGHTRED,
            "Could not send message... Problem unknown: MSGHDR\n\r");

        return;
    }

/*
    Make sure that we keep track of the block count!
*/

    block_count += msg_hdr->number_blocks;

    if (deallocate_message_when_done)
        abort_ra_mail();
}

/* **********************************************************************
   * Find the highest message number.                                   *
   *                                                                    *
   ********************************************************************** */

static short find_highest_message_number(char *directory)
{
    char result;
    short highest_message_number = 0;
    char directory_search[100];

    (void)strcpy(directory_search, directory);
    (void)strcat(directory_search, "*.msg");

    result = findfirst(directory_search, &file_block, 0x16);

    if (! result) {
        if (atoi(file_block.ff_name) > highest_message_number) {
            highest_message_number = atoi(file_block.ff_name);
        }
    }

    while (! result) {
        result = findnext(&file_block);
        if (! result) {
            if (atoi(file_block.ff_name) > highest_message_number) {
                highest_message_number = atoi(file_block.ff_name);
            }
        }
    }

    return(highest_message_number);
}

/* **********************************************************************
   * Make copies of the message that was just create, plugging the      *
   * destination addresses as needed.                                   *
   *                                                                    *
   ********************************************************************** */

static void copy_mail_to_all_nodes(void)
{
   char arecord[201], brecord[201], hold_record[201];
   short a_message_number;
   char byte_test;

   if (mail_type != 0) { return; }              /* FRED */

   (void)strcpy(arecord, subspace_mail);

   while (! feof(mail_list)) {
      fgets(hold_record, 200, mail_list);
      if (! feof(mail_list)) {

         if (hold_record[0] != ';' && strlen(hold_record) > 3) {
            a_message_number = 1 + find_highest_message_number(arecord);

            (void)sprintf(brecord, "copy %s%d.msg %s%d.msg > mayhem.tmp\r",
                arecord, original_message_number, arecord, a_message_number);

            (void)system(brecord);

            (void)sprintf(brecord, "%s%d.msg", arecord, a_message_number);

            if ((msg_file = mayhem_fopen(brecord, "r+b", msg_file)) == (FILE *)NULL) {
               c_out(WHITE, "\n\rUnable to create your subspace message. Problem unknown.\n\r");
               byte_test = fclose(mail_list);

               if (byte_test != 0) {
                  log_error(127);
               }

               return;
            }

            if (fread(message, sizeof(struct fido_msg), 1, msg_file) != 1) {
               c_out(LIGHTRED, "\n\rUnable to write subspace mail!\n\r");
               byte_test = fclose(mail_list);

               if (byte_test != 0) {
                  log_error(128);
               }

               mayhem_fclose(&msg_file);
               log_error(121);
               return;
            }

            (void)strcpy(record, hold_record);
            process_destination_network_address();
            message->destination_node = to_node_number;
            message->destination_net = to_node_network;
            message->destination_zone = to_node_zone;
            message->destination_point = to_node_point;
            fseek(msg_file, 0, SEEK_SET);

            if (fwrite(message, sizeof(struct fido_msg), 1, msg_file) != 1) {
               c_out(LIGHTRED, "\n\rUnable to write subspace mail!\n\r");
               byte_test = fclose(mail_list);

               if (byte_test != 0) {
                  log_error(129);
               }

               mayhem_fclose(&msg_file);
               log_error(122);
               return;
            }

            mayhem_fclose(&msg_file);
         }
      }
   }

/*
   Erase the original message addressed to NIL
*/

   (void)sprintf(brecord, "del %s%d.msg\r", arecord, original_message_number);
   (void)system(brecord);
   (void)strcpy(brecord, "del mayhem.tmp\r");
   (void)system(brecord);
   byte_test = fclose(mail_list);

   if (byte_test != 0) {
      log_error(130);
   }

   did_mail = TRUE;
}

/* **********************************************************************
   * Plug up the reaining header information.                           *
   *                                                                    *
   *                                                                    *
   * If 'mail_sending' is Network_Mail, then it's a network message,    *
   * otherwise it's an Echo Mail message.                               *
   *                                                                    *
   ********************************************************************** */

static char do_remaining_header(char mail_sending,
   short mail_zone,
   short mail_network,
   short mail_node,
   short mail_point,
   char file_attached,
   char use_this_board)
{
    char *atpoint, *point;
    char hold_buffer[20];

    if (mail_type == 0) {
        message->date[0] = the_date[8];
        message->date[1] = the_date[9];
        message->date[2] = ' ';
        message->date[3] = the_date[4];
        message->date[4] = the_date[5];
        message->date[5] = the_date[6];
        message->date[6] = ' ';
        message->date[7] = the_date[22];
        message->date[8] = the_date[23];
        message->date[9] = ' ';
        message->date[10] = ' ';
        message->date[11] = (char)NULL;
        atpoint = the_date;
        atpoint += 11;
        (void)strncat(message->date, atpoint, 8);
        message->destination_node = mail_node;
        message->originate_node = from_node_number;
        message->cost = 0;
        message->originate_net = from_node_network;
        message->destination_net = mail_network;
        message->destination_zone = mail_zone;
        message->originate_zone = from_node_zone;
        message->destination_point = mail_point;
        message->originate_point = from_node_point;
        message->reply = 0;

        message->attribute = Fido_Kill +
                             Fido_Local;

        if (file_attached) {
            message->attribute += Fido_File_Attach;
        }

        if (stat_hold) {
            message->attribute += Fido_Hold;
        }

        message->upwards_reply = 0;
        message->a_character_1 = 1;
        (void)strcpy(message->area_name, "AREA:MAYHEM");
    }
    else if (mail_type == 1) {
        point = the_date;

        (void)sprintf(hold_buffer, "%02d-%02d-%02d",
           to_month(point + 4), atoi(point + 8), atoi(point + 20));

        (void)strncpy(msg_hdr->post_date, hold_buffer, 8);
        msg_hdr->post_date[8] = (char)NULL;

        point = the_date;

        (void)sprintf(hold_buffer, "%02d:%02d",
            atoi(point + 11), atoi(point + 14));

        (void)strncpy(msg_hdr->post_time, hold_buffer, 5);
        msg_hdr->post_time[5] = (char)NULL;

        msg_hdr->pdlength = 8;
        msg_hdr->ptlength = 5;
        msg_hdr->wtlength = strlen(msg_hdr->who_to);
        msg_hdr->message_number = ra_highest + 1;
        msg_hdr->previous_reply = 0;
        msg_hdr->next_reply = 0;
        msg_hdr->times_read = 0;
        msg_hdr->destination_network = mail_network;
        msg_hdr->destination_node = mail_node;
        msg_hdr->destination_zone = mail_zone;
        msg_hdr->originating_network = from_node_network;
        msg_hdr->originating_node = from_node_number;
        msg_hdr->origination_zone = from_node_zone;
        msg_hdr->cost = 0;
        msg_hdr->board = use_this_board;

        if (mail_sending == Network_Mail) {
            msg_hdr->message_attribute =
                                Outbound_Network_Mail +
                                Is_Network_Mail +
                                RA_Local_Bit;

            msg_hdr->network_attribute = 0;

        }
        else {
            msg_hdr->message_attribute =
                                Outbound_Echo_Mail +
                                RA_Local_Bit;

            msg_hdr->network_attribute = 0;
        }

        if (file_attached) {
            msg_hdr->network_attribute +=
                                RA_File_Attach;
        }
    }
    else {
        log_error(149);
        return(FALSE);
    }

    if (mail_type == 0) {
        if (mayhem_fwrite(message, sizeof(struct fido_msg), 1, msg_file) != 1) {
            c_out(LIGHTRED, "\n\rTROUBLE: Can't write message header information!\n\r");
            c_out(LIGHTRED, "\n\rTROUBLE: Message file pointer: %p\n\r", msg_file);
            c_out(LIGHTRED, "Subspace message was not sent!\n\r");
            mayhem_fclose(&msg_file);
            log_error(120);
            return(FALSE);
        }
    }

    if (mail_type == 0) {
        msg_out(13);
        msg_out(10);
    }

    do_kludge();

    if (mail_type == 1) {
        (void)sprintf(record, "%cAREA:MAYHEM%c", 0x01, 0x0d);
        plug_msg(record, strlen(record));
    }
    else {
        msg_out(13);
        msg_out(10);
    }

    msg_out(1);                 /* Kludge */

    (void)strcpy(record,
        "NOTE:Fredric L. Rice (1:102/901) Subspace Mail");

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);

    return(TRUE);
}

/* **********************************************************************
   * The message name is valid. If the count number is -1 then it's a	*
   * message to all.							*
   *									*
   * If the 'all_or_good_name' is TRUE, then the user entered all or    *
   * a name that was found. If it's TRUE, we address the message to     *
   * either all players or to the specified ship. If it's FALSE, then   *
   * we use the offered name and address it as a remote player.         *
   *                                                                    *
   ********************************************************************** */

void message_to_this(short count, char all_or_good_name, char *use_this)
{
   char *atpoint;
   FILE *read_in;
   char ship_name[5];
   char test_byte;
   char *point;

   if (! all_or_good_name)
      STRNCPY(ship_name, use_this, 4);
         
ask_for_subject:
   c_out(WHITE, "Enter subject: ");
   timed_input(0);

   if (strlen(record) < 2)
      (void)strcpy(record, "Subspace message");

   record[35] = 0;

   if (! is_it_a_good_name(record)) {
      goto ask_for_subject;
   }

    if (mail_type == 0) {
        if (! make_new_message(FALSE)) {
            return;
        }
    }

/*
   Put in the origin name
   Put in the destination name
   Put in the subject
*/

   if (mail_type == 0) {
      (void)strncpy(message->from, hold[user_number]->names, 4);
      message->from[4] = (char)NULL;

      if (count == (short)NIL) {
         (void)strcpy(message->to, "All Universal Mayhem Players");
      }
      else if (all_or_good_name) {
         (void)strcpy(message->to, hold[count]->names);
      }
      else {
         (void)strcpy(message->to, ship_name);
         (void)strcat(message->to, ", remote player");
         c_out(LIGHTRED, "Message to: %s\n\r", message->to);
      }

      (void)strcpy(message->subject, record);
   }
   else if (mail_type == 1) {
      msg_hdr->wflength = 4;
      (void)strncpy(msg_hdr->who_from, hold[user_number]->names, 4);

      if (count == (short)NIL) {
         (void)strcpy(msg_hdr->who_to, "All Universal Mayhem Players");
      }
      else if (all_or_good_name) {
         (void)strcpy(msg_hdr->who_to, hold[count]->names);
      }
      else {
         (void)strcpy(msg_hdr->who_to, ship_name);
         (void)strcat(msg_hdr->who_to, ", remote player");
         c_out(LIGHTRED, "Message to: %s\n\r", msg_hdr->who_to);
      }

      msg_hdr->wtlength = strlen(msg_hdr->who_to);
      (void)strcpy(msg_hdr->subject, record);
      msg_hdr->slength = strlen(record);
   }
   else {
      log_error(149);
      return;
   }

/*
   Put the header data into the proper structures.
*/

   if (! do_remaining_header(Subspace_Mail,
       to_node_zone,
           to_node_network,
               to_node_number,
                   to_node_point,
                       FALSE,
                           echo_board_number)) {
        return;
   }

   c_out(WHITE, "\n\rEnter subspace message. To end, hit enter on a blank line.\n\r");

continue_message_entry:
   entering_mail = TRUE;

   while (strlen(record) > 0) {
      c_out(WHITE, "\n\rMail> ");
      timed_input(0);

      if (! is_it_a_good_name(record)) {
         goto continue_message_entry;
      }

      plug_msg(record, strlen(record));
      msg_out(13);
      msg_out(10);
   }

ask_for_the_option:
   entering_mail = FALSE;
   c_out(WHITE, "\n\r(C)ontinue, (S)ave, (A)bort");

   if (is_redirected == 0) {
      c_out(WHITE, ", (R)ead-In: ");
   }
   else {
      c_out(WHITE, ": ");
   }

   timed_input(0);
   ucase(record);
   record[1] = (char)NULL;

   if (record[0] == 'C') {
      goto continue_message_entry;
   }
   else if (record[0] == 'R' && is_redirected == 0) {
      c_out(LIGHTRED, "Enter file name to read into message: ");
      timed_input(0);

      if (strlen(record) < 1) {
         c_out(LIGHTRED, "Read-in aborted...\n\r");
         goto ask_for_the_option;
      }

      if ((read_in = fopen(record, "rt")) == (FILE *)NULL) {
         c_out(LIGHTRED, "File: %s wasn't found!\n\r", record);
         goto ask_for_the_option;
      }

      while (! feof(read_in)) {
         fgets(record, 200, read_in);

         if (! feof(read_in)) {
            c_out(LIGHTGREEN, "%s\r", record);
            plug_msg(record, strlen(record));
         }
      }

      test_byte = fclose(read_in);

      if (test_byte != 0) {
         log_error(123);
      }

      goto ask_for_the_option;
   }
   else if (record[0] == 'S') {
      (void)sprintf(record, "--- Mayhem %s%c\n", THE_VERSION, 0x0d);
      plug_msg(record, strlen(record));
      (void)strcpy(record, echo_origin);
      ucase(record);

      if (strncmp(record, "NONE", 4)) {

         (void)sprintf(record, " * Origin: %s (%d:%d/%d.%d)%c%c",
             echo_origin,
             from_node_zone,
             from_node_network,
             from_node_number,
             from_node_point,
             0x0d, 0x0a);

         plug_msg(record, strlen(record));
      }

      if (mail_type == 0) {
          msg_out(0);
      }

      if (mail_type == 0) {
         mayhem_fclose(&msg_file);
      }

      if (to_node_number != (short)NIL) {
         if (mail_type != 0) {
            send_ra_mail(TRUE, echo_board_number); /* deallocate message when done */
         }

         c_out(WHITE, "Message sent...\n\r");
         did_mail = TRUE;
         return;
      }

/*
    The to node number was NIL so we must send mail to all of the
    nodes that are listed in the to_file_listing file name offered.
*/

      if ((mail_list = fopen(to_file_listing, "rt")) == (FILE *)NULL) {
         log_error(115);
         return;
      }

      c_out(WHITE, "Message being sent to all points...\n\r");
      copy_mail_to_all_nodes();
      return;
   }
   else if (record[0] == 'A') {
      c_out(WHITE, "\n\r\n\rMessage entry aborted.\n\r\n\r");

      if (mail_type == 0) {
         mayhem_fclose(&msg_file);
         unlink(hold_message_file_name);
      }
      else if (mail_type == 1) {
         abort_ra_mail();
      }
      else {
         log_error(149);
         return;
      }

      return;
   }
   else {
      goto ask_for_the_option;
   }
}

/* **********************************************************************
   * Make a new message file.						*
   *									*
   ********************************************************************** */

unsigned short make_new_message(char statistics_message)
{
   int message_number;
   char arecord[201];
   char brecord[201];

/*
    If for some reason this routine gets called, make sure we are
    running the *.MSG format, otherwise return a FALSE.
*/

   if (mail_type != 0) {
      return(FALSE);
   }

   if (! statistics_message) {
      (void)strcpy(arecord, subspace_mail);
   }
   else {
      (void)strcpy(arecord, network_directory);
   }

   message_number = 1 + find_highest_message_number(arecord);

   original_message_number = message_number;
   (void)sprintf(brecord, "%s%d.msg", arecord, message_number);
   (void)strcpy(hold_message_file_name, brecord);

   if ((msg_file = mayhem_fopen(brecord, "wb", msg_file)) == (FILE *)NULL) {

      c_out(WHITE,
          "\n\rUnable to create your %s message. Problem unknown.\n\r",
          statistics_message ? "STATISTICS" : "subspace");

      return(FALSE);
   }

   return(TRUE);
}

/* **********************************************************************
   * Create a new message and compile some stats.                       *
   *                                                                    *
   * o The network address of your system                               *
   *                                                                    *
   * o How many active ships your system currently has                  *
   *                                                                    *
   * o Information on your systems strongest power player:              *
   *   o The name of the player                                         *
   *   o The strength of the player (in terms of power)                 *
   *   o Date and time the player was last active                       *
   *                                                                    *
   * o Information on your systems highest planet ownership player:     *
   *   o The name of the player                                         *
   *   o The number of planets owned by that player                     *
   *   o Date and time the player was last active                       *
   *                                                                    *
   * o Information on your systems highest killer player:               *
   *   o The name of the player                                         *
   *   o The number of kills that player has                            *
   *   o Date and time the player was last active                       *
   *                                                                    *
   * ------------------------------------------------------------------ *
   *                                                                    *
   * Take the ship that's the strongest ping and append its information *
   * to the statistics message after putting an end of file marker on   *
   * it.                                                                *
   *                                                                    *
   ********************************************************************** */

void create_stat_package(void)
{
    short s_count, l_count;
    unsigned long ship_power_value, test_value;
    short ship_kill_value;
    short ship_standing_value;
    char p_name[5], k_name[5], s_name[5];
    char *atpoint;
    char file_name[20];
    int how_many_command_files;
    FILE *attach_file, *list_file;
    char test_byte;
    unsigned long the_ping;
    char found_one;
    char duplication_name[81];
    FILE *dup_file;
    short use_zone, use_network, use_node, use_point;
    char fa_name[81];
    char original_mail_type;

    original_mail_type = mail_type;

    if (! make_new_message(TRUE)) {
        mail_type = original_mail_type;
        return;
    }

    (void)strcpy(p_name, "NONE");
    (void)strcpy(k_name, "NONE");
    (void)strcpy(s_name, "NONE");

    list_file = (FILE *)NULL;
    found_one = FALSE;

    ship_power_value = ship_kill_value = 0;
    ship_standing_value = 0;

    c_out(WHITE,
        "Universal Mayhem Statistics Coordinator in process: %d.MSG\n\r",
        original_message_number);

    (void)strcpy(message->from, "Mayhem Statistics Coordinator");
    (void)strcpy(message->to, "All Mayhem Sites");

    (void)sprintf(message->subject, "%s%04X%04X.MHN",
        statistics_outbound_directory,
        from_node_network,
        from_node_number);

/*
    See if we have a LIST of addresses to send to...

    If we do, select the first address in the list and use it as a
    template for all the others.
*/

    if (stat_node_zone == (short)NIL) {
        found_one = FALSE;
        use_zone = from_node_zone;
        use_network = from_node_network;
        use_node = from_node_number;
        use_point = from_node_point;

        if ((list_file = fopen(stat_file_name, "rt")) != (FILE *)NULL) {
            while (! feof(list_file) && !found_one) {
                fgets(record, 200, list_file);

                if (! feof(list_file)) {
                    point = record;
                    skipspace(point);

                    if (strlen(point) > 2 && *point != ';') {
                        use_zone = find_delimiter(':');
                        use_network = find_delimiter('/');
                        use_node = find_delimiter('.');
                        use_point = find_delimiter(' ');
                        found_one = TRUE;
                        (void)strcpy(duplication_name, hold_message_file_name);
                    }
                }
            }
        }

        if (! found_one) {
            fclose(list_file);
        }
    }
    else {              /* SHIP.CFG file entry address is used. */
        use_zone = stat_node_zone;
        use_network = stat_node_network;
        use_node = stat_node_number;
        use_point = stat_node_point;
    }

   if (! do_remaining_header(Network_Mail,
       use_zone,
           use_network,
               use_node,
                   use_point,
                       TRUE,
                           echo_board_number)) {  /* File Attach */
        if (found_one) {
            fclose(list_file);
        }

        mail_type = original_mail_type;
        return;
    }

    do_kludge();

    (void)sprintf(record, "Address %d:%d/%d.%d with %d active ships",
        from_node_zone,
        from_node_network,
        from_node_number,
        from_node_point,
        active_players);

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);

    how_many_command_files = 0;

    for (s_count = 1; s_count < players; s_count++) { /* No cops */
        read_enemy(s_count);

        if (enemy->ship_xpos != xsize / 2 && enemy->ship_xpos != ysize / 2) {
            test_value = enemy->ship_power + enemy->ship_shield + enemy->base_shield;

            if (enemy->local == 0) {
                if (command_file_exist(s_count)) {
                    how_many_command_files++;
                }
            }

            if (test_value > ship_power_value) {
                ship_power_value = test_value;
                (void)strcpy(p_name, enemy->ship_name);
            }

            if (enemy->total_kills > ship_kill_value) {
                ship_kill_value = enemy->total_kills;
                (void)strcpy(k_name, enemy->ship_name);
            }

            if (enemy->planets_owned > ship_standing_value) {
                ship_standing_value = enemy->planets_owned;
                (void)strcpy(s_name, enemy->ship_name);
            }
        }
    }

    the_ping = ship_power_value / 1000000l;

    (void)sprintf(record,
        "   [%s] with %ld 'ping' strength, [%s] with %d kills, [%s] with %d planets",
        p_name, the_ping,
        k_name, ship_kill_value,
        s_name, ship_standing_value);

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);

    (void)sprintf(record, "--- Mayhem %s%c\n", THE_VERSION, 0x0d);
    plug_msg(record, strlen(record));

    (void)strcpy(record, network_origin);
    ucase(record);

    if (strncmp(record, "NONE", 4)) {

       (void)sprintf(record, " * Origin: %s (%d:%d/%d.%d)%c%c",
          network_origin,
          from_node_zone,
          from_node_network,
          from_node_number,
          from_node_point,
          0x0d, 0x0a);

       plug_msg(record, strlen(record));
    }

    msg_out(0);                  /* End of message marker        */
    msg_out(26);                 /* End of file marker           */

    if (how_many_command_files == 0) {
        c_out(LIGHTGREEN, "\n\rThere are NO ships to export. None were found\n\r");
        c_out(LIGHTGREEN, "that has a *.SHP file to automate it!\n\r");

        message->attribute -= Fido_File_Attach;
        (void)strcpy(message->subject, "-> No exports <-");
        rewind(msg_file);

        if (mayhem_fwrite(message, sizeof(struct fido_msg), 1, msg_file) != 1) {
           log_error(141);
        }

        mayhem_fclose(&msg_file);
        did_mail = TRUE;

        if (found_one) {
            fclose(list_file);
        }

        mail_type = original_mail_type;
        return;
    }

    mayhem_fclose(&msg_file);

    did_mail = TRUE;

    (void)strcpy(fa_name, message->subject);

    if ((attach_file = fopen(fa_name, "wb")) == (FILE *)NULL) {
        c_out(LIGHTRED, "Unable to create attach file: %s\n\r", fa_name);
        log_error(124);

        if (found_one) {
            fclose(list_file);
        }

        mail_type = original_mail_type;
        return;
    }

    test_byte = fputc(how_many_command_files, attach_file);

    if (test_byte != how_many_command_files) {
        c_out(LIGHTRED, "Unable to continue to write attach file data!\n\r");
        log_error(118);

        if (found_one) {
            fclose(list_file);
        }

        mail_type = original_mail_type;
        return;
    }

    for (s_count = 1; s_count < players; s_count++) {
        read_enemy(s_count);

        if (enemy->ship_xpos != xsize / 2 && enemy->ship_xpos != ysize / 2) {
            (void)strcpy(p_name, enemy->ship_name);

            if (command_file_exist(s_count)) {
                if (enemy->local == 0) {
                    (void)strcpy(enemy->ship_pass, "{{{{"); /* Destroy password! */
                    enemy->local = 1;

                    if (mayhem_fwrite(enemy, sizeof(struct ship_file), 1, attach_file) != 1) {
                        c_out(LIGHTRED, "\n\rTROUBLE: write to attach file '%s' failed!\n\r", message->subject);
                        c_out(LIGHTRED, "\n\rTROUBLE: Attach file pointer: %p\n\r", attach_file);
                        c_out(LIGHTRED, "Export did not continue!\n\r");
                        mayhem_fclose(&attach_file);
                        log_error(125);

                        if (found_one) {
                            fclose(list_file);
                        }

                        mail_type = original_mail_type;
                        return;
                    }

                    (void)strncpy(file_name, hold[s_count]->names, 4);
                    file_name[4] = (char)NULL;
                    (void)strcat(file_name, ".SHP");

                    if ((command_file = mayhem_fopen(file_name, "rt", command_file)) != (FILE *)NULL) {
                        while (! feof(command_file)) {
                            fgets(record, 200, command_file);

                            if (! feof(command_file)) {
                                test_byte = fputs(record, attach_file);

                                if (test_byte == (char)EOF) {
                                   log_error(122);
                                }
                            }
                        }

                        mayhem_fclose(&command_file);
                    }
                    else {
                        c_out(LIGHTRED, "\n\rTROUBLE: couldn't find command file '%s'\n\r", file_name);
                        log_error(126);

                        if (found_one) {
                            fclose(list_file);
                        }

                        mail_type = original_mail_type;
                        return;
                    }           /* End of user information */
                    test_byte = fputc(123, attach_file);

                    if (test_byte != 123) {
                        c_out(LIGHTRED, "Unable to continue to write attach file data!\n\r");
                        log_error(119);

                        if (found_one) {
                            fclose(list_file);
                        }

                        mail_type = original_mail_type;
                        return;
                    }
                }
            }
        }
    }

    test_byte = fclose(attach_file);

    if (test_byte != 0) {
        log_error(131);
    }

    c_out(LIGHTGREEN, "---> There were %d exports in file: %s\n\r",
        how_many_command_files, fa_name);

/*
    If there was a list of network addresses to send to, extract the
    remaining information and make copies!
*/

    if (! found_one) {
        mail_type = original_mail_type;
        return;
    }

    if (list_file == (FILE *)NULL) {
        c_out(LIGHTRED, "Trouble: List File pointer is NULL!\n\r");
        mail_type = original_mail_type;
        return;
    }

    if ((dup_file = fopen(duplication_name, "rb")) == (FILE *)NULL) {
        mail_type = original_mail_type;
        return;
    }

    while (! feof(list_file)) {
        fgets(record, 200, list_file);

        if (! feof(list_file)) {
            point = record;
            skipspace(point);

            if (strlen(point) > 2 && *point != ';') {
                stat_node_zone = find_delimiter(':');
                stat_node_network = find_delimiter('/');
                stat_node_number = find_delimiter('.');
                stat_node_point = find_delimiter(' ');

                if (! make_new_message(TRUE)) {
                    fclose(list_file);
                    fclose(dup_file);
                    mail_type = original_mail_type;
                    return;
                }

                if (fread(message, sizeof(struct fido_msg), 1, dup_file) != 1) {
                    fclose(list_file);
                    fclose(dup_file);
                    mail_type = original_mail_type;
                    return;
                }

                if (! do_remaining_header(Network_Mail,
                    stat_node_zone,
                        stat_node_network,
                            stat_node_number,
                                stat_node_point,
                                    TRUE,
                                        echo_board_number)) { /* File Attach */

                    fclose(list_file);
                    fclose(dup_file);
                    mail_type = original_mail_type;
                    return;
                }

                while (! feof(dup_file)) {
                    fgets(record, 200, dup_file);
                    if (! feof(dup_file)) {
                        fputs(record, msg_file);
                    }
                }

                c_out(LIGHTGREEN, "Copies to %d:%d/%d.%d\n\r",
                    message->destination_zone,
                    message->destination_net,
                    message->destination_node,
                    message->destination_point);

                mayhem_fclose(&msg_file);
                rewind(dup_file);
            }
        }
    }

    fclose(dup_file);
    fclose(list_file);
    mail_type = original_mail_type;
}

/* **********************************************************************
   * See if a statistics package should be sent to me.                  *
   *                                                                    *
   * If not, simply return.                                             *
   *                                                                    *
   * If so, compose a statistics message and address it to the net and  *
   * node that's described in the configuration file.                   *
   *                                                                    *
   ********************************************************************** */

void mail_check_statistics_package(void)
{
   FILE *last_friday;
   char byte_test;

   if ((last_friday = fopen("ECHOSTAT.DAT", "rt")) == (FILE *)NULL) {
      if ((last_friday = fopen("ECHOSTAT.DAT", "wt")) == (FILE *)NULL) {
         return;
      }

      byte_test = fputs(the_date, last_friday);

      if (byte_test == (char)EOF) {
         log_error(120);
      }

      byte_test = fclose(last_friday);

      if (byte_test != 0) {
         log_error(124);
      }

      create_stat_package();
      return;
   }

   fgets(record, 200, last_friday);
   byte_test = fclose(last_friday);

   if (byte_test != 0) {
      log_error(125);
   }

   if (! strncmp(record, the_date, 1)) {
      return;
   }

   if ((last_friday = fopen("ECHOSTAT.DAT", "wt")) != (FILE *)NULL) {
      byte_test = fputs(the_date, last_friday);

      if (byte_test == (char)EOF) {
         log_error(121);
      }

      byte_test = fclose(last_friday);

      if (byte_test != 0) {
         log_error(126);
      }

      create_stat_package();
   }
}

/* **********************************************************************
   * Here we will invoke the stand-alone utility that will scan the     *
   * STATS.SHP file for the remote players.                             *
   *                                                                    *
   ********************************************************************** */

void import_statistics_file(void)
{
    if (statistics_enabled) {
       perform_quit(999);

       get_ready_to_leave();

       execl("UMREMOTE.EXE",
          "UMREMOTE",
          "Mayhem",
          want_color ? "Y" : "N",
          color_enable ? "Y" : "N",
          NULL);
    }
}

/* **********************************************************************
   * A ship or base has been killed.                                    *
   *                                                                    *
   * Create a message package for it.                                   *
   *                                                                    *
   * Killer -           Ship name that did the killing                  *
   *                                                                    *
   * Killed -           Ship or base name that got killed               *
   *                                                                    *
   * Killed_A_Ship -    TRUE if it was a ship, FALSE if it was a base   *
   *                                                                    *
   * How -              0 - Slaver Death triggered                      *
   *                    1 - Torpedo fire                                *
   *                    2 - Phaser fire                                 *
   *                    3 - Command file directive                      *
   *                                                                    *
   ********************************************************************** */

void inform_kill(char *killer, char *killed, char killed_a_ship, char how)
{
    char *atpoint;

    if (! mail_actives) return;

    if (mail_type == 0) {
        if (! make_new_message(FALSE)) {
            return;
        }
    }

    if (mail_type == 0) {
        (void)strcpy(message->from, "Mayhem Killer Information");
        (void)strcpy(message->to, "All Mayhem Sites");
        (void)strcpy(message->subject, "Death of a player!");
    }
    else if (mail_type == 1) {
        (void)strcpy(msg_hdr->who_from, "Mayhem Killer Information");
        (void)strcpy(msg_hdr->who_to, "All Mayhem Sites");
        (void)strcpy(msg_hdr->subject, "Death of a player!");
        msg_hdr->wflength = strlen(msg_hdr->who_from);
        msg_hdr->wtlength = strlen(msg_hdr->who_to);
        msg_hdr->slength = strlen(msg_hdr->subject);
    }
    else {
        log_error(149);
        return;
    }

   if (! do_remaining_header(Echo_Mail,
       to_node_zone,
           to_node_network,
               to_node_number,
                   to_node_point,
                       FALSE,
                           echo_board_number)) {
        return;
    }

    (void)sprintf(record,
        "Ship [%s] destroyed %s [%s] this day on system %d:%d/%d.%d",
        killer,
        killed,
        killed_a_ship ? "ship" : "base",
        from_node_zone,
        from_node_network,
        from_node_number,
        from_node_point);

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);

    (void)strcpy(record, "Destroyed by ");

    switch(how) {
        case 0: (void)strcat(record, "slaver death weapon!"); break;
        case 1: (void)strcat(record, "torpedo fire!"); break;
        case 2: (void)strcat(record, "phaser fire!"); break;
        case 3: (void)strcat(record, "an automated ship!"); break;
    }

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);

    (void)sprintf(record, "--- Mayhem %s%c\n", THE_VERSION, 0x0d);
    plug_msg(record, strlen(record));

    (void)strcpy(record, echo_origin);
    ucase(record);

    if (strncmp(record, "NONE", 4)) {

       (void)sprintf(record, " * Origin: %s (%d:%d/%d.%d)%c%c",
          echo_origin,
          from_node_zone,
          from_node_network,
          from_node_number,
          from_node_point,
          0x0d, 0x0a);

       plug_msg(record, strlen(record));
    }

    if (mail_type == 0) {
        msg_out(0);                  /* End of message marker        */
        msg_out(26);                 /* End of file marker           */
    }

    if (mail_type == 0) {
        mayhem_fclose(&msg_file);
    }
    else if (mail_type == 1) {
        send_ra_mail(TRUE, echo_board_number); /* deallocate message when done */
    }
    else {
        log_error(149);
        return;
    }

    did_mail = TRUE;
}

/* **********************************************************************
   * The current ship shoved a player around!                           *
   *                                                                    *
   ********************************************************************** */

void mail_leash(short enemy_number,
    long fromx,
    long fromy,
    long tox,
    long toy,
    char how)
{
    char *atpoint;

    if (! mail_actives) return;
    if (enemy_number == user_number) return;
    if (shoved_flag) return;

    if (mail_type == 0) {
        if (! make_new_message(FALSE)) {
            return;
        }
    }

    if (mail_type == 0) {
        (void)strcpy(message->from, "Mayhem Leash Information");
        (void)strcpy(message->to, "All Mayhem Sites");

        (void)sprintf(message->subject, "A Player has been %s around!",
            how ? "Dragged" : "Shoved");
    }
    else if (mail_type == 1) {
        (void)strcpy(msg_hdr->who_from, "Mayhem Leash Information");
        (void)strcpy(msg_hdr->who_to, "All Mayhem Sites");

        (void)sprintf(msg_hdr->subject, "A Player has been %s around!",
            how ? "Dragged" : "Shoved");

        msg_hdr->wtlength = strlen(msg_hdr->who_to);
        msg_hdr->wflength = strlen(msg_hdr->who_from);
        msg_hdr->slength = strlen(msg_hdr->subject);
    }
    else {
        log_error(149);
        return;
    }

   if (! do_remaining_header(Echo_Mail,
       to_node_zone,
           to_node_network,
               to_node_number,
                   to_node_point,
                       FALSE,
                           echo_board_number)) {
        return;
    }

    (void)sprintf(record,
        "Ship [%s] Leashed %s on system %d:%d/%d.%d from {%ld-%ld} to {%ld-%ld}",
        hold[user_number]->names,
        hold[enemy_number]->names,
        from_node_zone,
        from_node_network,
        from_node_number,
        from_node_point,
        fromx, fromy,
        tox, toy);

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);
    msg_out(13);
    msg_out(10);

    (void)sprintf(record, "--- Mayhem %s%c\n", THE_VERSION, 0x0d);
    plug_msg(record, strlen(record));

    (void)strcpy(record, echo_origin);
    ucase(record);

    if (strncmp(record, "NONE", 4)) {

       (void)sprintf(record, " * Origin: %s (%d:%d/%d.%d)%c%c",
          echo_origin,
          from_node_zone,
          from_node_network,
          from_node_number,
          from_node_point,
          0x0d, 0x0a);

       plug_msg(record, strlen(record));
    }

    if (mail_type == 0) {
        msg_out(0);                  /* End of message marker        */
        msg_out(26);                 /* End of file marker           */
    }

    if (mail_type == 0) {
        mayhem_fclose(&msg_file);
    }
    else if (mail_type == 1) {
        send_ra_mail(TRUE, echo_board_number); /* deallocate message when done */
    }
    else {
        log_error(149);
        return;
    }

    did_mail = TRUE;
}

/* **********************************************************************
   * The current ship boarded a ship or a base.                         *
   *                                                                    *
   ********************************************************************** */

void mail_board(short enemy_number,
   char boarded_ship,
   UL took_power,
   UL took_credits,
   UL took_cargo,
   UL took_torps,
   char friendly)
{
    char *atpoint;

    if (! mail_actives) return;
    if (enemy_number == user_number) return;

    if (mail_type == 0) {
        if (! make_new_message(FALSE)) {
            return;
        }
    }

    if (mail_type == 0) {
        (void)strcpy(message->from, "Mayhem Board Information");
        (void)strcpy(message->to, "All Mayhem Sites");

        if (! friendly) {
            (void)sprintf(message->subject, "A Players %s has been boarded!",
                boarded_ship ? "SHIP" : "BASE");
        }
        else {
            (void)strcpy(message->subject, "Player resupplied at friendly base!");
        }
    }
    else if (mail_type == 1) {
        (void)strcpy(msg_hdr->who_from, "Mayhem Board Information");
        (void)strcpy(msg_hdr->who_to, "All Mayhem Sites");

        if (! friendly) {
            (void)sprintf(msg_hdr->subject, "A Players %s has been boarded!",
                boarded_ship ? "SHIP" : "BASE");
        }
        else {
            (void)strcpy(msg_hdr->subject, "Player resupplied at friendly base!");
        }

        msg_hdr->wtlength = strlen(msg_hdr->who_to);
        msg_hdr->wflength = strlen(msg_hdr->who_from);
        msg_hdr->slength = strlen(msg_hdr->subject);
    }
    else {
        log_error(149);
        return;
    }

   if (! do_remaining_header(Echo_Mail,
       to_node_zone,
           to_node_network,
               to_node_number,
                   to_node_point,
                       FALSE,
                           echo_board_number)) {
        return;
    }

    if (! friendly) {
        (void)sprintf(record,
            "Ship [%s] Boarded [%s] on system %d:%d/%d.%d!",
            hold[user_number]->names,
            hold[enemy_number]->names,
            from_node_zone,
            from_node_network,
            from_node_number,
            from_node_point);
    }
    else {
        (void)sprintf(record,
            "Ship [%s] Resupplied from base [%s] on system %d:%d/%d.%d!",
            hold[user_number]->names,
            hold[enemy_number]->names,
            from_node_zone,
            from_node_network,
            from_node_number,
            from_node_point);
    }

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);

    if (! friendly) {
        if (boarded_ship) {
            (void)sprintf(record, "Spoils: Power %ld", took_power);
            plug_msg(record, strlen(record));
            msg_out(13);
            msg_out(10);
            (void)sprintf(record, "        Credits: %ld", took_credits);
            plug_msg(record, strlen(record));
            msg_out(13);
            msg_out(10);
            (void)sprintf(record, "        Cargo: %ld", took_cargo);
            plug_msg(record, strlen(record));
            msg_out(13);
            msg_out(10);
            (void)sprintf(record, "        Torpedoes: %ld", took_torps);
            plug_msg(record, strlen(record));
            msg_out(13);
            msg_out(10);
        }
        else {
            (void)sprintf(record, "Spoils: Credits %ld", took_credits);
            plug_msg(record, strlen(record));
            msg_out(13);
            msg_out(10);
            (void)sprintf(record, "        Cargo: %ld", took_cargo);
            plug_msg(record, strlen(record));
            msg_out(13);
            msg_out(10);
            msg_out(13);
            msg_out(10);
            (void)sprintf(record, "All enemy base crew members were killed in the action!");
            plug_msg(record, strlen(record));
            msg_out(13);
            msg_out(10);
        }
    }

    msg_out(13);
    msg_out(10);

    (void)sprintf(record, "--- Mayhem %s%c\n", THE_VERSION, 0x0d);
    plug_msg(record, strlen(record));

    (void)strcpy(record, echo_origin);
    ucase(record);

    if (strncmp(record, "NONE", 4)) {

       (void)sprintf(record, " * Origin: %s (%d:%d/%d.%d)%c%c",
          echo_origin,
          from_node_zone,
          from_node_network,
          from_node_number,
          from_node_point,
          0x0d, 0x0a);

       plug_msg(record, strlen(record));
    }

    if (mail_type == 0) {
        msg_out(0);                  /* End of message marker        */
        msg_out(26);                 /* End of file marker           */
    }

    if (mail_type == 0) {
        mayhem_fclose(&msg_file);
    }
    else if (mail_type == 1) {
        send_ra_mail(TRUE, echo_board_number); /* deallocate message when done */
    }
    else {
        log_error(149);
        return;
    }

    did_mail = TRUE;
}

/* **********************************************************************
   * The current player picked up a slaver piece! Tell everyone!        *
   *                                                                    *
   ********************************************************************** */

void mail_slaver(char piece)
{
    char *atpoint;

    if (! mail_actives) return;

    if (mail_type == 0) {
        if (! make_new_message(FALSE)) {
            return;
        }
    }

    if (mail_type == 0) {
        (void)strcpy(message->from, "Mayhem Slaver Information");
        (void)strcpy(message->to, "All Mayhem Sites");
        (void)strcpy(message->subject, "A Player has found a Slaver part!");
    }
    else if (mail_type == 1) {
        (void)strcpy(msg_hdr->who_from, "Mayhem Slaver Information");
        (void)strcpy(msg_hdr->who_to, "All Mayhem Sites");
        (void)strcpy(msg_hdr->subject, "A Player has found a Slaver part!");
        msg_hdr->wtlength = strlen(msg_hdr->who_to);
        msg_hdr->wflength = strlen(msg_hdr->who_from);
        msg_hdr->slength = strlen(msg_hdr->subject);
    }
    else {
        log_error(149);
        return;
    }

   if (! do_remaining_header(Echo_Mail,
       to_node_zone,
           to_node_network,
               to_node_number,
                   to_node_point,
                       FALSE,
                           echo_board_number)) {
        return;
    }

    (void)sprintf(record,
        "Ship [%s] on system %d:%d/%d.%d found part %d: %s!",
        hold[user_number]->names,
        from_node_zone,
        from_node_network,
        from_node_number,
        from_node_point,
        piece,
        goal_item_description[piece]);

    plug_msg(record, strlen(record));
    msg_out(13);
    msg_out(10);
    msg_out(13);
    msg_out(10);

    (void)sprintf(record, "--- Mayhem %s%c\n", THE_VERSION, 0x0d);
    plug_msg(record, strlen(record));

    (void)strcpy(record, echo_origin);
    ucase(record);

    if (strncmp(record, "NONE", 4)) {

       (void)sprintf(record, " * Origin: %s (%d:%d/%d.%d)%c%c",
          echo_origin,
          from_node_zone,
          from_node_network,
          from_node_number,
          from_node_point,
          0x0d, 0x0a);

       plug_msg(record, strlen(record));
    }

    if (mail_type == 0) {
        msg_out(0);                  /* End of message marker        */
        msg_out(26);                 /* End of file marker           */
    }

    if (mail_type == 0) {
        mayhem_fclose(&msg_file);
    }
    else if (mail_type == 1) {
        send_ra_mail(TRUE, echo_board_number); /* deallocate message when done */
    }
    else {
        log_error(149);
        return;
    }

    did_mail = TRUE;
}


