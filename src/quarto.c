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

#define QUARTO_CELL_SIZE 60
#define QUARTO_NUM_PIECES 17

#define QUARTO_BOARD_WID 9
#define QUARTO_BOARD_HEIT 4

#define QUARTO_EMPTY 0
#define QUARTO_SEPARATOR 17

#define QUARTO_SHAPE_MASK (1 << 0)
#define QUARTO_SIZE_MASK  (1 << 1)
#define QUARTO_HOLE_MASK  (1 << 2)
#define QUARTO_COLOR_MASK (1 << 3)
#define QUARTO_ALL_PIECES_MASK (QUARTO_SHAPE_MASK | QUARTO_SIZE_MASK | QUARTO_HOLE_MASK | QUARTO_COLOR_MASK)

char quarto_colors[9] = {200, 200, 200, 200, 200, 200, 0, 0, 0};

char quarto_highlight_colors[9] = {0, 0xff, 0, 0, 0, 0, 0, 0, 0};

int quarto_init_pos[QUARTO_BOARD_HEIT * QUARTO_BOARD_WID] = 
{
	0, 0, 0, 0, 17, 1,   2,  3,  4,
	0, 0, 0, 0, 17, 5,   6,  7,  8,
	0, 0, 0, 0, 17, 9,  10, 11, 12,
	0, 0, 0, 0, 17, 13, 14, 15, 16,
};



void quarto_init ();

Game Quarto = { QUARTO_CELL_SIZE, QUARTO_BOARD_WID, QUARTO_BOARD_HEIT, 
	QUARTO_NUM_PIECES,
	quarto_colors, quarto_init_pos, NULL, "Quarto", "k-in-a-row", quarto_init};


static int quarto_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType quarto_who_won (Pos *, Player, char **);
void quarto_init (void);
static byte * quarto_movegen (Pos *);
static ResultType quarto_eval (Pos *, Player, float *eval);
static void quarto_reset_uistate (Pos *pos);
static unsigned char * quarto_get_rgbmap (int idx, int color);
void quarto_get_render (Pos *pos, byte *move, int **rmovp);

void quarto_init ()
{
	game_eval = quarto_eval;
	game_movegen = quarto_movegen;
	game_who_won = quarto_who_won;
	game_getmove = quarto_getmove;
	game_get_rgbmap = quarto_get_rgbmap;
	game_draw_cell_boundaries = TRUE;
	game_highlight_colors = quarto_highlight_colors;
	game_reset_uistate = quarto_reset_uistate;
	game_get_render = quarto_get_render;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Quarto\n"
		"Two player game\n"
		"Status: Partially implemented\n"
		"URL: "GAME_DEFAULT_URL ("quarto");
	game_doc_rules = 
		"Two players take turns in placing the pieces from the right side of the board to the left side. Get 4 in a line (horizontal, vertical, diagonal) of any one type -- big, small, square, circle, hole, no hole, red or blue";
}

static ResultType quarto_who_won (Pos *pos, Player player, char **commp)
{
	ResultType result;
	float eval;
	result = quarto_eval (pos, player, &eval);
	if (result == RESULT_WHITE) *commp = "White won";
	if (result == RESULT_BLACK) *commp = "Black won";
	return result;
}

static gboolean eval_column (byte *board, int x, int y, int incx, int incy)
{
	int i;
	int and1, and2;
	and1 = and2 = QUARTO_ALL_PIECES_MASK;
	for (i=0; i<4; i++, x+=incx, y+=incy)
	{
		int val;
		val = board[y * board_wid + x];
		if (val == 0)
			return FALSE;
		val--;
		and1 &= val;
		and2 &= ~val;
	}
	if (and1 != 0 || and2 != 0)
		return TRUE;
	else
		return FALSE;
}

static ResultType quarto_eval (Pos *pos, Player player, float *eval)
{
	int i, j;
	int and1, and2;
	ResultType result = player == WHITE ? RESULT_BLACK : RESULT_WHITE;
	*eval = player == WHITE ? -1 : 1;

	for (i=0; i<4; i++)
		if (eval_column (pos->board, i, 0, 0, 1))
			return result;

	for (j=0; j<4; j++)
		if (eval_column (pos->board, 0, j, 1, 0))
			return result;

	if (eval_column (pos->board, 0, 0, 1, 1))
		return result;

	if (eval_column (pos->board, 0, 3, 1, -1))
		return result;

	*eval = random() * 0.01 / RAND_MAX;
	return RESULT_NOTYET;
}

static int cur_piece = -1, curx = -1, cury = -1;

int quarto_getmove 
	(Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
{
	static byte move[7];
	static int rmove[7];
	byte *mp = move;
	int *rp = rmove;

	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	
	if (x == 4) return 0;
	
	if (x > 4)
	{
		if (pos->render[y * board_wid + x] == RENDER_SHADE1)
			return -1;
		*rp++ = x;
		*rp++ = y;
		*rp++ = RENDER_HIGHLIGHT1;
		if (cur_piece >= 0)
		{
			*rp++ = curx;
			*rp++ = cury;
			*rp++ = RENDER_NONE;
		}
		*rp++ = -1;
		cur_piece = pos->board [y * board_wid + x];
		curx = x;
		cury = y;
		*rmovep = rmove;
		return 0;
	}

	if (cur_piece < 0)
		return -1;

	if (pos->board[y * board_wid + x] != QUARTO_EMPTY)
		return -1;
	*mp++ = x;
	*mp++ = y;
	*mp++ = cur_piece;
	*mp++ = -1;
	*movp = move;
	cur_piece = -1;
	*rp++ = curx;
	*rp++ = cury;
	*rp++ = 0;
	*rp++ = -1;
	*rmovep = rmove;
	return 1;
}

void quarto_get_render (Pos *pos, byte *move, int **rmovp)
{
	static int rmove[4];
	int *rp = rmove;
	int val, shadeval;
	int i, j;
	if (move[2] == QUARTO_EMPTY)
	{
		val = pos->board [move[1] * board_wid + move[0]];
		shadeval = RENDER_NONE;
	}
	else
	{ 
		val = move[2];
		shadeval = RENDER_SHADE1;
	}
	for (i=5; i<9; i++)
	for (j=0; j<4; j++)
		if (pos->board[j * board_wid + i] == val)
		{
			*rp++ = i;
			*rp++ = j;
			*rp++ = shadeval;
			*rp++ = -1;
		}
	*rmovp = rmove;
}

void quarto_reset_uistate (Pos *pos)
{
	cur_piece = curx = cury = -1;
}

byte *quarto_movegen (Pos *pos)
{
	byte movbuf[2048];
	byte *movlist, *movp = movbuf;
	int from_x, from_y, to_x, to_y;
	Player player = pos->player;

	int count = 0;
	for (to_x=0; to_x<4; to_x++)
	for (to_y=0; to_y<4; to_y++)
		if (pos->board [to_y * board_wid + to_x] != QUARTO_EMPTY)
			count++;
	assert ((player == WHITE && count % 2 == 0) || (player == BLACK && count % 2 == 1));

		
	for (from_x=5; from_x<9; from_x++)
	for (from_y=0; from_y<4; from_y++)
	{
		gboolean found = FALSE;
		int val = pos->board [from_y * board_wid + from_x];
		for (to_x=0; to_x<4; to_x++)
		for (to_y=0; to_y<4; to_y++)
			if (val == pos->board [to_y * board_wid + to_x])
			{
				found = TRUE;
				break;
			}
		if (!found)
		{
			for (to_x=0; to_x<4; to_x++)
			for (to_y=0; to_y<4; to_y++)
			{
				if (pos->board [to_y * board_wid + to_x] == QUARTO_EMPTY)
				{
					*movp++ = to_x;
					*movp++ = to_y;
					*movp++ = val;
					*movp++ = -1;
				}
			}
		}
	}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}

unsigned char * quarto_get_rgbmap (int idx, int color)
{
	int fg = 0, bg, i;
	char *colors;
	static char rgbbuf[3*QUARTO_CELL_SIZE*QUARTO_CELL_SIZE];
	int color1 = 0xff << 16, color2 = 0xff, sepcolor = 0xaaaa66;
	float grad = 30.0;
	int size;
	colors = quarto_colors;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	if (idx == QUARTO_SEPARATOR)
	{
		rgbmap_square_gen (QUARTO_CELL_SIZE, rgbbuf, sepcolor, bg, QUARTO_CELL_SIZE);
		return rgbbuf;
	}
	fg = ((idx - 1) & QUARTO_COLOR_MASK) ? color1 : color2;
	size = ((idx - 1) & QUARTO_SIZE_MASK) ? 
		3.0 * QUARTO_CELL_SIZE / 4 : QUARTO_CELL_SIZE / 2.0;
	if ((idx - 1) & QUARTO_SHAPE_MASK)
		rgbmap_ball_gen(QUARTO_CELL_SIZE, rgbbuf, fg, bg, size/2, grad);
	else
		rgbmap_square_gen (QUARTO_CELL_SIZE, rgbbuf, fg, bg, size);
	if ((idx - 1) & QUARTO_HOLE_MASK)
		rgbmap_ball_gen_nobg (QUARTO_CELL_SIZE, rgbbuf, bg, fg, QUARTO_CELL_SIZE/8.0, grad);
	return rgbbuf;
	
}


