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

/** \file pentaline.c */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "game.h"
#include "aaball.h"

#define PENTALINE_CELL_SIZE 40
#define PENTALINE_NUM_PIECES 2

#define PENTALINE_BOARD_WID 12
#define PENTALINE_BOARD_HEIT 12

#define PENTALINE_RP 1
#define PENTALINE_BP 2
#define PENTALINE_EMPTY 0

char pentaline_colors[9] = {200, 200, 200, 200, 200, 200, 0, 0, 0};

int * pentaline_initpos = NULL;


void pentaline_init ();

Game Pentaline = { PENTALINE_CELL_SIZE, PENTALINE_BOARD_WID, PENTALINE_BOARD_HEIT, 
	PENTALINE_NUM_PIECES,
	pentaline_colors,  NULL, NULL, "Pentaline", pentaline_init};


static int pentaline_getmove (Pos *, int, int, GtkboardEventType, Player, byte **);
static ResultType pentaline_who_won (Pos *, Player , char **);
static void pentaline_setinitpos (Pos *pos);
char ** pentaline_get_pixmap (int idx, int color);
float pentaline_eval_incr (Pos *, Player, byte *);
byte * pentaline_movegen (Pos *, Player);
float pentaline_eval (Pos *, Player);


void pentaline_init ()
{
	game_eval = pentaline_eval;
	game_movegen = pentaline_movegen;
	game_getmove = pentaline_getmove;
	game_who_won = pentaline_who_won;
	game_setinitpos = pentaline_setinitpos;
	game_get_pixmap = pentaline_get_pixmap;
	game_draw_cell_boundaries = TRUE;
	game_eval_incr = pentaline_eval_incr;
	game_doc_about = 
		"Pentaline\n"
		"Two player game\n"
		"Status: Fully implemented (But AI needs improvement)\n"
		"URL: "GAME_DEFAULT_URL("pentaline");
	game_doc_rules = 
		"Pentaline rules\n"
		"\n"
		"Two players take turns in placing balls of either color. The first to get 5 balls in a row wins.\n";
}

void pentaline_setinitpos (Pos *pos)
{
	int i;
	for (i=0; i<board_wid * board_heit; i++)
		pos->board [i] = PENTALINE_EMPTY;
}

byte * pentaline_movegen (Pos *pos, Player player)
{
	int i, j, k, l, x, y;
	byte movbuf [1024];
	byte *movlist, *movp = movbuf;
	int val, found = 0;
	int incx[4] = { 0, 1, 1, -1};
	int incy[4] = { 1, 0, 1,  1};
	int nbrx[] = { -1, -1, -1, 0, 0, 1, 1, 1};
	int nbry[] = { -1, 0, 1, -1, 1, -1, 0, 1};
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (board [j * board_heit + i] == PENTALINE_EMPTY)
			continue;
		for (k=0; k<4; k++)
		{
			int found = 1, val;
			for (l=0; l<5; l++)
			{
				if (j + l * incy[k] >= board_heit || i + l * incx[k] >= board_wid)
				{ found = 0; break; }
				val = board [(j + l * incy[k]) * board_wid + i + l * incx[k]];
				if (val == PENTALINE_EMPTY) {found = 0; break;}
				if (val != board [j * board_wid + i]) { found = 0; break; }
			}
			if (found) { break; }
		}
	}
	if (!found)
	{
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
		{
			if (board[j * board_wid + i] != PENTALINE_EMPTY) continue;
			for (k=0; k<8; k++)
			{
				x = i + nbrx[k];
				y = j + nbry[k];
				if (x >= 0 && y >= 0 && x < board_wid && y < board_heit 
						&& board [y * board_wid + x] != PENTALINE_EMPTY)
				{
					*movp++ = i;
					*movp++ = j;
					*movp++ = (player == WHITE ? PENTALINE_RP : PENTALINE_BP);
					*movp++ = -1;
				}
			}
		}
	}
	if (movp == movbuf) // empty board
	{
		*movp++ = rand () % board_wid;
		*movp++ = rand () % board_heit;
		*movp++ = (player == WHITE ? PENTALINE_RP : PENTALINE_BP);
		*movp++ = -1;
	}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}

ResultType pentaline_who_won (Pos *pos, Player to_play, char **commp)
{
	int i, j, k, l;
	int incx[4] = { 0, 1, 1, -1};
	int incy[4] = { 1, 0, 1,  1};
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	for (k=0; k<4; k++)
	{
		int found = 1, val;
		for (l=0; l<5; l++)
		{
			if (j + l * incy[k] >= board_heit || i + l * incx[k] >= board_wid)
			{ found = 0; break; }
			val = pos->board [(j + l * incy[k]) * board_wid + i + l * incx[k]];
			if (val == PENTALINE_EMPTY) {found = 0; break; }
			if (val != pos->board [j * board_wid + i]) { found = 0; break; }
		}
		if (found) {*commp = (to_play == WHITE ? "black won" : "white won");
			return (to_play == WHITE ? RESULT_BLACK : RESULT_WHITE);}
	}
	*commp = NULL;
	return RESULT_NOTYET;
}

int pentaline_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp)
{
	int val;
	static byte move[4];
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (pos->board [y * board_wid + x] != PENTALINE_EMPTY)
		return -1;
	move[0] = x;
	move[1] = y;
	move[2] = to_play == WHITE ? PENTALINE_RP : PENTALINE_BP;
	move[3] = -1;
	if (movp)
		*movp = move;	
	return 1;
}

char ** pentaline_get_pixmap (int idx, int color)
{
	int fg = 0, bg, i;
	char *colors;
	static char pixbuf[PENTALINE_CELL_SIZE*(PENTALINE_CELL_SIZE+1)];
	colors = pentaline_colors;
	if (idx == PENTALINE_RP) fg = 255 << 16;
	else if (idx == PENTALINE_BP) fg = 255;
	else { return NULL;}
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	return pixmap_ball_gen(PENTALINE_CELL_SIZE, pixbuf, fg, bg, 13.0, 30.0);
}

static float eval_line (byte *board, int x, int y, int incx, int incy)
{
	int open = 0;
	int newx, newy;
	int len, val, sgn;
	newx = x - incx, newy = y - incy;
	val = board [y * board_wid + x];
	if (val == PENTALINE_EMPTY) return 0;
	sgn = (val == PENTALINE_RP ? 1 : -1);
	if (newx >= 0 && newy >= 0 && newx < board_wid && newy < board_heit)
	{
		if(board [newy * board_wid + newx] == val)
			return 0;
		if(board [newy * board_wid + newx] == PENTALINE_EMPTY)
			open = 1;
	}
	for (len = 0; ; x+= incx, y+=incy, len++)
	{
		if (x < 0 || y < 0 || x >= board_wid || y >= board_heit) break;
		if (board [y * board_wid + x] != val) break;
	}	
	if (!(x < 0 || y < 0 || x >= board_wid || y >= board_heit)
			&& board [y * board_wid + x] == PENTALINE_EMPTY) 
		open++;
	if (len >= 5) return GAME_EVAL_INFTY * sgn;
	return open * open * (1 << len) * sgn;
}

static float eval_line_bidir (byte *board, int x, int y, int incx, int incy)
{
	int val = board[y * board_wid + x];
	do
	{
		x -= incx;
		y -= incy;
	}
	while (x >= 0 && y >= 0 && x < board_wid && y < board_heit 
			&& board [y * board_wid + x] == val);
	x += incx;
	y += incy;
	return eval_line (board, x, y, incx, incy);
}

static float eval_runs (byte *board)
{
	int i, j, k;
	int incx[4] = { 0, 1, 1, -1 };
	int incy[4] = { 1, 0, 1,  1 };
	float eval = 0;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (board [j * board_heit + i] == PENTALINE_EMPTY)
			continue;
		for (k=0; k<4; k++)
			eval += eval_line (board, i, j, incx[k], incy[k]);
	}
	return eval + 0.1 * rand() / RAND_MAX;
}

float pentaline_eval_incr (Pos *pos, Player to_play, byte *move)
{
	int  k;
	int incx[4] = { 0, 1, 1, -1 };
	int incy[4] = { 1, 0, 1,  1 };
	float eval = 0;
	pos->board [move[1] * board_wid + move[0]] = move[2];
	for (k=0; k<4; k++)
		eval += eval_line_bidir (pos->board, move[0], move[1], incx[k], incy[k]);
	pos->board [move[1] * board_wid + move[0]] = 0;
	return eval + 0.01 * rand() / RAND_MAX;
}

float pentaline_eval (Pos *pos, Player to_play)
{
	return eval_runs (pos->board);
}
