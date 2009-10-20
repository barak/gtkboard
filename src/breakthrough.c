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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "game.h"
#include "../pixmaps/chess.xpm"

#define BREAKTHROUGH_CELL_SIZE 54
#define BREAKTHROUGH_NUM_PIECES 2

#define BREAKTHROUGH_BOARD_WID 8
#define BREAKTHROUGH_BOARD_HEIT 8

#define BREAKTHROUGH_EMPTY 0
#define BREAKTHROUGH_WP 1
#define BREAKTHROUGH_BP 2

int breakthrough_initpos [BREAKTHROUGH_BOARD_WID*BREAKTHROUGH_BOARD_HEIT] = 
{
	2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 ,
	2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
	1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
};


char ** breakthrough_pixmaps [] = 
{
	chess_wp_54_xpm,
	chess_bp_54_xpm,
};

int breakthrough_curx = -1, breakthrough_cury = -1;

static char breakthrough_colors[] = 
	{200, 200, 130, 
	0, 140, 0};
static int breakthrough_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static int breakthrough_getmove_kb (Pos *, int, Player, byte ** , int **);
void breakthrough_init ();
static ResultType breakthrough_who_won (Pos *, Player, char **);
static ResultType breakthrough_eval (Pos *, Player, float *eval);
static ResultType breakthrough_eval_incr (Pos *, byte *, float *);
static byte * breakthrough_movegen (Pos *);
static void *breakthrough_newstate (Pos *, byte *);

Game Breakthrough = { BREAKTHROUGH_CELL_SIZE, BREAKTHROUGH_BOARD_WID, BREAKTHROUGH_BOARD_HEIT, 
	BREAKTHROUGH_NUM_PIECES, 
	breakthrough_colors, breakthrough_initpos, breakthrough_pixmaps, 
	"Breakthrough", "Chess variants", breakthrough_init};

void breakthrough_init ()
{
	game_getmove = breakthrough_getmove;
//	game_who_won = breakthrough_who_won;
	game_eval = breakthrough_eval;
//	game_eval_incr = breakthrough_eval_incr;
	game_movegen = breakthrough_movegen;
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_UNPLAYABLE;
	game_doc_about = 
		"Breakthrough\n"
		"Two player game\n"
		"Status: Partially implemented\n"
		"URL: "GAME_DEFAULT_URL ("breakthrough");
}

static gboolean eval_is_backward (byte *board, int x, int y)
{
	int incy, other, j;
	int val = board[y * board_wid + x];
	assert (val != BREAKTHROUGH_EMPTY);
	incy = val == BREAKTHROUGH_WP ? 1 : -1;
	other = val == BREAKTHROUGH_WP ? BREAKTHROUGH_BP : BREAKTHROUGH_WP;

	for (j = y; j < board_heit && j >= 0; j -= incy)
	{
		if (x - 1 >= 0 && board[j * board_wid + x - 1] == val) return FALSE;
		if (x + 1 < board_wid && board[j * board_wid + x + 1] == val) return FALSE;
	}
	
	for (j = y + incy; j < board_heit && j >= 0; j += incy)
	{
		if (board[j * board_wid + x] != BREAKTHROUGH_EMPTY) return FALSE;
		if (x - 1 >= 0 && j + incy >= 0 && j + incy < board_heit &&
				board[j * board_wid + x - 1] == val &&
				board[(j + incy) * board_wid + x - 1] == other) return TRUE;
		if (x + 1 >= 0 && j + incy >= 0 && j + incy < board_heit &&
				board[j * board_wid + x + 1] == val &&
				board[(j + incy) * board_wid + x + 1] == other) return TRUE;
	}
	return FALSE;
}

// Is this pawn a passer?
static gboolean eval_is_passer (byte *board, int x, int y)
{
	int incy, other;
	int val = board[y * board_wid + x];
	assert (val != BREAKTHROUGH_EMPTY);
	incy = val == BREAKTHROUGH_WP ? 1 : -1;
	other = val == BREAKTHROUGH_WP ? BREAKTHROUGH_BP : BREAKTHROUGH_WP;
	for (y += incy; y < board_heit && y >= 0; y += incy)
	{
		if (board[y * board_wid + x] != BREAKTHROUGH_EMPTY) return FALSE;
		if (x - 1 >= 0 && board[y * board_wid + x - 1] == other) return FALSE;
		if (x + 1 < board_wid && board[y * board_wid + x + 1] == other) return FALSE;
	}
	return TRUE;
}

static gboolean eval_is_blocked (byte *board, int x, int y)
{
	// TODO
	return FALSE;
}

static ResultType breakthrough_eval (Pos *pos, Player player, float *eval)
{
	float wtsum = 0;
	int i, j;
	int wcount[BREAKTHROUGH_BOARD_WID], bcount[BREAKTHROUGH_BOARD_WID];
	float doubled_pawn_penalty = 0.2;
	float edge_pawn_bonus = 0.1;
	float backward_pawn_penalty = 0.5;
	float blocked_pawn_penalty = 0.1;

	int passer_min_white = board_heit, passer_min_black = board_heit;
	
	for (i=0; i<board_wid; i++)
	{
		// cheap optimization trick 
		if (pos->board [0 * board_wid + i] == BREAKTHROUGH_WP && 
				pos->board [(board_heit - 1) * board_wid + i] == BREAKTHROUGH_BP)
			continue;
		for (j=0; j<board_heit; j++)
		{
			int val = pos->board[j * board_wid + i];
			if (val == BREAKTHROUGH_EMPTY) continue;
			if (eval_is_passer (pos->board, i, j))
			{
				if (val == BREAKTHROUGH_WP && (board_wid -1 - j < passer_min_white))
					passer_min_white = board_wid -1 - j;
				if (val == BREAKTHROUGH_BP && (j < passer_min_black))
					passer_min_black = j;
			}
		}
	}
	if (passer_min_white < board_heit || passer_min_black < board_heit)
	{
		int diff = passer_min_white - passer_min_black;
		if (diff < 0 || (diff == 0 && player == WHITE))
		{
			*eval = -diff + 1;
			return RESULT_WHITE;
		}
		if (diff > 0 || (diff == 0 && player == BLACK))
		{
			*eval = -diff - 1;
			return RESULT_BLACK;
		}
	}
	
	for (i=0; i<board_wid; i++)
		wcount[i] = bcount[i] = 0;

	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		int val = pos->board [j * board_wid + i];
		if (val == BREAKTHROUGH_EMPTY) continue;
		if (val == BREAKTHROUGH_WP)
		{
			wtsum += (1 + 0.1 * j);
			if (i == 0 || i == board_wid - 1)
				wtsum += edge_pawn_bonus;
			if (eval_is_backward (pos->board, i, j))
				wtsum -= backward_pawn_penalty;
			wcount[i]++;
		}
		else if (val == BREAKTHROUGH_BP)
		{
			wtsum -= (1 + 0.1 * (board_wid - 1 - j));
			if (i == 0 || i == board_wid - 1)
				wtsum -= edge_pawn_bonus;
			if (eval_is_backward (pos->board, i, j))
				wtsum += backward_pawn_penalty;
			bcount[i]++;
		}
	}
	
	for (i=0; i<board_wid; i++)
	{
		wtsum -= (wcount[i] > 1 ? wcount[i] - 1 : 0);
		wtsum += (bcount[i] > 1 ? bcount[i] - 1 : 0);
	}
	
	*eval = wtsum;
	return RESULT_NOTYET;
}

static ResultType breakthrough_eval_incr (Pos *pos, byte *move, float *eval)
{
	byte *board = pos->board;
	if (move[0] == move[3]) *eval = 0;
	else *eval = (pos->player == WHITE ? 1 : -1);
	*eval += 0.01 * random() / RAND_MAX;
	return RESULT_NOTYET;
}

static byte * breakthrough_movegen (Pos *pos)
{
	int i, j, m, n, xoff, yoff;
	byte movbuf [256];
	byte *movlist, *movp = movbuf;
	byte *board = pos->board;

	// generate a random permutation of the moves
	xoff = random() % board_wid;
	yoff = random() % board_heit;
	for (m=0; m<board_wid; m++)
	for (n=0; n<board_heit; n++)
	{
		int incx, incy;
		i = (m + xoff) % board_wid;
		j = (n + yoff) % board_heit;
		if (board [j * board_wid + i] != 
				(pos->player == WHITE ? BREAKTHROUGH_WP : BREAKTHROUGH_BP))
			continue;
		incy = board [j * board_wid + i] == BREAKTHROUGH_WP ? 1 : -1;
		for (incx = -1; incx <= 1; incx += 1)
		{
			int val;
			if (!ISINBOARD (i + incx, j + incy))
				continue;
			val = board [(j+incy) * board_wid + (i+incx)];
			if ((val == BREAKTHROUGH_EMPTY || val == board [j * board_wid + i]) 
					&& incx != 0)
				continue;
			if (val != BREAKTHROUGH_EMPTY && incx == 0) continue;
			*movp++ = i + incx;
			*movp++ = j + incy;
			*movp++ = (pos->player == WHITE ? BREAKTHROUGH_WP : BREAKTHROUGH_BP);
			*movp++ = i;
			*movp++ = j;
			*movp++ = 0;
			*movp++ = -1;
		}		
	}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}

int breakthrough_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player player, 
		byte **movp, int **rmovp)
{
	static byte move[128];
	byte *mp = move;
	int diffx, diffy;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (breakthrough_curx < 0)
	{
		if ((player == WHITE && pos->board [y * board_wid + x] == BREAKTHROUGH_WP)
				|| (player == BLACK && pos->board [y * board_wid + x] == BREAKTHROUGH_BP))
		{
			int incy = player == WHITE ? 1 : -1;
			int other = player == WHITE ? BREAKTHROUGH_BP : BREAKTHROUGH_WP;
			if ((x == 0 || pos->board [(y + incy) * board_wid + x - 1] != other) &&
				(x == board_wid - 1 || 
				 pos->board [(y + incy) * board_wid + x + 1] != other))
			{
				if (pos->board [(y + incy) * board_wid + x] != BREAKTHROUGH_EMPTY)
					return -1;
				else
				{
					*mp++ = x;
					*mp++ = y;
					*mp++ = 0;
					*mp++ = x;
					*mp++ = y + incy;
					*mp++ = pos->board [y * board_wid + x];
					*mp++ = -1;
					*movp = move;
					return 1;
				}
			}
			else
			{
				breakthrough_curx = x;
				breakthrough_cury = y;
			}
			return 0;
		}
		return -1;
	}
	diffx = x - breakthrough_curx;
	diffy = y - breakthrough_cury;
	if ((player == WHITE && pos->board [y * board_wid + x] == BREAKTHROUGH_WP)
		|| (player == BLACK && pos->board [y * board_wid + x] == BREAKTHROUGH_BP))
	{
		breakthrough_curx = breakthrough_cury = -1;
		return -1;
	}
	else if (pos->board[y * board_wid + x] == BREAKTHROUGH_EMPTY 
			&& 
			(
				(player == WHITE && (diffy != 1 || diffx != 0))
					||
				(player == BLACK && (diffy != -1 || diffx != 0))
			)
	   )
	{
		breakthrough_curx = breakthrough_cury = -1;
		return -1;
	}
	else if (((player == WHITE && pos->board [y * board_wid + x] == BREAKTHROUGH_BP)
		|| (player == BLACK && pos->board [y * board_wid + x] == BREAKTHROUGH_WP))
		&& 
			((player == WHITE && (diffy != 1 || abs (diffx) != 1))
		||(player == BLACK && (diffy != -1 || abs(diffx) != 1))))
	{
		breakthrough_curx = breakthrough_cury = -1;
		return -1;
	}
	*mp++ = x;
	*mp++ = y;
	*mp++ = (player == WHITE ? BREAKTHROUGH_WP : BREAKTHROUGH_BP);
	*mp++ = breakthrough_curx;
	*mp++ = breakthrough_cury;
	*mp++ = 0;
	*mp++ = -1;
	*movp = move;
	breakthrough_curx = breakthrough_cury = -1;
	return 1;
}

