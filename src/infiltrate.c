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
#include <math.h>

#include "game.h"
#include "move.h"
#include "aaball.h"

#define INFILTRATE_CELL_SIZE 40
#define INFILTRATE_NUM_PIECES 2

#define INFILTRATE_BOARD_WID 7
#define INFILTRATE_BOARD_HEIT 7

#define INFILTRATE_WP 1
#define INFILTRATE_BP 2

char infiltrate_colors[] = 
	{180, 180, 180, 
	200, 200, 200};
	
int	infiltrate_initpos[] = 
{
	 2 , 0 , 2 , 0 , 2 , 0 , 2 , 
	 0 , 2 , 0 , 2 , 0 , 2 , 0 , 
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 
	 0 , 1 , 0 , 1 , 0 , 1 , 0 , 
	 1 , 0 , 1 , 0 , 1 , 0 , 1 , 
};

static int infiltrate_max_moves = 200;


void infiltrate_init ();
int infiltrate_getmove (Pos *, int, int, int, Player, byte **);
//ResultType infiltrate_who_won (byte *, int, char **);
byte *infiltrate_movegen (Pos *, Player );
float infiltrate_eval (Pos *, Player);
char ** infiltrate_get_pixmap (int idx, int color);
	
Game Infiltrate = 
	{ INFILTRATE_CELL_SIZE, INFILTRATE_BOARD_WID, INFILTRATE_BOARD_HEIT, 
	INFILTRATE_NUM_PIECES,
	infiltrate_colors, infiltrate_initpos, NULL, "Infiltrate",
	infiltrate_init};

void infiltrate_init ()
{
	game_getmove = infiltrate_getmove;
	game_movegen = infiltrate_movegen;
	//game_who_won = infiltrate_who_won;
	game_eval = infiltrate_eval;
	game_get_pixmap = infiltrate_get_pixmap;
	game_doc_about = 
		"Infiltrate\n"
		"Two player game\n"
		"URL: "GAME_DEFAULT_URL ("infiltrate");
	game_doc_rules = 
		"Infiltrate rules\n"
		"\n"
		"The pieces move diagonally, one square at a time. The objective is to get all your pieces to the starting squares of your opponent's pieces.\n";
}

byte * infiltrate_movegen (Pos *pos, Player player)
{
	int i, j, diffx, diffy;
	byte movbuf [256];
	byte *movlist, *mp = movbuf;
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (player == BLACK && (board [j * board_wid + i] != INFILTRATE_BP))
			continue;
		if (player == WHITE && (board [j * board_wid + i] != INFILTRATE_WP))
			continue;
		for (diffx = -1; diffx <= 1; diffx += 2)
		for (diffy = -1; diffy <= 1; diffy += 2)
		{
			int x, y;
			if (!ISINBOARD(x = i+diffx, y = j+diffy)) continue;
			if (board [y * board_wid + x] != 0) continue;
			*mp++ = i; *mp++ = j; *mp++ = 0;
			*mp++ = i + diffx; *mp++ = j + diffy; 
			*mp++ = (player == WHITE ? INFILTRATE_WP : INFILTRATE_BP);
			*mp++ = -1;
		}
	}
	if (mp == movbuf)
		*mp++ = -1;
	*mp++ = -2;
	movlist = (byte *) (malloc (mp - movbuf));
	memcpy (movlist, movbuf, (mp - movbuf));
	return movlist;
}

float infiltrate_eval (Pos *pos, Player to_play)
{
	float sum = 0;
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (pos->board [j * board_wid + i])
			sum += j;
	return sum;

}

int infiltrate_getmove (Pos *pos, int x, int y, int type, Player to_play, 
		byte **movp)
{
	static byte move[10];
	byte *mp = move;
	static int oldx = -1, oldy = -1;
	byte *board = pos->board;
	if (type != GTKBOARD_BUTTON_RELEASE) return 0;
	if (oldx < 0)
	{
		int val = board [y * board_wid + x];
		if ((val == INFILTRATE_WP && !(to_play == WHITE)) ||
		(val == INFILTRATE_BP && !(to_play == BLACK)))
			return -1;
		oldx = x; oldy = y;
		return 0;
	}
	else
	{
		int diffx, diffy;
		diffx = x - oldx;
		diffy = y - oldy;
		if (abs (diffx) != 1 || abs (diffy) != 1)
		{
			oldx = -1; oldy = -1;
			return -1;
		}
		*mp++ = oldx; *mp++ = oldy; *mp++ = 0;
		*mp++ = x; *mp++ = y; *mp++ = board [oldy * board_wid + oldx];
		*mp++ = -1;
		*movp = move;
		oldx = oldy = -1;
		return 1;

	}
}

char ** infiltrate_get_pixmap (int idx, int color)
{
	int bg;
	int i;
	static char pixbuf[INFILTRATE_CELL_SIZE * (INFILTRATE_CELL_SIZE+1)];
	for(i=0, bg=0;i<3;i++) 
	{ int col = infiltrate_colors[i]; 
		if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	return pixmap_ball_gen (INFILTRATE_CELL_SIZE, pixbuf,
			idx == INFILTRATE_WP ? 0xffffff : 0x0000ff, bg, 
			8, 24);
}
