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

#include "game.h"
#include "../pixmaps/chess.xpm"


#define ANTICHESS_CELL_SIZE 54
#define ANTICHESS_NUM_PIECES 12

#define ANTICHESS_BOARD_WID 8
#define ANTICHESS_BOARD_HEIT 8

#define ANTICHESS_EMPTY 0
#define ANTICHESS_WK 1
#define ANTICHESS_WQ 2
#define ANTICHESS_WR 3
#define ANTICHESS_WB 4
#define ANTICHESS_WN 5
#define ANTICHESS_WP 6
#define ANTICHESS_BK 7
#define ANTICHESS_BQ 8
#define ANTICHESS_BR 9
#define ANTICHESS_BB 10
#define ANTICHESS_BN 11
#define ANTICHESS_BP 12

#define ANTICHESS_ISWHITE(x) (x >= 1 && x <= 6)
#define ANTICHESS_ISBLACK(x) (x >= 7 && x <= 12)

#ifndef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
#endif

static char antichess_colors[] = 
	{200, 200, 130, 
	0, 140, 0};
char antichess_highlight_colors[9] = {0xff, 0, 0};

static int antichess_init_pos[] = 
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
static int antichess_max_moves = 200;

static char ** antichess_pixmaps[] =

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



void antichess_init ();
int antichess_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
ResultType antichess_who_won (Pos *, Player, char **);
byte *antichess_movegen (Pos *);
ResultType antichess_eval (Pos *, Player, float *);
ResultType antichess_eval_incr (Pos *, Player, byte *, float *);
	
Game Antichess = 
	{ ANTICHESS_CELL_SIZE, ANTICHESS_BOARD_WID, ANTICHESS_BOARD_HEIT, 
	ANTICHESS_NUM_PIECES,
	antichess_colors, antichess_init_pos, antichess_pixmaps, "Antichess", 
	"Chess variants",
	antichess_init};

Game * plugin_game = &Antichess;

void antichess_init ()
{
	game_getmove = antichess_getmove;
	game_who_won = antichess_who_won;
	game_movegen = antichess_movegen;
	game_eval = antichess_eval;
//	game_eval_incr = antichess_eval_incr;
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	game_highlight_colors = antichess_highlight_colors;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Antichess\n"
		"Two player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL("antichess");
	game_doc_rules = 
		"The objective of the game is to force your opponent to capture all your pieces or to stalemate you. What makes this possible is that if you can capture, then you MUST.\n"
		"\n"
		"The pieces move as in chess. However:\n"
		"   - The king is NOT a special piece. No check(mate), no castling. The king can be captured like any other piece.\n"
		"   - No taking en passant.\n"
		"   - Pawn promotion is as in chess.\n"
		"\n"
		"A couple of notes on the gtkboard implementation of chess and antichess:\n"
		"   - Drag and drop doesn't work (yet). To make the move 1. e4, you have to click on e2 and then on e4.\n"
		"   - To promote, after clicking on the pawn and then on the promotion square, you have to click on a third square which indicates which piece you want to promote to. This square should be any one of the squares of the file of that piece. (Thus, to queen, click anywhere on the 'd' file, etc.)";
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
		case ANTICHESS_WK:
		case ANTICHESS_BK:
			{
				int diffx = abs (x - oldx), diffy = abs (y - oldy);
				return diffx <= 1 && diffy <= 1;
			}
		case ANTICHESS_WQ:
		case ANTICHESS_BQ:
			return isfreeline (board, oldx, oldy, x, y);
		case ANTICHESS_WR:
		case ANTICHESS_BR:
			if (!isfreeline (board, oldx, oldy, x, y))
				return 0;
			if (oldx == x || oldy == y)
				return 1;
			return 0;
		case ANTICHESS_WB:
		case ANTICHESS_BB:
			if (!isfreeline (board, oldx, oldy, x, y))
				return 0;
			if (oldx == x || oldy == y)
				return 0;
			return 1;
		case ANTICHESS_WN:
		case ANTICHESS_BN:
			{
				int diffx = abs (x - oldx), diffy = abs (y - oldy);
				if (diffx == 2 && diffy == 1)
					return 1;
				if (diffx == 1 && diffy == 2)
					return 1;
				return 0;
			}
		case ANTICHESS_WP:
			if (board [y * board_wid + x] == 0)
				return (x == oldx && (y == oldy + 1 || 
						(y == 3 && oldy == 1 && board [(2*board_wid + x)] == 0)));
			return y == oldy + 1  && (x == oldx + 1 || x == oldx - 1) ;
		case ANTICHESS_BP:
			if (board [y * board_wid + x] == 0)
				return (x == oldx && (y == oldy - 1 ||
						(y == 4 && oldy == 6 && board [5*board_wid + x] == 0)));
			return y == oldy - 1  && (x == oldx + 1 || x == oldx - 1) ;
		default:
			return 1;
	}
}

static int can_capture (Pos *pos, int player)
{
	int x1, y1, x2, y2, val;
	for (x1 = 0; x1 < board_wid; x1++)
	for (y1 = 0; y1 < board_heit; y1++)
	{
		val = pos->board [y1 * board_wid + x1];
		if (player == WHITE && !ANTICHESS_ISWHITE (val)) continue;
		if (player == BLACK && !ANTICHESS_ISBLACK (val)) continue;
		for (x2 = 0; x2 < board_wid; x2++)
		for (y2 = 0; y2 < board_heit; y2++)
		{
			val = pos->board [y2 * board_wid + x2];
			if (player == WHITE && !ANTICHESS_ISBLACK (val)) continue;
			if (player == BLACK && !ANTICHESS_ISWHITE (val)) continue;
			if (islegal (pos, x1, y1, x2, y2, player)) return 1;
		}		
	}
	return 0;
}

static int oppcolor (byte *pos, int oldx, int oldy, int x, int y)
	/* True if one square is W and the other is B */
{
	int oldv = pos [oldy * board_wid + oldx], v = pos [y * board_wid + x];
	if (ANTICHESS_ISWHITE (oldv) && ANTICHESS_ISBLACK (v)) return 1;
	if (ANTICHESS_ISBLACK (oldv) && ANTICHESS_ISWHITE (v)) return 1;
	return 0;
}

static int oldx = -1, oldy = -1, oldval = -1;
static int prom = 0, prom_x, prom_oldx;

void antichess_free ()
{
	oldx = -1, oldy = -1, prom = 0;
}

int antichess_getmove (Pos *pos, int x, int y, 
		GtkboardEventType type, Player player, byte ** movep, int ** rmovep)
	/* Translate mouse clicks into move */
{
	static byte move [7];
	static int rmove [7];
	int *rp = rmove;
	int val;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;

	val = pos->board [y * board_wid + x];

	if (oldx >= 0 && x == oldx && y == oldy)
	{
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		oldx = -1; oldy = -1; return 0;
	}
	
	// if you can capture you must capture
	if (oldx >= 0 && can_capture (pos, player) && 
			!((ANTICHESS_ISWHITE (val) && ANTICHESS_ISBLACK (oldval))
			 || (ANTICHESS_ISBLACK (val) && ANTICHESS_ISWHITE (oldval))))
	{
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		oldx = oldy = -1;
		return -1;
	}
	
	/* pawn promotion */ 
	if (prom)
	{
		int new_p = -1;
		prom = 0;
		switch(x)
		{
			case 0: case 7:
				new_p = (player == WHITE ? ANTICHESS_WR : ANTICHESS_BR); break;
			case 1: case 6:
				new_p = (player == WHITE ? ANTICHESS_WN : ANTICHESS_BN); break;
			case 2: case 5:
				new_p = (player == WHITE ? ANTICHESS_WB : ANTICHESS_BB); break;
			case 3: case 4:
				new_p = (player == WHITE ? ANTICHESS_WQ : ANTICHESS_BQ); break;
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
	if (oldx == -1)
	{
		if (val == 0) return -1;
		if (player == WHITE && !ANTICHESS_ISWHITE (val))
			return -1;
		if (player == BLACK && !ANTICHESS_ISBLACK (val))
			return -1;
		oldx = x; oldy = y, oldval = val;
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_HIGHLIGHT1; *rp++ = -1; *rmovep = rmove;
		return 0;
	}
	if (player == WHITE && ANTICHESS_ISWHITE (val))
	{
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		oldx = -1; oldy = -1;
		return -1;
	}
	if (player == BLACK && ANTICHESS_ISBLACK (val))
	{
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		oldx = -1; oldy = -1;
		return -1;
	}

	if (!islegal (pos, oldx, oldy, x, y, player))
		// FIXME
		//|| (can_capture (pos, player) && !oppcolor (pos, oldx, oldy, x, y)))
	{
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		oldx = -1; oldy = -1;
		return -1;
	}

	/* pawn promotion */
	if ((oldval == ANTICHESS_WP || oldval == ANTICHESS_BP) &&
			(y == 0 || y == board_heit - 1))
	{
		prom = 1;
		prom_oldx = oldx;
		prom_x = x;
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		oldx = oldy = -1;
		return 0;
	}

	move[0] = oldx; move[1] = oldy; move[2] = 0;
	move[3] = x; move[4] = y; move[5] = pos->board [oldy * board_wid + oldx];
	move[6] = -1;

	*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
	oldx = -1; oldy = -1;
	
	if (movep)
		*movep = move;
	return 1;
	/*if (player == WHITE && ANTICHESS_ISWHITE (val))
		return 0;*/
}

static int hasmove (Pos *pos, int player)
{
	int x1, y1, x2, y2, val;
	for (x1 = 0; x1 < board_wid; x1++)
	for (y1 = 0; y1 < board_heit; y1++)
	{
		val = pos->board [y1 * board_wid + x1];
		if (player == WHITE && !ANTICHESS_ISWHITE (val)) continue;
		if (player == BLACK && !ANTICHESS_ISBLACK (val)) continue;
		for (x2 = 0; x2 < board_wid; x2++)
		for (y2 = 0; y2 < board_heit; y2++)
			if (islegal (pos, x1, y1, x2, y2, player)) return 1;
	}
	return 0;
}

ResultType antichess_who_won (Pos *pos, Player player, char **commp)
{
	static char comment[32];
	char *who_str [3] = { "White won", "Black won", "Draw" };
	*commp = NULL;
	if (hasmove (pos, player)) //return RESULT_NOTYET;
	{
		if (pos->num_moves > antichess_max_moves)
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

int antichess_movegen_square (byte *pos, byte **movp, int player,
		int oldx, int oldy, int x, int y)
{
	int val;
	if (!ISINBOARD(x, y)) return 0;
	val = pos [y * board_heit + x];
	if ((player == WHITE && ANTICHESS_ISWHITE (val))
		|| (player == BLACK && ANTICHESS_ISBLACK (val)))
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

int antichess_movegen_line (byte *pos, byte **movp, int player,
		int x, int y, int incx, int incy)
{
	int oldx = x, oldy = y;
	int capture = 0, val;
	do
	{
		if (capture) return 1;
		x += incx;
		y += incy;
		val = pos [y * board_heit + x];
		if (ISINBOARD (x, y) && oppcolor (pos, oldx, oldy, x, y))
			capture = 1;
	} while (antichess_movegen_square (pos, movp, player, oldx, oldy, x, y));
	return 0;
}

static void antichess_movegen_promote (byte *board, byte **movp, int player,
		int oldx, int oldy, int x, int y)
{
	int i, j;
	int promote_pieces[2][4] = {
		{ANTICHESS_WQ, ANTICHESS_WR, ANTICHESS_WB, ANTICHESS_WN},
		{ANTICHESS_BQ, ANTICHESS_BR, ANTICHESS_BB, ANTICHESS_BN}
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

byte *antichess_movegen (Pos *pos)
{
	byte realbuf[4096];
	byte *realp = realbuf;
	byte movbuf[4096], *movp = movbuf;
	byte *movlist;
	int i, j, k, x, y, capture = 0;
	int incxr[] = {0, 0, 1, -1};
	int incyr[] = {1, -1, 0, 0};
	int incxb[] = {1, 1, -1, -1};
	int incyb[] = {1, -1, 1, -1};
	int incxn[] = {2, 2, -2, -2, 1, 1, -1, -1};
	int incyn[] = {1, -1, 1, -1, 2, -2, 2, -2};
	byte *board = pos->board;
	Player player = pos->player;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		int val = board[j * board_heit + i];
		if ((player == WHITE && ANTICHESS_ISWHITE (val))
			|| (player == BLACK && ANTICHESS_ISBLACK (val)))
		{
			switch (val)
			{
				case ANTICHESS_WP:
					if (ISINBOARD (i, j+1) && board [(j+1) * board_heit + i] == 0)
					{
						if (j == board_heit - 2)
							antichess_movegen_promote(board, &movp, player,
								i, j, i, j+1);
						else
							antichess_movegen_square (board, &movp, player,
								i, j, i, j+1);
					}
					if (ISINBOARD (i+1, j+1) && 
							ANTICHESS_ISBLACK (board [(j+1) * board_heit + i+1]))
					{
						if (j == board_heit - 2)
							antichess_movegen_promote (board, &movp, player,
								i, j, i+1, j+1);
						else
							antichess_movegen_square (board, &movp, player,
								i, j, i+1, j+1);
						capture ++;
					}
					if (ISINBOARD (i-1, j+1) && 
							ANTICHESS_ISBLACK (board [(j+1) * board_heit + i-1]))
					{
						if (j == board_heit - 2)
							antichess_movegen_promote (board, &movp, player,
								i, j, i-1, j+1);
						else
							antichess_movegen_square (board, &movp, player,
								i, j, i-1, j+1);
						capture ++;
					}
					if (j == 1 && board [2 * board_heit + i] == 0
							&& board [3 * board_heit + i] == 0)
						antichess_movegen_square (board, &movp, player,
								i, j, i, j+2);
					break;
				case ANTICHESS_BP:
					if (ISINBOARD (i, j-1) && board [(j-1) * board_heit + i] == 0)
					{
						if (j == 1)
							antichess_movegen_promote (board, &movp, player,
								i, j, i, j-1);
						else
							antichess_movegen_square (board, &movp, player,
								i, j, i, j-1);
					}
					if (ISINBOARD (i+1, j-1) && 
							ANTICHESS_ISWHITE (board [(j-1) * board_heit + i+1]))
					{
						if (j == 1)
							antichess_movegen_promote (board, &movp, player,
								i, j, i+1, j-1);
						else
							antichess_movegen_square (board, &movp, player,
								i, j, i+1, j-1);
						capture ++;
					}
					if (ISINBOARD (i-1, j-1) && 
							ANTICHESS_ISWHITE (board [(j-1) * board_heit + i-1]))
					{
						if (j == 1)
							antichess_movegen_promote (board, &movp, player,
								i, j, i-1, j-1);
						else
							antichess_movegen_square (board, &movp, player,
								i, j, i-1, j-1);
						capture ++;
					}
					if (j == 6 && board [5 * board_heit + i] == 0
							&& board [4 * board_heit + i] == 0)
						antichess_movegen_square (board, &movp, player,
								i, j, i, j-2);
					break;
				case ANTICHESS_WK:
				case ANTICHESS_BK:
					for (k=0; k<4; k++)
					{
						antichess_movegen_square(board, &movp, player, 
								i, j, i + incxr[k], j + incyr[k]);
						if (ISINBOARD(i + incxr[k], j + incyr[k]) 
								&& oppcolor (board, i, j, i+incxr[k], j+incyr[k]))
							capture ++;
					}
					for (k=0; k<4; k++)
					{
						antichess_movegen_square(board, &movp, player, 
								i, j, i + incxb[k], j + incyb[k]);
						if (ISINBOARD(i + incxb[k], j + incyb[k]) 
								&& oppcolor (board, i, j, i+incxb[k], j+incyb[k]))
							capture ++;
					}
					break;
				case ANTICHESS_WB:
				case ANTICHESS_BB:
					for (k=0; k<4; k++)
						capture += antichess_movegen_line (board, &movp, player, 
								i, j, incxb[k], incyb[k]);
					break;
				case ANTICHESS_WR:
				case ANTICHESS_BR:
					for (k=0; k<4; k++)
						capture += antichess_movegen_line (board, &movp, player, 
								i, j, incxr[k], incyr[k]);
					break;
				case ANTICHESS_WQ:
				case ANTICHESS_BQ:
					for (k=0; k<4; k++)
						capture += antichess_movegen_line (board, &movp, player, 
								i, j, incxb[k], incyb[k]);
					for (k=0; k<4; k++)
						capture += antichess_movegen_line (board, &movp, player, 
								i, j, incxr[k], incyr[k]);
					break;
				case ANTICHESS_WN:
				case ANTICHESS_BN:
					for (k=0; k<8; k++)
					{
						antichess_movegen_square(board, &movp, player, 
								i, j, i + incxn[k], j + incyn[k]);
						if (ISINBOARD(i + incxn[k], j + incyn[k]) 
								&& oppcolor (board, i, j, i+incxn[k], j+incyn[k]))
							capture ++;
					}
					break;
			}
		}
	}
	*movp++ = -2;

	/* if there is a capture eliminate all other moves */
	if (!capture)
	{
		movlist = (byte *) malloc (movp - movbuf);
		memcpy (movlist, movbuf, movp - movbuf);
		return movlist;
	}
	movp = movbuf;
	realp = realbuf;
	while (1)
	{
		byte *tmp;
		int w = 0, b = 0;
		if (*movp == -2)
			break;
		/* a capture is a move that involves a W as well as a B piece */
		for (tmp = movp; *tmp != -1; tmp += 3)
		{
			if (ANTICHESS_ISWHITE (board [tmp[1] * board_heit + tmp[0]]))	w = 1;
			if (ANTICHESS_ISBLACK (board [tmp[1] * board_heit + tmp[0]]))	b = 1;
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
}

// True if this square is a pawn and it can move
static gboolean eval_pawn_has_move (byte *board, int i, int j)
{
	int val = board [j * board_wid + i];
	int x, y;
	if (val == ANTICHESS_WP) y = j+1;
	else if (val == ANTICHESS_BP) y = j-1;
	else return FALSE;

	if (board [y * board_wid + i] == ANTICHESS_EMPTY) return TRUE;
	
	for (x=i-1; x<=i+1; x+=2)
	{
		if (x < 0 || x >= board_wid) continue;
		if (val == ANTICHESS_WP && ANTICHESS_ISBLACK (board [y * board_wid + x]))
			return TRUE;
		if (val == ANTICHESS_BP && ANTICHESS_ISWHITE (board [y * board_wid + x]))
			return TRUE;
	}

	return FALSE;	
}

ResultType antichess_eval (Pos * pos, Player player, float *eval)
{
	int wsum = 0, bsum = 0, i, j;
	gboolean wfound = FALSE, bfound = FALSE;
	for (i=0; i < board_wid; i++)
	for (j=0; j < board_heit; j++)
		if (ANTICHESS_ISWHITE (pos->board[j * board_wid + i]))
		{
			wsum++;
			if (!wfound && eval_pawn_has_move (pos->board, i, j))
				wfound = TRUE;
				
		}
		else if (ANTICHESS_ISBLACK (pos->board[j * board_wid + i]))
		{
			bsum++;
			if (!bfound && eval_pawn_has_move (pos->board, i, j))
				bfound = TRUE;
		}
	
	if (wsum == 0 || (!wfound && bfound && player == WHITE))
	{
		*eval = GAME_EVAL_INFTY;
		return RESULT_WHITE;
	}
	if (bsum == 0 || (!bfound && wfound && player == BLACK))
	{
		*eval = -GAME_EVAL_INFTY;
		return RESULT_BLACK;
	}
	if (!wfound && !bfound)
	{
		*eval = 0;
		return RESULT_TIE;
	}
	*eval = bsum - wsum;
	return RESULT_NOTYET;
}

ResultType antichess_eval_incr (Pos *pos, Player player, byte *move, float *eval)
	// check if there's a capture
{
	byte *board = pos->board;
	if (player == WHITE)
	{
		if (move[2] && board[move[1] * board_wid + move[0]]) *eval = -1;
		else if (move[5] && board[move[4] * board_wid + move[3]]) return -1;
		else *eval = 0;
	}
	else
	{
		if (move[2] && board[move[1] * board_wid + move[0]]) *eval = 1;
		else if (move[5] && board[move[4] * board_wid + move[3]]) *eval = 1;
		else *eval = 0;
	}
	return RESULT_NOTYET;
}
