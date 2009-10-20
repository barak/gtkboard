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
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "prefs.h"
#include "ui_common.h"
#include "sound.h"

#ifdef HAVE_GNOME
#include <libgnome/libgnome.h>
#endif

static Score scores[MAX_HIGHSCORES];
static int num_highscores = 0;
static gchar *gamename; // ugly

SCORE_FIELD prefs_score_fields_def[] = 
{SCORE_FIELD_USER, SCORE_FIELD_SCORE, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};

gchar* prefs_score_field_names_def[] = {"User", "Score", "Time", "Date", NULL};

ConfigVar prefs_config_vars[] = 
{
	{ "sound_dir", "Directory to load sounds from", NULL, NULL, NULL, NULL },
	{ "enable_sound", "Enable sound effects", NULL, "true", NULL, sound_enable_pref_cb },
	{ "recent_game_1", "Recent game 1", NULL, NULL, NULL, NULL },
	{ "recent_game_2", "Recent game 2", NULL, NULL, NULL, NULL },
	{ "recent_game_3", "Recent game 3", NULL, NULL, NULL, NULL },
	{ NULL} 
};


gboolean prefs_get_bool_val (gchar *value)
{
	if (!value) return FALSE;
	if (!strcasecmp (value, "false")) return FALSE;
	if (!strcasecmp (value, "no")) return FALSE;
	if (!strcasecmp (value, "0")) return FALSE;
	return TRUE;
}

static void prefs_strip_special_chars (gchar *str)
	// $ is the field separator in our scores file
	// '<' and '>' are used for markup in GtkLabel
{
	for (; *str; str++)
		if (*str == '$' || *str == '<' || *str == '>')
			*str = ' ';
}

static gboolean prefs_create_dir (gchar *dir)
{
	int retval;
	struct stat Stat;
	retval = stat (dir, &Stat);
	if (retval < 0)
	{
		// try to create the directory
		retval = mkdir (dir, 0755);
		if (retval < 0) return FALSE;
	}
	else if (!S_ISDIR (Stat.st_mode)) return FALSE;
	return TRUE;
}

static gboolean prefs_first_time ()
{
	static gboolean first_time = TRUE;
	static gboolean status = FALSE;
	gchar * prefsdir, *tempstr;
	if (!first_time) return status;
	first_time = FALSE;
	prefsdir = g_strdup_printf ("%s/%s", getenv("HOME"), ".gtkboard");
	if (!prefs_create_dir (prefsdir)) return FALSE;
	tempstr = g_strdup_printf ("%s/%s", prefsdir, "scores");
	if (!prefs_create_dir (tempstr)) return FALSE;
	g_free (tempstr);
	tempstr = g_strdup_printf ("%s/%s", prefsdir, "prefs");
	if (!prefs_create_dir (tempstr)) return FALSE;
	g_free (tempstr);
	tempstr = g_strdup_printf ("%s/%s", prefsdir, "plugins");
	if (!prefs_create_dir (tempstr)) return FALSE;
	g_free (tempstr);
	g_free (prefsdir);
	return status = TRUE;
}

gboolean prefs_load_scores (gchar *name)
{
	gchar *scorefile;
	FILE *in;
	char linebuf[128];
	int i;
	gamename = name;
	if (!prefs_first_time ()) return FALSE;
	scorefile = g_strdup_printf ("%s/%s/%s", getenv("HOME"), 
			".gtkboard/scores", name);
	in = fopen (scorefile, "r");
	if (!in) 
	{ 
		g_free (scorefile);		
		num_highscores = 0;
		return FALSE;
	}
	for (i=0; !feof (in) && i < MAX_HIGHSCORES;)
	{
		gchar **realline;
		gchar ** score_fields;
		/*
		if (fscanf (in, "%[^$]$%[^$]$%d$%d\n", 
				scores[i].name, scores[i].score, 
				&scores[i].time, &scores[i].date) == EOF)*/
		//scanf is buggy!!! - %[] shouldn't strip leading whitespace but does
		if (!fgets (linebuf, 128, in))
			break;
		realline = g_strsplit (linebuf, "#", -1);
		score_fields = g_strsplit (realline[0], "$", -1);
		if (!score_fields[0] || !score_fields[1])
		{
			g_strfreev (realline);
			g_strfreev (score_fields);
			continue;
		}
		if (!score_fields[2] || !score_fields[3])
		{
			sb_error ("Error loading scores file", FALSE);
			g_strfreev (realline);
			g_strfreev (score_fields);
			break;
		}
		strncpy (scores[i].name, score_fields[0], 31);
		strncpy (scores[i].score, score_fields[1], 31);
		scores[i].time = atoi(score_fields[2]);
		scores[i].date = atoi(score_fields[3]);
		g_strfreev (realline);
		g_strfreev (score_fields);
		i++;
	}
	num_highscores = i;
	fclose (in);
	g_free (scorefile);
	return TRUE;
}

static void prefs_show_scores_real (int index)
{
	int i, j;
	gchar tempstr [128], tempstr1[128];
	time_t temps;
	GtkWidget *dialog, *okay_button, *label, *hbox, *vboxes[5];
	
	snprintf (tempstr, 128, "%s highscores - gtkboard", gamename);

#if GTK_MAJOR_VERSION == 1
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW (dialog), tempstr);
	okay_button = gtk_button_new_with_label("OK");

	gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
			GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) dialog);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
			okay_button);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG(dialog)->vbox), 5);
	snprintf (tempstr, 128, "%s highscores", gamename);
	label = gtk_label_new (tempstr);
#else
	dialog = gtk_dialog_new_with_buttons (tempstr, GTK_WINDOW (main_window),
			GTK_DIALOG_MODAL, NULL);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 100);
	okay_button = gtk_dialog_add_button (GTK_DIALOG (dialog), 
			GTK_STOCK_OK, GTK_RESPONSE_NONE);
	g_signal_connect_swapped (GTK_OBJECT (dialog), 
			"response", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (dialog));
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG(dialog)->vbox), 5);
	snprintf (tempstr, 128, "<big>%s highscores</big>", gamename);
	label = gtk_label_new ("");
	gtk_label_set_markup (GTK_LABEL (label), tempstr);
#endif
	gtk_widget_grab_focus (okay_button);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	hbox = gtk_hbox_new (FALSE, 20);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);
	
	for (i=0; i<=num_highscores; i++)
	{
		int num_score_fields;
		gchar *strings[10];
		gboolean found = FALSE;
		for (j=0; !found && j < 10; j++)
		{
			switch (game_score_fields[j])
			{
				case SCORE_FIELD_NONE: found = TRUE; break;
				case SCORE_FIELD_RANK:
					   sprintf (tempstr, "%d", i);
					   strings[j] = i ? tempstr : game_score_field_names[j];
					   break;
				case SCORE_FIELD_USER:
						strings[j] = i ? scores[i-1].name : game_score_field_names[j];
						break;
				case SCORE_FIELD_SCORE:
						strings[j] = i ? scores[i-1].score : game_score_field_names[j];
						break;
				case SCORE_FIELD_TIME:
						strings[j] = i ? sb_ftime (scores[i-1].time) : game_score_field_names[j];
						break;
				case SCORE_FIELD_DATE:
						temps = scores[i-1].date;
						strncpy (tempstr1, ctime (&temps), 127);
						tempstr1 [strlen (tempstr1) - 1] = '\0';
						strings[j] = i ? tempstr1  : game_score_field_names[j];
						break;
				default:
						break;
			}
		}
		num_score_fields = j-1;
		for (j=0; j<num_score_fields; j++)
		{
			if (i == 0)
			{
				vboxes[j] = gtk_vbox_new (FALSE, 0);
				gtk_box_pack_start (GTK_BOX (hbox), vboxes[j], TRUE, TRUE, 0);
			}
			label = gtk_label_new ("");
#if GTK_MAJOR_VERSION > 1
			gtk_label_set_selectable (GTK_LABEL (label), TRUE);
			if (i > 0 && i-1 == index)
			{
				gchar *tempstr = g_strdup_printf 
					("<span foreground=\"blue\">%s</span>", strings[j]);
				gtk_label_set_markup (GTK_LABEL (label), tempstr);
				g_free (tempstr);
			}
			else
#endif
				gtk_label_set_text (GTK_LABEL (label), strings[j]);
			gtk_box_pack_start (GTK_BOX (vboxes[j]), label, FALSE, FALSE, 
					i ? 0 : 5);
		}
	}
	gtk_widget_show_all (dialog);
}

void prefs_show_scores ()
{
	if (!gamename)
		return;
	prefs_show_scores_real (-1);
}

gboolean prefs_save_scores (gchar *name)
{
	gchar *scorefile;
	FILE *out;
	char linebuf[128];
	int i;
	if (!prefs_first_time ()) return FALSE;
	scorefile = g_strdup_printf ("%s/%s/%s", getenv("HOME"), 
			".gtkboard/scores", name);
	out = fopen (scorefile, "w");
	if (!out) 
	{ 
		gchar *tempstr;
		sb_error (tempstr = g_strdup_printf 
				("couldn't write to %s", scorefile),
				FALSE);
		fprintf (stderr, "%s\n", tempstr);
		g_free (tempstr);
		g_free (scorefile);		
		return FALSE;
	}
	fprintf (out, "#This is the gtkboard highscores file for the game %s\n"
			"#The format is: name$score$time taken in ms$date (time (2))\n"
			"#VERSION=0.10.0\n"
			, gamename);
	for (i=0; i < num_highscores; i++)
	{
		fprintf (out, "%s$%s$%d$%d\n", 
				scores[i].name, scores[i].score, 
				scores[i].time, scores[i].date);
	}
	fclose (out);
	g_free (scorefile);
	gamename = NULL;
	return TRUE;
}

int prefs_scorecmp_dscore (gchar *score1, int temps1, gchar* score2, int temps2)
{
	int s1 = atoi (score1);
	int s2 = atoi (score2);
	if (s1 > s2) return 1;
	if (s1 < s2) return -1;
	if (temps1 < temps2) return 1;
	if (temps1 > temps2) return -1;
	return 0;
}

int prefs_scorecmp_iscore (gchar *score1, int temps1, gchar* score2, int temps2)
{
	int s1 = atoi (score1);
	int s2 = atoi (score2);
	if (s1 < s2) return 1;
	if (s1 > s2) return -1;
	if (temps1 < temps2) return 1;
	if (temps1 > temps2) return -1;
	return 0;
}

int prefs_scorecmp_time (gchar *score1, int temps1, gchar* score2, int temps2)
{
	if (temps1 < temps2) return 1;
	if (temps1 > temps2) return -1;
	return 0;
}

static int  highscore_temps, highscore_index, highscore_date;
static gchar highscore_score[32];

static void prefs_username_cb (GtkWidget *dialog, GtkEntry *entry)
{
	int j;
	gchar username[32], *namep;
	strncpy (username, gtk_entry_get_text (entry), 31);
	prefs_strip_special_chars (username);
	for (j = num_highscores + 1; j > highscore_index; j--)
	{
		if (j >= MAX_HIGHSCORES) continue;
		memcpy (scores + j, scores + j - 1, sizeof (Score));
	}
	strncpy (scores[highscore_index].name, username, 31);
	snprintf (scores[highscore_index].score, 32, "%d", atoi (highscore_score));
	scores[highscore_index].time = highscore_temps;
	scores[highscore_index].date = highscore_date;
	if (num_highscores < MAX_HIGHSCORES) num_highscores++;
	gtk_widget_destroy (GTK_WIDGET (dialog));
	prefs_show_scores_real (highscore_index);
}

void prefs_show_username_dialog ()
{
	GtkWidget *dialog, *label, *hbox, *entry;
	gchar *title = "Enter your name - gtkboard";
	
	entry = gtk_entry_new_with_max_length (31);
	gtk_entry_set_text (GTK_ENTRY (entry), getenv ("USER"));
	gtk_widget_grab_focus (entry);

#if GTK_MAJOR_VERSION == 1
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (main_window));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_signal_connect_object (GTK_OBJECT (entry), "activate",
			GTK_SIGNAL_FUNC (prefs_username_cb), GTK_OBJECT (dialog));
#else
	dialog = gtk_dialog_new_with_buttons (title, GTK_WINDOW (main_window),
			GTK_DIALOG_MODAL, NULL);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 100);
//	g_signal_connect_swapped (GTK_OBJECT (entry),
//			"activate", G_CALLBACK (prefs_username_cb), GTK_OBJECT (dialog));
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_OK, GTK_RESPONSE_NONE);
#endif

	label = gtk_label_new ("You've got a highscore!");
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	hbox = gtk_hbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);
	label = gtk_label_new ("Enter your name");
	gtk_container_add (GTK_CONTAINER (hbox), label);
	gtk_container_add (GTK_CONTAINER (hbox), entry);
	gtk_widget_show_all (dialog);
#if GTK_MAJOR_VERSION > 1
	gtk_dialog_run (GTK_DIALOG (dialog));
	prefs_username_cb (dialog, GTK_ENTRY (entry));
#endif
}

gboolean prefs_add_highscore (gchar *score, int temps)
	// the argument score is the string returned by game_who_won. We expect
	// it to have a substring which is an integer
{
	int i, j;
	char *realscore = strpbrk (score, "0123456789");
	if (!realscore) realscore = "";
	if (!game_scorecmp) return FALSE;
	for (i=0; i<num_highscores; i++)
		if (game_scorecmp (realscore, temps, 
					scores[i].score, scores[i].time) > 0)
			break;
	if (i == MAX_HIGHSCORES) return FALSE;
	highscore_index = i;
	strncpy (highscore_score, realscore, 31);
	prefs_strip_special_chars (highscore_score);
	highscore_temps = temps;
	highscore_date = time (0);
	prefs_show_username_dialog ();
	return TRUE;
}

void prefs_zap_highscores ()
{
	gchar *tempstr;
	GtkWidget *dialog;
	gint result;
	if (!gamename) return;
#if GTK_MAJOR_VERSION == 1
	menu_show_dialog ("Zapped highscores",
			tempstr = g_strdup_printf ("Zapped %s highscores", gamename));
	g_free (tempstr);
#else
	dialog = gtk_message_dialog_new (
			GTK_WINDOW (main_window),
			GTK_DIALOG_MODAL, 
			GTK_MESSAGE_WARNING,
			GTK_BUTTONS_OK_CANCEL,
			"This will clear your %s highscores.\n"
		   "Your highscores in other games will be unaffected.\nProceed?", gamename);
	gtk_widget_show_all (dialog);
	result = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (result != GTK_RESPONSE_OK)
		return;
#endif
	num_highscores = 0;
}

gchar *prefs_get_config_val (gchar *key)
{
	ConfigVar *entry = prefs_config_vars;
	while (entry->key)
	{
		if (strcasecmp (entry->key, key) == 0)
		{
			if (entry->cur_val) return entry->cur_val;
			if (entry->def_val) return entry->def_val;
			return NULL;
		}
		entry++;
	}
	fprintf (stderr, "warning: prefs_get_config_val (): no such key: %s", key);
	return NULL;
}

void prefs_set_config_val_real (char *key, char *value, gboolean callback)
{
	ConfigVar *entry = prefs_config_vars;
	while (entry->key)
	{
		if (strcasecmp (entry->key, key) == 0)
		{
			// NOTE: here the old value should not be freed because it is static
			entry->cur_val = g_strdup (value);
			if (callback && entry->callback)
				entry->callback (key, value);
			return;
		}
		entry++;
	}
	fprintf (stderr, "Warning: invalid key %s in config file\n", key);
}

void prefs_set_config_val (char *key, char *value)
{
	prefs_set_config_val_real (key, value, FALSE);
}

void prefs_write_config_file ()
{
	ConfigVar *entry = prefs_config_vars;
	char *prefs_file;
	FILE *out;
	prefs_file = g_strdup_printf ("%s/%s", getenv ("HOME"), ".gtkboard/gtkboardrc");
	out = fopen (prefs_file, "w");
	if (!out)
	{
		fprintf (stderr, "Couldn't open config file %s for writing: ", prefs_file);
		perror (NULL);
		g_free (prefs_file);
		return;
	}
	g_free (prefs_file);
	
	while (entry->key)
	{
		if (entry->description)
			fprintf (out, "#%s\n", entry->description);
		if (entry->comment)
			fprintf (out, "#\n#%s\n", entry->comment);
		fprintf (out, "%s = %s\n", entry->key, entry->cur_val ? entry->cur_val : 
				(entry->def_val ? entry->def_val : ""));
		entry++;
	}
	
	fclose (out);
}

void prefs_read_config_file ()
{
	char linebuf[1024];
	char *prefs_file;
	FILE *in;
	int line_num = 0;
	if (!prefs_first_time ())
		return;
	prefs_file = g_strdup_printf ("%s/%s", getenv ("HOME"), ".gtkboard/gtkboardrc");
	in = fopen (prefs_file, "r");
	if (!in)
	{
		fprintf (stderr, "Couldn't open config file %s for reading: ", prefs_file);
		perror (NULL);
		g_free (prefs_file);
		return;
	}
	g_free (prefs_file);
	
	while (!feof (in))
	{
		gchar **tokens;
		line_num ++;
		if (fgets (linebuf, 1024, in) == NULL)
			continue;
		g_strstrip (linebuf);
		if (linebuf[0] == '#' || linebuf[0] == '\0')
			continue;
		tokens = g_strsplit (linebuf, "=", 2);
		if (tokens[0] == NULL || tokens[1] == NULL)
		{
			fprintf (stderr, "Warning: line %d of config file (%s) not of the form \"key = value\"\n",
				   line_num, linebuf);
			g_strfreev (tokens);
			continue;
		}
		g_strstrip (tokens[0]);
		g_strstrip (tokens[1]);
		prefs_set_config_val_real (tokens[0], tokens[1][0] ? tokens[1] : NULL, TRUE);
		g_strfreev (tokens);
	}
	
	fclose (in);
}
