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
#ifndef _BOARD_H_
#define _BOARD_H_

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "game.h"


extern gboolean board_suspended ;
extern GtkWidget *board_area;
extern GtkWidget *board_rowbox, *board_colbox;
extern gboolean state_board_flipped;


void board_set_game_params ();
void board_set_cell (int, int, byte);
void board_refresh_cell (int, int);
void board_show();
void board_hide();
void board_init ();
void board_free ();
gboolean board_redraw (GtkWidget *, GdkEventExpose *);
void board_redraw_all ();
gint board_signal_handler (GtkWidget *, GdkEventButton *, gpointer);
void board_apply_refresh (byte *, int *);

#endif
