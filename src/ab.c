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

#include "game.h"
#include "move.h"
#include "engine.h"

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

extern int time_per_move;

extern gboolean engine_stop_search;

static gboolean ab_tree_exhausted = FALSE;

static int ab_leaf_cnt;  // how many leaves were eval'd

extern int hash_get_eval (byte *, int, int, int, float *);
extern void hash_print_stats ();
extern void hash_insert (byte *, int, int, int, float, byte *move);
extern void hash_clear ();
extern void hash_insert_move (byte *, int, int, byte *);
extern byte * hash_get_move (byte *, int, int);

extern gboolean opt_verbose;

// FIXME: move this to ui.c
gboolean game_use_hash = TRUE;
	
// FIXME: this function is too complicated
float ab_with_tt (Pos *pos, int player, int level, 
		float alpha, float beta, byte *best_movep)
	/* level is the number of ply to search */
{
	int to_play = player;
	float val, cacheval, best= 0, retval;
	gboolean first = TRUE;
	byte *movlist, *move;
	byte best_move [4096];
	Pos newpos;
	gboolean hashed_move = TRUE;
	float local_alpha = -1e+16, local_beta = 1e+16;
	byte *orig_move;
	best_move [0] = -1;
	
	engine_poll ();
	if (engine_stop_search) { ab_tree_exhausted = FALSE; return 0; }

	movlist = game_movegen (pos);
	if (movlist[0] == -2)		/* we have no move left */
	{
		free (movlist);
		game_eval (pos, to_play, &val);
		if (game_use_hash)
			hash_insert (pos->board, board_wid * board_heit, pos->num_moves, level, val, 
				NULL);
		return val;
	}
	move = NULL;
	orig_move = NULL;
	if (game_use_hash && level > 0)
		move = hash_get_move (pos->board, board_wid * board_heit, pos->num_moves);
	if (!move)
	{
		move = movlist;
		hashed_move = FALSE;
	}
	// origmove is the owning pointer and move is the aliasing pointer
	else orig_move = move = movdup (move);
	
	newpos.board = (char *) malloc (board_wid * board_heit);
	assert (newpos.board);
	if (game_stateful)
	{
		newpos.state = (void *) malloc (game_state_size);
		assert (newpos.state);
	}

	do
	{
		if (!orig_move || hashed_move || !movcmp_literal (orig_move, move))
		{
			ResultType result = RESULT_NOTYET;
			memcpy (newpos.board, pos->board, board_wid * board_heit);
			if (game_stateful)
			{
				void *newstate = game_newstate (pos, move);
				memcpy (newpos.state, newstate, game_state_size);
			}
			move_apply (newpos.board, move);
			newpos.num_moves = pos->num_moves + 1;
			newpos.search_depth = pos->search_depth + 1;
			newpos.player = pos->player == WHITE ? BLACK : WHITE;
			retval = 0;
			if (game_use_hash && level > 0)
				retval = hash_get_eval (newpos.board, board_wid * board_heit, 
						newpos.num_moves, level-1, &cacheval);
			if (retval && fabs (cacheval) < GAME_EVAL_INFTY) val = cacheval;
			else result = game_eval (&newpos, to_play == WHITE ? BLACK : WHITE, &val);
			if (level == 0)
			{
				ab_leaf_cnt ++;
				ab_tree_exhausted = FALSE;
/*				if (game_use_hash)
					hash_insert (newpos.board, board_wid * board_heit, 
						pos->num_moves, level, val, NULL);
*/
			}
			else 
			{
				if (fabs (val) >= GAME_EVAL_INFTY)
					val *= (1 + level);
				else if (result == RESULT_WHITE || result == RESULT_BLACK)
					val *= (1 + level);
				else if (result == RESULT_TIE)
					;
				else
				{
					val = ab_with_tt (&newpos, player == WHITE ? BLACK : WHITE, 
								level-1, alpha, beta, best_move);
				}
			}
			if((player == WHITE && val > local_alpha) 
					|| (player == BLACK && val < local_beta))
			{
				if (best_movep)	movcpy (best_movep, move);
				if (player == WHITE) local_alpha = val; else local_beta = val;
			}

			if((player == WHITE && val > alpha))
				alpha = val;
			if ((player == BLACK && val < beta))
				beta  = val;
			if (alpha >= beta || alpha >= GAME_EVAL_INFTY || beta <= -GAME_EVAL_INFTY)
				break;
		}
		if (hashed_move)
			move = movlist;
		else
			move = movlist_next (move);
		hashed_move = FALSE;
	}
	while (move[0] != -2);
	free (newpos.board);
	if (game_stateful) free (newpos.state);
	free (movlist);
	free (orig_move);
	if (game_use_hash)
		hash_insert (pos->board, board_wid * board_heit, pos->num_moves, level,
			player == WHITE ? alpha : beta, best_movep);
	return player == WHITE ? alpha : beta;
}

#if 0
// TODO: this is currently unused, and must be merged with the previous function 
float ab_with_tt_incr (Pos *pos, int player, int level, 
		float eval, float alpha, float beta, byte *best_movep)
	/* level is the number of ply to search */
{
	int to_play = player;
	float val, cacheval, best= 0, retval;
	int first = 1;
	byte *movlist, *move;
	byte best_move [4096];
	void *oldstate = NULL; // use the recursion to implement stack of states
	engine_poll ();
	if (engine_stop_search) 
	{ 
		ab_tree_exhausted = FALSE; 
		return 0; 
	}
	if (game_stateful)
	{
		oldstate = (void *) malloc (game_state_size);
		assert (oldstate);
	}
	movlist = game_movegen (pos);
	if (movlist[0] == -2)		/* we have no move left */
	{
		free (movlist);
		game_eval (pos, to_play, &val);
		return val;
	}
	move = movlist;
	do
	{
		float neweval = 0, incr_eval;
		ResultType result;
		result = game_eval_incr (pos, to_play, move, &incr_eval);
		neweval = eval + incr_eval;
		
		if (level == 0)
		{
			ab_leaf_cnt ++;
			ab_tree_exhausted = FALSE;
			val = neweval;
		}
		else if (fabs (incr_eval) >= GAME_EVAL_INFTY 
				|| result != RESULT_NOTYET)
			// one side has won; search no more
		{
			ab_leaf_cnt ++;
			val = neweval * (1 + level); // the sooner the better
		}
		else 
		{
			int found = 0;
			byte *movinv = mov_getinv (pos->board, move);
			move_apply (pos->board, move);
			if (game_stateful)
			{
				void *newstate = game_newstate (pos, move);
				memcpy (oldstate, pos->state, game_state_size);
				memcpy (pos->state, newstate, game_state_size);
			}
			pos->num_moves++;
			pos->player = pos->player == WHITE ? BLACK : WHITE;
			val = 0; // stop compiler warning
			if (level >= 1)
			{
				retval = hash_get_eval (pos->board, board_wid * board_heit, 
						pos->num_moves, level, &cacheval);
				if (retval && cacheval < GAME_EVAL_INFTY)  
					{ val = cacheval; found = 1; }
			}
			if (!found)
				val = ab_with_tt_incr
					(pos, player == WHITE ? BLACK : WHITE, 
						level-1, neweval, alpha, beta, best_move);
			if (level >= 1)
				hash_insert (pos->board, board_wid * board_heit, 
						pos->num_moves, level, val, NULL);
			move_apply (pos->board, movinv);
			free (movinv);
			memcpy (pos->state, oldstate, game_state_size);
			pos->num_moves--;
			pos->player = pos->player == WHITE ? BLACK : WHITE;
		}
		if (first)
		{
			if (best_movep) movcpy (best_movep, move);
			first = 0;
		}
		if ((player == WHITE && val > alpha) || (player == BLACK && val < beta))
		{
			if (best_movep) movcpy (best_movep, move);
			if (player == WHITE) alpha = val; else beta = val;
		}
		if (alpha >= beta)
			break;
		move = movlist_next (move);
	}
	while (move[0] != -2);
	free (movlist);
	if (game_stateful)
		free (oldstate);
	return player == WHITE ? alpha : beta;
}
#endif 

byte * ab_dfid (Pos *pos, int player)
{
	static byte best_move[4096];
	byte local_best_move[4096];
	int ply;
	float val = 0, eval = 0, oldval = 0;
	static GTimer *timer = NULL;
	gboolean found = FALSE;
	byte *move_list;
	engine_stop_search = 0;
	if (!game_movegen || !game_eval)
		return NULL;
	ab_leaf_cnt=0;

	move_list = game_movegen (pos);
	if (move_list[0] == -2)
	{
		free (move_list);
		if (opt_verbose) printf ("No move\n");
		return NULL;
	}
	if (movlist_next (move_list)[0] == -2)
	{
		movcpy (best_move, move_list);
		if (opt_verbose) printf ("Only one legal move\n");
		return best_move;
	}

	if (!timer) timer = g_timer_new ();
	g_timer_start (timer);
	
	for (ply = 0; !engine_stop_search; ply++)
	{
		oldval = val;
		ab_tree_exhausted = TRUE;
		pos->search_depth = 0;
		val = ab_with_tt (pos, player, ply, -1e+16, 1e+16, local_best_move);
		if (!engine_stop_search)
		{
			movcpy (best_move, local_best_move);
			found = TRUE;
		}
		
		if (ab_tree_exhausted)
		{
			if (opt_verbose)
				printf ("Searched whole tree. Moves=%d;\t Ply=%d\n",
					pos->num_moves, ply);
			ply++;
			break;
		}
		
		if (fabs (val) >= GAME_EVAL_INFTY)
		{
			if (opt_verbose)
				printf ("Solved the game. %s won. Moves=%d;\t Ply=%d\n",
					val > 0 ? "White" : "Black", pos->num_moves, ply);
			ply++;
			break;
		}

		{
			gulong micro_sec;
			float time_taken;
			time_taken = g_timer_elapsed (timer, &micro_sec);
			time_taken += micro_sec / 1000000.0;
			if (time_taken * 1000 > time_per_move / 2)
			{
				ply++;
				break;
			}
		}
	}
	
	if (game_use_hash)
	{
		hash_print_stats ();
		hash_clear ();
	}
	
	if (opt_verbose) 
	{ 
		printf ("ab_dfid(): leaves=%d \tply=%d\teval=%.1f\n", 
				ab_leaf_cnt, ply, oldval);
		printf ("ab_dfid(): move= "); 
		move_fwrite (best_move, stdout); 
	}
	return found ? best_move : NULL;
}
