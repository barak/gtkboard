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

extern gboolean engine_stop_search;

static int ab_tree_exhausted = 0;

static int ab_leaf_cnt;  // how many leaves were eval'd

extern int hash_get_eval (byte *, int, int, float *);
extern void hash_print_stats ();
extern void hash_insert (byte *, int, int, float);
extern void hash_clear ();

extern gboolean opt_verbose;

	
float game_ab (Pos *pos, int player, int level, 
		float alpha, float beta, byte **ret_movep)
	/* level is the number of ply to search */
{
	int to_play = player;
	float val, best= 0;
	int first = 1;
	byte *movlist, *move;
	byte best_move [1024];
	static byte ret_move [1024];
	Pos newpos;
	if (engine_stop_search) { ab_tree_exhausted = 0; return 0; }
	newpos.board = (char *) malloc (board_wid * board_heit);
	assert (newpos.board);
	memcpy (newpos.board, pos->board, board_wid * board_heit);
	if (game_stateful)
	{
		newpos.state = (void *) malloc (game_state_size);
		assert (newpos.state);
		memcpy (newpos.state, pos->state, game_state_size);
	}
	movlist = game_movegen (pos, player);
	if (movlist[0] == -2)		/* we have no move left */
	{
		if (ret_movep && !engine_stop_search)
			*ret_movep = NULL;
		free (newpos.board);
		if (game_stateful) free (newpos.state);
		free (movlist);
		return game_eval (pos, to_play);
	}
	move = movlist;
	do
	{
		memcpy (newpos.board, pos->board, board_wid * board_heit);
		if (game_stateful) 
		{
			void *newstate = game_newstate (pos, move);
			memcpy (newpos.state, newstate, game_state_size);
		}
		move_apply (newpos.board, move);
		if (level == 0)
		{
			ab_tree_exhausted = 0;
			val = game_eval (&newpos, to_play);
		}
		else 
		{
			if (player == WHITE)
				val = game_ab (&newpos, BLACK, level-1, alpha, beta, NULL);
			else
				val = game_ab (&newpos, WHITE, level-1, alpha, beta, NULL);
		}
		if (first || 
			(player == WHITE && val > alpha) || (player == BLACK && val < beta))
		{
			if (ret_movep)	movcpy (best_move, move);
			if (player == WHITE) alpha = val; else beta = val;
			first = 0;
		}
		if (alpha >= beta)
			break;
		move = movlist_next (move);
	}
	while (move[0] != -2);
	if (ret_movep && !engine_stop_search)
	{
		movcpy (ret_move, best_move);
		*ret_movep = ret_move;
	}
	free (newpos.board);
	if (game_stateful) free (newpos.state);
	free (movlist);
	return player == WHITE ? alpha : beta;
}

	
float game_ab_hash (Pos *pos, int player, int level, 
		float alpha, float beta, byte **ret_movep, int depth)
	/* level is the number of ply to search */
{
	int to_play = player;
	float val, cacheval, best= 0, retval;
	int first = 1;
	byte *movlist, *move;
	byte best_move [1024];
	static byte ret_move [1024];
	Pos newpos;
	engine_poll ();
	if (engine_stop_search) { ab_tree_exhausted = 0; return 0; }
	newpos.board = (char *) malloc (board_wid * board_heit);
	assert (newpos.board);
	memcpy (newpos.board, pos->board, board_wid * board_heit);
	if (game_stateful)
	{
		newpos.state = (void *) malloc (game_state_size);
		assert (newpos.state);
		memcpy (newpos.state, pos->state, game_state_size);
	}
	movlist = game_movegen (pos, player);
	if (movlist[0] == -2)		/* we have no move left */
	{
		if (ret_movep && !engine_stop_search)
			*ret_movep = NULL;
		free (newpos.board);
		if (game_stateful) free (newpos.state);
		free (movlist);
		val = game_eval (pos, to_play);
		hash_insert (pos->board, board_wid * board_heit, level, val);
		return val;
	}
	move = movlist;
	do
	{
		memcpy (newpos.board, pos->board, board_wid * board_heit);
		if (game_stateful)
		{
			void *newstate = game_newstate (pos, move);
			memcpy (newpos.state, newstate, game_state_size);
		}
		move_apply (newpos.board, move);
		if (level == 0)
		{
			ab_leaf_cnt ++;
			ab_tree_exhausted = 0;
			retval = hash_get_eval (newpos.board, board_wid * board_heit,
				   level, &cacheval);
			if (retval) val = cacheval;
			val = game_eval (&newpos, to_play);
		}
		else 
		{
			retval = hash_get_eval (newpos.board, board_wid * board_heit,
				   level, &cacheval);
			if (retval) val = cacheval;
			else
			{
				if (player == WHITE)
					val = game_ab_hash 
						(&newpos, BLACK, level-1, alpha, beta, NULL, depth+1);
				else
					val = game_ab_hash 
						(&newpos, WHITE, level-1, alpha, beta, NULL, depth+1);
			}
		}
		hash_insert (newpos.board, board_wid * board_heit, level, val);
		if (first || 
			(player == WHITE && val > alpha) || (player == BLACK && val < beta))
		{
			if (ret_movep)	movcpy (best_move, move);
			if (player == WHITE) alpha = val; else beta = val;
			first = 0;
		}
		if (alpha >= beta)
			break;
		move = movlist_next (move);
	}
	while (move[0] != -2);
	if (ret_movep && !engine_stop_search)
	{
		movcpy (ret_move, best_move);
		*ret_movep = ret_move;
	}
	free (newpos.board);
	if (game_stateful) free (newpos.state);
	free (movlist);
/*	if (depth == 0) 
		printf ("game_ab_hash: eval = %f\n", player == WHITE ? alpha : beta);
*/
	return player == WHITE ? alpha : beta;
}

float game_ab_hash_incr (Pos *pos, int player, int level, 
		float eval, float alpha, float beta, byte **ret_movep, int depth)
	/* level is the number of ply to search */
{
	int to_play = player;
	float val, cacheval, best= 0, retval;
	int first = 1;
	byte *movlist, *move;
	byte best_move [1024];
	static byte ret_move [1024];
	void *oldstate = NULL; // use the recursion to implement stack of states
	engine_poll ();
	if (engine_stop_search) 
	{ 
		ab_tree_exhausted = 0; 
		return 0; 
	}
	if (game_stateful)
	{
		oldstate = (void *) malloc (game_state_size);
		assert (oldstate);
	}
	movlist = game_movegen (pos, player);
	if (movlist[0] == -2)		/* we have no move left */
	{
		if (ret_movep && !engine_stop_search)
			*ret_movep = NULL;
		free (movlist);
		val = game_eval (pos, to_play);
		//hash_insert (pos->board, board_wid * board_heit, level, val);
		return val;
	}
	move = movlist;
	do
	{
		float neweval = 0, incr_eval = game_eval_incr (pos, to_play, move);
		neweval = eval + incr_eval;
		
		if (level == 0)
		{
			ab_leaf_cnt ++;
			ab_tree_exhausted = 0;
			val = neweval;//game_eval (pos, to_play);
		}
		else if (fabs (incr_eval) >= GAME_EVAL_INFTY) 
			// one side has won; search no more
		{
			ab_leaf_cnt ++;
			ab_tree_exhausted = 0;
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
			val = 0; // stop compiler warning
			if (level > 1)
			{
				retval = hash_get_eval (pos->board, board_wid * board_heit,
				   level, &cacheval);
				if (retval)  { val = cacheval; found = 1; }
			}
			if (!found)
				val = game_ab_hash_incr
					(pos, player == WHITE ? BLACK : WHITE, 
						level-1, neweval, alpha, beta, NULL, depth+1);
			if (level > 1)
				hash_insert (pos->board, board_wid * board_heit, level, val);
			move_apply (pos->board, movinv);
			free (movinv);
			memcpy (pos->state, oldstate, game_state_size);
		}
		if (first || 
			(player == WHITE && val > alpha) || (player == BLACK && val < beta))
		{
			if (ret_movep)	movcpy (best_move, move);
			if (player == WHITE) alpha = val; else beta = val;
			first = 0;
		}
		if (alpha >= beta)
			break;
		move = movlist_next (move);
	}
	while (move[0] != -2);
	if (ret_movep && !engine_stop_search)
	{
		movcpy (ret_move, best_move);
		*ret_movep = ret_move;
	}
	free (movlist);
	if (game_stateful)
		free (oldstate);
	return player == WHITE ? alpha : beta;
}

static void catch_USR1 (int sig)
{
	engine_stop_search = 1;
	signal (SIGUSR1, catch_USR1);
}

byte * game_ab_dfid (Pos *pos, int player)
{
	byte *best_move;
	int ply;
	float val, eval = 0;
	engine_stop_search = 0;
	signal (SIGUSR1, catch_USR1);
	ab_leaf_cnt=0;
	if (game_eval_incr)
		eval = game_eval (pos, player);
	for (ply = 0; !engine_stop_search; ply++)
	{
		ab_tree_exhausted = 1;
		if (game_eval_incr)
			val = game_ab_hash_incr (pos, player, ply, 
					eval, -1e+16, 1e+16, &best_move, 0);
		else
			val = game_ab_hash (pos, player, ply, -1e+16, 1e+16, &best_move, 0);
		if (ab_tree_exhausted)
			break;
	}
	if (opt_verbose) 
		printf ("game_ab_dfid(): leaves=%d \tply=%d\n", ab_leaf_cnt, ply);
	hash_print_stats ();
	hash_clear ();
	if (opt_verbose) 
	{ 
		printf ("game_ab_dfid(): move= "); 
		move_fwrite (best_move, stdout); 
	}
	return best_move;
}
