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

#define KNIGHTS_CELL_SIZE 54
#define KNIGHTS_NUM_PIECES 4

#define KNIGHTS_BOARD_WID 8
#define KNIGHTS_BOARD_HEIT 8

#define KNIGHTS_EMPTY 0
#define KNIGHTS_CLOSED 1
#define KNIGHTS_WN 2
#define KNIGHTS_BN 3

char knights_colors[] = {200, 200, 130, 0, 140, 0};

int knights_initpos [KNIGHTS_BOARD_WID*KNIGHTS_BOARD_HEIT] = 
{
	2 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 3 ,
};

static char * grey_square_54_xpm [] = // TODO: move to a header file or generate in code
{
"54 54 1 1",
". c #d7d7d7",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
};


char ** knights_pixmaps [] = 
{
	grey_square_54_xpm,
	chess_wn_54_xpm,
	chess_bn_54_xpm,
};


static int knights_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static int knights_getmove_kb (Pos *, int, Player, byte ** , int **);
void knights_init ();
ResultType knights_who_won (Pos *, Player, char **);
//ResultType knights_eval (Pos *, Player, float *eval);
//byte * knights_movegen (Pos *, Player);

Game Knights = { KNIGHTS_CELL_SIZE, KNIGHTS_BOARD_WID, KNIGHTS_BOARD_HEIT, 
	KNIGHTS_NUM_PIECES, 
	knights_colors, knights_initpos, knights_pixmaps, "Knights", knights_init};

static int knights_curx = - 1, knights_cury = -1;

void knights_init ()
{
	game_getmove = knights_getmove;
	game_getmove_kb = knights_getmove_kb;
	game_who_won = knights_who_won;
/*	game_eval = knights_eval;
	game_movegen = knights_movegen;
*/
	game_doc_about = 
		"Knights\n"
		"Two player game\n"
		"Status: Partially implemented\n"
		"URL: "GAME_DEFAULT_URL ("knights");
}

static int incx[] = { -2, -2, -1, -1, 1, 1, 2, 2};
static int incy[] = { -1, 1, -2, 2, -2, 2, -1, 1};

ResultType knights_who_won (Pos *pos, Player player, char **commp)
{
	int i=0, j=0, k;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (player == WHITE && pos->board [j * board_wid + i] == KNIGHTS_WN)
			goto found_knight;
		if (player == BLACK && pos->board [j * board_wid + i] == KNIGHTS_BN)
			goto found_knight;
	}
	// WARNING: goto used here to break out of nested loop
found_knight:
	for (k=0; k<8; k++)
	{
		int x = i + incx[k], y = j + incy[k];
		if (!ISINBOARD (x, y)) continue;
		if (pos->board[y * board_wid + x] != KNIGHTS_CLOSED)
			return RESULT_NOTYET;
	}
	*commp = player == WHITE ? "Black won" : "White won";
	return player == WHITE ? RESULT_BLACK : RESULT_WHITE;
}


int knights_getmove_kb (Pos *pos, int key, Player to_play, byte ** movp, int **rmovp)
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
	static byte move[128];
	byte *mp = move;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (knights_curx < 0)
	{
		if (player == WHITE && pos->board[y * board_wid + x] != KNIGHTS_WN)
			return -1;
		if (player == BLACK && pos->board[y * board_wid + x] != KNIGHTS_BN)
			return -1;
		knights_curx = x; knights_cury = y;
		return 0;
	}
	if (pos->board[y * board_wid + x] != KNIGHTS_EMPTY)
	{
		knights_curx = knights_cury = -1;
		return -1;
	}
	if (abs ((knights_curx - x) * (knights_cury - y)) != 2)
	{
		knights_curx = knights_cury = -1;
		return -1;
	}
	*mp++ = x;
	*mp++ = y;
	*mp++ = player == WHITE ? KNIGHTS_WN : KNIGHTS_BN;
	*mp++ = knights_curx;
	*mp++ = knights_cury;
	*mp++ = KNIGHTS_CLOSED;
	*mp++ = -1;
	knights_curx = knights_cury = -1;
	*movp = move;
	return 1;
}



