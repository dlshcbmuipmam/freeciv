/**********************************************************************
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef GENERATING_MAC  /* mac header(s) */
#include <Dialogs.h>
#include <Controls.h>
#include <bool.h> /*from STL, but works w/ c*/
#endif

#include "fcintl.h"
#include "log.h"
#include "shared.h"
#include "support.h"
#include "version.h"

#include "console.h"
#include "meta.h"
#include "sernet.h"
#include "srv_main.h"

#include "civserver.h"


#ifdef GENERATING_MAC
static void Mac_options(int *argc, char *argv[]);
#endif


/**************************************************************************
 Entry point for Freeciv server.  Basically, does two things:
  1. Parsees command-line arguments.
  2. Calls the main server-loop routine.
**************************************************************************/
int main(int argc, char *argv[])
{
  int inx;
  int showhelp = 0;
  int showvers = 0;
  char *option = NULL;

  /* initialize server */
  srv_init();

  /* disallow running as root -- too dangerous */
  dont_run_as_root(argv[0], "freeciv_server");

  /* parse command-line arguments... */

#ifdef GENERATING_MAC
  Mac_options(&argc, argv);
#endif
  /* no  we don't use GNU's getopt or even the "standard" getopt */
  /* yes we do have reasons ;)                                   */
  inx = 1;
  while (inx < argc) {
    if ((option = get_option("--file", argv, &inx, argc)) != NULL)
      srvarg.load_filename = option;
    else if (is_option("--help", argv[inx])) {
      showhelp = 1;
      break;
    } else if ((option = get_option("--log", argv, &inx, argc)) != NULL)
      srvarg.log_filename = option;
    else if ((option = get_option("--gamelog", argv, &inx, argc)) != NULL)
      srvarg.gamelog_filename = option;
    else if (is_option("--nometa", argv[inx])) {
      fprintf(stderr, _("Warning: the %s option is obsolete.  "
			"Use -m to enable the metaserver.\n"), argv[inx]);
      showhelp = 1;
    } else if (is_option("--meta", argv[inx]))
      srvarg.metaserver_no_send = 0;
    else if ((option = get_option("--Metaserver", argv, &inx, argc)) != NULL) {
      sz_strlcpy(srvarg.metaserver_addr, argv[inx]);
      meta_addr_split();
      srvarg.metaserver_no_send = 0;	/* --Metaserver implies --meta */
    } else if ((option = get_option("--port", argv, &inx, argc)) != NULL)
      srvarg.port = atoi(option);
    else if ((option = get_option("--read", argv, &inx, argc)) != NULL)
      srvarg.script_filename = option;
    else if ((option = get_option("--server", argv, &inx, argc)) != NULL)
      sz_strlcpy(srvarg.metaserver_servername, option);
    else if ((option = get_option("--debug", argv, &inx, argc)) != NULL) {
      srvarg.loglevel = log_parse_level_str(option);
      if (srvarg.loglevel == -1) {
	srvarg.loglevel = LOG_NORMAL;
	showhelp = 1;
	break;
      }
    } else if (is_option("--version", argv[inx]))
      showvers = 1;
    else {
      fprintf(stderr, _("Error: unknown option '%s'\n"), argv[inx]);
      showhelp = 1;
      break;
    }
    inx++;
  }

  if (showvers && !showhelp) {
    fprintf(stderr, FREECIV_NAME_VERSION "\n");
    exit(0);
  }
  con_write(C_VERSION, _("This is the server for %s"), FREECIV_NAME_VERSION);
  con_write(C_COMMENT, _("You can learn a lot about Freeciv at %s"),
	    WEBSITE_URL);

  if (showhelp) {
    fprintf(stderr, _("Usage: %s [option ...]\nValid options are:\n"), argv[0]);
    fprintf(stderr, _("  -h, --help\t\tPrint a summary of the options\n"));
    fprintf(stderr, _("  -r, --read FILE\tRead startup script FILE\n"));
    fprintf(stderr, _("  -f, --file FILE\tLoad saved game FILE\n"));
    fprintf(stderr, _("  -p, --port PORT\tListen for clients on port PORT\n"));
    fprintf(stderr, _("  -g, --gamelog FILE\tUse FILE as game logfile\n"));
    fprintf(stderr, _("  -l, --log FILE\tUse FILE as logfile\n"));
    fprintf(stderr, _("  -m, --meta\t\tSend info to metaserver\n"));
    fprintf(stderr, _("  -M, --Metaserver ADDR\tSet ADDR as metaserver address\n"));
    fprintf(stderr, _("  -s, --server HOST\tList this server as host HOST\n"));
#ifdef DEBUG
    fprintf(stderr, _("  -d, --debug NUM\tSet debug log level (0 to 4,"
		      " or 4:file1,min,max:...)\n"));
#else
    fprintf(stderr, _("  -d, --debug NUM\tSet debug log level (0 to 3)\n"));
#endif
    fprintf(stderr, _("  -v, --version\t\tPrint the version number\n"));
    fprintf(stderr, _("Report bugs to <%s>.\n"), BUG_EMAIL_ADDRESS);
    exit(0);
  }

  /* have arguments, call the main server loop... */
  srv_main();

  /* suppress warnings */
  logdebug_suppress_warning;

  /* done */
  return 0;
}

#ifdef GENERATING_MAC
/*************************************************************************
  generate an option dialog if no options have been passed in
*************************************************************************/
static void Mac_options(int *argc, char *argv[])
{
  if (argc == 0) /* always true curently, but... */
  {
    OSErr err;
    DialogPtr  optptr;
    short the_type;
    Handle the_handle;
    Rect the_rect;
    bool done, meta;
    char * str='\0';
    Str255 the_string;
    optptr=GetNewDialog(128,nil,(WindowPtr)-1);
    err=SetDialogDefaultItem(optptr, 2);
    if (err != 0)
    {
      exit(1); /*we don't have an error log yet so I just bomb.  Is this correct?*/
    }
    err=SetDialogTracksCursor(optptr, true);
    if (err != 0)
    {
      exit(1);
    }
    GetDItem(optptr, 16/*normal radio button*/, &the_type, &the_handle, &the_rect);
    SetCtlValue((ControlHandle)the_handle, true);

    while(!done)
    {
      short the_item, old_item;
      ModalDialog(0, &the_item);
      switch (the_item)
      {
        case 1:
          done = true;
        break;
        case 2:
          exit(0);
        break;
        break;
        case 13:
          GetDItem(optptr, 13, &the_type, &the_handle, &the_rect);
          meta=!GetCtlValue((ControlHandle)the_handle);
          SetCtlValue((ControlHandle)the_handle, meta);
        break;
        case 15:
        case 16:
        case 17:
          GetDItem(optptr, old_item, &the_type, &the_handle, &the_rect);
          SetCtlValue((ControlHandle)the_handle, false);
          old_item=the_item;
          GetDItem(optptr, the_item, &the_type, &the_handle, &the_rect);
          SetCtlValue((ControlHandle)the_handle, true);
        break;
      }
    }
    /*now, bundle the data into the argc/argv storage for interpritation*/
    GetDItem( optptr, 4, &the_type, &the_handle, &the_rect);
    GetIText( the_handle, (unsigned char *)str);
    if (str)
    {
      strcpy(argv[++(*argc)],"--file");
      strcpy(argv[++(*argc)], str);
    }
    GetDItem( optptr, 6, &the_type, &the_handle, &the_rect);
    GetIText( the_handle, (unsigned char *)str);
    if (str)
    {
      strcpy(argv[++(*argc)],"--gamelog");
      strcpy(argv[++(*argc)], str);
    }
    GetDItem( optptr, 8, &the_type, &the_handle, &the_rect);
    GetIText( the_handle, (unsigned char *)str);
    if (str)
    {
      strcpy(argv[++(*argc)],"--log");
      strcpy(argv[++(*argc)], str);
    }
    if (meta)
      strcpy(argv[++(*argc)], "--meta");
    GetDItem( optptr, 12, &the_type, &the_handle, &the_rect);
    GetIText( the_handle, the_string);
    if (atoi((char*)the_string)>0)
    {
      strcpy(argv[++(*argc)],"--port");
      strcpy(argv[++(*argc)], (char *)the_string);
    }
    GetDItem( optptr, 10, &the_type, &the_handle, &the_rect);
    GetIText( the_handle, (unsigned char *)str);
    if (str)
    {
      strcpy(argv[++(*argc)],"--read");
      strcpy(argv[++(*argc)], str);
    }
    GetDItem(optptr, 15, &the_type, &the_handle, &the_rect);
    if(GetControlValue((ControlHandle)the_handle))
    {
      strcpy(argv[++(*argc)],"--debug");
      strcpy(argv[++(*argc)],"0");
    }
    GetDItem(optptr, 16, &the_type, &the_handle, &the_rect);
    if(GetControlValue((ControlHandle)the_handle))
    {
      strcpy(argv[++(*argc)],"--debug");
      strcpy(argv[++(*argc)],"1");
    }
    GetDItem(optptr, 17, &the_type, &the_handle, &the_rect);
    if(GetControlValue((ControlHandle)the_handle))
    {
      strcpy(argv[++(*argc)],"--debug");
      strcpy(argv[++(*argc)],"2");
    }
    DisposeDialog(optptr);/*get rid of the dialog after sorting out the options*/
  }
}
#endif
