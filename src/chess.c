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
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "game.h"
#include "../pixmaps/chess.xpm"

#define CHESS_CELL_SIZE 54
#define CHESS_NUM_PIECES 12

#define CHESS_BOARD_WID 8
#define CHESS_BOARD_HEIT 8

#define CHESS_WK 1
#define CHESS_WQ 2
#define CHESS_WR 3
#define CHESS_WB 4
#define CHESS_WN 5
#define CHESS_WP 6
#define CHESS_BK 7
#define CHESS_BQ 8
#define CHESS_BR 9
#define CHESS_BB 10
#define CHESS_BN 11
#define CHESS_BP 12

#define CHESS_ISWHITE(x) (x >= 1 && x <= 6)
#define CHESS_ISBLACK(x) (x >= 7 && x <= 12)

#ifndef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
#endif

char chess_colors[] = 
	{200, 200, 130, 
	0, 140, 0};

int	chess_initpos[] = 
{
	 9 , 11, 10, 8 , 7 , 10, 11, 9  ,
	 12, 12, 12, 12, 12, 12, 12, 12 ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6  ,
	 3 , 5 , 4 , 2 , 1 , 4 , 5 , 3  ,
};
static int chess_max_moves = 200;

char ** chess_pixmaps[] =

{
	chess_wk_54_xpm, 
	chess_wq_54_xpm, 
	chess_wr_54_xpm, 
	chess_wb_54_xpm, 
	chess_wn_54_xpm, 
	chess_wp_54_xpm, 
	chess_bk_54_xpm, 
	chess_bq_54_xpm, 
	chess_br_54_xpm, 
	chess_bb_54_xpm, 
	chess_bn_54_xpm, 
	chess_bp_54_xpm,
};



void chess_init ();
int chess_getmove (Pos *, int, int, GtkboardEventType, Player, byte **);
ResultType chess_who_won (Pos *, Player, char **);
byte *chess_movegen (Pos *, Player );
float chess_eval (Pos *, Player);
void *chess_newstate (Pos *, byte *);
	
Game Chess = 
	{ CHESS_CELL_SIZE, CHESS_BOARD_WID, CHESS_BOARD_HEIT, 
	CHESS_NUM_PIECES,
	chess_colors, chess_initpos, chess_pixmaps, "Chess",
	chess_init};

typedef struct 
{
	int castle_WK:2;
	int castle_WQ:2;
	int castle_BK:2;
	int castle_BQ:2;
	int epfile:4;
} Chess_state;

//Chess_state state;

void chess_init ()
{
	game_getmove = chess_getmove;
	game_who_won = chess_who_won;
	game_movegen = chess_movegen;
	game_eval = chess_eval;
	game_stateful = TRUE;
	game_state_size = sizeof (Chess_state);
	game_newstate = chess_newstate;
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	game_doc_about = 
		"Chess\n"
		"Two player game\n"
		"Status: Partially implemented (currently unplayable)\n"
		"URL: "GAME_DEFAULT_URL("chess");
}

void *chess_newstate (Pos *pos, byte *move)
{
	static Chess_state state = {0, 0, 0, 0, -1};
	if (!pos->state) return &state;
	memcpy (&state, pos->state, sizeof (Chess_state));

	// if pawn moves two squares to 4th rank then set epf
	// FIXME: all this is useless until state can be sent
	// from server to client
	{
	int val;
	if (move[2] != 0) val = move[2];
	else if (move[5] != 0) val = move[5];
	else assert (0);
	if (
		(val == CHESS_WP 
			&& ((move[0] == 1 && move[3] == 3) || (move[0] == 2 && move[3] == 1)))
		||
		(val == CHESS_WP 
			&& ((move[0] == 1 && move[3] == 3) || (move[0] == 2 && move[3] == 1)))
	)
		state.epfile = move[1];
	else state.epfile = -1;
	} 
	
	return &state;
}
	
static int isfreeline (byte *pos, int oldx, int oldy, int newx, int newy)
{
	int x = oldx, y = oldy, dx, dy, diffx = newx - oldx, diffy = newy - oldy;
	if (abs (diffx) != abs(diffy) && diffx != 0 && diffy != 0)
		return 0;
	dx = (diffx ? (diffx / abs (diffx)) : 0);
	dy = (diffy ? (diffy / abs (diffy)) : 0);
	for (x+=dx, y+=dy; x != newx || y != newy; x+=dx, y+=dy)
		if (pos [y * board_wid + x] != 0)
			return 0;
	return 1;	
}

static int islegal (Pos *pos, int oldx, int oldy, int x, int y, int player)
{
	int piece = pos->board [oldy * board_wid + oldx];
	byte *board = pos->board;
	switch (piece)
	{
		case CHESS_WK:
		case CHESS_BK:
			{
				int rank, rook;
				int diffx = abs (x - oldx), diffy = abs (y - oldy);
				if (diffx <= 1 && diffy <= 1) return 1;
				// castling
				rank = (piece == CHESS_WK ? 0 : 7);
				rook = (piece == CHESS_WK ? CHESS_WR : CHESS_BR);
				if (oldx != 4) return 0;
				if (oldy != rank || y != rank) return 0;
				// TODO: not yet checking if 
				// 	- the intervening squares are uncontrolled
				// 	- king hasn't moved yet
				if (x == 6)
				{
					if (board [rank * board_wid + 7] != rook) return 0;
					if (board [rank * board_wid + 6] != 0) return 0;
					if (board [rank * board_wid + 5] != 0) return 0;
					return 1;
				}
				else if (x == 2)
				{
					if (board [rank * board_wid + 0] != rook) return 0;
					if (board [rank * board_wid + 1] != 0) return 0;
					if (board [rank * board_wid + 2] != 0) return 0;
					if (board [rank * board_wid + 3] != 0) return 0;
					return 1;
				}
				return 0;
			}
		case CHESS_WQ:
		case CHESS_BQ:
			return isfreeline (board, oldx, oldy, x, y);
		case CHESS_WR:
		case CHESS_BR:
			if (!isfreeline (board, oldx, oldy, x, y))
				return 0;
			if (oldx == x || oldy == y)
				return 1;
			return 0;
		case CHESS_WB:
		case CHESS_BB:
			if (!isfreeline (board, oldx, oldy, x, y))
				return 0;
			if (oldx == x || oldy == y)
				return 0;
			return 1;
		case CHESS_WN:
		case CHESS_BN:
			{
				int diffx = abs (x - oldx), diffy = abs (y - oldy);
				if (diffx == 2 && diffy == 1)
					return 1;
				if (diffx == 1 && diffy == 2)
					return 1;
				return 0;
			}
		case CHESS_WP:
			if (board [y * board_wid + x] == 0)
				return (x == oldx && (y == oldy + 1 || 
						(y == 3 && oldy == 1 && board [(2*board_wid + x)] == 0)));
			return y == oldy + 1  && (x == oldx + 1 || x == oldx - 1) ;
		case CHESS_BP:
			if (board [y * board_wid + x] == 0)
				return (x == oldx && (y == oldy - 1 ||
						(y == 4 && oldy == 6 && board [5*board_wid + x] == 0)));
			return y == oldy - 1  && (x == oldx + 1 || x == oldx - 1) ;
		default:
			return 1;
	}
}

static int oppcolor (byte *pos, int oldx, int oldy, int x, int y)
	/* True if one square is W and the other is B */
{
	int oldv = pos [oldy * board_wid + oldx], v = pos [y * board_wid + x];
	if (CHESS_ISWHITE (oldv) && CHESS_ISBLACK (v)) return 1;
	if (CHESS_ISBLACK (oldv) && CHESS_ISWHITE (v)) return 1;
	return 0;
}

int chess_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player player, 
		byte ** movep)
	/* Translate mouse clicks into move */
{
	static byte move [13];
	static int oldx = -1, oldy = -1, oldval = -1;
	static int prom = 0, prom_x, prom_oldx;
	int val;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;

	if (oldx >= 0 && x == oldx && y == oldy)
	{
		oldx = -1; oldy = -1; return 0;
	}
	
	/* pawn promotion */ 
	if (prom)
	{
		int new_p = -1;
		prom = 0;
		switch(x)
		{
			case 0: case 7:
				new_p = (player == WHITE ? CHESS_WR : CHESS_BR); break;
			case 1: case 6:
				new_p = (player == WHITE ? CHESS_WN : CHESS_BN); break;
			case 2: case 5:
				new_p = (player == WHITE ? CHESS_WB : CHESS_BB); break;
			case 3: case 4:
				new_p = (player == WHITE ? CHESS_WQ : CHESS_BQ); break;
		}
		move[0] = prom_oldx;
		move[1] = (player == WHITE ? 6 : 1);
		move[2] = 0;
		move[3] = prom_x;
		move[4] = (player == WHITE ? 7 : 0);
		move[5] = new_p;
		move[6] = -1;
		
		if (movep)
			*movep = move;
		return 1;
	}
	val = pos->board [y * board_wid + x];
	if (oldx == -1)
	{
		if (val == 0) return -1;
		if (player == WHITE && !CHESS_ISWHITE (val))
			return -1;
		if (player == BLACK && !CHESS_ISBLACK (val))
			return -1;
		oldx = x; oldy = y, oldval = val;
		return 0;
	}
	if (player == WHITE && CHESS_ISWHITE (val))
	{
		oldx = -1; oldy = -1;
		return -1;
	}
	if (player == BLACK && CHESS_ISBLACK (val))
	{
		oldx = -1; oldy = -1;
		return -1;
	}

	if (!islegal (pos, oldx, oldy, x, y, player))
	{
		oldx = -1; oldy = -1;
		return -1;
	}

	/* pawn promotion */
	if ((oldval == CHESS_WP || oldval == CHESS_BP) &&
			(y == 0 || y == board_heit - 1))
	{
		prom = 1;
		prom_oldx = oldx;
		prom_x = x;
		oldx = oldy = -1;
		return 0;
	}

	/* castling */
	if ((oldval == CHESS_WK || oldval == CHESS_BK)
			&& abs (x - oldx) > 1)
	{
		int rfile = x == 6 ? 7 : 0;
		int rook = pos->board [y * board_wid + rfile];
		move[0] = oldx; move[1] = oldy; move[2] = 0;
		move[3] = x; move[4] = y; move[5] = oldval;
		move[6] = (oldx + x)/2; move[7] = y; move[8] = rook;
		move[9] = rfile; move[10] = y; move[11] = 0;
		move[12] = -1;
		oldx = -1, oldy = -1;
		if (movep)
			*movep = move;
		return 1;
	}

	move[0] = oldx; move[1] = oldy; move[2] = 0;
	move[3] = x; move[4] = y; move[5] = oldval;
	move[6] = -1;

	oldx = -1; oldy = -1;
	
	if (movep)
		*movep = move;
	return 1;
	/*if (player == WHITE && CHESS_ISWHITE (val))
		return 0;*/
}

static int hasmove (Pos *pos, int player)
{
	int x1, y1, x2, y2, val;
	for (x1 = 0; x1 < board_wid; x1++)
	for (y1 = 0; y1 < board_heit; y1++)
	{
		val = pos->board [y1 * board_wid + x1];
		if (player == WHITE && !CHESS_ISWHITE (val)) continue;
		if (player == BLACK && !CHESS_ISBLACK (val)) continue;
		for (x2 = 0; x2 < board_wid; x2++)
		for (y2 = 0; y2 < board_heit; y2++)
			if (islegal (pos, x1, y1, x2, y2, player)) return 1;
	}
	return 0;
}

ResultType chess_who_won (Pos *pos, Player player, char **commp)
{
	static char comment[32];
	char *who_str [3] = { "white won", "black won", "draw" };
	*commp = NULL;
	if (hasmove (pos, player)) //return RESULT_NOTYET;
	{
		if (pos->num_moves > chess_max_moves)
		{
			fprintf (stderr, "max moves reached\n");
			snprintf (comment, 32, "%s", who_str[2]);
			*commp = comment;
			return RESULT_TIE;
		}
		return RESULT_NOTYET;
	}
	*commp = comment;
	if (hasmove (pos, player == WHITE ? BLACK : WHITE))
	{
		strncpy (comment, who_str[player == WHITE ? 0 : 1], 31);
		return player == WHITE ? RESULT_WHITE : RESULT_BLACK;
	}
	else 
	{
		strncpy (comment, who_str[2], 31);
		return RESULT_TIE;
	}
}

int chess_movegen_square (byte *pos, byte **movp, int player,
		int oldx, int oldy, int x, int y)
{
	int val;
	if (!ISINBOARD(x, y)) return 0;
	val = pos [y * board_heit + x];
	if ((player == WHITE && CHESS_ISWHITE (val))
		|| (player == BLACK && CHESS_ISBLACK (val)))
		return 0;
	*(*movp)++ = oldx;
	*(*movp)++ = oldy;
	*(*movp)++ = 0;
	*(*movp)++ = x;
	*(*movp)++ = y;
	*(*movp)++ = pos [oldy * board_heit + oldx];
	*(*movp)++ = -1;
	return 1;
}

int chess_movegen_line (byte *pos, byte **movp, int player,
		int x, int y, int incx, int incy)
{
	int oldx = x, oldy = y;
	int val;
	do
	{
		x += incx;
		y += incy;
		val = pos [y * board_heit + x];
		if (ISINBOARD (x, y) && oppcolor (pos, oldx, oldy, x, y))
			break;
	} while (chess_movegen_square (pos, movp, player, oldx, oldy, x, y));
	return 0;
}

static void chess_movegen_promote (byte *board, byte **movp, int player,
		int oldx, int oldy, int x, int y)
{
	int i, j;
	int promote_pieces[2][4] = {
		{CHESS_WQ, CHESS_WR, CHESS_WB, CHESS_WN},
		{CHESS_BQ, CHESS_BR, CHESS_BB, CHESS_BN}
	};
	i = (player == WHITE ? 0 : 1);
	for(j=0; j<4; j++) 
	{
		*(*movp)++ = oldx;
		*(*movp)++ = oldy;
		*(*movp)++ = 0;
		*(*movp)++ = x;
		*(*movp)++ = y;
		*(*movp)++ = promote_pieces[i][j];
		*(*movp)++ = -1;
	}
}

byte *chess_movegen (Pos *pos, Player player)
{
	byte realbuf[4096];
	byte *realp = realbuf;
	byte movbuf[4096], *movp = movbuf;
	byte *movlist;
	int i, j, k, x, y;
	int incxr[] = {0, 0, 1, -1};
	int incyr[] = {1, -1, 0, 0};
	int incxb[] = {1, 1, -1, -1};
	int incyb[] = {1, -1, 1, -1};
	int incxn[] = {2, 2, -2, -2, 1, 1, -1, -1};
	int incyn[] = {1, -1, 1, -1, 2, -2, 2, -2};
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		int val = board[j * board_heit + i];
		if ((player == WHITE && CHESS_ISWHITE (val))
			|| (player == BLACK && CHESS_ISBLACK (val)))
		{
			switch (val)
			{
				case CHESS_WP:
					if (ISINBOARD (i, j+1) && board [(j+1) * board_heit + i] == 0)
					{
						if (j == board_heit - 2)
							chess_movegen_promote(board, &movp, player,
								i, j, i, j+1);
						else
							chess_movegen_square (board, &movp, player,
								i, j, i, j+1);
					}
					if (ISINBOARD (i+1, j+1) && 
							CHESS_ISBLACK (board [(j+1) * board_heit + i+1]))
					{
						if (j == board_heit - 2)
							chess_movegen_promote (board, &movp, player,
								i, j, i+1, j+1);
						else
							chess_movegen_square (board, &movp, player,
								i, j, i+1, j+1);
					}
					if (ISINBOARD (i-1, j+1) && 
							CHESS_ISBLACK (board [(j+1) * board_heit + i-1]))
					{
						if (j == board_heit - 2)
							chess_movegen_promote (board, &movp, player,
								i, j, i-1, j+1);
						else
							chess_movegen_square (board, &movp, player,
								i, j, i-1, j+1);
					}
					if (j == 1 && board [2 * board_heit + i] == 0
							&& board [3 * board_heit + i] == 0)
						chess_movegen_square (board, &movp, player,
								i, j, i, j+2);
					break;
				case CHESS_BP:
					if (ISINBOARD (i, j-1) && board [(j-1) * board_heit + i] == 0)
					{
						if (j == 1)
							chess_movegen_promote (board, &movp, player,
								i, j, i, j-1);
						else
							chess_movegen_square (board, &movp, player,
								i, j, i, j-1);
					}
					if (ISINBOARD (i+1, j-1) && 
							CHESS_ISWHITE (board [(j-1) * board_heit + i+1]))
					{
						if (j == 1)
							chess_movegen_promote (board, &movp, player,
								i, j, i+1, j-1);
						else
							chess_movegen_square (board, &movp, player,
								i, j, i+1, j-1);
					}
					if (ISINBOARD (i-1, j-1) && 
							CHESS_ISWHITE (board [(j-1) * board_heit + i-1]))
					{
						if (j == 1)
							chess_movegen_promote (board, &movp, player,
								i, j, i-1, j-1);
						else
							chess_movegen_square (board, &movp, player,
								i, j, i-1, j-1);
					}
					if (j == 6 && board [5 * board_heit + i] == 0
							&& board [4 * board_heit + i] == 0)
						chess_movegen_square (board, &movp, player,
								i, j, i, j-2);
					break;
				case CHESS_WK:
				case CHESS_BK:
					for (k=0; k<4; k++)
						chess_movegen_square(board, &movp, player, 
								i, j, i + incxr[k], j + incyr[k]);
					for (k=0; k<4; k++)
						chess_movegen_square(board, &movp, player, 
								i, j, i + incxb[k], j + incyb[k]);
					break;
				case CHESS_WB:
				case CHESS_BB:
					for (k=0; k<4; k++)
						chess_movegen_line (board, &movp, player, 
								i, j, incxb[k], incyb[k]);
					break;
				case CHESS_WR:
				case CHESS_BR:
					for (k=0; k<4; k++)
						chess_movegen_line (board, &movp, player, 
								i, j, incxr[k], incyr[k]);
					break;
				case CHESS_WQ:
				case CHESS_BQ:
					for (k=0; k<4; k++)
						chess_movegen_line (board, &movp, player, 
								i, j, incxb[k], incyb[k]);
					for (k=0; k<4; k++)
						chess_movegen_line (board, &movp, player, 
								i, j, incxr[k], incyr[k]);
					break;
				case CHESS_WN:
				case CHESS_BN:
					for (k=0; k<8; k++)
						chess_movegen_square(board, &movp, player, 
								i, j, i + incxn[k], j + incyn[k]);
					break;
			}
		}
	}
	*movp++ = -2;
	
	movlist = (byte *) malloc (movp - movbuf);
	memcpy (movlist, movbuf, movp - movbuf);
	return movlist;

	/*movp = movbuf;
	realp = realbuf;
	while (1)
	{
		byte *tmp;
		int w = 0, b = 0;
		if (*movp == -2)
			break;
		for (tmp = movp; *tmp != -1; tmp += 3)
		{
			if (CHESS_ISWHITE (board [tmp[1] * board_heit + tmp[0]]))	w = 1;
			if (CHESS_ISBLACK (board [tmp[1] * board_heit + tmp[0]]))	b = 1;
		}
		if (w && b)
		{
			while (*movp != -1)
			{
				*realp++ = *movp++;
				*realp++ = *movp++;
				*realp++ = *movp++;
			}
			*realp++ = *movp++;
		}
		else 
			movp = tmp+1;
	}
	*realp++ = -2;
	movlist = (byte *) malloc (realp - realbuf);
	memcpy (movlist, realbuf, realp - realbuf);
	return movlist;
	*/
}

float getweight (byte val)
{
	if (val == 0)
		return 0;
	switch (val)
	{
		case CHESS_WK: return 100;
		case CHESS_WQ: return 9;
		case CHESS_WR: return 5;
		case CHESS_WB: return 3;
		case CHESS_WN: return 3;
		case CHESS_WP: return 1;
		case CHESS_BK: return -100;
		case CHESS_BQ: return -9;
		case CHESS_BR: return -5;
		case CHESS_BB: return -3;
		case CHESS_BN: return -3;
		case CHESS_BP: return -1;
		default: return 0;
	}
}

float chess_eval (Pos * pos, Player player)
{
	float sum = 0;
	int i;
	for (i=0; i<board_wid * board_heit; i++)
		sum += getweight (pos->board [i]);
	return sum;
}
