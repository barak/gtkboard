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
#include <dlfcn.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "config.h"
#include "ui.h"
#include "prefs.h"
#include "move.h"
#include "menu.h"
#include "ui_common.h"
#include "board.h"

extern Game 
	Othello, Samegame, Rgb, Fifteen, Memory, 
	Tetris, Chess, Antichess, Hiq, Checkers, 
	Plot4, Maze, Infiltrate, Hypermaze, Ataxx, 
	Pentaline, Mastermind, Pacman
	;

// TODO: these should be sorted at runtime instead of by hand
Game *games[] = { 
	&Antichess, &Ataxx, &Checkers, &Chess, &Fifteen, &Hiq, 
	&Hypermaze, &Infiltrate, &Mastermind, &Maze, &Memory, &Othello,
	&Pacman, &Pentaline, &Plot4, &Rgb, &Samegame, &Tetris, };

const int num_games = sizeof (games) / sizeof (games[0]);

gboolean engine_flag = FALSE; // are we client or server. server will set it to TRUE

/* streams to communicate with engine */
FILE *move_fin, *move_fout;

Pos cur_pos = {NULL, NULL, 0};

int board_wid, board_heit;

static int engine_pid = -1;

static gint animate_tag = -1;

int state_player = WHITE;

gboolean game_single_player = FALSE;
gboolean game_animation_use_movstack = TRUE;
gboolean game_allow_back_forw = TRUE;
int game_animation_time = 0;

gchar *game_doc_about = NULL;
gchar *game_doc_rules = NULL;
gchar *game_doc_strategy = NULL;

gboolean ui_gameover = FALSE;
gboolean ui_stopped = TRUE;
gboolean ui_cheated = FALSE;
gboolean game_stateful = FALSE;
gboolean state_gui_active = FALSE;
gboolean game_draw_cell_boundaries = FALSE;
gboolean game_start_immediately = FALSE;

HeurTab *game_htab = NULL;
int game_state_size = 0;

SCORE_FIELD * game_score_fields = prefs_score_fields_def;
gchar **game_score_field_names = prefs_score_field_names_def;

Game *opt_game = NULL;
FILE *opt_infile = NULL;
FILE *opt_logfile = NULL;
int opt_delay = 1000;
int opt_quiet = 0;
int opt_white = NONE;
int opt_black = NONE;
int ui_white = NONE;
int ui_black = NONE;
int opt_verbose = 0;

extern void engine_main (int, int);
extern float engine_eval (Pos *, Player);

gboolean impl_check ();

void ui_check_who_won ();
void game_setinitpos_def (Pos *);
int ui_get_machine_move ();
void ui_make_human_move (byte *);
void set_game_params ();

float (*game_eval) (Pos *, Player) = NULL;
float (*game_eval_incr) (Pos *, Player, byte *) = NULL;
float (*game_eval_white) (Pos *, int) = NULL;
float (*game_eval_black) (Pos *, int) = NULL;
byte * (*game_movegen) (Pos *, Player) = NULL;
int (*game_getmove) (Pos *, int, int, GtkboardEventType, Player, byte **) = NULL;
int (*game_getmove_kb) (Pos *, int, Player, byte **) = NULL;
ResultType (*game_who_won) (Pos *, Player, char **) = NULL;
int (*game_animate) (Pos *, byte **) = NULL;
char **( *game_get_pixmap) (int, int) = NULL;
guchar *( *game_get_rgbmap) (int, int) = NULL;
void (*game_free) () = NULL;
void * (*game_newstate) (Pos *, byte *) = NULL;
void (*game_setinitpos) (Pos *) = game_setinitpos_def;
void (*game_reset_uistate) () = NULL;
int (*game_scorecmp) (gchar *, int, gchar*, int) = NULL;
int (*game_scorecmp_def_dscore) (gchar *, int, gchar*, int) = prefs_scorecmp_dscore;
int (*game_scorecmp_def_iscore) (gchar *, int, gchar*, int) = prefs_scorecmp_iscore;
int (*game_scorecmp_def_time) (gchar *, int, gchar*, int) = prefs_scorecmp_time;

GtkWidget *main_window, *board_area = NULL;
GtkWidget *board_rowbox = NULL, *board_colbox = NULL;

void ui_cleanup ()
{
	if (opt_game)
		ui_terminate_game();
	if (engine_pid > 0)
		kill (engine_pid, SIGKILL);
	if (opt_verbose)
		printf ("gtkboard: normal exit.\n");
	exit (0);
}

void ui_segv_cleanup()
{
	if (engine_pid > 0)
		kill (engine_pid, SIGKILL);
	fprintf (stderr, "gtkboard: caught segv, exiting.\n");
	exit (1);
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
			ui_make_human_move (move);
		else
			move_apply_refresh (cur_pos.board, move);
	}
	return TRUE;
}

// Will be called on both ui and engine
void game_setinitpos_def (Pos *pos)
{
	int x, y;

	for (x=0; x<board_wid; x++)
		for (y=0; y<board_heit; y++)
			pos->board[y * board_wid + x] = 
				opt_game->initpos [(board_heit -1 - y) * board_wid + x];
}


// Will be called on both ui and engine
void reset_game_params ()
{
	if (game_free) game_free ();
	game_htab = NULL;
	game_eval = NULL;
	game_eval_incr = NULL;
	game_eval_white = NULL;
	game_eval_black = NULL;
	game_movegen = NULL;
	game_getmove = NULL;
	game_getmove_kb = NULL;
	game_who_won  = NULL;
	game_get_pixmap = NULL;
	game_get_rgbmap = NULL;
	game_setinitpos = game_setinitpos_def;
	game_animate = NULL;
	game_free = NULL;
	game_scorecmp = NULL;
	game_stateful = FALSE;
	game_animation_use_movstack = TRUE;
	game_allow_back_forw = TRUE;
	game_single_player = FALSE;
	game_doc_about = NULL;
	game_doc_rules = NULL;
	game_doc_strategy = NULL;
	state_player = WHITE;
	game_state_size = 0;
	game_newstate = NULL;
	game_reset_uistate = NULL;
	game_draw_cell_boundaries = FALSE;
	game_start_immediately = FALSE;
	game_score_fields = prefs_score_fields_def;
	game_score_field_names = prefs_score_field_names_def;
	if (cur_pos.board) free (cur_pos.board);
	cur_pos.board = NULL;
	cur_pos.state = NULL;
	cur_pos.num_moves = 0;
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
		prefs_save_scores (opt_game->name);
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
	if (opt_game->game_init)
		opt_game->game_init();
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
		prefs_load_scores (opt_game->name);
	ui_check_who_won();
	if (game_single_player && game_start_immediately)
		ui_stopped = FALSE;
}


//! game specific initialization
// FIXME: fork this into 2 functions for client and server
void set_game_params ()
{
	Game *game = opt_game;
	if (!game) return;
	board_wid = game->board_wid;
	board_heit = game->board_heit;

	cur_pos.board = (char *) malloc (board_wid * board_heit);
	assert (cur_pos.board);

	if (engine_flag) 
		// server always executes this
		game_setinitpos (&cur_pos);
	else if (game_setinitpos == game_setinitpos_def) 
		// client executes only if it is the default
		game_setinitpos (&cur_pos);
	
	if (!engine_flag)	
		if (move_fout)
		{
			fprintf (move_fout, "NEW_GAME %s\n", game->name);
			fflush (move_fout);
			// read the initial position
			if (game_setinitpos != game_setinitpos_def)
				fread (cur_pos.board, board_wid * board_heit, 1, move_fin);
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
	printf ("%s\n", line);
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
	if (game_single_player && !ui_cheated)
		prefs_add_highscore (line, sb_get_human_time ());
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
	/* this should be called even for opt_infile */
	gtk_timeout_add (opt_delay, ui_get_machine_move, NULL);
}

void ui_make_human_move (byte *move)
{
	move_apply_refresh (cur_pos.board, move);
	if (move_fout)
	{
		fprintf (move_fout, "TAKE_MOVE ");
		move_fwrite (move, move_fout);
		if (opt_logfile)
			move_fwrite (move, opt_logfile);
	}
	if (!game_single_player)
	{
		state_player = (state_player == WHITE ? BLACK : WHITE);
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
		g_assert (engine_pid >= 0);
		fprintf (move_fout, "MOVE_NOW \n");
		fflush (move_fout);
		move = move_fread_ack (move_fin);
		g_assert (move);
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
	move_apply_refresh (cur_pos.board, move);
	state_player = (state_player == WHITE ? BLACK : WHITE);
	cur_pos.num_moves ++;
	ui_check_who_won ();
	sb_update ();
	ui_send_make_move ();
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
}

gboolean impl_check ()
{
	/* check if reqd functions have been implemented */
	if (!opt_infile)
	{
		if (!game_single_player)
		if ((ui_white == MACHINE || ui_black == MACHINE)
				&& (!game_movegen || !game_eval))
			return FALSE;
		if ((ui_white == HUMAN || ui_black == HUMAN)
				&& !game_getmove && !game_getmove_kb)
			return FALSE;
	}
	return TRUE;
}


static void parse_opts (int argc, char **argv)
{
	char *wheur = NULL, *bheur = NULL;
	int c, i;
	while ((c = getopt (argc, argv, "g:G:d:f:l:p:w:b:qvh")) != -1)
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
						if (opt_game->game_init)
							opt_game->game_init();
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
				handle = dlopen (optarg, RTLD_LAZY);
				if (!handle)
				{
					fprintf (stderr, 
							"Failed to load plugin from file \"%s\": %s\n",
							optarg, dlerror ());
					exit (1);
				}

				game = dlsym(handle, "plugin_game");
				if ((error = dlerror()) != NULL)
				{
					fprintf (stderr, 
							"Failed to load plugin from file \"%s\": %s\n",
							optarg, error);
					exit(1);
				}
				//dlclose(handle);
				opt_game = *game;
				if (opt_game->game_init)
					opt_game->game_init();
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
					opt_delay = 1000;
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
				assert (0);
		}				

	}

	/* check sanity */
	if ((wheur && !bheur) || (bheur && !wheur))
	{
		printf ("specify heuristic for both players or neither\n");
		exit(1);
	}
	if (opt_infile && (opt_white != NONE || opt_black != NONE))
	{
		printf ("can't specify -f with -p\n");
		exit (1);
	}
	if (opt_quiet && (opt_white == HUMAN || opt_black == HUMAN))
	{
		printf ("can't be quiet with human players\n");
		exit (1);
	}
	if (game_single_player && (opt_infile))
	{
		printf ("can't load from file for single player game\n");
		exit (1);
	}

	if (wheur && bheur)
	{
		int i = 0;
		if (!opt_game)
		{
			printf ("heur fn specified but no game specified\n");
			exit (1);
		}
		if (!game_htab)
		{
			printf ("no support for changing eval fn. in %s\n", opt_game->name);
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
			printf ("%s: no such eval fn. in %s\n", wheur, opt_game->name);
			exit(1);
		}
		if (!game_eval_black)
		{
			printf ("%s: no such eval fn. in %s\n", bheur, opt_game->name);
			exit(1);
		}
		// FIXME: engine should parse opts separately
		game_eval = engine_eval;
	}

	if (game_single_player)
	{
		opt_white = HUMAN;
		opt_black = HUMAN;
	}
	else if (!opt_infile)
	{
		// default is human vs. machine
		if (opt_white == NONE) opt_white = 	HUMAN;
		if (opt_black == NONE) opt_black = MACHINE;
	}
	ui_white = opt_white;
	ui_black = opt_black;
}

void gui_init ()
{
	GtkWidget *hbox = NULL, *vbox = NULL, *frame = NULL;
	GtkWidget *separator;
	GtkAccelGroup *ag;
	GtkItemFactoryEntry game_items [num_games+1];
	GtkItemFactoryEntry items[] = 
	{
#if GTK_MAJOR_VERSION == 1
		{ "/_File", NULL, NULL, 0, "<Branch>" },
		{ "/File/_Load game", "<control>L", menu_load_file_dialog, 0, "" },
		{ "/File/_Save game", NULL, NULL, 0, "" },
		{ "/File/_Quit", "<control>Q", (GtkSignalFunc) ui_cleanup, 0, "" },
		{ "/_Game", NULL, NULL, 0, "<Branch>" },
		{ "/Game/Select _Game", NULL, NULL, 0, "<LastBranch>" },
		{ "/Game/Sep1", NULL, NULL, 0, "<Separator>" },
		{ "/Game/_New", "<control>N", menu_start_stop_game, MENU_RESET_GAME, "" },
		{ "/Game/_Start", "<control>G", menu_start_stop_game, MENU_START_GAME, "" },
		{ "/Game/_Pause", "<control>P", menu_start_stop_game, MENU_STOP_GAME, "" },
		{ "/Game/Sep2", NULL, NULL, 0, "<Separator>" },
		{ "/Game/_Highscores", NULL, prefs_show_scores, 0, ""},
		{ "/Game/_Zap Highscores", NULL, prefs_zap_highscores, 0, ""},
		{ "/_Move", NULL, NULL, 0, "<Branch>" },
		{ "/Move/_Back", "<control>B", menu_back_forw, MENU_BACK, "" },
		{ "/Move/_Forward", "<control>F", menu_back_forw, MENU_FORW, "" },
		{ "/Move/Sep1", NULL, NULL, 0, "<Separator>" },
		{ "/Move/_Move Now", "<control>M", 
			(GtkItemFactoryCallback) ui_get_machine_move, 0, "" },
#else
		{ "/_File", NULL, NULL, 0, "<Branch>" },
		{ "/File/_Load game", "<control>L", menu_load_file_dialog, 0, 
				"<StockItem>", GTK_STOCK_OPEN },
		{ "/File/_Save game", NULL, menu_save_file_dialog, 0, 
				"<StockItem>", GTK_STOCK_SAVE },
		{ "/File/_Quit", "<control>Q", (GtkSignalFunc) ui_cleanup, 0, 
				"<StockItem>", GTK_STOCK_QUIT },
		{ "/_Game", NULL, NULL, 0, "<Branch>" },
		{ "/Game/Select _Game", NULL, NULL, 0, "<LastBranch>" },
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
		{ "/_Move", NULL, NULL, 0, "<Branch>" },
		{ "/Move/_Back", "<control>B", menu_back_forw, 1, 
				"<StockItem>", GTK_STOCK_GO_BACK },
		{ "/Move/_Forward", "<control>F", menu_back_forw, 2, 
				"<StockItem>", GTK_STOCK_GO_FORWARD },
		{ "/Move/Sep1", NULL, NULL, 0, "<Separator>" },
		{ "/Move/_Move Now", "<control>M", 
			(GtkItemFactoryCallback) ui_get_machine_move, 0, "" },
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
		{ "/Settings/_Eval function", NULL, NULL, 0, "<Branch>" },
		{ "/Settings/_Eval function/_White", NULL, NULL, 0, "<Branch>" },
		{ "/Settings/_Eval function/_Black", NULL, NULL, 0, "<Branch>" },
		{ "/Settings/_Flip Board", "<control>T", menu_board_flip_cb, 0, "" },
		{ "/_Help", NULL, NULL, 0, "<LastBranch>" },
		{ "/Help/_About", NULL, menu_show_about_dialog, 0, ""},
	};
	int i;
	gdk_rgb_init ();
	main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_policy (GTK_WINDOW (main_window), FALSE, FALSE, TRUE);
	gtk_signal_connect (GTK_OBJECT (main_window), "delete_event",
		GTK_SIGNAL_FUNC(ui_cleanup), NULL);
	gtk_window_set_title (GTK_WINDOW (main_window), "GTKBoard");

	ag = gtk_accel_group_new();
	menu_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", ag);
	gtk_window_add_accel_group (GTK_WINDOW (main_window), ag);
			
	gtk_item_factory_create_items (menu_factory, 
			sizeof (items) / sizeof (items[0]), items, NULL);
	for (i=0; i<=num_games; i++)
	{
		game_items[i].path = g_strdup_printf ("/Game/Select Game/%s",  
				i ? games[i-1]->name : "none");
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
	menu_set_eval_function ();
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), menu_main, FALSE, FALSE, 0);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	{
	board_colbox = gtk_vbox_new (FALSE, 0);
	board_area = gtk_drawing_area_new ();
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), board_colbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), board_area, TRUE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	gtk_signal_connect (GTK_OBJECT (board_area), "expose_event",
		GTK_SIGNAL_FUNC (board_redraw), NULL);

   	gtk_widget_set_events(board_area, 
			gtk_widget_get_events (board_area) 
			|	GDK_BUTTON_PRESS_MASK 
			|   GDK_BUTTON_RELEASE_MASK
			|   GDK_POINTER_MOTION_MASK
			|	GDK_KEY_PRESS_MASK);
	gtk_signal_connect (GTK_OBJECT (board_area), "motion_notify_event",
		GTK_SIGNAL_FUNC (board_clicked), NULL);
	gtk_signal_connect (GTK_OBJECT (board_area), "button_release_event",
		GTK_SIGNAL_FUNC (board_clicked), NULL);
	gtk_signal_connect (GTK_OBJECT (board_area), "button_press_event",
		GTK_SIGNAL_FUNC (board_clicked), NULL);
	gtk_signal_connect (GTK_OBJECT (main_window), "key_press_event",
		GTK_SIGNAL_FUNC (board_clicked), NULL);
	hbox = gtk_hbox_new (FALSE, 0);
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
			
	separator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	board_rowbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), board_rowbox, FALSE, FALSE, 0);
	sb_message_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (sb_message_label), 0, 0.5);
	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), sb_message_label, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (main_window), vbox);
	// FIXME: board_init() needs show() to be called to get a gc, but
	// leads to the whole window not popping up at once
	gtk_widget_show_all (main_window);
	
	board_init ();

	gtk_timeout_add (100, sb_update_periodic, NULL);

	// this should be called before setting state_gui_active = TRUE
	if (opt_game) menu_put_game (); 
	state_gui_active = TRUE;

	if (opt_game) menu_start_game ();
	menu_put_player (TRUE);
	if (!opt_game) sb_message ("Select a game from the Game menu", FALSE);
	sb_update ();
}

void ignore() {}

int main (int argc, char **argv)
{
	srand (time(0));
	reset_game_params ();
	parse_opts (argc, argv);
	ui_start_player ();
	
	signal (SIGHUP, ignore);
	signal (SIGINT, ui_cleanup);
	signal (SIGTERM, ui_cleanup);
	signal (SIGSEGV, ui_segv_cleanup);
	gtk_init(&argc,&argv);    
	gdk_rgb_init();
	if (!opt_quiet)
		gui_init ();
	else	// background mode
	{
		set_game_params ();
		ui_stopped = FALSE;
		ui_send_make_move ();
	}
	gtk_main ();
	return 0;
}
