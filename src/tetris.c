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

#define TETRIS_CELL_SIZE 20
#define TETRIS_NUM_PIECES 32

#define TETRIS_REAL_WID 10
#define TETRIS_BOARD_WID 16
#define TETRIS_BOARD_HEIT 22
#define TETRIS_PREVIEW_HEIT 3

char tetris_colors[9] = {50, 50, 50, 50, 50, 50, 150, 150, 150};

static int tetris_init_pos [TETRIS_BOARD_WID*TETRIS_BOARD_HEIT] = 
{
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 0 , 0 , 0 , 0 , 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 0 , 0 , 0 , 0 , 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 0 , 0 , 0 , 0 , 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 0 , 0 , 0 , 0 , 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 32, 32, 32, 32, 32, 32, 
};


void tetris_init ();

Game Tetris = { TETRIS_CELL_SIZE, TETRIS_BOARD_WID, TETRIS_BOARD_HEIT, 
	TETRIS_NUM_PIECES,
	tetris_colors,  tetris_init_pos, NULL, "Tetris", "Arcade", tetris_init};

SCORE_FIELD tetris_score_fields[] = {SCORE_FIELD_USER, SCORE_FIELD_SCORE, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *tetris_score_field_names[] = {"User", "Score", "Date", NULL};

#define TETRIS_EMPTY 0
#define TETRIS_BRICK_4 1 
#define TETRIS_BRICK_22 2 
#define TETRIS_BRICK_121A 3 
#define TETRIS_BRICK_121B 4
#define TETRIS_BRICK_T 5 
#define TETRIS_BRICK_LA 6
#define TETRIS_BRICK_LB 7
#define TETRIS_BRICK_INACTIVE 8
#define TETRIS_BRICK_DYING 9
#define TETRIS_BRICK_MASK 15
#define TETRIS_BRICK_MOTION_MASK 16
#define TETRIS_UNUSED 32

static void tetris_set_init_pos (Pos *pos);
static char ** tetris_get_pixmap (int idx, int color);
static int tetris_getmove_kb (Pos *cur_pos, int key, byte **move, int **);
static int tetris_animate (Pos *pos, byte **movp);
static ResultType tetris_who_won (Pos *pos, Player to_play, char **commp);
static void *tetris_newstate (Pos *, byte *);
static void tetris_free ();

typedef struct
{
	int score;
}
Tetris_state;

static int num_bricks = 0;
static int level = 1;
static int anim_time_left = 0;
static int anim_time_def = 0;

void tetris_init ()
{
	game_single_player = TRUE;
	game_get_pixmap = tetris_get_pixmap;
	game_getmove_kb = tetris_getmove_kb;
	game_animation_time = 50;
	game_animate = tetris_animate;
	game_who_won = tetris_who_won;
	game_stateful = TRUE;
	game_state_size = sizeof (Tetris_state);
	game_newstate = tetris_newstate;
	game_scorecmp = game_scorecmp_def_dscore;
	game_allow_back_forw = FALSE;
	game_scorecmp = game_scorecmp_def_dscore;
	game_score_fields = tetris_score_fields;
	game_score_field_names = tetris_score_field_names;
	game_draw_cell_boundaries = TRUE;
	game_free = tetris_free;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Tetris\n"
		"Single player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL ("tetris");
	game_doc_rules =
		"This is a game of falling bricks. Your objective is to rotate the pieces as they fall in order to make complete rows of bricks. Use the arrow keys to move left or right, Up to rotate, and Space to fall. Completing a row gives you 40 points, two rows simultaneously 100 points, three rows 300 points, and four rows (a tetris) 1200 points.";
}

void tetris_free ()
{
	num_bricks = 0;
	level = 1;
	anim_time_left = 0;
}

void *tetris_newstate (Pos *pos, byte *move)
	// TODO: implement points for falling
{
	int i, score = 0;
	static Tetris_state state;
	int linepts[5] = {0, 40, 100, 300, 1200};
	for (i=0; move[3*i] >= 0; i++)
		if (move[3*i+2] == 0)
			score++;
	score /= TETRIS_REAL_WID;
	score = linepts [score];
	state.score = (pos->state ? ((Tetris_state *)pos->state)->score : 0) + score;
	return &state;
}

int tetris_game_over (byte *board)
{
	int i;
	for (i=0; i<TETRIS_REAL_WID; i++)
		if (board[(board_heit - 2) * board_wid + i] == TETRIS_BRICK_INACTIVE)
			return 1;
	return 0;
}


ResultType tetris_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i;
	int over = tetris_game_over (pos->board);
	snprintf (comment, 32, "%s %s %d", 
			over ? "Game over. " : "", "Score:",
			pos->state ? ((Tetris_state *)pos->state)->score : 0);
	*commp = comment;
	return over ? RESULT_WON : RESULT_NOTYET;
}

int tetris_fall (byte *pos, byte **movp, int height)
{
	static byte move[32];
	byte *mp = move;
	int moving = 0;
	int canfall = 1;
	int i, j, k;
	for (j=0; j<board_heit; j++)
	for (i=0; i<TETRIS_REAL_WID; i++)
	{
		if (pos [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
		{
			int tmp;
			if (height > j) height = j;
			moving = 1;
			if (j == 0) canfall = 0;
			for (k=1; k<=height; k++)
			{
				tmp = pos [(j-k) * board_wid + i];
				if (tmp != 0 && !(tmp & TETRIS_BRICK_MOTION_MASK))
					canfall = 0;
			}
		}
	}
	if (moving && canfall)
	{
		for (i=0; i<TETRIS_REAL_WID; i++)
		for (j=0; j<board_heit; j++)
		{
			if (j < board_heit - height)
			if (!(pos [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK) 
				&& (pos [(j+height) * board_wid + i] & TETRIS_BRICK_MOTION_MASK))
			{
				*mp++ = i; *mp++ = j; *mp++ = pos [(j+height) * board_wid + i];
			}
			if (pos [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK 
				&& (j >= board_heit - height || 
					!(pos [(j+height) * board_wid + i] & TETRIS_BRICK_MOTION_MASK)))
			{
				*mp++ = i; *mp++ = j; *mp++ = 0;
			}
		}
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	return -1;
}

int tetris_animate (Pos *pos, byte **movp)
{
	static byte move[1024];
	static int count = 0;
	byte *mp = move;
	byte *board = pos->board;
	int i, j;
	int level = num_bricks / 20 + 1;
	if (level > 10) level = 10;
	anim_time_def = (12 - level) / 2;
	if (anim_time_left) 
	{
		anim_time_left--;
		return 0;
	}

	anim_time_left = 12 - level;
	if (tetris_fall(board, movp, 1) > 0)
		return 1;

	for (i=0; i<TETRIS_REAL_WID; i++)
	for (j=0; j<board_heit; j++)
		if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
		{
			*mp++ = i; *mp++ = j; 
			*mp++ = TETRIS_BRICK_INACTIVE;
		}
	for (j=0; j<board_heit; j++)
	{
		int nfull = 0;
		while (nfull + j < board_heit)
		{
			int full = 1;
			for (i=0; i<TETRIS_REAL_WID; i++) 
				if (board [(j+nfull) * board_wid + i] == 0) { full = 0; break; }
			if (!full) break;
			nfull++;
		}
		if (nfull > 0)
		{
			for (; j+nfull<board_heit; j++)
			{
				for (i=0; i<TETRIS_REAL_WID; i++) 
				{
					if (board [j * board_wid + i] != 
							board [(j+nfull) * board_wid + i])
					{ *
						mp++ = i; *mp++ = j; 
						//*mp++ = board [(j+nfull) * board_wid + i]; 
						if ( board [(j+nfull) * board_wid + i] == 0)
							*mp++ = 0;
						else *mp++ =  TETRIS_BRICK_INACTIVE;
					}
				}
			}
			for (; j<board_heit; j++)
			{
				int empty = 1;
				for (i=0; i<TETRIS_REAL_WID; i++) 
				{
					if (board [j * board_wid + i] != 0)
					{
						*mp++ = i; *mp++ = j; *mp++ = 0;
					}
					if (board [j * board_wid + i] != 0) empty = 0;
				}
				if (empty) break;
			}
			*mp++ = -1;
			*movp = move;
			return 1;
		}
	}
	
	num_bricks ++;

	{
	int idx;
	byte *saved_m = mp;
	static int next_tile = -1;
	// This depends on the #defs!!
	// FIXME: change the shapes so that no brick is more than 2 rows tall
	byte shapes [][4][2] = {
		{ { 3, 1} , {4, 1}, {5, 1}, {6, 1} },
		{ { 4, 1} , {5, 1}, {4, 2}, {5, 2} },
		{ { 4, 2} , {5, 2}, {5, 1}, {6, 1} },
		{ { 4, 1} , {5, 1}, {5, 2}, {6, 2} },
		{ { 4, 1} , {5, 1}, {6, 1}, {5, 2} },
		{ { 6, 1} , {5, 1}, {4, 2}, {4, 1} },
		{ { 3, 1} , {4, 1}, {5, 1}, {5, 2} },
	};

	idx = next_tile;
	next_tile = random() % 7;
	if (idx < 0)
		idx = random() % 7;	
	for (i=0; i<4; i++)
	{
		*mp++ = shapes[idx][i][0]; 
		*mp++ = board_heit - shapes[idx][i][1]; 
		if (board [mp[-1] * board_wid + mp[-2]])
		{
			// we need to return the move up to the previous stage
			*saved_m++ = -1;
			*movp = move;
			return 1;
		}
		*mp++ = (idx+1) | TETRIS_BRICK_MOTION_MASK;
	}
	for (i=0; i<4; i++)
	for (j=0; j<4; j++)
	{
		*mp++ = TETRIS_REAL_WID + 1 + i;
		*mp++ = board_heit - 1 - TETRIS_PREVIEW_HEIT - j;
		*mp++ = 0;
	}
	for (i=0; i<4; i++)
	{
		*mp++ = TETRIS_REAL_WID + shapes[next_tile][i][1] + 1; 
		*mp++ = board_heit - TETRIS_PREVIEW_HEIT + shapes[next_tile][i][0] - 7;
		*mp++ = (next_tile+1);
	}
	*mp++ = -1;
	*movp = move;
	return 1;
	}
}


int tetris_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp)
{
	static byte move[32];
	byte *mp = move;
	int incx;
	int i, j;
	byte *board = pos->board;

	if (key == ' ')
	{
		for (i = board_heit; i>0; i--)
			if (tetris_fall(board, movp, i) > 0)
			{
				anim_time_left = anim_time_def;
				return 1;
			}
		return -1;
	}
	
	if (key == GDK_Down)
	{
		int retval = tetris_fall(board, movp, 1);
		if (retval > 0) anim_time_left = anim_time_def;
		return retval;
	}
	
	if (key == GDK_Up)
	{
		int sumx = 0, sumy = 0, k = 0, incy;
		int thebrick = 0;
		byte newboard [4][2];
		for (i=0; i<TETRIS_REAL_WID; i++)
		for (j=0; j<board_heit; j++)
			if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
			{ 
				sumx += i; sumy += j; 
				thebrick = board [j * board_wid + i] | TETRIS_BRICK_MOTION_MASK;
				if (j == 0 || ((board [(j-1) * board_wid + i] != 0) 
					&& !(board [(j-1) * board_wid + i] & TETRIS_BRICK_MOTION_MASK)))
					return -1;
			}
		if (sumy == 0) return -1;
		sumx += 3; incy = sumy % 4 > 0 ? 1 : 0; sumx /= 4; sumy /= 4;
		for (i=0; i<TETRIS_REAL_WID; i++)
		for (j=0; j<board_heit; j++)
			if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
			{
				newboard[k][0] = sumx + sumy - j;
				newboard[k][1] = sumy - sumx + i + incy;
				if (newboard[k][0] < 0 || newboard[k][1] < 0 || 
						newboard[k][0] >= TETRIS_REAL_WID || newboard[k][1] >= board_heit)
					return -1;
				if (board [newboard [k][1] * board_wid + newboard[k][0]]
						== TETRIS_BRICK_INACTIVE)
					return -1;
				k++;
			}
		for (i=0; i<TETRIS_REAL_WID; i++)
		for (j=0; j<board_heit; j++)
		{
			if (!(board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK))
			{
				int found = 0;
				for (k=0; k<4; k++) 
					if (newboard[k][0] == i && newboard[k][1] == j)
					{ found = 1; break; }
				if (found)
				{
					*mp++ = i; *mp++ = j; *mp++ = thebrick;
				}
			}
			if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
			{
				int found = 0;
				for (k=0; k<4; k++) 
					if (newboard[k][0] == i && newboard[k][1] == j)
					{ found = 1; break; }
				if (!found)
				{
					*mp++ = i; *mp++ = j; *mp++ = 0;
				}
			}
		}
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	switch (key)
	{
		case GDK_Left: incx = 1; break;
		case GDK_Right: incx = -1; break;
		default: return -1;
	}
	for (i=0; i<TETRIS_REAL_WID; i++)
	for (j=0; j<board_heit; j++)
		if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
		{
			if (i - incx < 0 || i - incx >= TETRIS_REAL_WID) return -1;
			if (board [j * board_wid + i - incx] != 0 && 
					!(board [j * board_wid + i - incx] & TETRIS_BRICK_MOTION_MASK))
				return -1;
		}
	for (i=0; i<TETRIS_REAL_WID; i++)
	for (j=0; j<board_heit; j++)
	{
		if (i+incx >= 0 && i+incx < TETRIS_REAL_WID)
		if (!(board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK) 
			&& (board [j * board_wid + i+incx] & TETRIS_BRICK_MOTION_MASK))
		{
			*mp++ = i; *mp++ = j; *mp++ = board [j * board_wid + i+incx];
		}
		if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK 
			&& (i+incx < 0 || i+incx >= TETRIS_REAL_WID || 
				!(board [j * board_wid + i+incx] & TETRIS_BRICK_MOTION_MASK)))
		{
			*mp++ = i; *mp++ = j; *mp++ = 0;
		}
	}
	*mp++ = -1;
	*movp = move;
	return 1;
}


char ** tetris_get_pixmap (int idx, int color)
{
	int i;
	static char *pixmap [TETRIS_CELL_SIZE + 2];
	char *line = "                    ";
	//pixmap = g_new(char *, TETRIS_CELL_SIZE + 2);
	pixmap[0] = "20 20 1 1";
	if (idx == TETRIS_UNUSED)
		pixmap[1] = "  c #969696";
	else
		switch (idx & TETRIS_BRICK_MASK)
		{
			case TETRIS_BRICK_4: pixmap[1] = "  c blue"; break;
			case TETRIS_BRICK_22: pixmap[1] = "  c red"; break;
			case TETRIS_BRICK_121A: pixmap[1] = "  c yellow"; break;
			case TETRIS_BRICK_121B: pixmap[1] = "  c magenta"; break;
			case TETRIS_BRICK_T: pixmap[1] = "  c green"; break;
			case TETRIS_BRICK_LA: pixmap[1] = "  c pink"; break;
			case TETRIS_BRICK_LB: pixmap[1] = "  c orange"; break;
			case TETRIS_BRICK_INACTIVE: pixmap[1] = "  c gray"; break;
			default: return NULL;
		}
	for (i=0; i<TETRIS_CELL_SIZE; i++) pixmap[2+i] = line;
	return pixmap;
}

