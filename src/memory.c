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
#include "../pixmaps/memory.xpm"

#define MEMORY_CELL_SIZE 48
#define MEMORY_NUM_PIECES 72

#define MEMORY_BOARD_WID 9
#define MEMORY_BOARD_HEIT 8


#define MEMORY_EMPTY 0
#define MEMORY_ISOPEN(x) ((x)>MEMORY_NUM_PIECES/2)
#define MEMORY_FLIP(x) ((x)>MEMORY_NUM_PIECES/2?\
		(x)-MEMORY_NUM_PIECES/2:(x)+MEMORY_NUM_PIECES/2)

char ** memory_pixmaps [] = {
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory_blank_xpm,
memory1_xpm,
memory2_xpm,
//memory3_xpm,
memory4_xpm,
memory5_xpm,
memory6_xpm,
memory7_xpm,
memory8_xpm,
//memory9_xpm,
memory10_xpm,
//memory11_xpm,
memory12_xpm,
memory13_xpm,
memory14_xpm,
memory15_xpm,
memory16_xpm,
memory17_xpm,
memory18_xpm,
memory19_xpm,
//memory20_xpm,
memory21_xpm,
memory22_xpm,
memory23_xpm,
//memory24_xpm,
memory25_xpm,
memory26_xpm,
memory27_xpm,
memory28_xpm,
memory29_xpm,
memory30_xpm,
memory31_xpm,
//memory32_xpm,
memory33_xpm,
memory34_xpm,
memory35_xpm,
memory36_xpm,
memory37_xpm,
memory38_xpm,
memory39_xpm,
memory40_xpm,
memory41_xpm,
memory42_xpm,
};

char memory_colors[9] = {220, 220, 180, 220, 220, 180, 0, 0, 0};

int * memory_init_pos = NULL;

void memory_init ();

Game Memory = { MEMORY_CELL_SIZE, MEMORY_BOARD_WID, MEMORY_BOARD_HEIT, 
	MEMORY_NUM_PIECES, memory_colors,  NULL, memory_pixmaps, 
	"Memory", NULL, memory_init};


typedef struct
{
	int num_moves;
	int num_found;
} Memory_state;

static Memory_state state = {0, 0};

static int memory_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static int memory_animate (Pos *pos, byte **movp);
static void memory_set_init_pos (Pos *pos);
void *memory_newstate (Pos *, byte *);
ResultType memory_who_won (Pos *, Player, char **);
void memory_free ();

void memory_init ()
{
	game_single_player = TRUE;
	game_set_init_pos = memory_set_init_pos;
	game_getmove = memory_getmove;
	game_animate = memory_animate;
	game_animation_time = 100;
	game_stateful = TRUE;
	game_state_size = sizeof (Memory_state);
	game_newstate = memory_newstate;
	game_who_won = memory_who_won;
	game_free = memory_free;
	game_draw_cell_boundaries = FALSE;
	game_allow_back_forw = FALSE;
	game_scorecmp = game_scorecmp_def_iscore;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Memory\n"
		"Single player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL("memory");
	game_doc_rules = 
		"There are 36 pairs of images. Your goal is to match these pairs in as few turns as possible.\n"
		"In each turn you click on two squares. If they match they stay uncovered for the rest of the game. If not they go back to being hidden.";
}

static int waiting = 0;

void memory_free ()
{
	waiting = 0;
}

void *memory_newstate (Pos *pos, byte *move)
{
	int i, j, found;
	state.num_moves = pos->state ? ((Memory_state *)pos->state)->num_moves : 0;
	state.num_found = pos->state ? ((Memory_state *)pos->state)->num_found : 0;
	if (move[2] <= MEMORY_NUM_PIECES/2)
		state.num_moves = pos->state ? ((Memory_state *)pos->state)->num_moves + 1 : 1;
	else
		for (i=0, found=0; i<board_wid; i++)
		{
			for (j=0; j<board_heit; j++)
			{
				if (i == move[0] && j == move[1])
					continue;
				if (pos->board [j * board_wid + i] == move[2])
				{
					state.num_found = pos->state ? ((Memory_state *)pos->state)->num_found + 1 : 1;
					found = 1;
					break;
				}
			}
			if (found) break;
		}
	return &state;
}

ResultType memory_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int found = pos->state ? ((Memory_state *)pos->state)->num_found : 0;
	snprintf (comment, 32, "%s %d;  %s %d", 
			"Missed:", pos->state ? ((Memory_state *)pos->state)->num_moves : 0,
			"Found:", found);
	*commp = comment;
	return (found == board_wid * board_heit / 2) ? RESULT_WON : RESULT_NOTYET;
}


int memory_animate (Pos *pos, byte **movp)
{
	static byte move[10];
	static int count = 0;
	byte *mp = move;
	int val;
	int i, j;
	int pair [MEMORY_BOARD_HEIT * MEMORY_BOARD_WID/2+1];
	if (waiting > 0) waiting ++;
	if (waiting == 10)
	{
		waiting = 0;
		for (i=0; i<=MEMORY_BOARD_WID * MEMORY_BOARD_HEIT/2; i++)
			pair [i] = 0;
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (MEMORY_ISOPEN (val = pos->board [j * board_wid + i]))
				pair [MEMORY_FLIP (val)] ++;
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
		{
			if (MEMORY_ISOPEN (val = pos->board [j * board_wid + i])
					&& pair [MEMORY_FLIP (val)] == 1)
			{
				*mp++ = i;
				*mp++ = j;
				*mp++ = MEMORY_FLIP(val);
			}
		}
	}
	else return -1;
	*mp++ = -1;
	*movp = move;
	return 1;
}

void memory_set_init_pos (Pos *pos)
{
	int i, j, tmp;
	int size = board_wid * board_heit;
	int swaps = 0;
	byte *board = pos->board;
	for (i=0; i<size; i++)
		board[i] = i/2+1;
	for (i=1; i<size; i++)
	{
		j = random() % i;
		tmp = board[i]; board[i] = board[j]; board[j] = tmp;
	}
}

int memory_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int **rmovp)
{
	static byte move[16];
	byte *mp = move;
	int val, num;
	int i, j;
	gboolean pair = FALSE;
	int found = 0;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (waiting > 0) return 0;
	if ((val = pos->board [y * board_wid + x]) == MEMORY_EMPTY) return 0;
	if (MEMORY_ISOPEN (val)) return 0;
	*mp++ = x; *mp++ = y;
	num = board_wid * board_heit / 2;
	if (val <= num) val += num; else val -= num;
	*mp++ = val;
	*mp++ = -1;
	*movp = move;
	
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (x == i && y == j) continue;
		if (MEMORY_ISOPEN(pos->board [j * board_wid + i]))
		{
			found++;
			if (pos->board [j * board_wid + i] == val) pair = TRUE;
		}
	}
	if (!pair && (found %2 == 1)) waiting = 1;
	return 1;
}

