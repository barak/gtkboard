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
void prefs_add_highscore (gchar *, int);
int prefs_scorecmp_dscore (gchar *, int, gchar*, int);
int prefs_scorecmp_iscore (gchar *, int, gchar*, int);
int prefs_scorecmp_time (gchar *, int, gchar*, int);
extern int (*game_scorecmp) (gchar *, int, gchar*, int);
#endif
