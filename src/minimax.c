
#include "game.h"
#include "move.h"
#include "engine.h"

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static int stop_search;

static int minimax_tree_exhausted = 0;

static int mm_leaf_cnt;  // how many leaves were eval'd

extern int hash_get_eval (byte *, int, int, float *);
extern void hash_print_stats ();
extern void hash_insert (byte *, int, int, float);
	
// TODO: convert this to stateful
float game_minimax (Pos *pos, int player, int level, byte **ret_movep)
	/* level is the number of ply to search */
{
	int to_play = player;
	float val, max= 0;
	int first = 1;
	byte *movlist, *move;
	byte best_move [1024];
	static byte ret_move [1024];
	Pos newpos;
	//char *newpos;
	if (stop_search) { minimax_tree_exhausted = 0; return 0; }
	newpos.board = (char *) malloc (board_wid * board_heit);
	assert (newpos.board);
	memcpy (newpos.board, pos->board, board_wid * board_heit);
	if (game_stateful)
	{
		newpos.state = (void *) malloc (game_state_size);
		assert (newpos.state);
		memcpy (newpos.state, pos->state, (game_state_size));
	}
	movlist = game_movegen (pos, player);
	if (movlist[0] == -2)		/* we have no move left */
	{
		if (ret_movep && !stop_search)
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
		if (game_stateful) memcpy (newpos.state, pos->state, game_state_size);
		move_apply (newpos.board, move);
		// TODO: here change the state using game_newstate
		if (level == 0)
		{
			minimax_tree_exhausted = 0;
			val = game_eval (&newpos, to_play);
		}
		else 
			val = game_minimax 
				(&newpos, player == WHITE?BLACK:WHITE, level-1, NULL);
		if (first || 
			(player == WHITE && val > max) || (player == BLACK && val < max))
		{
			movcpy (best_move, move);
			max = val;
			first = 0;
		}
		move = movlist_next (move);
	}
	while (move[0] != -2);
	if (ret_movep && !stop_search)
	{
		movcpy (ret_move, best_move);
		*ret_movep = ret_move;
	}
	free (newpos.board);
	if (game_stateful) free (newpos.state);
	free (movlist);
	return max;
}


float game_minimax_hash (Pos *pos, int player, int level, byte **ret_movep)
	/* level is the number of ply to search */
{
	int to_play = player;
	float val, cacheval, max= 0;
	int first = 1;
	int retval;
	byte *movlist, *move;
	byte best_move [1024];
	static byte ret_move [1024];
	Pos newpos;
	//char *newpos;
	if (stop_search) { minimax_tree_exhausted = 0; return 0; }
	newpos.board = (char *) malloc (board_wid * board_heit);
	assert (newpos.board);
	memcpy (newpos.board, pos->board, board_wid * board_heit);
	if (game_stateful)
	{
		newpos.state = (void *) malloc (game_state_size);
		assert (newpos.state);
		memcpy (newpos.state, pos->state, game_state_size);
	}
	/*newpos.board = (char *) malloc (board_wid * board_heit);
	assert (newpos.board);
	memcpy (newpos, pos, board_wid * board_heit);*/
	movlist = game_movegen (pos, player);
	if (movlist[0] == -2)		/* we have no move left */
	{
		if (ret_movep && !stop_search)
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
		if (game_stateful) memcpy (newpos.state, pos->state, game_state_size);
		move_apply (newpos.board, move);
		if (level == 0)
		{
			mm_leaf_cnt ++;
			minimax_tree_exhausted = 0;
			retval = hash_get_eval (newpos.board, board_wid * board_heit,
				   level, &cacheval);
			if (retval)
			   val = cacheval;
			else
				val = game_eval (&newpos, to_play);
			if (retval)
				if (val != cacheval)
				{
					printf ("%f %f \n", val, cacheval);
					exit (1);
				}
			
		}
		else 
		{
			retval = hash_get_eval (newpos.board, board_wid * board_heit,
				   level, &cacheval);
			if (retval) val = cacheval;
			else
				val = game_minimax_hash
					(&newpos, player == WHITE?BLACK:WHITE, level-1, NULL);
		}
		hash_insert (newpos.board, board_wid * board_heit, level, val);
		if (first || 
			(player == WHITE && val > max) || (player == BLACK && val < max))
		{
			movcpy (best_move, move);
			max = val;
			first = 0;
		}
		move = movlist_next (move);
	}
	while (move[0] != -2);
	if (ret_movep && !stop_search)
	{
		movcpy (ret_move, best_move);
		*ret_movep = ret_move;
	}
	free (newpos.board);
	if (game_stateful) free (newpos.state);
	free (movlist);
	return max;
}

static void catch_USR1 (int sig)
{
	stop_search = 1;
	signal (SIGUSR1, catch_USR1);
}

byte * game_minimax_dfid (Pos *pos, int player)
{
	byte *best_move;
	int ply;
	float val;
	stop_search = 0;
	signal (SIGUSR1, catch_USR1);
	//signal (SIGUSR1, SIG_IGN);
	mm_leaf_cnt=0;
	//for (ply = 4; ply<5; ply++)
	for (ply = 0; !stop_search; ply++)
	{
		minimax_tree_exhausted = 1;
		val = game_minimax_hash (pos, player, ply, &best_move);
		if (minimax_tree_exhausted)
			break;
	}
	printf ("leaves: %d \tply: %d\n", mm_leaf_cnt, ply);
	hash_print_stats ();
	move_fwrite (best_move, stdout);
	return best_move;
}


