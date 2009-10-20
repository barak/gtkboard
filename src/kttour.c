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
#include <time.h>

#include "game.h"
#include "aaball.h"
#include "../pixmaps/chess.xpm"
#include "../pixmaps/misc.xpm"

#define KTTOUR_CELL_SIZE 54
#define KTTOUR_NUM_PIECES 4

#define KTTOUR_BOARD_WID 8
#define KTTOUR_BOARD_HEIT 8

#define KTTOUR_EMPTY 0
#define KTTOUR_CUR 1
#define KTTOUR_START 2
#define KTTOUR_USED 3
#define KTTOUR_HINT 4

char kttour_colors[6] = {200, 200, 160, 200, 200, 160};

void kttour_init ();

Game Kttour = { KTTOUR_CELL_SIZE, KTTOUR_BOARD_WID, KTTOUR_BOARD_HEIT, 
	KTTOUR_NUM_PIECES, 
	kttour_colors, NULL, /*kttour_pixmaps,*/ NULL, "Knight's Tour", NULL,
	kttour_init};

SCORE_FIELD kttour_score_fields[] = {SCORE_FIELD_RANK, SCORE_FIELD_USER, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *kttour_score_field_names[] = {"Rank", "User", "Time", "Date", NULL};

static int kttour_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType kttour_who_won (Pos *, Player, char **);
static char **kttour_get_pixmap (int , int);

void kttour_init ()
{
	game_single_player = 1;
	game_getmove = kttour_getmove;
	game_get_pixmap = kttour_get_pixmap;
	game_who_won = kttour_who_won;
	game_scorecmp = game_scorecmp_def_time;
	game_score_fields =  kttour_score_fields;
	game_score_field_names = kttour_score_field_names;
	game_draw_cell_boundaries = TRUE;
	game_allow_undo = TRUE;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Kttour\n"
		"Single player game\n"
		"Status: Partially implemented\n"
		"URL: "GAME_DEFAULT_URL("kttour");
	game_doc_rules = 
		"  Complete the knight's tour of the chessboard.\n"
		"\n"
		"  In the initial position click on any square to start the tour on that square. "
		"Next click on the square you want the knight to move to, and so on. "
		"The square where you started will be shown in green and the other squares in the tour will be grey. "
		"At any point you can click on the green square to make it the current square, in which case the current square will become the \"start\" square. "
		"\n\n"
		"  The objective is to fill all 64 squares in such a way that the last square is one knight-move away from the first, in as little time as possible. "
		"In this game, you can undo your moves freely. This won't prevent you from getting a highscore. "
		"\n\n"
		"  The blue balls that you see are hints. They are there to make your life easier, but you don't have to necessarily click on a ball."
		;
}

static char **kttour_get_pixmap (int idx, int color)
{
	static char pixbuf[KTTOUR_CELL_SIZE*(KTTOUR_CELL_SIZE+1)];
	int i, bg;
	for(i=0, bg=0;i<3;i++) 
	{ int col = kttour_colors[i]; if (col<0) col+=256; bg += col * (1 << (16-8*i));}
	switch (idx)
	{
		case KTTOUR_CUR:
			return chess_wn_54_xpm;
			
		// simulate square using ball of large radius
		case KTTOUR_START:
			return pixmap_ball_gen (KTTOUR_CELL_SIZE, pixbuf, 0x80ff80, bg,
					KTTOUR_CELL_SIZE, 1);
		case KTTOUR_USED:
			return pixmap_ball_gen (KTTOUR_CELL_SIZE, pixbuf, 0x808080, bg,
					KTTOUR_CELL_SIZE, 1);
			
		case KTTOUR_HINT:
			return pixmap_ball_gen (KTTOUR_CELL_SIZE, pixbuf, 0x8080ff, bg,
					KTTOUR_CELL_SIZE/4, 30.0);
		default: return NULL;
	}
}

#define abs(x) ((x)<0?-(x):(x))

static gboolean are_nbrs (int x1, int y1, int x2, int y2)
{
	return abs ((x1 - x2) * (y1 - y2)) == 2 ? TRUE : FALSE;
}

/* TODO: this should be implemented in game_common.c or something like that so that
all games can access it */
static void find_xy (byte *board, int *x, int *y, int val)
{
	int i, j;
	*x = -1;
	*y = -1;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_wid; j++)
		if (board[j * board_wid + i] == val)
		{
			*x = i;
			*y = j;
			return;
		}
}

ResultType kttour_who_won (Pos *pos, Player to_play, char **commp)
{
	int x1, y1, x2, y2;
	int i;
	gboolean found = FALSE;
	for (i=0; i<board_wid*board_heit; i++)
		if (pos->board [i] == KTTOUR_EMPTY)
			return RESULT_NOTYET;
	find_xy (pos->board, &x1, &y1, KTTOUR_CUR);
	find_xy (pos->board, &x2, &y2, KTTOUR_START);
	return are_nbrs (x1, y1, x2, y2) ? RESULT_WON : RESULT_NOTYET;
}

static int incx[] = {-2, -2, -1, -1, 1, 1, 2, 2};
static int incy[] = {-1, 1, -2, 2, -2, 2, -1, 1};

#define IS_FREE(x) ((x) == KTTOUR_EMPTY || (x) == KTTOUR_HINT)

static int get_degree (byte *board, int i, int j)
{
	int k, x, y;
	int count = 0;
	for (k=0; k<8; k++)
	{
		x = i + incx[k];
		y = j + incy[k];
		if (ISINBOARD (x, y) && IS_FREE(board [y * board_wid + x]))
			count++;
	}
	return count;
}

static int get_min_degree (byte *board, int i, int j)
{
	int k, x, y;
	int min_deg = 8;
	for (k=0; k<8; k++)
	{
		int deg;
		x = i + incx[k];
		y = j + incy[k];
		if (ISINBOARD (x, y) && IS_FREE(board [y * board_wid + x]))
			if ((deg = get_degree (board, x, y)) < min_deg)
				min_deg = deg;
	}
	return min_deg;
}

static void add_hints 
	(byte *board, int new_x, int new_y, byte **mp)
{
	int min_deg_new, min_deg_old;
	int k, x, y;
	min_deg_new = get_min_degree (board, new_x, new_y);
	
	for (k=0; k<8; k++)
	{
		x = new_x + incx[k];
		y = new_y + incy[k];
		if (!ISINBOARD (x, y) || board [y * board_wid + x] != KTTOUR_EMPTY)
				continue;
		if (get_degree (board, x, y) == min_deg_new)
		{
			*(*mp)++ = x;
			*(*mp)++ = y;
			*(*mp)++ = KTTOUR_HINT;
		}
	}
	
	for (x=0; x<board_wid; x++)
	for (y=0; y<board_heit; y++)
	{
		if (board [y * board_wid + x] != KTTOUR_HINT)
			continue;
		if (x == new_x && y == new_y)
			continue;
		if (are_nbrs (x, y, new_x, new_y) && get_degree (board, x, y) == min_deg_new)
			continue;
		*(*mp)++ = x;
		*(*mp)++ = y;
		*(*mp)++ = KTTOUR_EMPTY;
	}
}

int kttour_getmove 
	(Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
{
	static byte move[7];
	byte *mp = move;
	int val;
	int cur_x, cur_y;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	val = pos->board[y * board_wid + x];
	if (val == KTTOUR_CUR)
		return 0;
	if (val == KTTOUR_USED)
		return -1;
	find_xy (pos->board, &cur_x, &cur_y, KTTOUR_CUR);
	if (val == KTTOUR_START)
	{
		*mp++ = x;
		*mp++ = y;
		*mp++ = KTTOUR_CUR;
		*mp++ = cur_x;
		*mp++ = cur_y;
		*mp++ = KTTOUR_START;
		add_hints (pos->board, x, y, &mp);
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	if (cur_x >= 0 && !are_nbrs (cur_x, cur_y, x, y))
		return -1;
	*mp++ = x;
	*mp++ = y;
	*mp++ = KTTOUR_CUR;
	if (cur_x >= 0)
	{
		*mp++ = cur_x;
		*mp++ = cur_y;
		*mp++ = pos->num_moves == 1 ? KTTOUR_START : KTTOUR_USED;
	}
	add_hints (pos->board, x, y, &mp);
	*mp++ = -1;
	*movp = move;
	return 1;
}

