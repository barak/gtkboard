#ifndef _BOARD_H_
#define _BOARD_H_

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "game.h"


extern GdkColor gdk_colors[3];
extern GdkGC *board_gcs[3];
extern gboolean board_suspended ;
extern GtkWidget *board_area;
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
gint board_clicked (GtkWidget *, GdkEventButton *, gpointer);

#endif
