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
#ifndef _MENU_H_
#define _MENU_H_
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "game.h"

enum {MENU_START_GAME = 1, MENU_STOP_GAME, MENU_RESET_GAME};
enum {MENU_BACK = 1, MENU_FORW};
enum {MENU_SENS_MACHINE_THINKING = 1, MENU_SENS_MACHINE_NOT_THINKING,
	MENU_SENS_NO_GAME, MENU_SENS_NO_BACK_FORW,
	MENU_SENS_SINGLE_PLAYER, MENU_SENS_TWO_PLAYERS,
	MENU_SENS_UI_STOPPED, MENU_SENS_EVAL_FUNCTION};
enum {MENU_DOC_ABOUT, MENU_DOC_RULES, MENU_DOC_STRATEGY, MENU_DOC_HISTORY};

void menu_resensitize (int);
void menu_desensitize (int);
void menu_start_stop_game (gpointer , guint );
void menu_set_eval_function ();
void menu_load_file_dialog ();
void menu_save_file_dialog ();
void menu_set_delay_cb (gpointer data, guint delay, GtkWidget *widget);
void menu_back_forw (gpointer data, guint what);
void menu_set_player (gpointer *, guint, GtkWidget *);
void menu_put_player (gboolean);
void menu_start_game ();
void menu_put_game ();
void menu_board_flip_cb ();
void menu_set_game (gpointer, guint, GtkWidget *);
void menu_insert_game_item (gchar *, int);
void menu_put_level (char *);
void menu_enable_sound_cb (gpointer data, guint what);
void sb_set_score (gchar *score);
void sb_update ();
gboolean sb_update_periodic ();
void sb_reset_human_time ();
int sb_get_human_time ();
void menu_help_home_page (gpointer);
void menu_show_about_dialog (gpointer);
void menu_show_begging_dialog (gpointer);
void menu_show_dialog (gchar *, gchar *);
void menu_show_game_doc (gpointer, guint);	
gchar *menu_get_game_name ();
gchar *menu_get_game_name_with_level ();
void sb_set_turn_image ();

extern GtkWidget *sb_message_label, *sb_game_label, *sb_score_label,
	*sb_who_label, *sb_player_label, *sb_time_label, *sb_turn_image,
	*menu_main, *menu_info_bar, *menu_info_separator, *menu_warning_bar;
extern GtkWidget *sb_game_separator, *sb_player_separator, *sb_who_separator, 
	*sb_score_separator, *sb_time_separator, *sb_turn_separator, 
	*sb_warning_separator;
extern GtkItemFactory *menu_factory;
#endif
