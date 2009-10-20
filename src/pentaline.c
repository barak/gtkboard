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

char pentaline_colors[9] = {200, 220, 200, 200, 220, 200, 0, 0, 0};


void pentaline_init ();

Game Pentaline = { PENTALINE_CELL_SIZE, PENTALINE_BOARD_WID, PENTALINE_BOARD_HEIT, 
	PENTALINE_NUM_PIECES,
	pentaline_colors,  NULL, NULL, "Pentaline", "k-in-a-row", pentaline_init};


static int pentaline_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType pentaline_who_won (Pos *, Player , char **);
static void pentaline_set_init_pos (Pos *pos);
unsigned char * pentaline_get_rgbmap (int idx, int color);
ResultType pentaline_eval_incr (Pos *, Player, byte *, float *);
byte * pentaline_movegen (Pos *);
ResultType pentaline_eval (Pos *, Player, float *);
void *pentaline_newstate (Pos *pos, byte *move);

typedef struct 
{
	// length, open/closed, white/black
	byte chains[5][2][2];
} Pentaline_state;


void pentaline_init ()
{
	game_eval = pentaline_eval;
	game_movegen = pentaline_movegen;
	game_getmove = pentaline_getmove;
	game_who_won = pentaline_who_won;
	game_get_rgbmap = pentaline_get_rgbmap;
	game_draw_cell_boundaries = TRUE;
//	game_eval_incr = pentaline_eval_incr;
	game_white_string = "Red";
	game_black_string = "Blue";
	game_stateful = TRUE;
	game_state_size = sizeof (Pentaline_state);
	game_newstate = pentaline_newstate;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Pentaline\n"
		"Two player game\n"
		"Status: Fully implemented (But AI needs improvement)\n"
		"URL: "GAME_DEFAULT_URL("pentaline");
	game_doc_rules = 
		"Two players take turns in placing balls of either color. The first to get 5 balls in a row wins.\n\n"
		"This game is the same as the free-style variant of GoMoku.\n";
}

byte * pentaline_movegen (Pos *pos)
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
	Player player = pos->player;
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
		*movp++ = board_wid / 2 - random() % 2;
		*movp++ = board_heit / 2 - random() % 2;
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
		if (found) {*commp = (to_play == WHITE ? "Blue won" : "Red won");
			return (to_play == WHITE ? RESULT_BLACK : RESULT_WHITE);}
	}
	*commp = NULL;
/*	{
		int len, open, color;
		for (color = 0; color < 2 && pos->state; color++)
		{
			printf ("player = %s:   ", color==0?"Red ":"Blue");
			for (open = 0; open < 2; open++)
			{
				printf ("open=%d: ", open+1);
				for (len = 0; len < 5; len++)
				printf ("%d ", 
						((Pentaline_state *)pos->state)->chains[len][open][color]);
				printf ("\t");
			}
			printf ("\n");
		}
		printf ("\n");
	}
*/
	return RESULT_NOTYET;
}

int pentaline_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
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

unsigned char * pentaline_get_rgbmap (int idx, int color)
{
	int fg, bg, i;
	char *colors;
	static char rgbbuf[3 * PENTALINE_CELL_SIZE * PENTALINE_CELL_SIZE];
	colors = pentaline_colors;
	fg = (idx == PENTALINE_RP ? 0xee << 16 : 0xee);
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	rgbmap_ball_shadow_gen(PENTALINE_CELL_SIZE, rgbbuf, fg, bg, 13.0, 30.0, 2);
	return rgbbuf;
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
		if (board [j * board_wid + i] == PENTALINE_EMPTY)
			continue;
		for (k=0; k<4; k++)
			eval += eval_line (board, i, j, incx[k], incy[k]);
	}
	return eval;
}

static int incx[4] = { 0, 1, 1, -1 };
static int incy[4] = { 1, 0, 1,  1 };

ResultType pentaline_eval_incr (Pos *pos, Player to_play, byte *move, float *eval)
{
	int  k;
	float val = 0;
	pos->board [move[1] * board_wid + move[0]] = move[2];
	for (k=0; k<4; k++)
		val += eval_line_bidir (pos->board, move[0], move[1], incx[k], incy[k]);
	pos->board [move[1] * board_wid + move[0]] = 0;
	*eval = val;
	return RESULT_NOTYET;
}

ResultType pentaline_eval (Pos *pos, Player player, float *eval)
{
#define FIRST_WON { *eval = player == WHITE ? (1 << 20) : - (1 << 20); return player == WHITE ? RESULT_WHITE : RESULT_BLACK; }
#define SECOND_WON { *eval = player == WHITE ? - (1 << 20) : (1 << 20); return player == WHITE ? RESULT_BLACK : RESULT_WHITE; }
	int color = player == WHITE ? 0 : 1;
	int len, open;
	Pentaline_state *state;
	*eval = 0;
	state = ((Pentaline_state *)pos->state);
	
	// 5 in a row
	if (state->chains[4][1][color] > 0 || state->chains[4][0][color] > 0)
	{
		*eval = player == WHITE ? (1 << 20) : - (1 << 20);
		return player == WHITE ? RESULT_WHITE : RESULT_BLACK;
	}
	
	// opponent: 5-in-a-row
	if (state->chains[4][1][1-color] > 0 || state->chains[4][0][1-color] > 0)
	{
		*eval = player == WHITE ? - (1 << 20) : (1 << 20);
		return player == WHITE ? RESULT_BLACK : RESULT_WHITE;
	}
	
	// 4-in-a-row
	if (state->chains[3][1][color] > 0 || state->chains[3][0][color] > 0)
	{
		*eval = player == WHITE ? (1 << 20) : - (1 << 20);
		return player == WHITE ? RESULT_WHITE : RESULT_BLACK;
	}
	
	// opponent: 4-in-a-row, both sides open
	if (state->chains[3][1][1-color] > 0)
		*eval += (player == WHITE ? -(1 << 18) : (1 << 18));
	
	// opponent: 2 4-in-a-row's
	if (state->chains[3][0][1-color] > 1)
		*eval += (player == WHITE ? -(1 << 18) : (1 << 18));
	
	// 3-in-a-row, both sides open; opponent doesn't have 4-in-a-row
	if (state->chains[2][1][color] > 0 && state->chains[3][0][1-color] == 0)
		*eval += (player == WHITE ? (1 << 16) : - (1 << 16));
	
	// opponent: 2 3-in-a-row's, both sides open 
	if (state->chains[2][1][1-color] > 1)
		*eval += (player == WHITE ? -(1 << 14) : (1 << 14));
	
	// opponent: a 4 and a doubly open 3
	if (state->chains[3][0][1-color] > 0 && state->chains[2][1][1-color] > 0)
		*eval += (player == WHITE ? - (1 << 12) : (1 << 12));

	// These seem to be all the winning patterns. Can't find any more.
	
	*eval = 0;
	for (len = 0; len < 4; len++)
	for (open = 0; open < 2; open++)
	{
		*eval += state->chains[len][open][0] * (1 + open) * (1 + open) * (1 << len);
		*eval -= state->chains[len][open][1] * (1 + open) * (1 + open) * (1 << len);
	}
	return RESULT_NOTYET;
}


// given a square and a direction, find the length of the chain it defines {0, 1, ... 4}  and the number of ends of the chain that are unoccupied {0, 1, 2}
static void get_chain_info (byte *board, int x, int y, int dx, int dy,
	   	int *len, int *open, int *color)
{
	int i;
	int val = board [y * board_wid + x];
	*open = 0;
	*len = 0;
	if (!ISINBOARD (x, y))
		return;
	if (val == PENTALINE_EMPTY)
		return;
	*color = (val == PENTALINE_RP ? 0 : 1);
		
	do
	{
		x -= dx;
		y -= dy;
	}
	while (ISINBOARD (x, y)	&& board [y * board_wid + x] == val);
	if (ISINBOARD (x, y) && board [y * board_wid + x] == PENTALINE_EMPTY) (*open)++;
	
	do
	{
		x += dx;
		y += dy;
		(*len)++;
	}
	while (ISINBOARD (x, y)	&& board [y * board_wid + x] == val);
	if (ISINBOARD (x, y) && board [y * board_wid + x] == PENTALINE_EMPTY) (*open)++;
	(*len)--;
}

static void update_state (byte chains[5][2][2], int len, int open, int color, int inc)
{
	if (len == 0) return;
	if (len >= 5)
	{ 
		len = 5;
		open = 1;
	}
	if (open == 0) return;
	if (inc == -1) assert (chains[len-1][open-1][color] > 0);
	chains[len-1][open-1][color] += inc;
}

void *pentaline_newstate (Pos *pos, byte *move)
{
	int k=0;
	static Pentaline_state state;
	Pentaline_state def_state = 
		{{{{0, 0},{0, 0}},{{0, 0},{0, 0}},{{0, 0},{0, 0}},{{0, 0},{0, 0}},
	}};
	int len, open;
	int newcolor, oldcolor;
	int val = move[2];
	if (pos->state)
		memcpy (&state, pos->state, sizeof (Pentaline_state));
	else
		memcpy (&state, &def_state, sizeof (Pentaline_state));
	for (k=0; k<4; k++)
	{
		get_chain_info (pos->board, move[0] + incx[k], move[1] + incy[k], 
				incx[k], incy[k], &len, &open, &oldcolor);
/*		if (len != 0 && len <= 5 && open != 0)
		{
			if (len > 5) len = 5;
			assert (state.chains[len-1][open-1][oldcolor] > 0);
			state.chains[len-1][open-1][oldcolor]--;
		}
*/
		update_state (state.chains, len, open, oldcolor, -1);
		get_chain_info (pos->board, move[0] - incx[k], move[1] - incy[k], 
				-incx[k], -incy[k], &len, &open, &oldcolor);
/*		if (len != 0 && len <= 5 && open != 0)
		{
			if (len > 5) len = 5;
			assert (state.chains[len-1][open-1][oldcolor] > 0);
			state.chains[len-1][open-1][oldcolor]--;
		}
*/
		update_state (state.chains, len, open, oldcolor, -1);
	}

	pos->board [move[1] * board_wid + move[0]] = move[2]; 
	for (k=0; k<4; k++)
	{
		int x = move[0], y = move[1];
		if (ISINBOARD (x + incx[k], y + incy[k]) 
				&& pos->board [(y + incy[k]) * board_wid + (x + incx[k])] != val)
		{
			get_chain_info (pos->board, move[0] + incx[k], move[1] + incy[k], 
					incx[k], incy[k], &len, &open, &oldcolor);
/*			if (len != 0 && len <= 5 && open != 0)
			{
				if (len > 5) len = 5;
				state.chains[len-1][open-1][oldcolor]++;
			}
*/
			update_state (state.chains, len, open, oldcolor, +1);
		}
		if (ISINBOARD (x - incx[k], y - incy[k]) 
				&& pos->board [(y - incy[k]) * board_wid + (x - incx[k])] != val)
		{
			get_chain_info (pos->board, move[0] - incx[k], move[1] - incy[k], 
					-incx[k], -incy[k], &len, &open, &oldcolor);
/*			if (len != 0 && len <= 5 && open != 0)
			{
				if (len > 5) len = 5;
				state.chains[len-1][open-1][oldcolor]++;
			}
*/
			update_state (state.chains, len, open, oldcolor, +1);
		}
		get_chain_info (pos->board, move[0], move[1], 
				incx[k], incy[k], &len, &open, &newcolor);
/*		if (len != 0 && len <= 5 && open != 0)
		{
			if (len > 5) len = 5;
			state.chains[len-1][open-1][newcolor]++;
		}
*/
		update_state (state.chains, len, open, newcolor, +1);
	}
	pos->board [move[1] * board_wid + move[0]] = 0; 
	return &state;
}
