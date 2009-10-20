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
#include "../pixmaps/chess.xpm"
#include "../pixmaps/misc.xpm"

#define EIGHTQUEENS_CELL_SIZE 54
#define EIGHTQUEENS_NUM_PIECES 2

#define EIGHTQUEENS_BOARD_WID 8
#define EIGHTQUEENS_BOARD_HEIT 8

#define EIGHTQUEENS_EMPTY 0
#define EIGHTQUEENS_QUEEN 1
#define EIGHTQUEENS_CONTROLLED 2

#define abs(x) ((x) < 0 ? -(x) : (x))
#define ATTACKS(i, j, x, y) ((i)==(x) || (j)==(y) || abs((i)-(x)) == abs((j)-(y)))

char eightqueens_colors[6] = {200, 200, 160, 200, 200, 160};

void eightqueens_init ();

char ** eightqueens_pixmaps [] = 
{
	chess_wq_54_xpm,
	grey_square_54_xpm,
};

Game Eightqueens = { EIGHTQUEENS_CELL_SIZE, 
	EIGHTQUEENS_BOARD_WID, EIGHTQUEENS_BOARD_HEIT, 
	EIGHTQUEENS_NUM_PIECES, 
	eightqueens_colors, NULL, eightqueens_pixmaps, "Eight Queens Puzzle", 
	"Logic puzzles",
	eightqueens_init};

SCORE_FIELD eightqueens_score_fields[] = {SCORE_FIELD_RANK, SCORE_FIELD_USER, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *eightqueens_score_field_names[] = {"Rank", "User", "Time", "Date", NULL};


static int eightqueens_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType eightqueens_who_won (Pos *, Player, char **);

void eightqueens_init ()
{
	game_single_player = 1;
	game_getmove = eightqueens_getmove;
	game_who_won = eightqueens_who_won;
	game_scorecmp = game_scorecmp_def_time;
	game_score_fields =  eightqueens_score_fields;
	game_score_field_names = eightqueens_score_field_names;
	game_draw_cell_boundaries = TRUE;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Eightqueens\n"
		"Single player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL("eightqueens");
	game_doc_rules = 
		"Place 8 non-attacking queens on the chessboard";
		;
}

ResultType eightqueens_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i, qcount;
	for (i=0, qcount = 0; i<board_wid*board_heit; i++)
		qcount += (pos->board [i] == EIGHTQUEENS_QUEEN ? 1 : 0);
	snprintf (*commp = comment, 32, "Queens: %d", qcount);
	return qcount == 8 ? RESULT_WON : RESULT_NOTYET;
}

static int num_attacks (byte *board, int x, int y)
{
	int i, j, count = 0;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (!(i==x && j==y) && board [j * board_wid + i] == EIGHTQUEENS_QUEEN)
			 count += (ATTACKS (i, j, x, y) ? 1 : 0);
	return count;
}

int eightqueens_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player player, byte **movp, int ** rmovep)
{
	static byte move[256];
	int i, j;
	byte *mp = move;
	*movp = move;
	if (type != GTKBOARD_BUTTON_RELEASE) return 0;
	if (pos->board[y * board_wid + x] == EIGHTQUEENS_CONTROLLED) return -1;
	if (pos->board[y * board_wid + x] == EIGHTQUEENS_QUEEN)
	{
		*mp++ = x, *mp++ = y, *mp++ = EIGHTQUEENS_EMPTY;
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (pos->board[j * board_wid + i] == EIGHTQUEENS_CONTROLLED
					&& ATTACKS(i, j, x, y)
					&& num_attacks (pos->board, i, j) == 1)
				*mp++ = i, *mp++ = j, *mp++ = EIGHTQUEENS_EMPTY;
		*mp++ = -1;
		return 1;
	}
	*mp++ = x, *mp++ = y, *mp++ = EIGHTQUEENS_QUEEN;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (!(i==x && j==y) && pos->board[j * board_wid + i] == EIGHTQUEENS_EMPTY 
				&& ATTACKS (i, j, x, y))
			*mp++ = i, *mp++ = j, *mp++ = EIGHTQUEENS_CONTROLLED;
	*mp++ = -1;
	return 1;
}

