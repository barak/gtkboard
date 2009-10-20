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
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <glib.h>

#ifndef G_MODULE_IMPORT 
#include <gmodule.h>		// Why isn't this included by glib.h??? 
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#ifdef HAVE_GNOME
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif

#include "config.h"
#include "ui.h"
#include "prefs.h"
#include "move.h"
#include "menu.h"
#include "ui_common.h"
#include "board.h"
#include "sound.h"

//! Default thinking time per move
#define DEF_TIME_PER_MOVE 2000

extern Game 
	Othello, Samegame, Rgb, Fifteen, Memory, 
	Tetris, Chess, Antichess, Hiq, Checkers, 
	Plot4, Maze, Infiltrate, Hypermaze, Ataxx, 
	Pentaline, Mastermind, Pacman, Flw, Wordtris,
	Ninemm, Stopgate, Knights, Breakthrough, 
	CapturePento, Towers, Quarto, Kttour, Eightqueens, Dnb,
	Blet, Othello6x6
	;

Game *games[] = { 
	&Chess, 
	&Antichess, 
	&Breakthrough, 
	
	&Pacman, 
	&Fifteen, 
	&Samegame, 
	&Tetris, 

	&Blet, 
	&Eightqueens, 
	&Towers,
	&Hiq,
	
	&Plot4, 
	&Quarto, 
	&Rgb, 
	&Pentaline,
	
	&Dnb, 
	&Stopgate, 
	&CapturePento, 
	&Knights, 

	&Othello, 
	&Othello6x6, 
	
	&Wordtris,
	&Flw, 
		
	&Maze, 
	&Hypermaze, 
	
	&Infiltrate, 
	&Kttour, 
	&Mastermind,
	&Ataxx, 
	&Checkers, 
	&Memory, 
	&Ninemm, 
		
};

const int num_games = sizeof (games) / sizeof (games[0]);

gboolean engine_flag = FALSE; // are we client or server. server will set it to TRUE

/* streams to communicate with engine */
FILE *move_fin, *move_fout;

static GIOChannel *ui_in = NULL;

Pos cur_pos = {NULL, NULL, NULL, WHITE, NULL, NULL, 0, 0};

int board_wid, board_heit;

static int engine_pid = -1;

static gint animate_tag = -1;

//int state_player = WHITE;

gboolean game_allow_undo = FALSE;

gboolean game_single_player = FALSE;
gboolean game_animation_use_movstack = TRUE;
gboolean game_allow_back_forw = TRUE;
int game_animation_time = 0;

gchar *game_doc_about = NULL;
gchar *game_doc_rules = NULL;
gchar *game_doc_strategy = NULL;
gchar *game_doc_history = NULL;
CompletionStatus game_doc_about_status = STATUS_NONE;
 

gchar *game_white_string = "White", *game_black_string = "Black";

gboolean ui_gameover = FALSE;
gboolean ui_stopped = TRUE;
gboolean ui_cheated = FALSE;
gboolean game_stateful = FALSE;
gboolean state_gui_active = FALSE;
gboolean game_draw_cell_boundaries = FALSE;
gboolean game_start_immediately = FALSE;
gboolean game_allow_flip = FALSE;
gboolean game_file_label = 0,  game_rank_label = 0;

char *game_highlight_colors = NULL;
char game_highlight_colors_def[9] = {0xff, 0xff, 0, 0, 0, 0, 0, 0, 0};

GameLevel *game_levels = NULL;
HeurTab *game_htab = NULL;
int game_state_size = 0;

SCORE_FIELD * game_score_fields = prefs_score_fields_def;
gchar **game_score_field_names = prefs_score_field_names_def;

char **game_bg_pixmap = NULL;

Game *opt_game = NULL;
FILE *opt_infile = NULL;
FILE *opt_logfile = NULL;
int opt_delay = DEF_TIME_PER_MOVE;
int opt_quiet = 0;
int opt_white = NONE;
int opt_black = NONE;
int ui_white = NONE;
int ui_black = NONE;
int opt_verbose = 0;
static gboolean opt_html_help = FALSE;

extern void engine_main (int, int);
extern ResultType engine_eval (Pos *, Player, float *);

gboolean impl_check ();

void ui_check_who_won ();
void game_set_init_pos_def (Pos *);
int ui_get_machine_move ();
void ui_make_human_move (byte *, int *);
void set_game_params ();

ResultType (*game_eval) (Pos *, Player, float *) = NULL;
ResultType (*game_eval_incr) (Pos *, byte *, float *) = NULL;
gboolean (*game_use_incr_eval) (Pos *) = NULL;
float (*game_eval_white) (Pos *, int) = NULL;
float (*game_eval_black) (Pos *, int) = NULL;
void (*game_search) (Pos *, byte **) = NULL;
byte * (*game_movegen) (Pos *) = NULL;
InputType (*game_event_handler) (Pos *, GtkboardEvent *, MoveInfo *) = NULL;
int (*game_getmove) (Pos *, int, int, GtkboardEventType, Player, byte **, int **) = NULL;
int (*game_getmove_kb) (Pos *, int, byte **, int **) = NULL;
ResultType (*game_who_won) (Pos *, Player, char **) = NULL;
int (*game_animate) (Pos *, byte **) = NULL;
char **( *game_get_pixmap) (int, int) = NULL;
guchar *( *game_get_rgbmap) (int, int) = NULL;
void (*game_free) () = NULL;
void * (*game_newstate) (Pos *, byte *) = NULL;
void (*game_set_init_pos) (Pos *) = game_set_init_pos_def;
void (*game_set_init_render) (Pos *) = NULL;
void (*game_get_render) (Pos *, byte *, int **) = NULL;
void (*game_reset_uistate) () = NULL;
int (*game_scorecmp) (gchar *, int, gchar*, int) = NULL;
int (*game_scorecmp_def_dscore) (gchar *, int, gchar*, int) = prefs_scorecmp_dscore;
int (*game_scorecmp_def_iscore) (gchar *, int, gchar*, int) = prefs_scorecmp_iscore;
int (*game_scorecmp_def_time) (gchar *, int, gchar*, int) = prefs_scorecmp_time;

GtkWidget *main_window, *board_area = NULL;
GtkWidget *board_rowbox = NULL, *board_colbox = NULL;

static void ignore() {}
static void html_help_gen ();

void ui_cleanup ()
{
	if (opt_game)
		ui_terminate_game();
	sound_stop ();
	prefs_write_config_file ();
	signal (SIGCHLD, ignore);
	if (engine_pid > 0)
		kill (engine_pid, SIGKILL);
	if (opt_verbose)
		printf ("gtkboard: normal exit.\n");
	exit (0);
}

void ui_segv_cleanup ()
{
	signal (SIGCHLD, ignore);
	if (engine_pid > 0)
		kill (engine_pid, SIGKILL);
	fprintf (stderr, "gtkboard: caught segv, exiting.\n");
	exit (1);
}


void ui_child_cleanup ()
{
	int status;
	waitpid (engine_pid, &status, WNOHANG | WUNTRACED);
	if (!WIFSIGNALED (status))
		return;
	if (WTERMSIG (status) == SIGSEGV)
	{
		fprintf (stderr, "gtkboard: engine appears to have died, exiting.\n");
		exit (1);
	}
}


int ui_animate_cb ()
{
	int ret;
	byte *move;
	if (ui_stopped) return TRUE;
	if (!game_animate) return FALSE;
	if (game_animate (&cur_pos, &move) > 0)
	{
		if (game_animation_use_movstack)
			ui_make_human_move (move, NULL);
		else
			board_apply_refresh (move, NULL);
	}
	return TRUE;
}

// Will be called on both ui and engine
void game_set_init_pos_def (Pos *pos)
{
	int x, y;

	for (x=0; x<board_wid; x++)
		for (y=0; y<board_heit; y++)
			pos->board[y * board_wid + x] = 
				opt_game->init_pos ? 
				opt_game->init_pos [(board_heit -1 - y) * board_wid + x] : 0;
}


// Will be called on both ui and engine
void reset_game_params ()
{
	if (game_free) game_free ();
	game_levels = NULL;
	game_htab = NULL;
	game_eval = NULL;
	game_eval_incr = NULL;
	game_use_incr_eval = NULL;
	game_eval_white = NULL;
	game_eval_black = NULL;
	game_search = NULL;
	game_movegen = NULL;
	game_event_handler = NULL;
	game_getmove = NULL;
	game_getmove_kb = NULL;
	game_who_won  = NULL;
	game_get_pixmap = NULL;
	game_get_rgbmap = NULL;
	game_set_init_pos = game_set_init_pos_def;
	game_set_init_render = NULL;
	game_get_render = NULL;
	game_animate = NULL;
	game_free = NULL;
	game_scorecmp = NULL;
	game_stateful = FALSE;
	game_animation_use_movstack = TRUE;
	game_allow_back_forw = TRUE;
	game_single_player = FALSE;
	game_allow_undo = FALSE;
	game_doc_about_status = STATUS_NONE;
	game_doc_about = NULL;
	game_doc_rules = NULL;
	game_doc_strategy = NULL;
	game_doc_history = NULL;
	game_white_string = "White";
	game_black_string = "Black";
	//state_player = WHITE;
	// TODO: replace state_player by cur_pos.player globally
	cur_pos.player = WHITE;
	game_state_size = 0;
	game_newstate = NULL;
	game_reset_uistate = NULL;
	game_highlight_colors = game_highlight_colors_def;
	game_draw_cell_boundaries = FALSE;
	game_start_immediately = FALSE;
	game_allow_flip = FALSE;
	game_file_label = FILERANK_LABEL_TYPE_NONE;
	game_rank_label = FILERANK_LABEL_TYPE_NONE;
	game_score_fields = prefs_score_fields_def;
	game_score_field_names = prefs_score_field_names_def;
	game_bg_pixmap = NULL;
	if (cur_pos.board) free (cur_pos.board);
	if (cur_pos.render) free (cur_pos.render);
	cur_pos.game = NULL;
	cur_pos.board = NULL;
	cur_pos.render = NULL;
	cur_pos.state = NULL;
	cur_pos.ui_state = NULL;
	cur_pos.num_moves = 0;
	cur_pos.search_depth = 0;
}

void ui_terminate_game ()
{
	// FIXME: are we sure -1 is an invalid value?
	if (animate_tag >= 0)
	{
		gtk_timeout_remove (animate_tag);
		animate_tag = -1;
	}
	if (game_single_player)
		prefs_save_scores (menu_get_game_name_with_level());
	board_free ();
	reset_game_params ();
	if (opt_infile)
	{
		fclose (opt_infile);
		opt_infile = NULL;
	}
	if (game_reset_uistate) game_reset_uistate();
	sb_reset_human_time ();
	sb_update();
	ui_stopped = TRUE;
	ui_cheated = FALSE;
}

void ui_start_game ()
{
	cur_pos.game = opt_game;
	if (opt_game->game_init)
		opt_game->game_init(opt_game);
	if (game_single_player)
	{
		ui_white = HUMAN;
		ui_black = HUMAN;
	}
	else if (opt_infile)
	{
		ui_white = NONE;
		ui_black = NONE;
	}
	else
	{
		ui_white = HUMAN;
		ui_black = MACHINE;
	}
	set_game_params ();
	board_set_game_params (); // FIXME: this is ugly
	if (game_animate) // FIXME: shouldn't this be done somewhere else?
		animate_tag = gtk_timeout_add (game_animation_time, ui_animate_cb, NULL);
	board_init ();
	board_redraw_all ();
	menu_put_player (FALSE);
	menu_set_eval_function ();
	if (game_single_player)
		prefs_load_scores (menu_get_game_name_with_level());
	ui_check_who_won();
	if (game_single_player && game_start_immediately)
		ui_stopped = FALSE;
	if (state_gui_active)
	{
		gchar *tempstr = g_strdup_printf ("%s - gtkboard", menu_get_game_name());
		gtk_window_set_title (GTK_WINDOW (main_window), tempstr);
		g_free (tempstr);
	}
}


//! game specific initialization
// FIXME: fork this into 2 functions for client and server
void set_game_params ()
{
	Game *game = opt_game;
	if (!game) return;
	board_wid = game->board_wid;
	board_heit = game->board_heit;

	cur_pos.board = (byte *) malloc (board_wid * board_heit);
	assert (cur_pos.board);

	if (!engine_flag)
	{
		cur_pos.render = (int *) malloc (sizeof (int) * board_wid * board_heit);
		memset (cur_pos.render, 0, sizeof (int) * board_wid * board_heit);
		assert (cur_pos.render);
	}
	
	if (engine_flag) 
		// server always executes this
		game_set_init_pos (&cur_pos);
	else
	{
		if (game_set_init_pos == game_set_init_pos_def) 
		// client executes only if it is the default
		game_set_init_pos (&cur_pos);

		if (game_set_init_render)
			game_set_init_render (&cur_pos);
	}
	
	if (!engine_flag)	
		if (move_fout)
		{
			fprintf (move_fout, "NEW_GAME %s\n", game->name);
			fflush (move_fout);
			// read the initial position
			if (game_set_init_pos != game_set_init_pos_def)
				fread (cur_pos.board, board_wid * board_heit, 1, move_fin);
			fprintf (move_fout, "MSEC_PER_MOVE %d\n", opt_delay);
			fflush (move_fout);
		}
}

void ui_check_who_won()
{
	char *line, *who_str = NULL;
	int who, len;
	if (!move_fout)
		return;
	fprintf (move_fout, "WHO_WON \n");
	fflush (move_fout);
	line = line_read(move_fin);
	if (g_strncasecmp(line, "ACK", 3))
	{
		// NAK ==> not implemented
		ui_gameover = FALSE;
		sb_set_score ("");
		return;
	}
	line += 4;
	line = g_strstrip(line);
	who_str = line;
	while(!isspace(*line) && *line) line++;
	while(isspace(*line)) line++;
	sb_set_score (line);
	if (!g_strncasecmp(who_str, "NYET", 4))
	{
		ui_gameover = FALSE;
		return;
	}
	ui_stopped = TRUE;
	ui_gameover = TRUE;
	if (opt_logfile)
		fprintf(opt_logfile, "RESULT: %s\n", who_str);
	if (!state_gui_active)
		ui_cleanup();
	sb_update ();
	if (game_single_player && !ui_cheated && !g_strncasecmp(who_str, "WON", 3))
	{
		gboolean retval;
		retval = prefs_add_highscore (line, sb_get_human_time ());
		if (retval)
			sound_play (SOUND_HIGHSCORE);
		else 
			sound_play (SOUND_WON);
		if (game_levels)
		{
			GameLevel *next_level = game_levels;
			while (next_level->name)
			{
				if (next_level->game == opt_game)
					break;
				next_level++;
			}
			next_level++;
			if (next_level->name)
				menu_put_level (next_level->name);
		}
	}
	if (game_single_player && !ui_cheated && !g_strncasecmp(who_str, "LOST", 4))
		sound_play (SOUND_LOST);
}

void ui_send_make_move ()
{
	if (ui_stopped)
		return;
	if (player_to_play == HUMAN)
		return;
	if (move_fout && player_to_play == MACHINE)
	{
		fprintf (move_fout, "MAKE_MOVE \n");
		fflush (move_fout);
	}

	if (opt_infile)
		g_timeout_add (opt_delay, ui_get_machine_move, NULL);

	else
		g_io_add_watch (ui_in, G_IO_IN, (GIOFunc) ui_get_machine_move, NULL);
}

gboolean ui_send_make_move_bg (gpointer data)
{
	ui_send_make_move ();
	return FALSE;
}

void ui_make_human_move (byte *move, int *rmove)
{
	board_apply_refresh (move, rmove);
	if (!move) return;
	if (move_fout)
	{
		fprintf (move_fout, "TAKE_MOVE ");
		move_fwrite (move, move_fout);
		if (opt_logfile)
			move_fwrite (move, opt_logfile);
	}
	if (!game_single_player)
	{
		cur_pos.player = (cur_pos.player == WHITE ? BLACK : WHITE);
	}
	cur_pos.num_moves ++;
	ui_check_who_won ();
	sb_update ();
	ui_send_make_move ();
}

int ui_get_machine_move ()
{
	byte *move;
	if (player_to_play == HUMAN || ui_stopped)
		return FALSE;
	if (!opt_infile)
	{
		move = move_fread_ack (move_fin);
		if (!move)
		{
			sb_error ("Couldn't make move\n", TRUE);
			ui_stopped = TRUE;
			sb_update ();
			return FALSE;
		}
		if (opt_logfile)
			move_fwrite (move, opt_logfile);
	}
	else 		// file mode
	{
		//TODO: should communicate the move to the engine
		move = move_fread (opt_infile);
		if (opt_logfile)
			move_fwrite (move, opt_logfile);
	}
	board_apply_refresh (move, NULL);
	if (!game_single_player)
		cur_pos.player = (cur_pos.player == WHITE ? BLACK : WHITE);
	cur_pos.num_moves ++;
	sound_play (SOUND_MACHINE_MOVE);
	ui_check_who_won ();
	sb_update ();
	ui_send_make_move ();
	return FALSE;
}

int ui_move_now_cb ()
{
	if (player_to_play == HUMAN || ui_stopped)
		return FALSE;
	g_assert (engine_pid >= 0);
	fprintf (move_fout, "MOVE_NOW \n");
	fflush (move_fout);
	ui_get_machine_move ();
	return FALSE;
}

void ui_cancel_move ()
{
	if (player_to_play != MACHINE) return;
	if (!opt_infile)
	{
		g_assert (engine_pid >= 0);
		fprintf (move_fout, "CANCEL_MOVE \n");
		fflush (move_fout);
	}
}

void ui_start_player ()
{
	int fd[2][2], ret, i;

	for (i=0; i<2; i++)
	{
		ret = pipe (fd[i]);
		assert (!ret);
	}
	
	/* fork a child to do the crunching */
	if (!(engine_pid = fork ()))
	{
		engine_main (fd[0][0], fd[1][1]);
		exit (0);
	}
	if (opt_verbose) printf ("forked engine pid = %d\n", engine_pid);
	move_fin = fdopen (fd[1][0], "r");
	move_fout = fdopen (fd[0][1], "w");
	ui_in = g_io_channel_unix_new (fd[1][0]);
}

gboolean impl_check ()
{
	/* check if reqd functions have been implemented */
	if (!opt_infile)
	{
		if (!game_single_player)
		if ((ui_white == MACHINE || ui_black == MACHINE)
				&& (!game_movegen || !game_eval) && !game_search)
			return FALSE;
		if ((ui_white == HUMAN || ui_black == HUMAN)
				&& !game_getmove && !game_getmove_kb && !game_event_handler)
			return FALSE;
	}
	return TRUE;
}


static void parse_opts (int argc, char **argv)
{
	char *wheur = NULL, *bheur = NULL;
	int c, i;
	while ((c = getopt (argc, argv, "g:G:d:f:l:p:w:b:Hqvh")) != -1)
	{
		switch (c)
		{
			case 'g':
				{
				int found = 0;
				for (i=0; i<num_games; i++)
					if (!strcasecmp (optarg, games[i]->name))
					{
						opt_game = games[i];
//						if (opt_game->game_init)
//							opt_game->game_init(opt_game);
						found = 1;
					}
				if (!found)
				{
					fprintf (stderr, "%s: no such game\n", optarg);
					exit(1);
				}
				}
				break;

			case 'G':
				{
				void *handle;
				char *error;
				Game **game;
				GModule *module;
				module = g_module_open (optarg, G_MODULE_BIND_LAZY);
				if (!module)
				{
					fprintf (stderr, 
							"Failed to load plugin from file \"%s\": %s\nTry specifying an absolute file name\n",
							optarg, g_module_error ());
					exit (1);
				}

				if (!g_module_symbol (module, "plugin_game", (gpointer *) &game))
				{
					fprintf (stderr, 
							"Failed to load plugin from file \"%s\": %s\n",
							optarg, g_module_error ());
					exit(1);
				}
				printf ("Successfully loaded game %s\n", (*game)->name);
				opt_game = *game;
//				if (opt_game->game_init)
//					opt_game->game_init(opt_game);
				}
				break;
			/* FIXME : make these long options */
			case 'w':
				wheur = optarg;
				break;
			case 'b':
				bheur = optarg;
				break;
				
			case 'p':
				switch (optarg[0])
				{
					case 'h':
						opt_white = HUMAN;
						break;
					case 'm':
						opt_white = MACHINE;
						break;
					default:
						printf ("player must be h(human) or m(machine)\n");
						exit (1);
				}
				switch (optarg[1])
				{
					case 'h':
						opt_black = HUMAN;
						break;
					case 'm':
						opt_black = MACHINE;
						break;
					default:
						printf ("player must be h(human) or m(machine)\n");
						exit (1);
				}
				break;
			case 'f':
				if (opt_white != NONE || opt_black != NONE)
				{
					printf ("can't specify -f with -p\n");
					exit (1);
				}
				opt_infile  = fopen (optarg, "r");
				if (!opt_infile)
				{
					fprintf (stderr, "can't open %s for reading\n", optarg);
					exit (1);
				}
				break;
			case 'l':
				opt_logfile  = fopen (optarg, "a");
				if (!opt_logfile)
				{
					fprintf (stderr, "can't open %s for writing\n", optarg);
					exit (1);
				}
				break;
			case 'd':
				opt_delay = atoi (optarg);
				if (opt_delay <= 0)
					opt_delay = 3000;
				break;
			case 'H':
				opt_html_help = TRUE;
				break;
			case 'q':
				opt_quiet = 1;
				break;
			case 'v':
				opt_verbose = 1;
				break;
			case 'h':
				printf ("Usage: gtkboard \t[-qvh] "
						"[-g game] [-G file] [-f file] [-l logfile] [-d msec]\n"
						"\t\t\t[-p XX] [-w wheur -b bheur] "
						"\n"
						"\t-g\tname of the game\n"
						"\t-G\tplugin file to load game from\n"
						"\t-f\tfile to load game from\n"
						"\t-l\tlog file to record game\n"
						"\t-q\tdon't show board\n"
						"\t-d\tdelay in milliseconds\n"
						"\t-p\thuman or machine players. Each X must be 'h' or 'm'\n"
						"\t-w\tname of heuristic function for white\n"
						"\t-b\tname of heuristic function for black\n"
						"\t-v\tbe verbose\n"
						"\t-h\tprint this help\n"
					   );
				exit (0);
			default:
				exit (1);
		}				

	}

	/* check sanity */
	if ((wheur && !bheur) || (bheur && !wheur))
	{
		fprintf (stderr, "specify heuristic for both players or neither\n");
		exit(1);
	}
	if (opt_infile && (opt_white != NONE || opt_black != NONE))
	{
		fprintf (stderr, "can't specify -f with -p\n");
		exit (1);
	}
	if (opt_quiet && (opt_white == HUMAN || opt_black == HUMAN))
	{
		fprintf (stderr, "can't be quiet with human players\n");
		exit (1);
	}
	if (game_single_player && (opt_infile))
	{
		fprintf (stderr, "can't load from file for single player game\n");
		exit (1);
	}
	if (opt_quiet && (opt_white != MACHINE || opt_black != MACHINE))
	{
		fprintf (stderr, "both white and black have to be machine for quiet mode.\n");
		exit (1);
	}
	if (opt_quiet && !opt_game)
	{
		fprintf (stderr, "game must be specified for quiet mode.\n");
		exit (1);
	}
	if (opt_quiet && !opt_logfile)
	{
		fprintf (stderr, "warning: no logfile specified in quiet mode.\n");
	}

	if (wheur && bheur)
	{
		int i = 0;
		if (!opt_game)
		{
			fprintf (stderr, "heur fn specified but no game specified\n");
			exit (1);
		}
		if (!game_htab)
		{
			fprintf (stderr, "no support for changing eval fn. in %s\n", opt_game->name);
			exit(1);
		}
		for (i=0; game_htab[i].name; i++)
		{
			if (!strcasecmp(wheur, game_htab[i].name))
				game_eval_white = game_htab[i].eval_fun;
			if (!strcasecmp(bheur, game_htab[i].name))
				game_eval_black = game_htab[i].eval_fun;
		}
		if (!game_eval_white)
		{
			fprintf (stderr, "%s: no such eval fn. in %s\n", wheur, opt_game->name);
			exit(1);
		}
		if (!game_eval_black)
		{
			fprintf (stderr, "%s: no such eval fn. in %s\n", bheur, opt_game->name);
			exit(1);
		}
		// FIXME: engine should parse opts separately
		game_eval = engine_eval;
	}

/*	if (game_single_player)
	{
		opt_white = HUMAN;
		opt_black = HUMAN;
	}
*/
	else if (!opt_infile)
	{
		// default is human vs. machine
		if (opt_white == NONE) opt_white = 	HUMAN;
		if (opt_black == NONE) opt_black = MACHINE;
	}
	ui_white = opt_white;
	ui_black = opt_black;

	if (opt_html_help)
	{
		html_help_gen ();
		exit (0);
	}
}

void gui_init ()
{
	GtkWidget *hbox = NULL, *vbox = NULL, *vbox1 = NULL, *frame = NULL;
	GtkWidget *separator;
	GtkAccelGroup *ag;
	GtkItemFactoryEntry game_items [num_games+1];
	GtkItemFactoryEntry items[] = 
	{
#if GTK_MAJOR_VERSION == 1
/*		{ "/_File", NULL, NULL, 0, "<Branch>" },
		{ "/File/_Load game", "<control>L", menu_load_file_dialog, 0, "" },
		{ "/File/_Save game", NULL, NULL, 0, "" },
		{ "/File/_Quit", "<control>Q", (GtkSignalFunc) ui_cleanup, 0, "" },
*/
		{ "/_Game", NULL, NULL, 0, "<Branch>" },
		{ "/Game/Select _Game", NULL, NULL, 0, "<LastBranch>" },
		{ "/Game/_Levels", NULL, NULL, 0, "<Branch>"},
		{ "/Game/Sep1", NULL, NULL, 0, "<Separator>" },
		{ "/Game/_New", "<control>N", menu_start_stop_game, MENU_RESET_GAME, "" },
		{ "/Game/_Start", "<control>G", menu_start_stop_game, MENU_START_GAME, "" },
		{ "/Game/_Pause", "<control>P", menu_start_stop_game, MENU_STOP_GAME, "" },
		{ "/Game/Sep2", NULL, NULL, 0, "<Separator>" },
		{ "/Game/_Highscores", NULL, prefs_show_scores, 0, ""},
		{ "/Game/_Zap Highscores", NULL, prefs_zap_highscores, 0, ""},
		{ "/Game/Sep3", NULL, NULL, 0, "<Separator>" },
		{ "/Game/_Quit", "<control>Q", (GtkSignalFunc) ui_cleanup, 0, "" },
		{ "/_Move", NULL, NULL, 0, "<Branch>" },
		{ "/Move/_Back", "<control>B", menu_back_forw, MENU_BACK, "" },
		{ "/Move/_Forward", "<control>F", menu_back_forw, MENU_FORW, "" },
		{ "/Move/Sep1", NULL, NULL, 0, "<Separator>" },
		{ "/Move/_Move Now", "<control>M", 
			(GtkItemFactoryCallback) ui_move_now_cb, 0, "" },
#else
/*		{ "/_File", NULL, NULL, 0, "<Branch>" },
		{ "/File/_Load game", "<control>L", menu_load_file_dialog, 0, 
				"<StockItem>", GTK_STOCK_OPEN },
		{ "/File/_Save game", NULL, menu_save_file_dialog, 0, 
				"<StockItem>", GTK_STOCK_SAVE },
		{ "/File/_Quit", "<control>Q", (GtkSignalFunc) ui_cleanup, 0, 
				"<StockItem>", GTK_STOCK_QUIT },
*/
		{ "/_Game", NULL, NULL, 0, "<Branch>" },
		{ "/Game/Select _Game", NULL, NULL, 0, "<LastBranch>" },
		{ "/Game/Levels", NULL, NULL, 0, "<Branch>"},
		{ "/Game/Sep1", NULL, NULL, 0, "<Separator>" },
		{ "/Game/_New", "<control>N", menu_start_stop_game, MENU_RESET_GAME, 
				"<StockItem>", GTK_STOCK_NEW },
		{ "/Game/_Start", "<control>G", menu_start_stop_game, MENU_START_GAME, 
				"<StockItem>", GTK_STOCK_YES  },
		{ "/Game/_Pause", "<control>P", menu_start_stop_game, MENU_STOP_GAME, 
				"<StockItem>", GTK_STOCK_STOP },
		{ "/Game/Sep2", NULL, NULL, 0, "<Separator>" },
		//FIXME: there's a scores stock item but I can't seem to find it
		{ "/Game/_Highscores", NULL, prefs_show_scores, 0, ""},
		{ "/Game/_Zap Highscores", NULL, prefs_zap_highscores, 0, ""},
		{ "/Game/Sep3", NULL, NULL, 0, "<Separator>" },
		{ "/Game/_Quit", "<control>Q", (GtkSignalFunc) ui_cleanup, 0, "" },
		{ "/_Move", NULL, NULL, 0, "<Branch>" },
		{ "/Move/_Back", "<control>B", menu_back_forw, 1, 
				"<StockItem>", GTK_STOCK_GO_BACK },
		{ "/Move/_Forward", "<control>F", menu_back_forw, 2, 
				"<StockItem>", GTK_STOCK_GO_FORWARD },
		{ "/Move/Sep1", NULL, NULL, 0, "<Separator>" },
		{ "/Move/_Move Now", "<control>M", 
			(GtkItemFactoryCallback) ui_move_now_cb, 0, "" },
#endif
		{ "/_Settings", NULL, NULL, 0, "<Branch>" },
		{ "/Settings/_Player", NULL, NULL, 0, "<Branch>" },
		{ "/Settings/Player/File", NULL, NULL, 0, "<RadioItem>" },
		{ "/Settings/Player/Human-Human", NULL, menu_set_player, 1, "/Settings/Player/File" },
		{ "/Settings/Player/Human-Machine", NULL, menu_set_player, 2, 
									"/Settings/Player/File" },
		{ "/Settings/Player/Machine-Human", NULL, menu_set_player, 3, 
									"/Settings/Player/File" },
		{ "/Settings/Player/Machine-Machine", NULL, menu_set_player, 4, 
									"/Settings/Player/File" },
//		{ "/Settings/_Eval function", NULL, NULL, 0, "<Branch>" },
//		{ "/Settings/_Eval function/_White", NULL, NULL, 0, "<Branch>" },
//		{ "/Settings/_Eval function/_Black", NULL, NULL, 0, "<Branch>" },
		{ "/Settings/_Flip Board", "<control>T", menu_board_flip_cb, 0, "" },
		{ "/Settings/_Enable Sound", NULL, menu_enable_sound_cb, 1, ""},
		{ "/Settings/_Disable Sound", NULL, menu_enable_sound_cb, 0, ""},
		{ "/Settings/_Time per move", NULL, NULL, 0, "<Branch>" },
		{ "/Settings/_Time per move/Default", NULL, 
			menu_set_delay_cb, DEF_TIME_PER_MOVE, "<RadioItem>" },
		{ "/Settings/_Time per move/100 milliseconds", NULL, 
			menu_set_delay_cb, 100, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/200 milliseconds", NULL, 
			menu_set_delay_cb, 200, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/500 milliseconds", NULL, 
			menu_set_delay_cb, 500, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/1 second", NULL, 
			menu_set_delay_cb, 1000, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/2 seconds", NULL, 
			menu_set_delay_cb, 2000, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/5 seconds", NULL, 
			menu_set_delay_cb, 5000, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/10 seconds", NULL, 
			menu_set_delay_cb, 10000, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/30 seconds", NULL, 
			menu_set_delay_cb, 30000, "/Settings/Time per move/Default" },
		{ "/Settings/Time per move/1 minute", NULL, 
			menu_set_delay_cb, 600000, "/Settings/Time per move/Default" },
		{ "/_Help", NULL, NULL, 0, "<Branch>" },
		{ "/Help/_About", NULL, menu_show_about_dialog, 0, ""},
#ifdef HAVE_GNOME
		{ "/Help/_Home Page", NULL, menu_help_home_page, 0, "<StockItem>", GTK_STOCK_HOME},
#endif
		// TODO: implement context help
//		{ "/Help/_Context help", NULL, ui_set_context_help, 0, ""},
	};
	int i;
	gdk_rgb_init ();
	main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_policy (GTK_WINDOW (main_window), FALSE, FALSE, TRUE);
	gtk_signal_connect (GTK_OBJECT (main_window), "delete_event",
		GTK_SIGNAL_FUNC(ui_cleanup), NULL);
	gtk_window_set_title (GTK_WINDOW (main_window), "Gtkboard");

	ag = gtk_accel_group_new();
	menu_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", ag);
	gtk_window_add_accel_group (GTK_WINDOW (main_window), ag);
			
	gtk_item_factory_create_items (menu_factory, 
			sizeof (items) / sizeof (items[0]), items, NULL);
	for (i=0; i<=num_games; i++)
	{
		if (i==0) 
			game_items[i].path = "/Game/Select Game/none";
		else 
		{
			if (games[i-1]->group)
			{
				GtkItemFactoryEntry group_item = {NULL, NULL, NULL, 0, "<Branch>"};
				group_item.path = g_strdup_printf ("/Game/Select Game/%s",
						games[i-1]->group);
				// FIXME: this is O(N^2) where N is the number of games
				if (gtk_item_factory_get_widget (menu_factory, group_item.path) == NULL)
					gtk_item_factory_create_item (menu_factory, &group_item, NULL, 1);
				game_items[i].path = g_strdup_printf ("/Game/Select Game/%s/%s",
					games[i-1]->group ? games[i-1]->group : "", games[i-1]->name);
			}
			else
				game_items[i].path = g_strdup_printf ("/Game/Select Game/%s",
						games[i-1]->name);
		}
		game_items[i].accelerator = NULL;
		game_items[i].callback = menu_set_game;
		game_items[i].callback_action = i-1;
		game_items[i].item_type = (i == 0 ? "<RadioItem>": "/Game/Select Game/none");
	}
	gtk_item_factory_create_items (menu_factory, 
			num_games+1, game_items, NULL);
	// ugly hack to create a group of radio button with no button selected by default
	gtk_item_factory_delete_item (menu_factory, "/Game/Select Game/none");

	menu_main = gtk_item_factory_get_widget (menu_factory, "<main>");
	gtk_widget_set_state (gtk_item_factory_get_widget (menu_factory, 
				"/Settings/Player/File"), GTK_STATE_INSENSITIVE);

	for (i=1; i<=NUM_RECENT_GAMES; i++)
	{
		gchar *tmp;
		gchar *gamename;
		gamename = prefs_get_config_val (tmp = g_strdup_printf ("recent_game_%d", i));
		g_free (tmp);
		if (gamename && gamename[0] != '\0')
			menu_insert_game_item (gamename, i);
	}

	menu_set_eval_function ();
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), menu_main, FALSE, FALSE, 0);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

	{
	GtkWidget *innerframe;
	board_colbox = gtk_vbox_new (FALSE, 0);
	board_area = gtk_drawing_area_new ();
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), board_colbox, FALSE, FALSE, 0);
	vbox1 = gtk_vbox_new (FALSE, 0);
	board_rowbox = gtk_hbox_new (FALSE, 0);
	innerframe = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (innerframe), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (vbox1), innerframe);
	gtk_container_add (GTK_CONTAINER (innerframe), board_area);
	gtk_container_add (GTK_CONTAINER (vbox1), board_rowbox);
	gtk_box_pack_start (GTK_BOX (hbox), vbox1, TRUE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	gtk_signal_connect (GTK_OBJECT (board_area), "expose_event",
		GTK_SIGNAL_FUNC (board_redraw), NULL);

   	gtk_widget_set_events(board_area, 
			gtk_widget_get_events (board_area) 
			|   GDK_BUTTON_PRESS_MASK
			|   GDK_BUTTON_RELEASE_MASK
			|   GDK_POINTER_MOTION_MASK
			|   GDK_KEY_PRESS_MASK
			|	GDK_KEY_RELEASE_MASK
			|	GDK_LEAVE_NOTIFY_MASK
			);

	gtk_signal_connect (GTK_OBJECT (board_area), "leave_notify_event",
		GTK_SIGNAL_FUNC (board_signal_handler), NULL);
	gtk_signal_connect (GTK_OBJECT (board_area), "motion_notify_event",
		GTK_SIGNAL_FUNC (board_signal_handler), NULL);
	gtk_signal_connect (GTK_OBJECT (board_area), "button_release_event",
		GTK_SIGNAL_FUNC (board_signal_handler), NULL);
	gtk_signal_connect (GTK_OBJECT (board_area), "button_press_event",
		GTK_SIGNAL_FUNC (board_signal_handler), NULL);
	gtk_signal_connect (GTK_OBJECT (main_window), "key_press_event",
		GTK_SIGNAL_FUNC (board_signal_handler), NULL);
	gtk_signal_connect (GTK_OBJECT (main_window), "key_release_event",
		GTK_SIGNAL_FUNC (board_signal_handler), NULL);
	menu_info_bar = hbox = gtk_hbox_new (FALSE, 0);
	sb_game_label = gtk_label_new (opt_game ? opt_game->name : NULL);
	gtk_box_pack_start (GTK_BOX (hbox), sb_game_label, FALSE, FALSE, 3);
	sb_game_separator = gtk_vseparator_new ();
	gtk_box_pack_start (GTK_BOX (hbox), sb_game_separator, FALSE, FALSE, 0);
	sb_player_label = gtk_label_new (NULL);
	gtk_box_pack_start (GTK_BOX (hbox), sb_player_label, FALSE, FALSE, 3);
	sb_player_separator = gtk_vseparator_new ();
	gtk_box_pack_start (GTK_BOX (hbox), sb_player_separator, FALSE, FALSE, 0);
	sb_who_label = gtk_label_new (NULL);
	gtk_box_pack_start (GTK_BOX (hbox), sb_who_label, FALSE, FALSE, 3);
	sb_who_separator = gtk_vseparator_new ();
	gtk_box_pack_start (GTK_BOX (hbox), sb_who_separator, FALSE, FALSE, 0);
	sb_score_label = gtk_label_new (NULL);
	gtk_box_pack_start (GTK_BOX (hbox), sb_score_label, FALSE, FALSE, 3);
	sb_score_separator = gtk_vseparator_new ();
	gtk_box_pack_start (GTK_BOX (hbox), sb_score_separator, FALSE, FALSE, 0);
#if GTK_MAJOR_VERSION == 2
	sb_turn_image = gtk_image_new_from_stock (GTK_STOCK_YES, GTK_ICON_SIZE_MENU);
	gtk_box_pack_end (GTK_BOX (hbox), sb_turn_image, FALSE, FALSE, 0);
	sb_turn_separator = gtk_vseparator_new ();
	gtk_box_pack_end (GTK_BOX (hbox), sb_turn_separator, FALSE, FALSE, 0);
#endif
	sb_time_label = gtk_label_new (NULL);
	gtk_box_pack_end (GTK_BOX (hbox), sb_time_label, FALSE, FALSE, 0);
	sb_time_separator = gtk_vseparator_new ();
	gtk_box_pack_end (GTK_BOX (hbox), sb_time_separator, FALSE, FALSE, 0);
	}
			
	menu_info_separator = separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
	menu_warning_bar = gtk_label_new ("Warning: this game has not yet been completely implemented.");
	gtk_box_pack_start (GTK_BOX (vbox), menu_warning_bar, FALSE, FALSE, 0);
	sb_warning_separator = separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
	sb_message_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (sb_message_label), 0, 0.5);
	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), sb_message_label, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (main_window), vbox);
	// FIXME: board_init() needs show() to be called to get a gc, but
	// leads to the whole window not popping up at once
	gtk_widget_show_all (main_window);
	
	if (!opt_game) board_init ();

	gtk_timeout_add (100, sb_update_periodic, NULL);

	// this should be called before setting state_gui_active = TRUE
	if (opt_game) menu_put_game (); 
	state_gui_active = TRUE;

	if (opt_game) menu_start_game ();
	menu_put_player (TRUE);
//	if (!opt_game) sb_message ("Select a game from the Game menu", FALSE);
	sb_update ();
}

void html_help_gen_format (FILE *fout, gchar *outfile, gchar *title, gchar *string)
{
	gchar *tmpfilename = "tmp";
	FILE *ftmp = fopen (tmpfilename, "w");
	gchar *command;
	if (!string) return;
	if (!ftmp)
	{
		fprintf (stderr, "couldn't open %s for writing", tmpfilename);
		perror (NULL);
		exit(1);
	}
	fprintf (ftmp, string);
	fclose (ftmp);
	fprintf (fout, "<h2> %s </h2>\n\n <pre>", title);
	fflush (fout);
	command = g_strdup_printf ("lynx -dump -dont_wrap_pre \"%s\" | fmt -s >> \"%s\"", tmpfilename, outfile);
	if (system (command) < 0)
	{
		fprintf (stderr, "failed to execute command %s: ", command);
		perror (NULL);
		exit (1);
	}
	fseek (fout, 0, SEEK_END);
	fprintf (fout, "\n</pre>\n");
	g_free (command);
}

void html_help_gen_game (Game *game)
{
	FILE *fout;
	gchar *filename;
	mkdir (filename = g_strdup_printf ("%s", game->name), 0777);
	g_free (filename);
	filename = g_strdup_printf ("%s/index.html", game->name);
	fout = fopen (filename, "w");
	if (!fout)
	{
		fprintf (stderr, "couldn't open %s for writing: ", filename);
		perror (NULL);
		g_free (filename);
		return;
	}
	reset_game_params ();
	opt_game = game;
	game->game_init(game);
	fprintf (fout, 
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
			"<html> \n <head> \n"
			"<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\"> \n"
			"<title>%s - gtkboard </title> \n"
			"<link rel=\"stylesheet\" type=\"text/css\" href=\"/styles/default.css\"> \n"
			"</head>\n"
			"<body>\n"
			"<!--#include virtual=\"/header.html\"-->\n",
			game->name);
	fprintf (fout, "<h1 align=\"center\"> %s </h1>\n\n", game->name);
	fprintf (fout, "<table width=\"100%%\"> \n <tr> \n <td width=\"2%%\"></td> \n <td width=\"75%%\" valign=\"top\"> \n");
	fprintf (fout, "%s<br>\n", game_single_player ? "Single player game" : "Two player game");
	html_help_gen_format (fout, filename, "Rules", game_doc_rules);
	html_help_gen_format (fout, filename, "Strategy", game_doc_strategy);
	html_help_gen_format (fout, filename, "History", game_doc_history);
	fprintf (fout, "<h2> Screenshot </h2>\n <p align=\"center\">" 
			"<img  src=\"/screenshots/%s default.png\" alt=\"%s screenshot\"/> </p>\n", 
			game->name, game->name);
	fprintf (fout, "</td> \n <td width=\"3%%\"></td> \n" 
			"<td width=\"20%%\" valign=\"top\"> \n <!--#include virtual=\"/gamelist.html\"-->\n" 
			"</td> \n</tr>\n </table>"
			"<hr/> \n<!--#include virtual=\"/footer.html\"-->\n"			
			"</body> \n</html>");
	g_free (filename);
	fclose (fout);
}

void html_help_gen_gamelist ()
{
	int i;
	static GSList *group_list = NULL;
	gchar *group;
	FILE *fout;
	fout = fopen ("gamelist.html", "w");
	if (!fout)
	{
		fprintf (stderr, "couldn't open gamelist.html for writing: ");
		perror (NULL);
		exit (1);
	}
	for (i=0; i<num_games; i++)
	{
		if (!games[i]->group) games[i]->group = "";
		if (!g_slist_find (group_list, games[i]->group))
			group_list = g_slist_append (group_list, games[i]->group);
	}

	fprintf (fout, "<h2> Games </h2> \n\n<ul>");
	while ((group = g_slist_nth_data (group_list, 0)))
	{
		if (group[0] != '\0')
			fprintf (fout, "<li> %s <ul>\n", group);
		for (i=0; i<num_games; i++)
		{
			if (strcmp (games[i]->group, group)) continue;
			fprintf (fout, "<li> <a href=\"/games/%s/\">%s</a> </li>\n", games[i]->name, games[i]->name);
		}
		if (group[0] != '\0')
			fprintf (fout, "</ul> </li>\n");
			
		group_list = g_slist_nth (group_list, 1);
	}
	fprintf (fout, "</ul>\n\n");
	fclose (fout);
}

void html_help_gen ()
{
	int i;
	char dirbuf[1024];
	getcwd (dirbuf, 1024);
	if (strcmp (basename (dirbuf), "games"))
	{
		fprintf (stderr, "To generate html help, you must be in the \"games\" directory.\n");
		exit (1);
	}
	if (opt_game)
	{
		html_help_gen_game (opt_game);
		return;
	}
	html_help_gen_gamelist ();
	for (i=0; i<num_games; i++)
		html_help_gen_game (games[i]);
}

static int get_seed ()
{
	GTimeVal timeval;
	g_get_current_time (&timeval);
	return timeval.tv_usec;
}
                         
//! A wrapper around sound_init so that we can return a value to g_idle_add
gboolean ui_sound_init (gpointer data)
{
	sound_init ();
	sound_play (SOUND_PROGRAM_START);
	return FALSE;
}

int main (int argc, char **argv)
{
#ifdef HAVE_GNOME
	GnomeProgram *app;
#endif
	srandom (get_seed());
	reset_game_params ();
	prefs_read_config_file ();
	parse_opts (argc, argv);
	ui_start_player ();
	
	signal (SIGINT, ui_cleanup);
	signal (SIGTERM, ui_cleanup);
	signal (SIGSEGV, ui_segv_cleanup);
	signal (SIGCHLD, ui_child_cleanup);
	if (!opt_quiet)
	{
		gtk_init(&argc,&argv);    
		gdk_rgb_init();
#ifdef HAVE_GNOME
		app = gnome_program_init (PACKAGE, VERSION, LIBGNOME_MODULE, argc, argv, GNOME_PARAM_NONE);
#endif
		gui_init ();
		g_idle_add ((GSourceFunc) ui_sound_init, NULL);
		gtk_main ();
	}
	else	// background mode
	{
		GMainLoop *loop;
		signal (SIGHUP, ignore);
		set_game_params ();
		ui_stopped = FALSE;
		sound_set_enabled (FALSE);
		g_idle_add (ui_send_make_move_bg, NULL);
#if GLIB_MAJOR_VERSION > 1
		loop = g_main_loop_new (NULL, TRUE);
#else
		loop = g_main_new (TRUE);
#endif
		g_main_run (loop);
		
	}
	return 0;
}
