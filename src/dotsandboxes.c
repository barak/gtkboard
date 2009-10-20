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
#include "aaball.h"

#define DNB_CELL_SIZE 24
#define DNB_NUM_PIECES 5

#define DNB_BOARD_SIZE 10

#define DNB_BOARD_WID (2*DNB_BOARD_SIZE+1)
#define DNB_BOARD_HEIT (2*DNB_BOARD_SIZE+1)

char dnb_colors[6] = {220, 220, 220, 220, 220, 220};

static char * dnb_red_24_xpm [] =
{
"24 24 1 1",
"  c #ee0000",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};

static char * dnb_blue_24_xpm [] =
{
"24 24 1 1",
"  c #0000ee",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};

static char * dnb_vertical_24_xpm [] =
{
"24 24 2 1",
"  c none",
". c #101010",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
"         ......         ",
};

static char * dnb_horizontal_24_xpm [] =
{
"24 24 2 1",
"  c none",
". c #101010",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"........................",
"........................",
"........................",
"........................",
"........................",
"........................",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
};


/*char ** dnb_pixmaps [] = 
{
	blue_gate_horiz_40_xpm,
	blue_gate_south_40_xpm,
	blue_gate_east_40_xpm,
	blue_gate_west_40_xpm,
};*/


#define DNB_EMPTY 0
#define DNB_DOT   1
#define DNB_HOR   2
#define DNB_VERT  3
#define DNB_RED   4
#define DNB_BLUE  5

#define abs(x) ((x) < 0 ? -(x) : (x))

int dnb_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
void dnb_search (Pos *pos, byte **movp);
void dnb_init ();
ResultType dnb_who_won (Pos *, Player, char **);
void dnb_set_init_pos (Pos *pos);
char ** dnb_get_pixmap (int idx, int color);
int dnb_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int **rmovp);
static void dnb_get_render (Pos *pos, byte *move, int **rmovp);

Game Dnb = { DNB_CELL_SIZE, DNB_BOARD_WID, DNB_BOARD_HEIT, 
	DNB_NUM_PIECES, dnb_colors, NULL, NULL, "Dots and Boxes", "Nimlike games", dnb_init};

static int dnb_curx = - 1, dnb_cury = -1;


void dnb_init ()
{
//	game_getmove = dnb_getmove;
/*	game_who_won = dnb_who_won;
	game_eval = dnb_eval;
	game_movegen = dnb_movegen;
*/
	game_set_init_pos = dnb_set_init_pos;
	game_get_pixmap = dnb_get_pixmap;
	game_get_render = dnb_get_render;
	game_white_string = "Red";
	game_black_string = "Blue";
	game_getmove = dnb_getmove;
	game_search = dnb_search;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_UNPLAYABLE;
	game_doc_about = 
		"Dots and boxes\n"
		"Two player game\n"
		"Status: partially implemented (the AI is totally dumb)\n"
		"URL: "GAME_DEFAULT_URL ("dnb");
}

char ** dnb_get_pixmap (int idx, int color)
{
	int bg, i;
	char *colors = dnb_colors;
	static char pixbuf[DNB_CELL_SIZE * (DNB_CELL_SIZE+1)];
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	if (idx == DNB_DOT)
		return pixmap_ball_gen (DNB_CELL_SIZE, pixbuf, 0x101010, bg, 8, 30);
	if (idx == DNB_VERT) return dnb_vertical_24_xpm;
	if (idx == DNB_HOR) return dnb_horizontal_24_xpm;
	if (idx == DNB_RED) return dnb_red_24_xpm;
	if (idx == DNB_BLUE) return dnb_blue_24_xpm;
	return NULL;
}

void dnb_set_init_pos (Pos *pos)
{
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (i%2 == 0 && j%2 == 0)
			pos->board[j * board_wid + i] = DNB_DOT;
		else
			pos->board[j * board_wid + i] = DNB_EMPTY;
}

/*
ResultType dnb_who_won (Pos *pos, Player player, char ** commp)
{
	char *who_str[2] = {"Vertical won", "Horizontal won"};
	int wscore, bscore, i, j;
	float eval;
	ResultType result;
	result = dnb_eval (pos, player, &eval);
	if (result == RESULT_WHITE) *commp = who_str[0];
	if (result == RESULT_BLACK) *commp = who_str[1];
	printf ("%f\n", eval);
	return result;
}
*/

static int dnb_incx[] = {-1, 0, 0, 1};
static int dnb_incy[] = {0, -1, 1, 0};

static int num_nbr_walls (byte *board, int *render, int x, int y)
{
	int k, newx, newy;
	int count;
	if (x % 2 == 0 || y % 2 == 0)
		return -1;
	for (k=0, count=0; k<4; k++)
	{
		newx = x + dnb_incx[k];
		newy = y + dnb_incy[k];
		if (board [newy * board_wid + newx] != DNB_EMPTY ||
				(render && render[newy * board_wid + newx] != RENDER_NONE))
			count++;
	}
	return count;
}

int dnb_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player player, 
		byte **movp, int **rmovp)
{
	int incx, incy;
	int newx, newy;
	static byte move[2048];
	static int rmove[16];	
	byte *mp = move;
	int *rp = rmove;
	int i, j, k;
	gboolean found;
	
	if (type == GTKBOARD_BUTTON_PRESS)
	{
		if (pos->board [y * board_wid + x] != DNB_DOT)
			return -1;
		dnb_curx = x;
		dnb_cury = y;
		return 0;
	}
	
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	
	if (dnb_curx < 0) return -1;
	
	if ((x != dnb_curx && y != dnb_cury) || (x == dnb_curx && y == dnb_cury))
		{ dnb_curx = dnb_cury = -1;	return -1; }

	if (x == dnb_curx)
	{
		incx = 0;
		incy = y > dnb_cury ? 1 : -1;
	}
	else
	{
		incy = 0;
		incx = x > dnb_curx ? 1 : -1;
	}
	
	newx = dnb_curx + incx;
	newy = dnb_cury + incy;
	if (pos->board [newy * board_wid + newx] != DNB_EMPTY || 
			pos->render [newy * board_wid + newx] != RENDER_NONE)
	{ dnb_curx = dnb_cury = -1; return -1; }
	
	*rp++ = newx;
	*rp++ = newy;
	*rp++ = RENDER_REPLACE | ((dnb_curx == x ? DNB_VERT : DNB_HOR) << 8);
	dnb_curx = dnb_cury = -1;

	// do we complete a square
	found = FALSE;
	for (k=0; k<4; k++)
	{
		x = newx + dnb_incx[k];
		y = newy + dnb_incy[k];
		if (pos->board [y * board_wid + x] == DNB_EMPTY)
			if (num_nbr_walls (pos->board, pos->render, x, y) == 3)
			{
				*rp++ = x;
				*rp++ = y;
				*rp++ = RENDER_REPLACE | ((player == WHITE ? DNB_RED : DNB_BLUE) << 8);
				found = TRUE;
			}
	}
	*rp++ = -1;
	if (found)
	{
		*rmovp = rmove;
		return 0;
	}

	else
	{
		for (i=0; rmove[3*i] >= 0; i++)
			if ((rmove [3*i+2] & 0xff) == RENDER_REPLACE)
			{
				*mp++ = rmove [3*i];
				*mp++ = rmove [3*i+1];
				*mp++ = rmove [3*i+2] >> 8;
			}
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if ((pos->render [j * board_wid + i] & 0xff) == RENDER_REPLACE)
			{
				*mp++ = i;
				*mp++ = j;
				*mp++ = pos->render [j * board_wid + i] >> 8;
			}
		*mp++ = -1;
		*movp = move;
		return 1;
	}
}

static void dnb_get_render (Pos *pos, byte *move, int **rmovp)
{
	static int rmove[2048];
	int *rp = rmove;
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (pos->render [j * board_wid + i] != RENDER_NONE)
		{
			*rp++ = i;
			*rp++ = j;
			*rp++ = RENDER_NONE;
		}
	*rp++ = -1;
	*rmovp = rmove;
}


void dnb_search (Pos *pos, byte **movp)
{
	/* first greedily close all possible squares, then choose some edge arbitrarily */
	// TODO: this AI needs MAJOR improvement
	static byte move[2048];
	byte *mp = move;
	int i, j;
	gboolean found;
	static byte newboard[DNB_BOARD_WID * DNB_BOARD_HEIT];
	Player player = pos->player;
	memcpy (newboard, pos->board, board_wid * board_heit);
	
	do // very slow, but speed probably doesn't matter here
	{
		found = FALSE;
		for (i=1; i<board_wid; i+=2)
		for (j=1; j<board_heit; j+=2)
			if (num_nbr_walls (newboard, NULL, i, j) == 3)
			{
				int k, newx, newy, newval;
				for (k=0; k<4; k++)
				{
					newx = i + dnb_incx[k];
					newy = j + dnb_incy[k];
					if (newboard [newy * board_wid + newx] == DNB_EMPTY)
					{
						newval = (newx % 2 == 0 ? DNB_VERT : DNB_HOR);
						*mp++ = newx;
						*mp++ = newy;
						*mp++ = newval;
						newboard [newy * board_wid + newx] = newval;
					}
				}
				newval = (player == WHITE ? DNB_RED : DNB_BLUE);
				*mp++ = i;
				*mp++ = j;
				*mp++ = newval;
				newboard [j * board_wid + i] = newval;
				found = TRUE;
			}
	} while (found);

	while (1) // FIXME: inf. loop when game is over
	{
		i = random() % board_wid;
		j = random() % board_heit;
		if ((i+j) % 2 == 0) continue;
		if (newboard [j * board_wid + i] != DNB_EMPTY) continue;
		*mp++ = i;
		*mp++ = j;
		*mp++ = (i % 2 == 0 ? DNB_VERT : DNB_HOR);
		*mp++ = -1;
		*movp = move;
		return;
	}
}
