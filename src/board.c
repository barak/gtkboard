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
#include "sound.h"
#include "../pixmaps/splash.xpm"

//! for showing names of rows and columns
static GtkWidget *board_rowbox_real = NULL, *board_colbox_real = NULL;

//! Colors for the light squares, dark squares, lines
GdkColor board_colors[6];

//! Colors for highlighting
GdkColor board_highlight_colors[3];

//! Colors for turning pixmaps into buttons
GdkColor board_buttonize_colors[2];

//! gcs for light squares, dark squares and lines
GdkGC *board_gcs[3] = {NULL, NULL, NULL};

//! gcs for highlighting
GdkGC *board_highlight_gcs[3] = {NULL, NULL, NULL};

//! gcs for drawing buttons
GdkGC *board_buttonize_gcs[3] = {NULL, NULL, NULL};

//! Images representing the pieces
static GdkPixmap **pieces = NULL;

//! Bitmaps to specify transparency for the pieces
// *sigh* I wish we had a _high-level_ widget set
static GdkBitmap **piece_masks = NULL;

//! Backround image
static GdkPixmap *board_bgimage = NULL;

//! This is TRUE when the game is paused
gboolean board_suspended = FALSE;

//! Is the board flipped (rotated 180 deg)
gboolean state_board_flipped = FALSE;

//! default background
char board_default_colors [9] = {215, 215, 215, 215, 215, 215, 0, 0, 0};

static int cell_size, num_pieces;

extern void ui_make_human_move (byte *move, int *rmove);

void board_set_game_params ()
{
	cell_size = opt_game->cell_size;
	num_pieces = opt_game->num_pieces;
}

void board_set_cell (int x, int y, byte val)
{
	cur_pos.board[y * board_wid + x] = val;
}

void board_apply_refresh (byte *move, int *rmove)
{
	int i, x, y;
	byte *board = cur_pos.board;
	int *rmove_tmp = NULL;
	
	if (move && game_get_render)
		game_get_render (&cur_pos, move, &rmove);
	
	if (move)
	{
		for (i=0; move[3*i] != -1; i++)
		{
			x = move[3*i]; y = move[3*i+1];
			cur_pos.board[y * board_wid + x] = move [3*i+2];
			board_refresh_cell (x, y);
		}
	}
	
	if (rmove)
	{
		for (i=0; rmove[3*i] != -1; i++)
		{
			x = rmove[3*i]; y = rmove[3*i+1];
			cur_pos.render[y * board_wid + x] = rmove [3*i+2];
			board_refresh_cell (x, y);
		}
	}

	if (rmove_tmp)
	{
		for (i=0; rmove[3*i] != -1; i++)
		{
			x = rmove_tmp[3*i]; y = rmove_tmp[3*i+1];
			cur_pos.render[y * board_wid + x] = rmove_tmp[3*i+2];
			board_refresh_cell (x, y);
		}
	}

}

//! Draws the square (x, y). On the board it is shown at (real_x, real_y)
void board_refresh_cell_real (int x, int y, int real_x, int real_y)
{
	GdkGC *gc;
	int parity = (board_wid * board_heit + x + y + 1) % 2;
	int thepiece;
	if (opt_quiet) return;
	if (!cur_pos.board) return;
	gc = board_area->style->bg_gc[GTK_STATE_NORMAL];
	gdk_gc_set_clip_mask (gc, NULL);
	gdk_gc_set_clip_origin (gc, real_x * cell_size, real_y * cell_size);
	thepiece = cur_pos.board[y * board_wid + x] -1 + num_pieces * parity;
	if ((cur_pos.render[y * board_wid + x] & 0xFF) == RENDER_REPLACE)
		thepiece = (cur_pos.render[y * board_wid + x] >> 8) -1 + num_pieces * parity;
	if ((cur_pos.board[y * board_wid + x] != 0
			|| (cur_pos.render[y * board_wid + x] & 0xFF) == RENDER_REPLACE)
			&& !board_suspended)
	{
		/* FIXME: current impl is that if bgimage is set then bgcolor is irrelevant. Maybe we should change it so that bgimage is layered on top of bgcolor */
		if (board_bgimage)
		{
			gdk_draw_pixmap (board_area->window,
					gc, (GdkDrawable *) board_bgimage,
					real_x * cell_size, real_y * cell_size,
					real_x * cell_size, real_y * cell_size,
					cell_size, cell_size
					);
			gdk_gc_set_clip_mask (gc, piece_masks [thepiece]);
			gdk_gc_set_clip_origin (gc, real_x * cell_size, real_y * cell_size);
		}
		gdk_draw_pixmap (board_area->window, 
				gc, (GdkDrawable *) pieces [thepiece],
				0, 0, real_x * cell_size, real_y * cell_size, 
				-1, -1);
	}
	else
	{
		if (board_bgimage)
		{
			gdk_draw_pixmap (board_area->window,
					gc, (GdkDrawable *) board_bgimage,
					real_x * cell_size, real_y * cell_size,
					real_x * cell_size, real_y * cell_size,
					cell_size, cell_size
					);
		}
		else
			gdk_draw_rectangle (board_area->window, board_gcs[parity], 1, 
					real_x * cell_size, real_y * cell_size,
					cell_size, cell_size);
	}

	if (cur_pos.render[y * board_wid + x] == RENDER_SHADE1			
			&& cur_pos.board[y * board_wid + x] != 0
		   	&& !board_suspended)
	{
#if GTK_MAJOR_VERSION > 1
		GdkPixbuf *pixbuf;
		int i;
		guchar *pixels;
		pixbuf = gdk_pixbuf_get_from_drawable (NULL, pieces[thepiece], NULL,
				0, 0, 0, 0, cell_size, cell_size);
		pixels = gdk_pixbuf_get_pixels (pixbuf);
		for (i=0; i<3*cell_size*cell_size; i++)
			pixels[i] = (pixels[i] + 127)/2;
		gdk_pixbuf_render_to_drawable (pixbuf, board_area->window, gc, 0, 0,
				real_x * cell_size, real_y * cell_size, cell_size, cell_size,
				GDK_RGB_DITHER_NONE, 0, 0);
		// FIXME: find out the  correct way to free it
		g_free (pixels);
		g_free (pixbuf);
#else
		fprintf (stderr, "Warning: RENDER_SHADE currently unimplemented in gtk1 version\n");
#endif
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
	
	// TODO: do HIGHLIGHT2 and 3 also
	if (cur_pos.render[y * board_wid + x] == RENDER_HIGHLIGHT1 && !board_suspended)
	{
		int incr = game_draw_cell_boundaries ? 1 : 0;
		gdk_draw_line (board_area->window, board_highlight_gcs[0],
			real_x * cell_size + incr, real_y * cell_size + incr,
			real_x * cell_size + incr, (real_y + 1) * cell_size - 1);
		gdk_draw_line (board_area->window, board_highlight_gcs[0],
			real_x * cell_size + incr, real_y * cell_size + incr,
			(real_x + 1) * cell_size - 1, real_y * cell_size + incr);
		gdk_draw_line (board_area->window, board_highlight_gcs[0],
			(real_x + 1) * cell_size - 1, real_y * cell_size + incr,
			(real_x + 1) * cell_size - 1, (real_y + 1) * cell_size - 1);
		gdk_draw_line (board_area->window, board_highlight_gcs[0],
			real_x * cell_size + incr, (real_y + 1) * cell_size - 1,
			(real_x + 1) * cell_size - 1, (real_y + 1) * cell_size - 1);
	}
	
	if (cur_pos.render[y * board_wid + x] == RENDER_BUTTONIZE && !board_suspended)
	{
		int incr = game_draw_cell_boundaries ? 1 : 0;
		gdk_draw_line (board_area->window, board_buttonize_gcs[0],
			real_x * cell_size + incr, real_y * cell_size + incr,
			real_x * cell_size + incr, (real_y + 1) * cell_size - 1);
		gdk_draw_line (board_area->window, board_buttonize_gcs[0],
			real_x * cell_size + incr, real_y * cell_size + incr,
			(real_x + 1) * cell_size - 1, real_y * cell_size + incr);
		gdk_draw_line (board_area->window, board_buttonize_gcs[1],
			(real_x + 1) * cell_size - 1, real_y * cell_size + incr,
			(real_x + 1) * cell_size - 1, (real_y + 1) * cell_size - 1);
		gdk_draw_line (board_area->window, board_buttonize_gcs[1],
			real_x * cell_size + incr, (real_y + 1) * cell_size - 1,
			(real_x + 1) * cell_size - 1, (real_y + 1) * cell_size - 1);
	}
	
}

//! A wrapper around board_refresh_cell_real() to take care of whether the board is flipped
void board_refresh_cell (int x, int y)
{
	int real_x, real_y;
	real_x = (state_board_flipped ? board_wid - 1 - x : x);
	real_y = (state_board_flipped ? y : board_heit - 1 - y);
	board_refresh_cell_real (x, y, real_x, real_y);
}



//! Redraws the exposed area of the board
gboolean board_redraw (GtkWidget *widget, GdkEventExpose *event)
{
	int x, y;
	int xmin = 0, ymin = 0, xmax = board_wid, ymax = board_heit;
	if (!opt_game)
	{
		GdkPixmap *splash_pixmap;
		splash_pixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, 
				gdk_colormap_get_system (), NULL, NULL, splash_xpm);
		gdk_draw_pixmap ((GdkDrawable *)board_area->window, 
				board_area->style->bg_gc[GTK_STATE_NORMAL],
				(GdkDrawable *)splash_pixmap, 
				0, 0, 0, 0, -1, -1);
		gdk_pixmap_unref (splash_pixmap);
		return TRUE;
	}
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

static void board_get_cell (GdkEventButton *event, 
		int *row, int *col, int *pixel_x, int *pixel_y)
{
	*row = ((int)event->x / cell_size);
	*col = board_heit - 1 - ((int)event->y / cell_size);
	*pixel_x = (int)event->x;
	*pixel_y = board_heit * cell_size - 1 - (int)event->y;
	if (state_board_flipped) 
	{
		*row = board_wid - 1 - *row;
		*col = board_heit - 1 - *col;
		*pixel_x = board_wid * cell_size - 1 - *pixel_x;
		*pixel_y = board_heit * cell_size - 1 - *pixel_y;
	}
}

//! handles mouse clicks as well as key presses
gint board_signal_handler (GtkWidget *widget, GdkEventButton *event, 
		gpointer data)
/* FIXME: clean up this function */
{
	int row, col, pixel_x, pixel_y, type;
	int status = 0;
	byte *move = NULL;
	int *rmove = NULL;
	GtkboardEvent our_event;
	MoveInfo minfo = {NULL, NULL, NULL, NULL, NULL};
	if (!opt_game) return FALSE;
	if (ui_gameover) return FALSE;
	if (event->type == GDK_KEY_PRESS && 
			!(((GdkEventKey *)event)->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)))
	{
		if (!game_getmove_kb && !game_event_handler) return FALSE;
		if (game_event_handler) 
		{
			our_event.type = GTKBOARD_KEY_PRESS;
			our_event.key = ((GdkEventKey *)event)->keyval;
			status = game_event_handler (&cur_pos, &our_event, &minfo);
			move = minfo.move;
			rmove = minfo.rmove;
		}
		else //(if game_getmove_kb)
			status = game_getmove_kb (&cur_pos, 
				((GdkEventKey *)event)->keyval, &move, &rmove);
	}
	else
	{
		if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE)
		{
			if (player_to_play != HUMAN)
			{
				sb_message ("Machine's turn", FALSE);
				return FALSE;
			}
			if (!impl_check()) { sb_error ("Not yet implemented", TRUE); 
				return FALSE; }
		}
		if (!game_getmove && !game_event_handler) {return FALSE;}
		board_get_cell (event, &row, &col, &pixel_x, &pixel_y);
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
			case GDK_LEAVE_NOTIFY:
				type = GTKBOARD_LEAVE_NOTIFY; break;
			default:
				return FALSE;
		}
		if (game_event_handler)
		{
			our_event.type = type;
			our_event.x = row;
			our_event.y = col;
			our_event.pixel_x = pixel_x;
			our_event.pixel_y = pixel_y;
			status = game_event_handler (&cur_pos, &our_event, &minfo);
			move = minfo.move;
			rmove = minfo.rmove;
		}
		else
			status = game_getmove (&cur_pos, row, col, 
					type, cur_pos.player, &move, &rmove);
		if (status < 0)
		{
			gchar *tmpstr = minfo.help_message ? 
				g_strdup_printf ("Illegal move: %s", minfo.help_message) 
				: g_strdup_printf ("Illegal move");
			sb_error (tmpstr, FALSE);
			g_free (tmpstr);
			sound_play (SOUND_ILLEGAL_MOVE);
		}
	}
	if (status <= 0)
	{
		ui_make_human_move (NULL, rmove);
		if (rmove)
			menu_start_stop_game (NULL, MENU_START_GAME); 
		return FALSE;
	}
	sound_play (SOUND_USER_MOVE);
	menu_start_stop_game (NULL, MENU_START_GAME); 
	ui_make_human_move (move, rmove);
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
		{
			if (pieces[i] == pieces[i + num_pieces])
				pieces[i] = NULL;
			if (piece_masks[i] == piece_masks[i + num_pieces])
				piece_masks[i] = NULL;
		}
		for (i=0; i<2*num_pieces; i++)
		{
			if (pieces[i]) 
			{
				gdk_pixmap_unref (pieces[i]);
				pieces[i] = NULL;
			}
			if (piece_masks[i]) 
			{
				gdk_pixmap_unref (piece_masks[i]);
				piece_masks[i] = NULL;
			}
		}
		free (pieces);
		free (piece_masks);
		pieces = NULL;
		piece_masks = NULL;
	}
	if (board_bgimage)
	{
		free (board_bgimage);
		board_bgimage = NULL;
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

static void board_color_init (char *color, GdkColor *gdkcolor, GdkGC **gc, GdkColormap *cmap, GtkWidget *board_area)
{
	gdkcolor->red = 256 * (color ? color[0] : 215);
	gdkcolor->green = 256 * (color ? color[1] : 215);
	gdkcolor->blue = 256 * (color ? color[2] : 215);
	gdk_color_alloc (cmap, gdkcolor);
	if (*gc) 
		gdk_gc_unref (*gc);
	*gc = gdk_gc_new(board_area->window);
	gdk_gc_set_foreground (*gc, gdkcolor);

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
#if GTK_MAJOR_VERSION == 1
		gtk_drawing_area_size (GTK_DRAWING_AREA (board_area), 300, 300);
#else
		gtk_widget_set_size_request (GTK_WIDGET (board_area), 300, 300);
#endif
		return;
	}
	
#if GTK_MAJOR_VERSION == 1
	gtk_drawing_area_size (GTK_DRAWING_AREA (board_area), 
			cell_size * board_wid, cell_size * board_heit);
#else
	gtk_widget_set_size_request (GTK_WIDGET (board_area), 
			cell_size * board_wid, cell_size * board_heit);
#endif
	pieces = (GdkPixmap **) malloc (2 * num_pieces * sizeof (GdkPixmap *));
	g_assert (pieces);
	piece_masks = (GdkBitmap **) malloc (2 * num_pieces * sizeof (GdkBitmap *));
	g_assert (piece_masks);
	for (i=0; i<2*num_pieces; i++)
		pieces[i] = NULL;

	if (game_file_label)
	{
		board_rowbox_real = gtk_hbox_new (TRUE, 0);
		gtk_box_pack_end (GTK_BOX (board_rowbox), board_rowbox_real, FALSE, FALSE, 0);
#if GTK_MAJOR_VERSION == 1
		gtk_widget_set_usize (GTK_WIDGET (board_rowbox_real),
				cell_size * board_wid, -1);
#else
		gtk_widget_set_size_request 
			(GTK_WIDGET (board_rowbox_real), cell_size * board_wid, -1);
#endif
		for (i=0; i<board_wid; i++)
			gtk_container_add (GTK_CONTAINER (board_rowbox_real), 
				gtk_label_new (board_get_file_label_str (game_file_label, i)));
		gtk_widget_show_all (board_rowbox);
	}
	if (game_rank_label)
	{
		board_colbox_real = gtk_vbox_new (TRUE, 0);
		gtk_box_pack_start (GTK_BOX (board_colbox), board_colbox_real, FALSE, FALSE, 0);
#if GTK_MAJOR_VERSION == 1
		gtk_widget_set_usize (GTK_WIDGET (board_colbox_real),
				-1, cell_size * board_heit);
#else
		gtk_widget_set_size_request 
			(GTK_WIDGET (board_colbox_real), -1, cell_size * board_heit);
#endif
		for (i=0; i<board_heit; i++)
			gtk_container_add (GTK_CONTAINER (board_colbox_real), 
				gtk_label_new (board_get_rank_label_str 
					(game_rank_label, i)));
		gtk_widget_show_all (board_colbox);
	}
	
	
	if (game->colors == NULL) game->colors = board_default_colors;
	board_colormap = gdk_colormap_get_system ();

	board_color_init (&game->colors[0], 
			&board_colors[0], &board_gcs[0], board_colormap, board_area);
	board_color_init (&game->colors[3], 
			&board_colors[1], &board_gcs[1], board_colormap, board_area);
	if (game_draw_cell_boundaries)
		board_color_init (&game->colors[6], 
				&board_colors[2], &board_gcs[2], board_colormap, board_area);
	if (game_highlight_colors)
		for (i=0; i<3; i++)
			board_color_init (&game_highlight_colors[3*i],
					&board_highlight_colors[i], &board_highlight_gcs[i], 
					board_colormap, board_area);
	{
	char buttonize_colors [6] = {240, 240, 240, 128, 128, 128};
	for (i=0; i<2; i++)
	board_color_init (&buttonize_colors[3*i], &board_buttonize_colors[i],
			&board_buttonize_gcs[i], board_colormap, board_area);
	}

	g_assert (num_pieces);
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
				piece_masks[i] = piece_masks[i-num_pieces];
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
			piece_masks[i] = NULL;
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
					board_colormap, &piece_masks[i], 
					board_colors + i / num_pieces, pixmap);
				assert (pieces[i]);
			}
			else piece_masks[i] = NULL;
		}
	}

	if (game_bg_pixmap)
	{
		board_bgimage = gdk_pixmap_colormap_create_from_xpm_d (NULL,
			board_colormap, NULL, board_colors, game_bg_pixmap);
		assert (board_bgimage);
	}

	gdk_gc_destroy (def_gc);
}


