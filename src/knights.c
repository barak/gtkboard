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
#include "gdk/gdkkeysyms.h"
#include "../pixmaps/chess.xpm"
#include "../pixmaps/misc.xpm"

#define KNIGHTS_CELL_SIZE 54
#define KNIGHTS_NUM_PIECES 3

#define KNIGHTS_BOARD_WID 7
#define KNIGHTS_BOARD_HEIT 7

#define KNIGHTS_EMPTY 0
#define KNIGHTS_CLOSED 1
#define KNIGHTS_WN 2
#define KNIGHTS_BN 3

char knights_colors[] = {200, 200, 130, 0, 140, 0};

int knights_initpos [KNIGHTS_BOARD_WID*KNIGHTS_BOARD_HEIT] = 
{
	3 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 2 ,
};


char ** knights_pixmaps [] = 
{
	grey_square_54_xpm,
	chess_wn_54_xpm,
	chess_bn_54_xpm,
};

typedef struct
{
	int num_pauses;
}Knights_state;

static int knights_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static int knights_getmove_kb (Pos *, int, byte ** , int **);
void knights_init ();
static ResultType knights_who_won (Pos *, Player, char **);
static ResultType knights_eval (Pos *, Player, float *eval);
static ResultType knights_eval_real (Pos *, Player, float *eval, gboolean);
static byte * knights_movegen (Pos *);
static void *knights_newstate (Pos *, byte *);

Game Knights = { KNIGHTS_CELL_SIZE, KNIGHTS_BOARD_WID, KNIGHTS_BOARD_HEIT, 
	KNIGHTS_NUM_PIECES, 
	knights_colors, knights_initpos, knights_pixmaps, "Balanced Joust", "Nimlike games",
	knights_init};

void knights_init ()
{
	game_getmove = knights_getmove;
	game_getmove_kb = knights_getmove_kb;
	game_who_won = knights_who_won;
	game_eval = knights_eval;
	game_movegen = knights_movegen;
	game_stateful = TRUE;
	game_state_size = sizeof (Knights_state);
	game_newstate = knights_newstate;
	game_draw_cell_boundaries = TRUE;
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_rules = "Two players take turns in moving their respective knights on a 7x7 chessboard. Squares that have been visited are considered \"eaten\" and cannot be revisited. When the knights are attacking each other, the player to move can pass by hitting Space. If both players pass in the same position, the game is a draw. The goal is to be the last player to make a move.";
	game_doc_strategy = "As the game progresses, there will eventually appear a single square, which, if eaten, will partition the board into two, such that a knight cannot move from one part to the other. The player who eats this square is often at an advantage because they can choose which part to move to.";
}

static int incx[] = { -2, -2, -1, -1, 1, 1, 2, 2};
static int incy[] = { -1, 1, -2, 2, -2, 2, -1, 1};

static void *knights_newstate (Pos *pos, byte *move)
{
	static Knights_state state;
	if (!pos->state)
	{
		state.num_pauses = 0;
		return &state;
	}
	if (move[0] == -1)
		state.num_pauses = ((Knights_state *)pos->state)->num_pauses + 1;
	else state.num_pauses = 0;
	return &state;
}

static void get_cur_pos (byte *board, Player player, int *x, int *y)
{
	int i=0, j=0;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if ((player == WHITE && board [j * board_wid + i] == KNIGHTS_WN)
			|| (player == BLACK && board [j * board_wid + i] == KNIGHTS_BN))
		{
			*x = i;
			*y = j;
			return;
		}
	}
}

ResultType knights_who_won (Pos *pos, Player player, char **commp)
{
	int i=0, j=0, k;
	float eval;
	ResultType result = knights_eval_real (pos, player, &eval, TRUE);
	if (result == RESULT_NOTYET)
		;
	else if (result == RESULT_TIE) *commp = "Draw";
	else if (result == RESULT_WHITE) *commp = "White won";
	else if (result == RESULT_BLACK) *commp = "Black won";
	return result;
}


int knights_getmove_kb (Pos *pos, int key, byte ** movp, int **rmovp)
{
	int i, j, wx = 0, wy = 0, bx = 0, by = 0;
	static byte move[1] = {-1};
	if (key != GDK_p)
		return 0;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (pos->board[j * board_wid + i] == KNIGHTS_WN)
			wx = i, wy = j;
		if (pos->board[j * board_wid + i] == KNIGHTS_BN)
			bx = i, by = j;
	}
	if (abs ((wx - bx) * (wy - by)) != 2)
		return -1;		
	*movp = move;
	return 1;
}

int knights_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player player, 
		byte **movp, int **rmovp)
{
	int curx = -1, cury = -1;
	static byte move[128];
	byte *mp = move;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (pos->board[y * board_wid + x] == (player == WHITE ? KNIGHTS_WN : KNIGHTS_BN))
		return 0;
	if (pos->board[y * board_wid + x] != KNIGHTS_EMPTY)
		return -1;
	get_cur_pos (pos->board, player, &curx, &cury);
	if (abs ((curx - x) * (cury - y)) != 2)
		return -1;
	*mp++ = x;
	*mp++ = y;
	*mp++ = (player == WHITE ? KNIGHTS_WN : KNIGHTS_BN);
	*mp++ = curx;
	*mp++ = cury;
	*mp++ = KNIGHTS_CLOSED;
	*mp++ = -1;
	*movp = move;
	return 1;
}


byte * knights_movegen (Pos *pos)
{
	int i, j, k;
	byte movbuf [64];
	byte *movlist, *movp = movbuf;
	Player player = pos->player;
	get_cur_pos (pos->board, player, &i, &j);
	for (k=0; k<8; k++)
	{
		int x = i + incx[k], y = j + incy[k];
		int val;
		if (!ISINBOARD (x, y)) continue;
		if ((val = pos->board[y * board_wid + x]) == KNIGHTS_EMPTY)
		{
			*movp++ = i;
			*movp++ = j;
			*movp++ = KNIGHTS_CLOSED;
			*movp++ = x;
			*movp++ = y;
			*movp++ = (player == WHITE ? KNIGHTS_WN : KNIGHTS_BN);
			*movp++ = -1;
		}
		else if (val == KNIGHTS_WN || val == KNIGHTS_BN)
		{
			*movp++ = -1;
		}
	}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}

static gboolean eval_disconnected (byte *theboard)
{
	byte board[KNIGHTS_BOARD_WID * KNIGHTS_BOARD_HEIT];
	int stack[KNIGHTS_BOARD_WID * KNIGHTS_BOARD_HEIT];
	int stack_top = 0;
	int i, curx, cury, x, y;

	for (i=0; i<board_wid * board_heit; i++)
		board[i] = theboard[i];

	get_cur_pos (board, WHITE, &curx, &cury);
	
	stack[stack_top++] = cury * board_wid + curx;
	while (stack_top > 0)
	{
		stack_top--;
		curx = stack[stack_top] % board_wid;
		cury = stack[stack_top] / board_wid;
		for (i=0; i<8; i++)
		{
			x = curx + incx[i];
			y = cury + incy[i];
			if (!ISINBOARD (x, y)) continue;
			if (board[y * board_wid + x] == KNIGHTS_BN)
				return FALSE;
			if (board[y * board_wid + x] != KNIGHTS_EMPTY)
				continue;
			board[y * board_wid + x] = KNIGHTS_CLOSED;
			stack[stack_top++] = y * board_wid + x;
		}
	}
	return TRUE;
}

// exhaustive DFS to solve the position exactly
static int eval_max_path_len (byte *theboard, Player player)
{
	byte board[KNIGHTS_BOARD_WID * KNIGHTS_BOARD_HEIT];
	int stack [KNIGHTS_BOARD_WID * KNIGHTS_BOARD_HEIT];
	int current [KNIGHTS_BOARD_WID * KNIGHTS_BOARD_HEIT];
	int stack_top = 0;
	int i, curx, cury, x, y;
	int max_len = 0;

	for (i=0; i<board_wid * board_heit; i++)
		board[i] = theboard[i];

	get_cur_pos (board, player, &curx, &cury);
	
	current[stack_top] = 0;
	stack[stack_top] = cury * board_wid + curx;

	while (stack_top >= 0)
	{
		if (stack_top > max_len)
			max_len = stack_top;
		i = current[stack_top]++;
		if (i == 8)
		{
			stack_top--;
			continue;
		}
		curx = stack[stack_top] % board_wid;
		cury = stack[stack_top] / board_wid;
		x = curx + incx[i];
		y = cury + incy[i];
		if (!ISINBOARD (x, y)) continue;
		if (board[y * board_wid + x] != KNIGHTS_EMPTY)
			continue;
		board[y * board_wid + x] = KNIGHTS_CLOSED;
		stack_top++;
		current[stack_top] = 0;
		stack[stack_top] = y * board_wid + x;
	}
	return max_len;
}

// We may want to continue the game even when a result is apparent. The
// parameter strict is for this. who_won() sets it to TRUE and eval() to FALSE.
static ResultType knights_eval_real (Pos *pos, Player player, float *eval, gboolean strict)
{
	int i, j, k;
	int wcnt = 0, bcnt = 0;
	static int disconn_cnt [2 * KNIGHTS_BOARD_WID * KNIGHTS_BOARD_HEIT] = {0};
	static int total_cnt [2 * KNIGHTS_BOARD_WID * KNIGHTS_BOARD_HEIT] = {0};

	if (pos->state && ((Knights_state *)pos->state)->num_pauses >= 2)
	{
		*eval = 0;
		return RESULT_TIE;
	}
	
	if (!strict && eval_disconnected (pos->board))
	{
		int wlen = 0, blen = 0;
		disconn_cnt [pos->num_moves]++;
		total_cnt[pos->num_moves]++;
		wlen = eval_max_path_len (pos->board, WHITE);
		blen = eval_max_path_len (pos->board, BLACK);
		*eval = 2 * (wlen - blen) + (player == WHITE ? -1 : 1);
		if (wlen > blen) return RESULT_WHITE;
		else if (wlen < blen) return RESULT_BLACK;
		else return player == WHITE ? RESULT_BLACK : RESULT_WHITE;
	}

	total_cnt[pos->num_moves]++;
//	if (total_cnt[pos->num_moves] % 10000 == 0) printf ("Ply: %d;\t total: %d;\t disc: %d\n", pos->num_moves, total_cnt[pos->num_moves], disconn_cnt[pos->num_moves]);

	get_cur_pos (pos->board, WHITE, &i, &j);
	for (k=0; k<8; k++)
	{
		int x = i + incx[k], y = j + incy[k];
		if (!ISINBOARD (x, y)) continue;
		if (pos->board[y * board_wid + x] == KNIGHTS_EMPTY)
			wcnt++;
	}

	get_cur_pos (pos->board, BLACK, &i, &j);
	for (k=0; k<8; k++)
	{
		int x = i + incx[k], y = j + incy[k];
		if (!ISINBOARD (x, y)) continue;
		if (pos->board[y * board_wid + x] == KNIGHTS_EMPTY)
			bcnt++;
	}
	*eval = wcnt - bcnt;
	if (player == WHITE && wcnt == 0)
	{
		if (bcnt == 0) *eval -= 1;
		*eval *= GAME_EVAL_INFTY;
		return RESULT_BLACK;
	}
	if (player == BLACK && bcnt == 0)
	{
		if (wcnt == 0) *eval += 1;
		*eval *= GAME_EVAL_INFTY;
		return RESULT_WHITE;
	}
	return RESULT_NOTYET;
}

ResultType knights_eval (Pos *pos, Player player, float *eval)
{
	return knights_eval_real (pos, player, eval, FALSE);
}
