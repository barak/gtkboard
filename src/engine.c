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
/** \file engine.c
 \brief The engine forms the backend that does the number crunching
 */
#include "config.h"

#include "game.h"
#include "move.h"
#include "stack.h"
#include "engine.h"

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

extern int board_wid, board_heit;
extern int opt_verbose;
//extern int state_player;

// FIXME: this is ugly. Code should be refactored.
extern gboolean engine_flag;

extern Pos cur_pos;

extern Game *opt_game, *games[];
extern int num_games;
byte * engine_search (Pos *);
static FILE *engine_fin, *engine_fout;

//! Eval fn for white (can be NULL, in which case game_eval will be used for both)
ResultType (*game_eval_white) (Pos *, Player, float *);
//! Eval fn for black (can be NULL, in which case game_eval will be used for both)
ResultType (*game_eval_black) (Pos *, Player, float *);

// FIXME: following 3 extern decls must be removed by refactoring (i.e, move all fns common to client and server to a new file)
extern void reset_game_params ();
extern void set_game_params ();
extern void game_set_init_pos_def (Pos *);

//! Alpha-beta search function (using depth first iterative deepening).
extern byte *ab_dfid (Pos *, int);

//! The input pipe is accessed through a GIOChannel so that we can register a callback for events
static GIOChannel *channel_in = NULL;

//! If an event occurs when we are thinking this will be set to TRUE so that we will know to stop thinking
gboolean engine_stop_search = FALSE;

//! Indicates whether we have to stop and return the move or stop and cancel the move
static gboolean cancel_move = FALSE;

//! Max time per move. alpha-beta will often return earlier than this.
int time_per_move = 5000;

gboolean engine_hup_cb ()
{
	if (opt_verbose)
		fprintf (stderr, "engine: Connection broken. Exiting.\n");
	// FIXME: do we want to free anything here?
	exit (1);
}

ResultType engine_eval (Pos *pos, /*Player player,*/ float *eval)
{
	ResultType result;
	result = pos->player == WHITE ? game_eval_white(pos, pos->player, eval) :
	game_eval_black (pos, pos->player, eval);
	return result;
}

void engine_set_to_play (char *line)
{
	if (!strncasecmp (line, "white", 5))
		cur_pos.player = WHITE;
	else if (!strncasecmp (line, "black", 5))
		cur_pos.player = BLACK;
}

void engine_take_move (char *line)
{
	byte *move = move_read (line);
	movstack_trunc ();
	movstack_push (cur_pos.board, move);
	if (game_stateful)
	{
		void *newstate = game_newstate (&cur_pos, move);
		statestack_push (newstate);
		cur_pos.state = statestack_peek ();
	}
	move_apply (cur_pos.board, move);
	cur_pos.num_moves++;
	cur_pos.player = cur_pos.player == WHITE ? BLACK : WHITE;
}

void engine_make_move ()
{
	byte *move;
	movstack_trunc ();
	cancel_move = FALSE;
	move = engine_search (&cur_pos);
	if (cancel_move)
		return;
	if (!move)
	{
		move_fwrite_nak ("Nice try", engine_fout);
		return;
	}
	movstack_push (cur_pos.board, move);
	if (game_stateful)
	{
		void *newstate = game_newstate (&cur_pos, move);
		statestack_push (newstate);
		cur_pos.state = statestack_peek ();
	}
	move_apply (cur_pos.board, move);
	cur_pos.num_moves++;
	move_fwrite_ack (move, engine_fout);
	cur_pos.player = cur_pos.player == WHITE ? BLACK : WHITE;
}

void engine_new_game (char *gamename)
{
	int i, len;
	Game *old_game = opt_game;
	opt_game = NULL;
	// strip trailing newline
	if (gamename[len = strlen(gamename) - 1] == '\n')
		gamename[len] = 0;
	for (i=0; i<num_games; i++)
		if (!strcmp (games[i]->name, gamename))
		{
			opt_game = games[i];
			break;
		}
	if (!opt_game)
	{
		// FIXME: isn't there a more elegant way to do this?
		GameLevel *level = game_levels;
		assert (level);
		while (level->name)
		{
			if (!strcmp (level->game->name, gamename))
			{
				opt_game = level->game;
				break;
			}
			level++;
		}
	}
	if (!opt_game)
	{
		fprintf (stderr, "engine: unknown game: %s\n", gamename);
		exit(1);
	}
	reset_game_params ();
	if (opt_game->game_init)
		opt_game->game_init(opt_game);
	set_game_params ();
	if (game_set_init_pos != game_set_init_pos_def) 
	{
		fwrite (cur_pos.board, board_wid * board_heit, 1, engine_fout);
		fflush (engine_fout);
	}
	stack_free ();
}

void engine_reset_game ()
{
	stack_free ();
	// FIXME : there should be a separate reset_game_params() for engine
	cur_pos.player = WHITE;
	cur_pos.state = NULL;
	cur_pos.num_moves = 0;
	game_set_init_pos (&cur_pos);
}

void engine_back_move ()
{
	byte *move = movstack_back ();
	if (game_stateful) cur_pos.state = statestack_back ();
	if (!move)
	{
		move_fwrite_nak (NULL, engine_fout);
		return;
	}
	move_apply (cur_pos.board, move);
	cur_pos.num_moves--;
	cur_pos.player = cur_pos.player == WHITE ? BLACK : WHITE;
	move_fwrite_ack (move, engine_fout);
}

void engine_forw_move ()
{
	byte *move = movstack_forw ();
	void *state;
	if (game_stateful && (state = statestack_forw ()))
		cur_pos.state = state;
	if (!move)
	{
		move_fwrite_nak (NULL, engine_fout);
		return;
	}
	move_apply (cur_pos.board, move);
	cur_pos.num_moves++;
	cur_pos.player = cur_pos.player == WHITE ? BLACK : WHITE;
	move_fwrite_ack (move, engine_fout);
}

void engine_msec_per_move (char *line)
{
	if (!line) return;
	time_per_move = atoi (line);
	if (time_per_move < 0)
		time_per_move = 3000;
}

void engine_who_won (char *line)
{
	int who;
	char *msg = NULL;
	char *who_str;
	if (!game_who_won)
	{
		move_fwrite_nak (NULL, engine_fout);
		return;
	}
	who = game_who_won (&cur_pos, cur_pos.player, &msg);
	switch(who)
	{
		case RESULT_WHITE: who_str = "WHITE"; break;
		case RESULT_BLACK: who_str = "BLACK"; break;
		case RESULT_TIE  : who_str = "TIE"  ; break;
		case RESULT_WON  : who_str = "WON" ; break;
		case RESULT_LOST : who_str = "LOST" ; break;
		case RESULT_MISC : who_str = "MISC" ; break;
		case RESULT_NOTYET:
		default:
						who_str = "NYET"; break; // ;^)
	}
	
	if (msg)
		fprintf (engine_fout, "ACK %s %s\n", who_str, msg);
	else
		fprintf (engine_fout, "ACK %s\n", who_str);
	fflush (engine_fout);

}

void engine_move_now (char *line)
{
	engine_stop_search = TRUE;
}

int engine_timeout_cb ()
{
	engine_stop_search = TRUE;
	return FALSE;
}

void engine_cancel_move (char *line)
{
	engine_stop_search = TRUE;
	cancel_move = TRUE;
}


//! This structure defines the protocol
Command commands[] = 
{
	{ "MSEC_PER_MOVE"  , 1 , engine_msec_per_move},
	{ "SUGGEST_MOVE"    , 0 , NULL},
	{ "TAKE_MOVE"       , 1 , engine_take_move},
	{ "BACK_MOVE"       , 1 , engine_back_move},
	{ "FORW_MOVE"       , 1 , engine_forw_move},
	{ "MAKE_MOVE"       , 1 , engine_make_move},
	{ "MOVE_NOW"		, 1 , engine_move_now },
	{ "CANCEL_MOVE"		, 1 , engine_cancel_move },
	{ "END_GAME"        , 0 , NULL},
	{ "RESET_GAME"      , 1 , engine_reset_game},
	{ "TO_PLAY"         , 1 , engine_set_to_play},
	{ "SET_POSITION"    , 0 , NULL},
	{ "NEW_GAME"        , 1 , engine_new_game},
	{ "GET_EVAL"        , 0 , NULL},
	{ "SET_HEUR"        , 0 , NULL},
	{ "SET_STRATEGY"    , 0 , NULL},
	{ "WHO_WON"			, 1 , engine_who_won},
};

#define NUM_COMMANDS (sizeof (commands) / sizeof (commands[0]))

//! Parse the command and pass control to the corresponding function pointer
static void execute_command (char *line)
{
	char *tail;
	int i;
	tail = strpbrk (line, " \t");
	if (tail)
		*tail = 0;
	for (i=0; i<NUM_COMMANDS; i++)
		if (!strcmp (line, commands[i].proto_str))
		{
			if (!commands[i].isimpl)
			{
				fprintf (stderr, "warning: command %s not yet implemented\n",
						commands[i].proto_str);
				return;
			}
			commands[i].impl_func (tail ? (tail+1) : 0);
			return;
		}
	fprintf (stderr, "warning: unknown command \"%s\" \n", line);
}


static GSList *command_list = NULL;
static gboolean process_line ()
{
	char *line;
   	line = (char *) g_slist_nth_data (command_list, 0);
	if (!line) return FALSE;
	command_list = g_slist_remove (command_list, line);
	execute_command (line);
	g_free (line);
	return TRUE;
}

static gboolean channel_process_input ()
{
	static char linebuf[4096];
	char *linep = linebuf;
	char *line;
	int bytes_read;
#if GLIB_MAJOR_VERSION > 1
	// we need to call this again because we will get new events before returning
	// from this function
	// semantics of add_watch silently changing between glib versions!!!!
	g_io_add_watch (channel_in, G_IO_IN, (GIOFunc) channel_process_input, NULL);
#endif
	g_io_channel_read (channel_in, linebuf, 4096, &bytes_read);
	linebuf[bytes_read] = '\0';
	while (*linep != '\0')
	{
		line = linep;
		while (*linep++ != '\n')
			g_assert (linep[-1] != '\0');
		linep[-1] = '\0';
		if (opt_verbose) printf ("engine got command \"%s\"\n", line);
		command_list = g_slist_append (command_list, g_strdup (line));
	}	
	while (process_line ())
		;
#if GLIB_MAJOR_VERSION == 1
	return TRUE;
#else
	return FALSE;
#endif
}

void engine_poll ()
{
	static int poll_count = 0;
	if (++poll_count == 100)
	{
		poll_count = 0;
		// listen for input in the pipe
		while (g_main_iteration (FALSE))
			;
		// execute pending commands
		// we execute only ONE command so that if there is a CANCEL_MOVE followed by a MAKE_MOVE we won't start on the new move before finishing this one
		process_line ();
	}
}

static void ignore () {}

//! This is the main function for the engine
void engine_main (int infd, int outfd)
{
	GMainLoop *loop;
	char *line;
	engine_flag = TRUE;
	signal (SIGHUP, ignore);
	signal (SIGINT, ignore);
	engine_fin = fdopen (infd, "r");
	engine_fout = fdopen (outfd, "w");
	assert (engine_fin);
	assert (engine_fout);
	channel_in = g_io_channel_unix_new (infd);
	g_io_add_watch (channel_in, G_IO_IN, (GIOFunc) channel_process_input, NULL);
	g_io_add_watch (channel_in, G_IO_HUP | G_IO_NVAL, (GIOFunc) engine_hup_cb, NULL);
#if GLIB_MAJOR_VERSION > 1
	loop = g_main_loop_new (NULL, TRUE);
#else
	loop = g_main_new (TRUE);
#endif
	g_main_run (loop);
}

byte * engine_search (Pos *pos/*, int player*/)
{
	int tag;
	byte *move;
	engine_stop_search = FALSE;
	if (game_search)
		game_search (pos, &move);
	else if (game_single_player)
		move = NULL;
	else
	{
		tag = g_timeout_add (2 * time_per_move, engine_timeout_cb, NULL);
		move = ab_dfid (pos, pos->player);
		g_source_remove (tag);
	}
	return move;
}

