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
static byte * breakthrough_movegen (Pos *, Player);
static void *breakthrough_newstate (Pos *, byte *);

Game Breakthrough = { BREAKTHROUGH_CELL_SIZE, BREAKTHROUGH_BOARD_WID, BREAKTHROUGH_BOARD_HEIT, 
	BREAKTHROUGH_NUM_PIECES, 
	breakthrough_colors, breakthrough_initpos, breakthrough_pixmaps, "Breakthrough", breakthrough_init};

void breakthrough_init ()
{
	game_getmove = breakthrough_getmove;
//	game_who_won = breakthrough_who_won;
	game_eval = breakthrough_eval;
	game_movegen = breakthrough_movegen;
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	game_doc_about = 
		"Breakthrough\n"
		"Two player game\n"
		"Status: Partially implemented\n"
		"URL: "GAME_DEFAULT_URL ("breakthrough");
}

static ResultType breakthrough_eval (Pos *pos, Player player, float *eval)
{
	*eval = random ();
	return RESULT_NOTYET;
}

static byte * breakthrough_movegen (Pos *pos, Player player)
{
	int i, j;
	byte movbuf [256];
	byte *movlist, *movp = movbuf;
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		int incx, incy;
		if (board [j * board_wid + i] != (player == WHITE ? BREAKTHROUGH_WP : BREAKTHROUGH_BP))
			continue;
		incy = board [j * board_wid + i] == WHITE ? 1 : -1;
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
			*movp++ = (player == WHITE ? BREAKTHROUGH_WP : BREAKTHROUGH_BP);
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
			breakthrough_curx = x;
			breakthrough_cury = y;
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

