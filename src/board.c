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
/** \file board.c
  \brief Functions to manipulate the drawing_area where the board is drawn */

#include <stdlib.h>
#include <assert.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "board.h"
#include "ui_common.h"
#include "ui.h"
#include "menu.h"

//! for showing names of rows and columns
static GtkWidget *board_rowbox_real = NULL, *board_colbox_real = NULL;

//! Colors for the light squares, dark squares and the lines
GdkColor gdk_colors[3];

//! gcs for light squares, dark squares and lines
GdkGC *board_gcs[3] = {NULL, NULL, NULL};

//! Images representing the pieces
static GdkPixmap **pieces = NULL;

//! This is TRUE when the game is paused
gboolean board_suspended = FALSE;

//! Is the board flipped (rotated 180 deg)
gboolean state_board_flipped = FALSE;

static int cell_size, num_pieces;

void ui_make_human_move (byte *move);

void board_set_game_params ()
{
	cell_size = opt_game->cell_size;
	num_pieces = opt_game->num_pieces;
}

void board_set_cell (int x, int y, byte val)
{
	cur_pos.board[y * board_wid + x] = val;
}

//! Draws the square (x, y). On the board it is shown at (real_x, real_y)
void board_refresh_cell_real (int x, int y, int real_x, int real_y)
{
	int parity = (board_wid * board_heit + x + y + 1) % 2;
	if (opt_quiet) return;
	if (cur_pos.board[y * board_wid + x] != 0 && !board_suspended)
	{
		gdk_draw_pixmap (board_area->window, 
				board_area->style->bg_gc[GTK_STATE_NORMAL],
				(GdkDrawable *) pieces
				[cur_pos.board[y * board_wid + x] -1 + num_pieces * parity],
				0, 0, real_x * cell_size, real_y * cell_size, 
				cell_size, cell_size);
	}
	else
	{
		gdk_draw_rectangle (board_area->window, board_gcs[parity], 1, 
				real_x * cell_size, real_y * cell_size,
				cell_size, cell_size);
	}
	if (game_draw_cell_boundaries)
	{
		if (real_x > 0)
			gdk_draw_line (board_area->window, board_gcs[2],
				real_x * cell_size, real_y * cell_size,
				real_x * cell_size, (real_y + 1) * cell_size);
		if (real_y > 0)
			gdk_draw_line (board_area->window, board_gcs[2],
				real_x * cell_size, real_y * cell_size,
				(real_x + 1) * cell_size, real_y * cell_size);
	}
}

//! A wrapper around board_refresh_cell_real() to take care of whether the board is flipped
void board_refresh_cell (int x, int y)
{
	int real_x = (state_board_flipped ? board_wid - 1 - x : x);
	int real_y = (state_board_flipped ? y : board_heit - 1 - y);
	board_refresh_cell_real (x, y, real_x, real_y);
}



//! Redraws the exposed area of the board
gboolean board_redraw (GtkWidget *widget, GdkEventExpose *event)
{
	int x, y;
	int xmin = 0, ymin = 0, xmax = board_wid, ymax = board_heit;
	if (!opt_game) return TRUE;
	if (event)
	{
		xmin = event->area.x / cell_size;
		ymin = event->area.y / cell_size;
		xmax = (event->area.x + event->area.width) / cell_size + 1;
		ymax = (event->area.y + event->area.height) / cell_size + 1;
		if (ymin < 0) ymin = 0;
		if (xmax > board_wid) xmax = board_wid;
		if (ymax > board_heit) ymax = board_heit;
	}
	for (x=xmin; x<xmax; x++)
		for (y=ymin; y<ymax; y++)
				board_refresh_cell_real (
						state_board_flipped ? board_wid - 1 - x : x, 
						state_board_flipped ? y : (board_heit - 1 - y), 
						x, y);
	return TRUE;
}

//! Redraw the whole board
void board_redraw_all ()
{
	board_redraw (main_window, NULL);
}

//! Called when the game is unpaused
void board_show ()
{
	if (!board_suspended) return;
	board_suspended = FALSE;
	board_redraw_all ();
}

//! Hide the board when the game is paused
void board_hide ()
{
//	GdkGC *def_gc = gdk_gc_new ((GdkWindow *)board_area->window);
	g_assert (opt_game);
	board_suspended = TRUE;
/*	gdk_draw_rectangle ((GdkDrawable *)board_area->window, def_gc, TRUE, 0, 0, board_wid * cell_size, board_heit * cell_size);
	gdk_gc_destroy (def_gc);*/
	// FIXME: how do we get this screen to be the default background color?
	board_redraw_all();
}

static void board_get_cell (GdkEventButton *event, int *row, int *col)
{
	*row = ((int)event->x / cell_size);
	*col = board_heit - 1 - ((int)event->y / cell_size);
	if (state_board_flipped) 
	{
		*row = board_wid - 1 - *row;
		*col = board_heit - 1 - *col;
	}
}

gint board_key_pressed (GtkWidget *widget, GdkEventKey *event, 
		gpointer data, byte **movp)
{
	int status;
	if (event->type != GDK_KEY_PRESS)
		return -1;
	if (!game_getmove_kb) return -1;
	status = game_getmove_kb (&cur_pos, event->keyval, state_player, movp);
	return status;
}

//! handles mouse clicks as well as key presses
gint board_clicked (GtkWidget *widget, GdkEventButton *event, 
		gpointer data)
/* FIXME: clean up this function */
{
	int row, col, type;
	int status;
	byte *move;
	if (!opt_game) return FALSE;
	if (ui_gameover) return FALSE;
	if (event->type == GDK_KEY_PRESS)
	{
		status = board_key_pressed (widget, (GdkEventKey *)event, data, &move);
		if (status < 0) return FALSE;
	}
	else
	{
		if (event->type != GDK_MOTION_NOTIFY)
		{
			if (player_to_play != HUMAN)
			{
				sb_message ("Machine's turn", FALSE);
				return FALSE;
			}
			if (!impl_check()) { sb_error ("Not yet implemented", TRUE); 
				return FALSE; }
			if (!game_getmove) {return FALSE;}
		}
		if (!game_getmove) {return FALSE;}
		board_get_cell (event, &row, &col);
		if (!(row >= 0 && row < board_wid)) return FALSE;
		if (!(col >= 0 && col < board_heit)) return FALSE;
		switch (event->type)
		{
			case GDK_BUTTON_PRESS:
				type = GTKBOARD_BUTTON_PRESS; break;
			case GDK_BUTTON_RELEASE:
				type = GTKBOARD_BUTTON_RELEASE; break;
			case GDK_MOTION_NOTIFY:
				type = GTKBOARD_MOTION_NOTIFY; break;
			default:
				return FALSE;
		}
		status = game_getmove (&cur_pos, row, col, type, state_player, &move);
		if (status < 0)
		{
			sb_error ("Illegal Move", FALSE);
			return FALSE;
		}
	}
	if (status == 0)
		return FALSE;
	menu_start_stop_game (NULL, MENU_START_GAME); 
	ui_make_human_move (move);
	return FALSE;
}

//! Free all malloc()d stuff
void board_free ()
{
	int i;
	if (pieces)
	{
		/* It may be the case that the latter half of pieces[] is a copy
		   of the first half */
		for (i=0; i<num_pieces; i++)
			if (pieces[i] == pieces[i + num_pieces])
				pieces[i] = NULL;
		for (i=0; i<2*num_pieces; i++)
		{
			if (pieces[i]) 
			{
				gdk_pixmap_unref (pieces[i]);
				pieces[i] = NULL;
			}
		}
		free (pieces);
		pieces = NULL;
	}
	if (board_rowbox_real)
	{
		gtk_widget_destroy (board_rowbox_real);
		board_rowbox_real = NULL;
	}
	if (board_colbox_real)
	{
		gtk_widget_destroy (board_colbox_real);
		board_colbox_real = NULL;
	}
}

static gchar *board_get_filerank_label_str (int label, int idx)
	// callers must free returned string
{
	static gchar tempstr[8];
	int tmp = label & FILERANK_LABEL_TYPE_MASK;
	if (tmp == FILERANK_LABEL_TYPE_NUM) 
		snprintf (tempstr, 8, " %d ", idx + 1);
	if (tmp == FILERANK_LABEL_TYPE_ALPHA) 
		snprintf (tempstr, 8, " %c ", 'a' + idx);
	if (tmp == FILERANK_LABEL_TYPE_ALPHA_CAPS) 
		snprintf (tempstr, 8, " %c ", 'A' + idx);
	return tempstr;
}

static gchar *board_get_file_label_str (int label, int idx)
{
	if (label & FILERANK_LABEL_DESC) idx = board_wid - 1 - idx;
	return board_get_filerank_label_str (label, idx);
}

static gchar *board_get_rank_label_str (int label, int idx)
{
	if (label & FILERANK_LABEL_DESC) idx = board_heit - 1 - idx;
	return board_get_filerank_label_str (label, idx);
}

//! initialization of the board
void board_init ()
{
	int i;
	GdkColormap *board_colormap;
	Game *game = opt_game;
	GdkGC *def_gc = gdk_gc_new ((GdkWindow *)board_area->window);
//	GtkWidget *hbox, *vbox;

	if (!game)
	{
		gtk_widget_set_size_request (GTK_WIDGET (board_area), 300, 300);
		gdk_draw_rectangle ((GdkDrawable *)board_area->window, def_gc, TRUE, 0, 0, 300, 300);
		return;
	}
	
	gtk_widget_set_size_request (GTK_WIDGET (board_area), 
			cell_size * board_wid, cell_size * board_heit);
	pieces = (GdkPixmap **) malloc (2 * num_pieces * sizeof (GdkPixmap *));
	g_assert (pieces);
	for (i=0; i<2*num_pieces; i++)
		pieces[i] = NULL;

	if (game_file_label)
	{
		board_rowbox_real = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_end (GTK_BOX (board_rowbox), board_rowbox_real, FALSE, FALSE, 0);
		gtk_widget_set_size_request 
			(GTK_WIDGET (board_rowbox_real), cell_size * board_wid, -1);
		for (i=0; i<board_wid; i++)
			gtk_container_add (GTK_CONTAINER (board_rowbox_real), 
				gtk_label_new (board_get_file_label_str (game_file_label, i)));
		gtk_widget_show_all (board_rowbox);
	}
	if (game_rank_label)
	{
		board_colbox_real = gtk_vbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (board_colbox), board_colbox_real, FALSE, FALSE, 0);
		gtk_widget_set_size_request 
			(GTK_WIDGET (board_colbox_real), -1, cell_size * board_heit);
		for (i=0; i<board_heit; i++)
			gtk_container_add (GTK_CONTAINER (board_colbox_real), 
				gtk_label_new (board_get_rank_label_str 
					(game_rank_label, i)));
		gtk_widget_show_all (board_colbox);
	}
	
	
	board_colormap = gdk_colormap_get_system ();

	for (i=0; i<=2; i++)
	{
		if (i == 2 && !game_draw_cell_boundaries) continue;
		gdk_colors[i].red = 256 * game->colors[i*3 + 0];
		gdk_colors[i].green = 256 * game->colors[i*3 + 1];
		gdk_colors[i].blue = 256 * game->colors[i*3 + 2];
		gdk_color_alloc (board_colormap, &gdk_colors[i]);
		if (board_gcs[i]) 
			gdk_gc_unref (board_gcs[i]);
		board_gcs[i] = gdk_gc_new(board_area->window);
		gdk_gc_set_foreground (board_gcs[i], &gdk_colors[i]);
	}

	for (i=0; i<2*num_pieces; i++)
	{
		char **pixmap = NULL;
		guchar *rgbbuf = NULL;
		{
			byte *colors = game->colors;
			if (i >= num_pieces && colors[0] == colors[3] && colors[1] == colors[4] 
					&& colors[2] == colors[5])
			{
				pieces[i] = pieces[i-num_pieces];
				continue;
			}
		}
		if (game_get_rgbmap)
		{
			rgbbuf = game_get_rgbmap(1+i%num_pieces, i < num_pieces ? WHITE: BLACK);
			if (rgbbuf)
			{
				pieces[i] = gdk_pixmap_new (
						board_area->window, cell_size, cell_size, -1);
				gdk_draw_rgb_image ((GdkDrawable *)pieces[i], 
						def_gc, 0, 0, 
						cell_size, cell_size, GDK_RGB_DITHER_MAX,
						rgbbuf, cell_size * 3);
			}
		}			
		else 
		{
			if (game_get_pixmap)
				pixmap = game_get_pixmap
					(1+i%num_pieces, i < num_pieces ? WHITE: BLACK);
			else 
				pixmap = game->pixmaps [i%num_pieces];
			if (pixmap)
			{
				pieces[i] = gdk_pixmap_colormap_create_from_xpm_d (NULL,
					board_colormap, NULL, gdk_colors + i / num_pieces, pixmap);
				assert (pieces[i]);
			}
		}
	}
	gdk_gc_destroy (def_gc);
}


