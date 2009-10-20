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
#include <assert.h>
#include <stdlib.h>

#include "game.h"
#include "aaball.h"

#define RGB_CELL_SIZE 55
#define RGB_NUM_PIECES 3

#define RGB_BOARD_WID 3
#define RGB_BOARD_HEIT 3

#define RGB_RP 1
#define RGB_GP 2
#define RGB_BP 3
#define RGB_EMPTY 0

char rgb_colors[9] = {200, 200, 200, 200, 200, 200, 0, 0, 0};

int * rgb_init_pos = NULL;

void rgb_init ();

Game Rgb = { RGB_CELL_SIZE, RGB_BOARD_WID, RGB_BOARD_HEIT, 
	RGB_NUM_PIECES,
	rgb_colors,  NULL, NULL, "Rgb", "k-in-a-row", rgb_init};


static int rgb_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType rgb_who_won (Pos *, Player, char **);
static void rgb_set_init_pos (Pos *pos);
void rgb_init (void);
static byte * rgb_movegen (Pos *);
static ResultType rgb_eval (Pos *, Player, float *eval);

static unsigned char * rgb_get_rgbmap (int idx, int color);

void rgb_init ()
{
	game_eval = rgb_eval;
	game_movegen = rgb_movegen;
	game_getmove = rgb_getmove;
	game_who_won = rgb_who_won;
	game_set_init_pos = rgb_set_init_pos;
	game_get_rgbmap = rgb_get_rgbmap;
	game_draw_cell_boundaries = TRUE;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Rgb\n"
		"Two player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL ("rgb");
	game_doc_rules = 
		"Rgb, short for red-green-blue, is a harder version of tic-tac-toe. The goal is to get 3 balls in a line (row, column, or diagonal) of any one color. Clicking on an empty square puts a red ball on it, clicking on a red ball turns it green, and clicking on a green ball turns it blue.";
}

static void rgb_set_init_pos (Pos *pos)
{
	int i;
	for (i=0; i<board_wid * board_heit; i++)
		pos->board [i] = RGB_EMPTY;
}

static byte * rgb_movegen (Pos *pos)
{
	int i, j;
	byte movbuf [64];
	byte *movlist, *movp = movbuf;
	int lines[8][2] = 
	{ 
		{0, 1}, {3, 1}, {6, 1},
		{0, 3}, {1, 3}, {2, 3},
		{0, 4}, {2, 2},
	};
	int val, found;
	for (i=0; i<8; i++)
	{
		val = -1; found = 1;
		for (j=0; j<3; j++)
		{
			if (val >= 0 && pos->board[lines[i][0] + j * lines[i][1]] != val) 
			{ found = 0; break; }
			val = pos->board[lines[i][0] + j * lines[i][1]];
			if (val == RGB_EMPTY) { found = 0; break; }
		}
		if (found) 
			break;
	}
	if (!found)
	{
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (pos->board[j * board_wid + i] != RGB_BP)
			{
				*movp++ = i;
				*movp++ = j;
				*movp++ = pos->board[j * board_wid + i] + 1;
				*movp++ = -1;
			}
	}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}

static ResultType rgb_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	char *who_str [] = { "White won", "Black won"};
	int lines[8][2] = 
	{ 
		{0, 1}, {3, 1}, {6, 1},
		{0, 3}, {1, 3}, {2, 3},
		{0, 4}, {2, 2},
	};
	int i, j;
	int val, found;
	for (i=0; i<8; i++)
	{
		val = -1; found = 1;
		for (j=0; j<3; j++)
		{
			if (val >= 0 && pos->board[lines[i][0] + j * lines[i][1]] != val) 
			{ found = 0; break; }
			val = pos->board[lines[i][0] + j * lines[i][1]];
			if (val == RGB_EMPTY) { found = 0; break; }
		}
		if (found) 
		{
			*commp = who_str[to_play == WHITE ? 1 : 0];
			return to_play == WHITE ? RESULT_BLACK : RESULT_WHITE;
		}
	}
	*commp = NULL;
	return RESULT_NOTYET;
}

static int rgb_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
{
	int val;
	static byte move[4];
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	val = pos->board [y * board_wid + x];
	if (val == RGB_BP) return -1;
	move[0] = x;
	move[1] = y;
	move[2] = val+1;
	move[3] = -1;
	if (movp)
		*movp = move;	
	return 1;
}

static unsigned char * rgb_get_rgbmap (int idx, int color)
{
	int fg = 0, bg, i;
	char *colors;
	static char rgbbuf[3 * RGB_CELL_SIZE * RGB_CELL_SIZE];
	colors = rgb_colors;
	if (idx == RGB_RP) fg = 255 << 16;
	else if (idx == RGB_GP) fg = 255 << 8;
	else if (idx == RGB_BP) fg = 255;
	else { return NULL;}
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	rgbmap_ball_shadow_gen(55, rgbbuf, fg, bg, 17.0, 35.0, 3);
	return rgbbuf;
}


static ResultType rgb_eval (Pos *pos, Player to_play, float *eval)
{
	int i, j;
	int lines[8][2] = 
	{ 
		//{start, incr}
		// rows
		{0, 1}, {3, 1}, {6, 1},
		// cols
		{0, 3}, {1, 3}, {2, 3},
		// diagonals
		{0, 4}, {2, 2},
	};
	int val, found;
	for (i=0; i<8; i++)
	{
		val = -1; found = 1;
		for (j=0; j<3; j++)
		{
			if (val >= 0 && pos->board[lines[i][0] + j * lines[i][1]] != val) 
			{ found = 0; break; }
			val = pos->board[lines[i][0] + j * lines[i][1]];
			if (val == RGB_EMPTY) { found = 0; break; }
		}
		if (found)
		{
			*eval = (to_play == WHITE ? -2*GAME_EVAL_INFTY : 2*GAME_EVAL_INFTY);
			return RESULT_NOTYET;
		}
	}
	*eval = 0;
	return RESULT_NOTYET;
}
