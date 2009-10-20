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
#include "aaball.h"

#define FIFTEEN_CELL_SIZE 60
#define FIFTEEN_NUM_PIECES 16

#define FIFTEEN_BOARD_WID 4
#define FIFTEEN_BOARD_HEIT 4

char fifteen_colors[6] = {220, 220, 220, 220, 220, 220};

int * fifteen_init_pos = NULL;

void fifteen_init ();

Game Fifteen = { FIFTEEN_CELL_SIZE, FIFTEEN_BOARD_WID, FIFTEEN_BOARD_HEIT, 
	FIFTEEN_NUM_PIECES, fifteen_colors,  NULL, NULL, "Fifteen Puzzle", "Arcade",
	fifteen_init};

static void fifteen_set_init_pos (Pos *pos);
static char ** fifteen_get_pixmap (int idx, int color);
static guchar * fifteen_get_rgbmap (int idx, int color);
int fifteen_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player, byte **, int **);
static int fifteen_getmove_kb (Pos *cur_pos, int key, byte **move, int **);
void fifteen_free ();
void fifteen_init ();
ResultType fifteen_who_won (Pos *, Player, char **);

int fifteen_done (byte *board)
{
	int size = board_wid * board_heit, i;
	for (i=0; i<size; i++)
		if (board[i] != (i+1) && board[i] != 0) return 0;
	return 1;
}

//! will the game be completed by moving piece
int fifteen_nearly_done (byte *board, int piece)
{
	int size = board_wid * board_heit, i;
	int empty = -1, count;
	for (i=0, count=0; i<size; i++)
	{
		if (board[i] == 0) empty = i;
		else if (board[i] != i+1) count++;
//		if (board[i] != (i+1) && board[i] != 0) return 0;
	}
	if (count != 1) return 0;
	if (piece != empty+1) return 0;
	return 1;
	
}

// After fifteen_done returns true user must click on the remaining empty square to complete the game
int fifteen_really_done (byte *board)
{
	int size = board_wid * board_heit, i;
	for (i=0; i<size; i++)
		if (board[i] != (i+1)) return 0;
	return 1;
}

void fifteen_init ()
{
	game_single_player = TRUE;
	game_set_init_pos = fifteen_set_init_pos;
	game_get_pixmap = fifteen_get_pixmap;
	game_get_rgbmap = fifteen_get_rgbmap;
	game_getmove = fifteen_getmove;
	game_getmove_kb = fifteen_getmove_kb;
	game_who_won = fifteen_who_won;
	game_scorecmp = game_scorecmp_def_iscore;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_rules = 
		"The classic fifteen puzzle (Sam Loyd, c.a. 1870). On each turn you move to the empty square one of the adjacent pieces. The objective is to complete the pattern. In the gtkboard implementation the pattern is a pair of concentric circles.\n";
}


void fifteen_get_cur (byte *pos, int *x, int *y)
{
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (pos [j * board_wid + i] == 0)
		{ *x = i; *y = j; return;
		}
}

ResultType fifteen_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int over = fifteen_really_done (pos->board);
	char *scorestr = over ? "You won. Moves:" : "Moves:";
	// last move is a dummy
	snprintf (comment, 32, "%s %d", scorestr, pos->num_moves - (over ? 1 : 0));
	*commp = comment;
	return over ? RESULT_WON : RESULT_NOTYET;
}


int fifteen_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp)
{
	static byte move[10];
	byte *mp = move;
	int i, j, incx, incy;
	switch (key)
	{
		case GDK_Up: incx = 0; incy = 1; break;
		case GDK_Down: incx = 0; incy = -1; break;
		case GDK_Right: incx = 1; incy = 0; break;
		case GDK_Left: incx = -1; incy = 0; break;
		default: return -1;
	}
	incx *= -1; incy *= -1;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (pos->board [j * board_wid + i] == 0)
		{
			int x = i + incx, y = j + incy;
			if (x < 0 || y < 0 || x >= board_wid || y >= board_heit)
				return -1;
			*mp++ = x; *mp++ = y; *mp++ = 0;
			*mp++ = i; *mp++ = j; *mp++ = pos->board [y * board_wid + x];
			*mp++ = -1;
			*movp = move;
			return 1;
		}
	}
	g_assert_not_reached();
	return -1;
}

void fifteen_set_init_pos (Pos *pos)
{
	int i, j;
	int size = board_wid * board_heit;
	int swaps = 0;
	byte *board = pos->board;
	for (i=0; i<size; i++)
		board[i] = i+1;
	board[random()%(board_heit*board_wid)] = 0;
	for (i=1; i<size; i++)
	{
		int tmp;
		if (!board[i]) continue;
		do j = random() % (i + 1); while (!board[j]);
		if (j == i) continue;
		tmp = board[i]; board[i] = board[j]; board[j] = tmp;
		swaps += 1;
	}
	
	if (swaps % 2 != 0)
	{
		int tmp;
		i=0; if (!board[0] || !board[1]) i=2; j = i+1;
	   	tmp = board[i]; board[i] = board[j]; board[j] = tmp;
	}
}


int fifteen_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int ** rmovep)
{
	int k;
	static byte move[16];
	byte *mp = move;
	int incx[] = {0, 0, 1, -1};
	int incy[] = {1, -1, 0, 0};
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (pos->board [y * board_wid + x] == 0 && fifteen_done (pos->board))
	{
		*mp++ = x; *mp++ = y; *mp++ = y * board_wid + x + 1;
		*mp++ = -1; *movp = move; return 1;
	}
	for (k=0; k<4; k++)
	{
		int newx = x + incx[k], newy = y + incy[k];
		if (newx < 0 || newy < 0 || newx >= board_wid || newy >= board_heit)
			continue;
		if (pos->board [newy * board_wid + newx] == 0)
		{
			*mp++ = x; *mp++ = y; 
			if (fifteen_nearly_done (pos->board, pos->board[y * board_wid + x]))
				*mp++ = y * board_wid + x + 1;
			else *mp++ = 0;
			*mp++ = newx; *mp++ = newy; *mp++ = pos->board [y * board_wid + x];
			*mp++ = -1;
			*movp = move;
			return 1;
		}
	}
	return -1;
}

char ** fifteen_pixmap_square_gen (char *col)
{
	int i, j;
	char **pixmap;
	static char line[FIFTEEN_CELL_SIZE];
	memset(line, ' ', FIFTEEN_CELL_SIZE);
	pixmap = g_new(char *, FIFTEEN_CELL_SIZE + 2);
	pixmap[0] = "60 60 1 1"; // FIXME: dont hard code
	// FIXME: not freed
	pixmap[1] = g_strdup_printf (" c %s", col);
	for (i=0; i<FIFTEEN_CELL_SIZE; i++) pixmap[i+2] = line; return pixmap;
}

char ** fifteen_get_pixmap (int idx, int color)
{
	int fg, bg, i;
	char *colors = fifteen_colors;
	return fifteen_pixmap_square_gen("#e7d7d7");
}

guchar *fifteen_get_rgbmap (int idx, int color)
{
	static guchar buf[3 * FIFTEEN_CELL_SIZE * FIFTEEN_CELL_SIZE];
	int i, j;
	int xoff, yoff;
	guchar *bp = buf;
	idx--;
	xoff = (idx % FIFTEEN_BOARD_WID) * FIFTEEN_CELL_SIZE;
	yoff = (FIFTEEN_BOARD_HEIT - 1 - (idx / FIFTEEN_BOARD_WID)) * FIFTEEN_CELL_SIZE;
	for (j=0; j<FIFTEEN_CELL_SIZE; j++)
	for (i=0; i<FIFTEEN_CELL_SIZE; i++)
	{
		int maxx = FIFTEEN_BOARD_WID * FIFTEEN_CELL_SIZE;
		int maxy = FIFTEEN_BOARD_HEIT * FIFTEEN_CELL_SIZE;
		float x = 1.0*(xoff + i)/maxx, y = 1.0*(yoff + j)/maxy;
		float d = sqrt ((x-0.5) * (x-0.5) + (y-0.5) * (y-0.5));
		d *= 2;
		if ((d > 0.9 && d < 0.98))
		{	
			float frac = fabs ((d - 0.94) / 0.04);
			frac = 2 * (frac - 0.5);
			if (frac < 0) frac = 0;
			*bp++ = sqrt (1 - frac) * 255;
			*bp++ = sqrt (frac) * 256 * x; 
			*bp++ = sqrt (frac) * 256 * y;
		}
		else if ((d>0.3 && d < 0.36))
		{
			float frac = fabs ((d - 0.33) / 0.03);
			frac = 2 * (frac - 0.5);
			if (frac < 0) frac = 0;
			*bp++ = sqrt (1 - frac) * 255;
			*bp++ = sqrt (frac) * 256 * x; 
			*bp++ = sqrt (frac) * 256 * y;
		}
		else
		{
			*bp++ = 0;
			*bp++ = 256 * x;
			*bp++ = 256 * y;
		}
	}
	return buf;
}
