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
#ifndef _PREFS_H_
#define _PREFS_H_

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "game.h"

#define MAX_HIGHSCORES 10

typedef struct
{
	char name[32];
	char score[32];
	int time;
	int date;
} Score;


extern SCORE_FIELD prefs_score_fields_def[];
extern gchar* prefs_score_field_names_def[];

gboolean prefs_load_scores (gchar *);
gboolean prefs_save_scores (gchar *);
void prefs_show_scores ();
void prefs_zap_highscores ();
gboolean prefs_add_highscore (gchar *, int);
int prefs_scorecmp_dscore (gchar *, int, gchar*, int);
int prefs_scorecmp_iscore (gchar *, int, gchar*, int);
int prefs_scorecmp_time (gchar *, int, gchar*, int);
extern int (*game_scorecmp) (gchar *, int, gchar*, int);
#endif
