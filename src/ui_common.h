/*  This file is a part of gtkboard, a board games system.
    Copyright (C) 2003, Arvind Narayanan <arvindn@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

*/
#ifndef _UI_COMMON_H_
#define _UI_COMMON_H_

/** \file ui_common.h
  \brief function declarations common to all ui modules
  */

#include "config.h"

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

//! The main application window
extern GtkWidget *main_window;

//! Show a message to the user
void sb_message (char *msg, gboolean serious);

//! Inform user of an error
void sb_error (char *msg, gboolean serious);

//! Converts a time in milliseconds into human-parsable format
gchar *sb_ftime(int temps);
	
//! Shows a dialog with given title and message
void menu_show_dialog (gchar *title, gchar *message);
#endif
