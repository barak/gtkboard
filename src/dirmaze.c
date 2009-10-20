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
#include <math.h>
#include <gdk/gdkkeysyms.h>

#include "game.h"
#include "../pixmaps/arrows.xpm"
#include "aaball.h"

#define HYPERMAZE_CELL_SIZE 20
#define HYPERMAZE_NUM_PIECES 10

#define HYPERMAZE_BOARD_WID 25
#define HYPERMAZE_BOARD_HEIT 25

char hypermaze_colors[6] = {100, 150, 200, 100, 150, 200};

int * hypermaze_init_pos = NULL;

int hypermaze_hypermaze[HYPERMAZE_BOARD_WID][HYPERMAZE_BOARD_HEIT] = {{0}};

#define HYPERMAZE_CUR 1
#define HYPERMAZE_N 2
#define HYPERMAZE_E 3
#define HYPERMAZE_S 4
#define HYPERMAZE_W 5
#define HYPERMAZE_NE 6
#define HYPERMAZE_SE 7
#define HYPERMAZE_SW 8
#define HYPERMAZE_NW 9
#define HYPERMAZE_WALL 10

void hypermaze_init ();

SCORE_FIELD hypermaze_score_fields[] = {SCORE_FIELD_USER, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *hypermaze_score_field_names[] = {"User", "Time", "Date", NULL};

Game Hypermaze = { HYPERMAZE_CELL_SIZE, HYPERMAZE_BOARD_WID, HYPERMAZE_BOARD_HEIT, 
	HYPERMAZE_NUM_PIECES, hypermaze_colors,  NULL, NULL, "Hypermaze", "Maze", hypermaze_init};


static void hypermaze_set_init_pos (Pos *pos);
static char ** hypermaze_get_pixmap (int idx, int color);
static int hypermaze_getmove_kb (Pos *, int, byte **, int **);
int hypermaze_getmove (Pos *pos, int, int, GtkboardEventType, Player, byte **, int **);
ResultType hypermaze_who_won (Pos *, Player, char **);


void hypermaze_init ()
{
	game_single_player = TRUE;
	game_set_init_pos = hypermaze_set_init_pos;
	game_get_pixmap = hypermaze_get_pixmap;
	game_getmove_kb = hypermaze_getmove_kb;
	game_getmove = hypermaze_getmove;
	game_who_won = hypermaze_who_won;
	game_start_immediately = TRUE;
	game_scorecmp = game_scorecmp_def_time;
	game_score_fields = hypermaze_score_fields;
	game_score_field_names = hypermaze_score_field_names;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"HyperMaze\n"
		"Single player game\n"
		"Status: Partially implemented (playable)\n"
		"URL: "GAME_DEFAULT_URL ("hypermaze");
	game_doc_rules = 
		"Your goal is to get from the lower left corner to the upper right. But you can travel only along the arrows. Beware - the arrows are one-way! You can easily get trapped in a blind end from which there is no escape."
		"\n"
		"To move with the mouse, click on the square to which you want to move. To move with the keyboard, use the NumPad keys.\n";
}

void hypermaze_get_cur_pos (byte *pos, int *x, int *y)
{
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (pos [j * board_wid + i] == HYPERMAZE_CUR)
		{ *x = i; *y = j; return;
		}
}


ResultType hypermaze_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	gboolean over = (pos->board [board_wid * board_heit - 1] == HYPERMAZE_CUR);
	snprintf (comment, 32, "%sMoves: %d", 
			over ? "You won. " : "",
			pos->num_moves);
	*commp = comment;
	return over ? RESULT_WON : RESULT_NOTYET;
}

int hypermaze_canmove (byte *board, int curx, int cury, int incx, int incy)
{
	switch (board [(cury + incy/2) * board_wid + curx + incx/2])
	{
		case HYPERMAZE_N: if (incx != 0 || incy != 2) return 0; break;
		case HYPERMAZE_S: if (incx != 0 || incy != -2) return 0; break;
		case HYPERMAZE_E: if (incx != 2 || incy != 0) return 0; break;
		case HYPERMAZE_W: if (incx != -2 || incy != 0) return 0; break;
		case HYPERMAZE_NE: if (incx != 2 || incy != 2) return 0; break;
		case HYPERMAZE_NW: if (incx != -2 || incy != 2) return 0; break;
		case HYPERMAZE_SE: if (incx != 2 || incy != -2) return 0; break;
		case HYPERMAZE_SW: if (incx != -2 || incy != -2) return 0; break;
		case HYPERMAZE_WALL: return 0;
	}
	return 1;
}

int hypermaze_getmove_common (Pos *pos, byte **movp, 
		int x, int y, int curx, int cury, int incx, int incy)
{
	static byte move[10];
	if (x < 0 || y < 0 || x >= board_wid || y >= board_heit) return -1;
	if (pos->board[y * board_wid + x] == HYPERMAZE_WALL) return -1;
	if (!hypermaze_canmove (pos->board, curx, cury, incx, incy)) return -1;
	move[0] = curx; move[1] = cury; move[2] = 0;
	move[3] = x; move[4] = y; move[5] = HYPERMAZE_CUR;
	move[6] = -1;
	*movp = move;
	return 1;
}

int hypermaze_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
{
	int curx = -1, cury = -1;
	int incx = -1, incy = -1;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	hypermaze_get_cur_pos (pos->board, &curx, &cury);
	g_assert (curx >= 0 && cury >= 0);
	if (abs (x - curx) != 2 && abs (x - curx) != 0) return -1;
	if (abs (y - cury) != 2 && abs (y - cury) != 0) return -1;
	if (abs (x - curx) == 0 && abs (y - cury) == 0) return -1;
	return hypermaze_getmove_common (pos, movp, 
			x, y, curx, cury, x - curx, y - cury);
	
}

int hypermaze_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp)
{
	//static byte move[10];
	int curx = -1, cury = -1;
	int incx = -1, incy = -1;
	int x, y;
	hypermaze_get_cur_pos (pos->board, &curx, &cury);
	g_assert (curx >= 0 && cury >= 0);
	switch (key)
	{
		case GDK_KP_Home: incx = -2; incy = 2; break;
		case GDK_KP_Page_Up: incx = 2; incy = 2; break;
		case GDK_KP_End: incx = -2; incy = -2; break;
		case GDK_KP_Page_Down: incx = 2; incy = -2; break;
		case GDK_Up:
		case GDK_KP_Up: incx = 0; incy = 2; break;
		case GDK_Down:
		case GDK_KP_Down: incx = 0; incy = -2; break;
		case GDK_Right:
		case GDK_KP_Right: incx = 2; incy = 0; break;
		case GDK_Left:
		case GDK_KP_Left: incx = -2; incy = 0; break;
		default: return -1;
	}
	x = curx + incx;
	y = cury + incy;
	return hypermaze_getmove_common (pos, movp, x, y, curx, cury, incx, incy);
}

static void recursive_pathgen (byte *board, int x, int y, int val)
{
	static int incx[8] = {-2, 0, 0, 2, -2, -2, 2, 2};
	static int incy[8] = {0, -2, 2, 0, -2, 2, -2, 2};

	int i;
	if (!ISINBOARD(x, y)) return;
	if (board[y * board_wid + x] == val) return;
	board [y * board_wid + x] = val;
	for (i=0; i<8; i++)
		if (hypermaze_canmove (board, x, y, incx[i], incy[i]))
			recursive_pathgen (board, x+incx[i], y+incy [i], val);
}

void hypermaze_set_init_pos (Pos *pos)
{
	int i, j;
	int x, y;
	do
	{
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (i % 2 || j % 2)
			{
				if ((i + j) % 2 == 0)
					pos->board [j * board_wid + i] = 6 + random() % 4;
				else
					pos->board [j * board_wid + i] = 2 + random() % 4;
			}
			else 
				pos->board [j * board_wid + i] = 0;

		// edges can't have arbitrary arrows
		for (i=0; i<board_wid; i+=(board_wid-1))
		for (j=1; j<board_heit; j+=2)
			pos->board [j *  board_wid + i] = (random() % 2 ? HYPERMAZE_N : HYPERMAZE_S);
		for (i=1; i<board_wid; i+=2)
		for (j=0; j<board_heit; j+=(board_heit-1))
			pos->board [j *  board_wid + i] = (random() % 2 ? HYPERMAZE_E : HYPERMAZE_W);

		recursive_pathgen (pos->board, 0, 0, -1);
	}
	while (pos->board [(board_heit - 1) * board_wid + (board_wid - 1)] != -1);
	for (i=0; i<board_wid; i+=2)
	for (j=0; j<board_heit; j+=2)
		pos->board [j * board_wid + i] = 0;
	pos->board [0] = HYPERMAZE_CUR;
}

char ** hypermaze_pixmap_square_gen (int idx, char *col)
{
	int i, j;
	char **pixmap;
	char *line = "        ";
	pixmap = g_new(char *, HYPERMAZE_CELL_SIZE + 2);
	pixmap[0] = "20 20 1 1";
	pixmap[1] = g_strdup_printf (" c %s", col);
	for (i=0; i<HYPERMAZE_CELL_SIZE; i++) pixmap[i+2] = line; return pixmap;
}

char ** hypermaze_get_pixmap (int idx, int color)
{
	int fg, bg, i;
	char *colors = hypermaze_colors;
	static char pixbuf[HYPERMAZE_CELL_SIZE * (HYPERMAZE_CELL_SIZE+1)];
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	if (idx == HYPERMAZE_WALL)
		return hypermaze_pixmap_square_gen(idx, "#443333");
	if (idx == HYPERMAZE_CUR)
		return pixmap_ball_gen (HYPERMAZE_CELL_SIZE, pixbuf,
			0xffffff, bg, 6, 20);
	//	return hypermaze_pixmap_square_gen(idx, "#ff7777");
	switch (idx)
	{
		case HYPERMAZE_N: return arrow_blue_n_xpm;
		case HYPERMAZE_S: return arrow_blue_s_xpm;
		case HYPERMAZE_E: return arrow_blue_e_xpm;
		case HYPERMAZE_W: return arrow_blue_w_xpm;
		case HYPERMAZE_NE: return arrow_blue_ne_xpm;
		case HYPERMAZE_NW: return arrow_blue_nw_xpm;
		case HYPERMAZE_SW: return arrow_blue_sw_xpm;
		case HYPERMAZE_SE: return arrow_blue_se_xpm;
		default: assert (0);
	}
}

