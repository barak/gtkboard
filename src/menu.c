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
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "config.h"
#include "ui.h"
#include "menu.h"
#include "move.h"
#include "ui_common.h"
#include "aaball.h"
#include "board.h"
#include "prefs.h"
#include "sound.h"

#ifdef HAVE_GNOME
#include "libgnome/libgnome.h"
#include "libgnomeui/libgnomeui.h"
#include "../pixmaps/logo48.xpm"
#endif

GtkWidget *sb_message_label, *sb_game_label, *sb_score_label,
	*sb_who_label, *sb_player_label, *sb_time_label, *sb_turn_image,
	*menu_main, *menu_info_bar, *menu_info_separator, *menu_warning_bar;
GtkWidget *sb_game_separator, *sb_who_separator, *sb_score_separator,
	*sb_player_separator, *sb_time_separator, *sb_turn_separator,
	*sb_warning_separator;
#define SB_MESSAGE_STRLEN 64
gchar sb_message_str[SB_MESSAGE_STRLEN] = "";
#define SB_SCORE_STRLEN 32
gchar sb_score_str[SB_SCORE_STRLEN] = "";
GtkItemFactory *menu_factory = NULL;

void sb_messagebar_message (gchar *);
void menu_cleanup_var_menus ();

static char * menu_paths_sens_machine_thinking[] = 
{
//	"/File/Load game",
//	"/File/Save game",
	"/Game/Start",
	"/Game/New",
	"/Game/Select Game",
	"/Settings",
	"/Move/Back",
	"/Move/Forward",
};

static char * menu_paths_sens_no_game[] = 
{
	"/Game/Start",
	"/Game/Pause",
	"/Game/New",
	"/Game/Highscores",
	"/Game/Zap Highscores",
//	"/File/Save game",
	"/Move/Back",
	"/Move/Forward",
	"/Settings/Flip Board",
	"/Settings/Player",
};

static char * menu_paths_sens_no_back_forw[] = 
{
	"/Move/Back",
	"/Move/Forward",
};

static char *menu_paths_sens_single_player[] =
{
	"/Settings/Player",
	"/Settings/Flip Board",
//	"/Settings/Eval function",
};

static char *menu_paths_sens_two_players[] =
{
	"/Game/Highscores",
	"/Game/Zap Highscores",
};

static char *menu_paths_sens_ui_stopped[] = 
{
	"/Game/Pause",
};

static char *menu_paths_sens_machine_not_thinking[] = 
{
	"/Move/Move Now",
};

static char *menu_paths_sens_eval_function[] = 
{
//	"/Settings/Eval function",
};

void sb_set_score (gchar *score)
{
	strncpy (sb_score_str, score, SB_SCORE_STRLEN-1);
	sb_update ();
}

void menu_sensitize (int which, gboolean sens)
{
	int i, num_paths = -1;
	char **menu_des_paths = NULL;
	if (!state_gui_active) return;
	switch (which)
	{
		case MENU_SENS_MACHINE_THINKING: 
			menu_des_paths = menu_paths_sens_machine_thinking;
			num_paths = sizeof (menu_paths_sens_machine_thinking) / 
				sizeof (menu_paths_sens_machine_thinking[0]);
			break;
		case MENU_SENS_MACHINE_NOT_THINKING: 
			menu_des_paths = menu_paths_sens_machine_not_thinking;
			num_paths = sizeof (menu_paths_sens_machine_not_thinking) / 
				sizeof (menu_paths_sens_machine_not_thinking[0]);
			break;
		case MENU_SENS_NO_GAME: 
			menu_des_paths = menu_paths_sens_no_game;
			num_paths = sizeof (menu_paths_sens_no_game) / 
				sizeof (menu_paths_sens_no_game[0]);
			break;
		case MENU_SENS_NO_BACK_FORW: 
			menu_des_paths = menu_paths_sens_no_back_forw;
			num_paths = sizeof (menu_paths_sens_no_back_forw) / 
				sizeof (menu_paths_sens_no_back_forw[0]);
			break;
		case MENU_SENS_SINGLE_PLAYER: 
			menu_des_paths = menu_paths_sens_single_player;
			num_paths = sizeof (menu_paths_sens_single_player) / 
				sizeof (menu_paths_sens_single_player[0]);
			break;
		case MENU_SENS_TWO_PLAYERS: 
			menu_des_paths = menu_paths_sens_two_players;
			num_paths = sizeof (menu_paths_sens_two_players) / 
				sizeof (menu_paths_sens_two_players[0]);
			break;
		case MENU_SENS_UI_STOPPED: 
			menu_des_paths = menu_paths_sens_ui_stopped;
			num_paths = sizeof (menu_paths_sens_ui_stopped) / 
				sizeof (menu_paths_sens_ui_stopped[0]);
			break;
		case MENU_SENS_EVAL_FUNCTION:
			menu_des_paths = menu_paths_sens_eval_function;
			num_paths = sizeof (menu_paths_sens_eval_function) / 
				sizeof (menu_paths_sens_eval_function[0]);
			break;
		default:
			g_assert_not_reached ();
	}
	for (i=0; i<num_paths; i++)
		gtk_widget_set_sensitive (gtk_item_factory_get_widget (menu_factory, 
					menu_des_paths[i]), sens);
}

void sb_set_turn_image ()
{
#if GTK_MAJOR_VERSION == 2
	// FIXME: can't get existing bgcolor
	const int size = 20;
	static char pixbufs [7][size*(size+1)];
	char **pixmap_data;
	static GdkPixmap *pixmaps[7];
	int colors[7] = {0x007700, 0x77ff77, 0x770000, 0xff7777, 0x000077, 0x7777ff,
					0x777777};
	int i, j;
	static int first = 1;
	static int previndex = -1;
	int index = -1;
	if (first)
	{
		int i;
		for (i=0; i<7; i++)
		{
			pixmap_data = pixmap_ball_gen (size, pixbufs[i], colors[i], 
					0xffffff, 6.5, 24);
			pixmaps[i] = gdk_pixmap_create_from_xpm_d 
				((GdkWindow *)main_window->window, NULL, NULL, pixmap_data);
		}
		gtk_image_set_from_pixmap (GTK_IMAGE (sb_turn_image), 
				pixmaps[previndex = 6], NULL);
		first = 0;
	}
	if (opt_infile || !opt_game)
		index = 6;
	else 
	{
		if (!game_single_player && ui_white == HUMAN && ui_black == HUMAN
				&& cur_pos.player == BLACK) index = 4;
		else index =  (player_to_play == HUMAN ? 0 : 2);
		if (ui_stopped) index++;
	}
	g_assert (index >= 0 && index <= 6);
	if (index == previndex) return;
	gtk_image_set_from_pixmap (GTK_IMAGE (sb_turn_image), 
			pixmaps[previndex = index], NULL);
#endif
}

static void sb_set_cursor ()
{
	static GdkCursor *cursor_normal = NULL, *cursor_busy = NULL, 
		*cursor_inactive = NULL;
	// FIXME: is it ok to hard code the shape of the normal cursor?
	if (!cursor_normal) cursor_normal = gdk_cursor_new (GDK_LEFT_PTR);
	if (!cursor_busy) cursor_busy = gdk_cursor_new (GDK_WATCH);
	if (!cursor_inactive) cursor_inactive = gdk_cursor_new (GDK_XTERM);
	if (player_to_play == MACHINE && !ui_stopped)
		gdk_window_set_cursor (board_area->window, cursor_busy);
	else if (player_to_play == MACHINE && ui_stopped)
		gdk_window_set_cursor (board_area->window, cursor_inactive);
	else
		gdk_window_set_cursor (board_area->window, cursor_normal);
}

void menu_board_flip_cb ()
{
	if (game_single_player)
	{
		sb_error ("Can't flip board in single player game", TRUE);
		return;
	}
	if (!game_allow_flip)
	{
		sb_error ("This game doesn't allow the board to be flipped", TRUE);
		return;
	}
	state_board_flipped = state_board_flipped ? FALSE : TRUE;
	board_redraw (NULL, NULL);
}


static void menu_show_dialog_real (gchar *title, gchar *message, gboolean wrap)
	// A modal dialog with a label. Don't use it for showing large amounts of text
{
	GtkWidget *dialog, *okay_button, *label;

#if GTK_MAJOR_VERSION == 1
	label = gtk_label_new (message);
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (main_window));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	okay_button = gtk_button_new_with_label("OK");

	gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
			GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) dialog);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
			okay_button);
#else
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), message);
	dialog = gtk_dialog_new_with_buttons (title, GTK_WINDOW (main_window),
			GTK_DIALOG_MODAL, NULL);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 100);
	okay_button = gtk_dialog_add_button (GTK_DIALOG (dialog), 
			GTK_STOCK_OK, GTK_RESPONSE_NONE);
	g_signal_connect_swapped (GTK_OBJECT (dialog), 
			"response", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (dialog));
	gtk_label_set_selectable (GTK_LABEL (label), TRUE);
#endif
	gtk_label_set_line_wrap (GTK_LABEL (label), wrap);
	gtk_widget_grab_focus (okay_button);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show_all (dialog);
}

void menu_show_dialog (gchar *title, gchar *message)
{
	menu_show_dialog_real (title, message, FALSE);
}

void menu_show_dialog_wrap (gchar *title, gchar *message)
{
	menu_show_dialog_real (title, message, TRUE);
}
	

void menu_pause_cb (GtkWidget *dialog)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
	menu_start_stop_game (NULL, MENU_START_GAME);
}

void menu_show_pause_dialog ()
	// Game paused
{
	GtkWidget *dialog, *okay_button, *label;
	gchar *title = "Game paused - gtkboard";
	gchar *msg = "Game paused. Click OK to continue";

	board_hide();

	label = gtk_label_new (msg);
#if GTK_MAJOR_VERSION == 1
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (main_window));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	okay_button = gtk_button_new_with_label("OK");

	gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
			GTK_SIGNAL_FUNC (menu_pause_cb), (gpointer) dialog);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
			okay_button);
	gtk_widget_grab_focus (okay_button);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show_all (dialog);
#else
	dialog = gtk_message_dialog_new (GTK_WINDOW (main_window), 
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			msg);
/*	dialog = gtk_dialog_new_with_buttons (title, GTK_WINDOW (main_window),
			GTK_DIALOG_MODAL, NULL);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 100);
	okay_button = gtk_dialog_add_button (GTK_DIALOG (dialog), 
			GTK_STOCK_OK, GTK_RESPONSE_NONE);
	gtk_label_set_selectable (GTK_LABEL (label), TRUE);
*/
#endif

#if GTK_MAJOR_VERSION > 1
	gtk_dialog_run (GTK_DIALOG (dialog));
	menu_pause_cb (dialog);
#endif
}

void menu_show_about_dialog (gpointer data)
{
#ifdef HAVE_GNOME
	GtkWidget *about;
	GdkPixmap *logo_pixmap;
	GdkPixbuf *logo_pixbuf;
	const gchar *authors[] = {"Arvind Narayanan <arvindn@users.sourceforge.net>", "Ron Hale-Evans <rwhe@ludism.org>", NULL};
	gchar *logo_filename;
	logo_filename = g_strdup_printf ("%s/pixmaps/gtkboard.png", DATADIR);
	logo_pixbuf = gdk_pixbuf_new_from_file (logo_filename, NULL);
	if (!logo_pixbuf)
	{
		fprintf (stderr, "Warning: couldn't load icon from file %s\n", logo_filename);
		logo_pixmap = gdk_pixmap_create_from_xpm_d (main_window->window, NULL, NULL, logo48_xpm);
		logo_pixbuf = gdk_pixbuf_get_from_drawable (NULL, logo_pixmap, 
				gdk_colormap_get_system (), 0, 0, 0, 0, -1, -1);
		gdk_pixmap_unref (logo_pixmap);
	}
	g_free (logo_filename);
	about = gnome_about_new (PACKAGE, VERSION, 
			"Copyright (C) 2003 Arvind Narayanan",
			NULL, authors, NULL, NULL, logo_pixbuf);
	gtk_widget_show (about);
#else
	menu_show_dialog ("About gtkboard", 
			"gtkboard " VERSION "\n"
			"http://gtkboard.sourceforge.net/\n"
			"Maintainer: Arvind Narayanan <arvindn@users.sourceforge.net>\n"
			"Released under the GNU General Public License\n"
			"See the file COPYING for details\n"
			"\n"
			"The documentation is available in the doc/ directory\n"
			"of the source distribution or in the directory\n" 
			"/usr/local/doc/gtkboard-" VERSION "/ if you installed from binary rpm.\n"
			"The latest documentation will always be available at\n"
			"http://gtkboard.sourceforge.net/doc/"
			);
#endif
}

void menu_show_begging_dialog (gpointer data)
{
	menu_show_dialog_wrap ("Begging bowl",
			"Thanks for using gtkboard. I hope you liked it.\n\n"
			"The download counter on sourceforge is broken. "
			"They call it an \"inaccuracy\", but the fact of the matter is that it's firmly stuck at zero, which means that I have no idea how many people are downloading/using the software. So you see, I'm as lonely as a lark (or whatever it is that's supposed to be very lonely.) So if you have any comments or suggestions, or even just some kind words, it would be nice if you can mail them me. My email is arvindn@users.sourceforge.net. Thanks."
			);
}

void menu_help_home_page (gpointer data)
{
#ifdef HAVE_GNOME
	if (!gnome_url_show ("http://gtkboard.sourceforge.net/", NULL))
		sb_error ("Couldn't launch home page", TRUE);
#else
	fprintf (stderr, "warning: menu_show_home_page() called even though gnome integration not compiled in\n");
#endif
}


	
void menu_put_player ();

void menu_start_stop_game (gpointer data, guint what)
{
	switch (what)
	{
		case MENU_START_GAME:
			if (!opt_game) break;
			if (!ui_stopped)
				break;
			board_show();
			if (ui_gameover)
			{
				sb_error ("Game over", FALSE);
				break;
			}
			if (!impl_check()) 
			{	
				sb_error ("Not yet implemented", TRUE);
				break;
			}
			ui_stopped = FALSE;
			ui_send_make_move ();
			sb_update();
			break;
		case MENU_STOP_GAME:
			if (!opt_game) break;
			if (ui_stopped) break;
			ui_stopped = TRUE;
			ui_cancel_move ();
			sb_update();
			if (game_single_player && ui_white == HUMAN)
				menu_show_pause_dialog ();
			break;
		case MENU_RESET_GAME:
			{
			int saved_white = ui_white;
			int saved_black = ui_black;
			if (!opt_game) break;
			ui_terminate_game ();
			ui_start_game ();
			ui_white = saved_white;
			ui_black = saved_black;
			menu_put_player(FALSE);
			sb_reset_human_time ();
			ui_stopped = FALSE;
			ui_send_make_move ();
			sb_update();
			break;
			}
		default:
			printf ("menu_start_stop_game: %d\n", what);
			assert (0);
	}
}


// set the menu corresponding to ui_white and ui_black
// if first is TRUE use opt_white and opt_black
void menu_put_player (gboolean first)
{
	gchar *path, *paths[4] = { 
		"/Settings/Player/Human-Human", "/Settings/Player/Human-Machine",
		"/Settings/Player/Machine-Human", "/Settings/Player/Machine-Machine"};
	if (!state_gui_active) return;
	if (first) {ui_white = opt_white; ui_black = opt_black;}
	if (opt_infile)
	{
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
		(gtk_item_factory_get_widget (menu_factory, "/Settings/Player/File")), 
		              TRUE);
			
		return;
	}	
	else if (ui_white == HUMAN && ui_black == HUMAN)
		path = paths[0];
	else if (ui_white == HUMAN && ui_black == MACHINE)
		path = paths[1];
	else if (ui_white == MACHINE && ui_black == HUMAN)
		path = paths[2];
	else if (ui_white == MACHINE && ui_black == MACHINE)
		path = paths[3];
	else
		return;
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
			(gtk_item_factory_get_widget (menu_factory, path)), TRUE);
}

void menu_put_game ()
{
	gchar path[128] = "/Game/Select Game/";
	if (opt_game->group)
	{
		strcat (path, opt_game->group);
		strcat (path, "/");
	}
	strcat (path, opt_game->name);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
			(gtk_item_factory_get_widget (menu_factory, path)), TRUE);
}

GtkWidget *menu_selector;
	
void menu_load_file (GtkFileSelection *selector, gpointer user_data) 
{
	gchar const *filename;
	static FILE *in;
	filename = gtk_file_selection_get_filename 
		(GTK_FILE_SELECTION(menu_selector));
	if (!(in = fopen (filename, "r")))
	{
		gchar *tempstr = g_strdup_printf 
			("Could not open file \"%s\" for reading", filename);
		sb_error (tempstr, TRUE);
		g_free (tempstr);
		return;
	}
	if (opt_infile)
		fclose (opt_infile);
	opt_infile = in;
	// FIXME: shouldn't this be MACHINE ?
	ui_white = ui_black = NONE;
	menu_put_player (FALSE);
	cur_pos.player = WHITE;
	game_set_init_pos (&cur_pos);
	board_redraw_all ();
	sb_message ("Opened file", FALSE);
}

void menu_set_player (gpointer *data, guint what, GtkWidget *widget)
{
	/* the callback for a radio button appears to
	   be called TWICE, once when selected and once when something else
	   is selected. */
	
	if (!GTK_CHECK_MENU_ITEM(widget)->active)
		return;

	if (what >= 1 && what <= 4)
		if (opt_infile)
		{
			fclose (opt_infile);
			opt_infile = NULL;
		}

	ui_stopped = TRUE;

	switch (what)
	{
		case 1:
			ui_white = HUMAN;
			ui_black = HUMAN;
			break;
		case 2:
			ui_white = HUMAN;
			ui_black = MACHINE;
			break;
		case 3:
			ui_white = MACHINE;
			ui_black = HUMAN;
			break;
		case 4:
			ui_white = MACHINE;
			ui_black = MACHINE;
			break;

		default:
			ui_white = NONE;
			ui_black = NONE;
			break;
	}

	sb_update ();
}

void menu_save_file_dialog ()
{
	sb_error ("Not yet implemented", TRUE);
}

void menu_load_file_dialog ()
{
	if (game_single_player)
	{
		sb_error ("Can't load from file for single player game.", FALSE);
		return;
	}
	menu_selector = gtk_file_selection_new ("");
	g_assert (menu_selector);
	/*gtk_file_selection_complete (
			GTK_FILE_SELECTION (menu_selector), "*.cbgf");*/
	
	gtk_signal_connect (GTK_OBJECT 
		(GTK_FILE_SELECTION(menu_selector)->ok_button),
		"clicked", GTK_SIGNAL_FUNC (menu_load_file), NULL);
			   
	gtk_signal_connect_object (GTK_OBJECT 
		(GTK_FILE_SELECTION(menu_selector)->ok_button),
		"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
		(gpointer) menu_selector);

	gtk_signal_connect_object (
		GTK_OBJECT (GTK_FILE_SELECTION(menu_selector)->cancel_button),
		"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
		(gpointer) menu_selector);
   
   gtk_widget_show (menu_selector);
}

void menu_show_game_doc (gpointer data, guint which)
{
	GtkWidget *dialog, *msgarea, *okay_button, *vbar, *hbox, *scrwin;
	char *impl;
	char *msgstr = "Nothing available"; // default
	char titlestr[64];
	switch (which)
	{
		case MENU_DOC_ABOUT:
			switch (game_doc_about_status)
			{
				case STATUS_NONE: impl = ""; break;
				case STATUS_UNPLAYABLE: impl = "Partially implemented (unplayable)"; break;
				case STATUS_PARTIAL: impl = "Partially implemented (playable)"; break;
				case STATUS_COMPLETE: impl = "Fully implemented"; break;
				default: assert (0);
			}
			snprintf (titlestr, 64, "About %s - gtkboard", menu_get_game_name());
#if GTK_MAJOR_VERSION > 1
			msgstr = g_strdup_printf ("<big><b>%s</b></big> <i>%s</i>\n<b>Status:</b> %s\n<b>URL:</b> <tt>http://gtkboard.sourceforge.net/games/%s</tt>",
#else
			msgstr = g_strdup_printf ("%s -- %s\nStatus: %s\nURL: http://gtkboard.sourceforge.net/games/%s",
#endif
					opt_game->name,
					game_single_player ? "Single player game" : "Two player game",
					impl, 
					opt_game->name
					);
			menu_show_dialog (titlestr, msgstr);
			g_free (msgstr);
			return;
		case MENU_DOC_RULES:
			snprintf (titlestr, 64, "%s rules - gtkboard", menu_get_game_name());
			if (game_doc_rules) msgstr = game_doc_rules;
			break;
		case MENU_DOC_STRATEGY:
			snprintf (titlestr, 64, "%s strategy - gtkboard", menu_get_game_name());
			if (game_doc_strategy) msgstr = game_doc_strategy;
			break;
		case MENU_DOC_HISTORY:
			snprintf (titlestr, 64, "%s history - gtkboard", menu_get_game_name());
			if (game_doc_history) msgstr = game_doc_history;
			break;
		default:
			assert (0);
	}

	vbar = gtk_vscrollbar_new(NULL);
#if GTK_MAJOR_VERSION == 1
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW (dialog), titlestr);
	hbox = gtk_hbox_new (FALSE, 0);
	msgarea = gtk_text_new (NULL, 
			gtk_range_get_adjustment (GTK_RANGE (vbar)));
	gtk_text_set_word_wrap (GTK_TEXT (msgarea), TRUE);
	gtk_text_insert (GTK_TEXT (msgarea), NULL, NULL, NULL, msgstr, -1);
	gtk_box_pack_start (GTK_BOX (hbox), msgarea, TRUE, TRUE, 10);
	gtk_box_pack_start (GTK_BOX (hbox), vbar, FALSE, FALSE, 0);
	okay_button = gtk_button_new_with_label("  OK  ");
	gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
			GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) dialog);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->action_area), 
			okay_button, FALSE, FALSE, 0);
	gtk_widget_grab_focus (okay_button);	// contributed by Paddu
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);

#else
	dialog = gtk_dialog_new_with_buttons (titlestr, GTK_WINDOW (main_window),
			0, GTK_STOCK_OK, GTK_RESPONSE_NONE, NULL);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 400, 300);
	g_signal_connect_swapped (GTK_OBJECT (dialog),
			"response", G_CALLBACK (gtk_widget_destroy),
			GTK_OBJECT (dialog));
	msgarea = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (msgarea), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (msgarea), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (msgarea), FALSE);
	gtk_text_buffer_set_text (gtk_text_view_get_buffer 
			(GTK_TEXT_VIEW (msgarea)), msgstr, -1);
	scrwin = gtk_scrolled_window_new (NULL,
		gtk_range_get_adjustment (GTK_RANGE(vbar)));
	gtk_container_add (GTK_CONTAINER (scrwin), msgarea);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), scrwin);
#endif
	gtk_widget_show_all (dialog);
}

void menu_put_level (char *level_name)
{
	gchar path[128] = "/Game/Levels/";
	strcat (path, level_name);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
			(gtk_item_factory_get_widget (menu_factory, path)), TRUE);
}

gchar *menu_get_game_name ()
{
	if (game_levels) return game_levels[0].game->name;
	else return opt_game->name;
}

gchar *menu_get_game_name_with_level ()
{
	static gchar name[128];
	if (game_levels)
	{
		gchar *tempstr;
		GameLevel *level = game_levels;
		while (level->game != opt_game) { assert (level->name); level++; }
		tempstr = g_strdup_printf ("%s (%s)", 
				game_levels[0].game->name, level->name);
		strncpy (name, tempstr, 127);
		return name;
	}
	else return opt_game->name;
}

void menu_set_level (gpointer data, guint which, GtkWidget *widget)
{
	Game *old_game;
	if (!GTK_CHECK_MENU_ITEM(widget)->active)
		return;
	if (!state_gui_active)
		return;
	assert (game_levels);
	old_game = game_levels[which].game;
	ui_terminate_game ();
	opt_game = old_game;
	gtk_label_set_text (GTK_LABEL (sb_game_label), 
			menu_get_game_name_with_level ());
	ui_start_game();
}

GtkWidget *menu_recent_widgets[NUM_RECENT_GAMES] = {NULL};


static void menu_recent_game_cb (GtkWidget *widget, gpointer gamename)
{
	int i;
	gchar *name = gamename;
	Game *old_game = opt_game;
	for (i=0; i<num_games; i++)
	{
		if (!strcasecmp (games[i]->name, name))
		{
			menu_cleanup_var_menus ();
			if (opt_game)
				ui_terminate_game ();
			opt_game = games[i];
			if (old_game == opt_game) menu_start_game ();
			else menu_put_game ();
		}
	}
}

void menu_insert_game_item (gchar *gamename, int position)
{
	GtkWidget * child;
	GtkMenu *game_menu;
	int offset = 1; // number of menu entries before start of recent items
	game_menu = (GtkMenu *) gtk_item_factory_get_widget (menu_factory, "/Game");
	assert (game_menu);
	child = gtk_menu_item_new_with_label (gamename);
	gtk_menu_shell_insert (GTK_MENU_SHELL (game_menu), child, position - 1 + offset);
	gtk_widget_show(GTK_WIDGET (child));
	menu_recent_widgets [position-1] = child;
	gtk_signal_connect (GTK_OBJECT (child), "activate", GTK_SIGNAL_FUNC (menu_recent_game_cb), (gpointer) gamename);
}

void menu_insert_recent_game (gchar *gamename)
{
	int i, j;
	gchar *tmp;
	gchar *tmpname;
	for (i=0; i<NUM_RECENT_GAMES; i++)
	{
		tmpname = prefs_get_config_val (tmp = g_strdup_printf ("recent_game_%d", i+1));
		g_free (tmp);
		if (!tmpname) break;
		if (!strcasecmp (gamename, tmpname))
			break;
	}
	
	if (tmpname)
	{
		GtkWidget *wid;
		gtk_widget_destroy (menu_recent_widgets[i == NUM_RECENT_GAMES ? i - 1 : i]);
		menu_recent_widgets[i] = NULL;
		gtk_item_factory_delete_item (menu_factory, tmp = g_strdup_printf ("/Game/%s", tmpname));
		g_free (tmp);
	}
	
	for (j=i; j>0; j--)
	{
		if (j == NUM_RECENT_GAMES) continue;
		tmpname = prefs_get_config_val (tmp = g_strdup_printf ("recent_game_%d", j));
		g_free (tmp);
		prefs_set_config_val (tmp = g_strdup_printf ("recent_game_%d", j+1), tmpname);
		menu_recent_widgets [j] = menu_recent_widgets[j-1];
	}
	prefs_set_config_val ("recent_game_1",  gamename);

	menu_insert_game_item (gamename, 1);
}

void menu_start_game ()
{
	ui_start_game ();
	
	{
	int i;
	GtkItemFactoryEntry help_items [3];
	help_items[0].path = g_strdup_printf ("/Help/%s", opt_game->name);
	help_items[0].accelerator = NULL;
	help_items[0].callback = NULL;
	help_items[0].item_type = "<Branch>";
	gtk_item_factory_create_item (menu_factory, help_items, NULL, 1);
	
	help_items[0].path = g_strdup_printf ("/Help/%s/_About", opt_game->name); 
	help_items[0].callback_action = MENU_DOC_ABOUT;
	help_items[1].path = g_strdup_printf ("/Help/%s/_Rules", opt_game->name); 
	help_items[1].callback_action = MENU_DOC_RULES;
	help_items[2].path = g_strdup_printf ("/Help/%s/_Strategy", opt_game->name); 
	help_items[2].callback_action = MENU_DOC_STRATEGY;
	help_items[3].path = g_strdup_printf ("/Help/%s/_History", opt_game->name); 
	help_items[3].callback_action = MENU_DOC_HISTORY;
	for (i=0; i<4; i++)
	{
		help_items[i].accelerator = NULL;
		help_items[i].callback = menu_show_game_doc;
		help_items[i].item_type = "";
	}
	gtk_item_factory_create_items (menu_factory, 
			4, help_items, NULL);
	}
	
	gtk_label_set_text (GTK_LABEL (sb_game_label), 
			menu_get_game_name_with_level());

	if (game_levels)
	{
		int cnt, i;
		GtkItemFactoryEntry *level_items, level_item;
		for (cnt = 0; game_levels[cnt].name; cnt++)
			;
		level_items = g_new0 (GtkItemFactoryEntry, cnt);

/*		level_item.path = "/Game/Levels";
		level_item.accelerator = NULL;
		level_item.callback = NULL;
		level_item.item_type = "<Branch>";
		gtk_item_factory_create_item (menu_factory, &level_item, NULL, 1);
*/		
		gtk_widget_show (gtk_item_factory_get_item (menu_factory,
			"/Game/Levels"));
			
		for (i=0; i<cnt; i++)
		{
			level_items[i].path = g_strdup_printf ("/Game/Levels/%s",
					game_levels[i].name);
			level_items[i].callback_action = i;
			level_items[i].accelerator = NULL;
			level_items[i].callback = menu_set_level;
			level_items[i].item_type = i == 0 ? "<RadioItem>":  
				g_strdup_printf ("/Game/Levels/%s", game_levels[0].name);
			// FIXME: possible memory leak here
		}
		gtk_item_factory_create_items (menu_factory, 
				cnt, level_items, NULL);
	}

	menu_insert_recent_game (menu_get_game_name ());
}

void menu_cleanup_var_menus ()
{
	gchar *tempstr;
	
	if (opt_game)
	{
		gchar *name = menu_get_game_name();
		// FIXME: do we need to delete recursively?
		gtk_item_factory_delete_item (menu_factory, 
				tempstr = g_strdup_printf ("/Help/%s", name));
		g_free (tempstr);
	}

	if (game_levels)
	{
		gchar *tmp;
		int i;
		for (i = 0; game_levels[i].name; i++)
		{
			gtk_item_factory_delete_item (menu_factory, 
					tmp = g_strdup_printf ("/Game/Levels/%s", game_levels[i].name));
			g_free (tmp);
		}
		gtk_widget_hide (gtk_item_factory_get_widget (menu_factory, "/Game/Levels"));
	}
}

void menu_set_game (gpointer data, guint which, GtkWidget *widget)
{
	if (!GTK_CHECK_MENU_ITEM(widget)->active)
		return;
	if (!state_gui_active)
		return;
	g_assert (which >= 0 && which < num_games);
	
	menu_cleanup_var_menus ();
	if (opt_game)
		ui_terminate_game ();
	opt_game = games[which];
	menu_start_game ();
	sb_update ();
}


void menu_set_delay_cb (gpointer data, guint delay, GtkWidget *widget)
{
	if (!GTK_CHECK_MENU_ITEM(widget)->active)
		return;
	if (move_fout)
	{
		fprintf (move_fout, "MSEC_PER_MOVE %d\n", opt_delay = delay);
		fflush (move_fout);
	}
}


void menu_back_forw (gpointer data, guint what)
{
	byte *move;
	switch (what)
	{
		case MENU_BACK:
			if (!game_allow_back_forw) break;
			if (!opt_game) break;
			if (!game_allow_undo)
				ui_stopped = TRUE;
			if (move_fout)
			{
				fprintf (move_fout, "BACK_MOVE \n");
				fflush (move_fout);
			}
			move = move_fread_ack (move_fin);
			if (!move)
			{
				sb_error ("Initial position. Can't go back.", FALSE);
				break;
			}
			board_apply_refresh (move, NULL);
			if (!game_single_player)
				cur_pos.player = (cur_pos.player == WHITE ? BLACK : WHITE);
			cur_pos.num_moves --;
			if (game_single_player && !game_allow_undo)
			{
				if (!ui_cheated && game_scorecmp)
					sb_message ("You cheated! No highscore for this game.", FALSE);
				ui_cheated = TRUE;
			}
			// FIXME: there should be only one round of communication 
			// in which client gets both the move and who_won
			ui_check_who_won ();
			if (game_reset_uistate) game_reset_uistate();
			sb_update ();
			break;
		case MENU_FORW:
			if (!game_allow_back_forw) break;
			if (!opt_game) break;
			if (move_fout)
			{
				fprintf (move_fout, "FORW_MOVE \n");
				fflush (move_fout);
			}
			move = move_fread_ack (move_fin);
			if (!move)
			{
				sb_error ("Final position. Can't go forward.", FALSE);
				break;
			}
			board_apply_refresh (move, NULL);
			if (!game_single_player)
				cur_pos.player = (cur_pos.player == WHITE ? BLACK : WHITE);
			cur_pos.num_moves ++;
			ui_check_who_won ();
			if (game_reset_uistate) game_reset_uistate();
			sb_update ();
			break;
		default:
			assert (0);
	}
}

//! This function is no longer used. Eval function should be set only from the command line.
void menu_set_eval_function ()
{
	int i;
	GtkWidget *menu, *menuitem;
	GtkItemFactoryEntry heur_item;
	static HeurTab *oldtab = NULL;
	char *colors[2], **color, pathbuf[64];
	return;
	colors[0] = game_white_string;
	colors[1] = game_black_string;
	for (color = colors; color <= colors+1; color++)
	{
		if (oldtab)
		for (i=0; oldtab[i].name; i++)
		{
			char *path = g_strdup_printf ("/Settings/Eval function/%s/%s",
				*color, oldtab[i].name);
			gtk_item_factory_delete_item (menu_factory, path);
			g_free (path);
		}
		if (game_htab)
		for (i=0; game_htab[i].name; i++)
		{
			heur_item.path = g_strdup_printf ("/Settings/Eval function/%s/%s", 
					*color, game_htab[i].name);
			if (i == 0) strncpy (pathbuf, heur_item.path, 63);
			heur_item.accelerator = NULL;
			heur_item.callback = NULL;
			heur_item.callback_action = 0;
			heur_item.item_type = (i == 0 ? "<RadioItem>" : pathbuf);
			gtk_item_factory_create_item (menu_factory, &heur_item, NULL, 1);
			g_free (heur_item.path);
		}
	}
	oldtab = game_htab;
}

void menu_enable_sound_cb (gpointer data, guint what)
{
	sound_set_enabled (what == 0 ? FALSE : TRUE);
	sb_update ();
}

void sb_message_dialog_show (char *msg, gboolean error)
{
	GtkWidget *dialog;
#if GTK_MAJOR_VERSION > 1
	dialog = gtk_message_dialog_new (GTK_WINDOW (main_window), 
			GTK_DIALOG_MODAL,
			error ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			"%s", msg);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);		
#else
	menu_show_dialog ("Error - gtkboard", msg);
#endif
}	
void sb_error (char *msg, gboolean serious)
{
	if (!state_gui_active)
	{
		fprintf (stderr, "Fatal error: %s\n", msg);
		exit (2);
	}
	if (serious)
		//menu_show_dialog ("Error - gtkboard", msg);
		sb_message_dialog_show (msg, TRUE);
	else
		sb_messagebar_message (msg);
}

void sb_message (char *msg, gboolean serious)
{
	if (!state_gui_active)
		fprintf (stderr, "%s\n", msg);
	else
	{
		if (serious)
			//sb_error (msg, serious);
			sb_message_dialog_show (msg, FALSE);
		else
			sb_messagebar_message (msg);
	}
}

gchar *sb_ftime(int temps)
{
	static gchar ftime[10] = "  :  : ";
	ftime[6] = temps % 10 + '0';
	temps /= 10;
	ftime[4] = temps % 10 + '0';
	temps /= 10;
	ftime[3] = temps % 6 + '0';
	temps /= 6;
	ftime[1] = temps % 10 + '0';
	temps /= 10;
	ftime[0] = temps % 6 + '0';
	temps /= 6;
	return ftime;
}

static int sb_human_time = 0;

void sb_reset_human_time ()
{
	gchar *tempstr;
	sb_human_time = 0;
	if (!state_gui_active)
		return;
	gtk_label_set_text (GTK_LABEL(sb_time_label), tempstr
			 = g_strdup_printf ("Time:%s", sb_ftime(sb_human_time)));
	g_free (tempstr);
}

int sb_get_human_time ()
{
	return sb_human_time;
}

static int sb_last_msg_time = -1;

void sb_messagebar_message (gchar *msg)
{
	sb_last_msg_time = 0;
	strncpy (sb_message_str, msg, SB_MESSAGE_STRLEN-1);
	sb_update ();
}

void menu_update ()
	// first call all the sens and then all the desens
{
	// FIXME: isn't there a more elegant way to do this?
	gboolean machine_thinking, machine_not_thinking,
		no_game, single_player, two_players,
		no_back_forw, eval_fn;
	machine_thinking = (!ui_stopped && player_to_play == MACHINE);
	machine_not_thinking = !machine_thinking;
	no_game = opt_game ? FALSE : TRUE;
	single_player = game_single_player;
	two_players = !single_player;
	no_back_forw = !game_allow_back_forw;
	eval_fn = game_htab ? FALSE : TRUE;

	if (!machine_thinking) menu_sensitize (MENU_SENS_MACHINE_THINKING, TRUE);
	if (!machine_not_thinking) menu_sensitize (MENU_SENS_MACHINE_NOT_THINKING, TRUE);
	if (!no_game) menu_sensitize (MENU_SENS_NO_GAME, TRUE);
	if (!single_player) menu_sensitize (MENU_SENS_SINGLE_PLAYER, TRUE);
	if (!two_players) menu_sensitize (MENU_SENS_TWO_PLAYERS, TRUE);
	if (!no_back_forw) menu_sensitize (MENU_SENS_NO_BACK_FORW, TRUE);
	if (!ui_stopped) menu_sensitize (MENU_SENS_UI_STOPPED, TRUE);
	if (!eval_fn) menu_sensitize (MENU_SENS_EVAL_FUNCTION, TRUE);

	if (machine_thinking) menu_sensitize (MENU_SENS_MACHINE_THINKING, FALSE);
	if (machine_not_thinking) menu_sensitize (MENU_SENS_MACHINE_NOT_THINKING, FALSE);
	if (no_game) menu_sensitize (MENU_SENS_NO_GAME, FALSE);
	if (single_player) menu_sensitize (MENU_SENS_SINGLE_PLAYER, FALSE);
	if (two_players) menu_sensitize (MENU_SENS_TWO_PLAYERS, FALSE);
	if (no_back_forw) menu_sensitize (MENU_SENS_NO_BACK_FORW, FALSE);
	if (ui_stopped) menu_sensitize (MENU_SENS_UI_STOPPED, FALSE);
	if (eval_fn) menu_sensitize (MENU_SENS_EVAL_FUNCTION, FALSE);

	// TODO: express this in data rather than code
	if (ui_stopped) 
	{
		gtk_widget_show (gtk_item_factory_get_widget (menu_factory,
			"/Game/Start"));
		gtk_widget_hide (gtk_item_factory_get_widget (menu_factory,
			"/Game/Pause"));
	}
	else
	{
		gtk_widget_hide (gtk_item_factory_get_widget (menu_factory,
			"/Game/Start"));
		gtk_widget_show (gtk_item_factory_get_widget (menu_factory,
			"/Game/Pause"));
	}

	if (sound_get_enabled ())
	{
		gtk_widget_show (gtk_item_factory_get_widget (menu_factory,
					"/Settings/Disable Sound"));
		gtk_widget_hide (gtk_item_factory_get_widget (menu_factory,
					"/Settings/Enable Sound"));
	}
	else
	{
		gtk_widget_hide (gtk_item_factory_get_widget (menu_factory,
					"/Settings/Disable Sound"));
		gtk_widget_show (gtk_item_factory_get_widget (menu_factory,
					"/Settings/Enable Sound"));
	}

	if (no_game)
	{
		gtk_widget_hide (menu_info_bar);
		gtk_widget_hide (menu_info_separator);
	}
	else
	{
		gtk_widget_show (menu_info_bar);
		gtk_widget_show (menu_info_separator);
	}
	if (game_levels)
		gtk_widget_show (gtk_item_factory_get_item (menu_factory, "/Game/Levels"));
	else
		gtk_widget_hide (gtk_item_factory_get_item (menu_factory, "/Game/Levels"));
	if (game_doc_about_status == STATUS_COMPLETE || game_doc_about_status == STATUS_NONE)
	{
		gtk_widget_hide (menu_warning_bar);
		gtk_widget_hide (sb_warning_separator);
	}
	else
	{
		gtk_widget_show (menu_warning_bar);
		gtk_widget_show (sb_warning_separator);
	}
	
}
	
//! NOTE: sb_update() is idempotent. When in doubt, call sb_update().
void sb_update ()
{
	char player[5] = "?/?";
	if (!state_gui_active) return;
	menu_update ();
	if (game_single_player)
	{
		gtk_widget_hide (sb_player_label);
		gtk_widget_hide (sb_who_label);
		gtk_widget_hide (sb_player_separator);
		gtk_widget_hide (sb_who_separator);
		gtk_widget_show (sb_time_separator);
		gtk_widget_show (sb_time_label);
	}
	else
	{
		gtk_widget_show (sb_player_label);
		gtk_widget_show (sb_who_label);
		gtk_widget_show (sb_player_separator);
		gtk_widget_show (sb_who_separator);
		gtk_widget_hide (sb_time_separator);
		gtk_widget_hide (sb_time_label);
		if (ui_white == HUMAN) player[0] = 'H';
		if (ui_white == MACHINE) player[0] = 'M';
		if (ui_black == HUMAN) player[2] = 'H';
		if (ui_black == MACHINE) player[2] = 'M';
		gtk_label_set_text (GTK_LABEL(sb_player_label), 
				ui_white != NONE ? player : "File");
		gtk_label_set_text (GTK_LABEL(sb_who_label), cur_pos.player == WHITE ? 
				game_white_string : game_black_string);
	}
	gtk_label_set_text (GTK_LABEL(sb_score_label), sb_score_str);
	sb_set_turn_image();
	sb_set_cursor ();
}

gboolean sb_update_periodic ()
{
	static gboolean first = TRUE;
	if (!state_gui_active) return TRUE;
	if (sb_message_str[0] != '\0' && sb_last_msg_time < 0)
		sb_last_msg_time = 0;
	if (sb_message_str[0] == '\0') sb_last_msg_time = -1;
	if (sb_last_msg_time >= 0) sb_last_msg_time++;
	if (sb_last_msg_time >= 30)
		sb_message_str[0] = '\0';
	gtk_label_set_text (GTK_LABEL(sb_message_label), sb_message_str);
	{
	gchar *tempstr = NULL;
	if (!ui_stopped && (player_to_play == HUMAN || game_single_player))
		gtk_label_set_text (GTK_LABEL(sb_time_label),
			 tempstr = g_strdup_printf ("Time:%s", sb_ftime(sb_human_time++)));
	if (first)
		gtk_label_set_text (GTK_LABEL(sb_time_label),
			 tempstr = g_strdup_printf ("Time:%s", sb_ftime(sb_human_time)));
	if (tempstr) g_free (tempstr);
	}
	first = FALSE;
	return TRUE;
}
