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

#define STOPGATE_CELL_SIZE 40
#define STOPGATE_NUM_PIECES 4

#define STOPGATE_BOARD_WID 9
#define STOPGATE_BOARD_HEIT 9

char stopgate_colors[6] = {180, 200, 180, 200, 140, 140};

static char * blue_gate_north_40_xpm [] =
{
"40 40 2 1",
"  c none",
". c #0000ff",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
};

static char * blue_gate_south_40_xpm [] =
{
"40 40 2 1",
"  c none",
". c #0000ff",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
"              ............              ",
};

static char * blue_gate_east_40_xpm [] =
{
"40 40 2 1",
"  c none",
". c #0000ff",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"              ..........................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
};

static char * blue_gate_west_40_xpm [] =
{
"40 40 2 1",
"  c none",
". c #0000ff",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"..........................              ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
};


char ** stopgate_pixmaps [] = 
{
	blue_gate_north_40_xpm,
	blue_gate_south_40_xpm,
	blue_gate_east_40_xpm,
	blue_gate_west_40_xpm,
};


#define STOPGATE_NORTH 1
#define STOPGATE_SOUTH 2
#define STOPGATE_EAST  3
#define STOPGATE_WEST  4
#define STOPGATE_EMPTY 0

#define abs(x) ((x) < 0 ? -(x) : (x))

int stopgate_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
void stopgate_init ();
ResultType stopgate_who_won (Pos *, Player, char **);
ResultType stopgate_eval (Pos *, Player, float *eval);
byte * stopgate_movegen (Pos *);

Game Stopgate = { STOPGATE_CELL_SIZE, STOPGATE_BOARD_WID, STOPGATE_BOARD_HEIT, 
	STOPGATE_NUM_PIECES, 
	stopgate_colors, NULL, stopgate_pixmaps, "Stopgate", "Nimlike games", stopgate_init};

static int stopgate_curx = - 1, stopgate_cury = -1;


void stopgate_init ()
{
	game_getmove = stopgate_getmove;
	game_who_won = stopgate_who_won;
	game_eval = stopgate_eval;
	game_movegen = stopgate_movegen;
	game_white_string = "Vertical";
	game_black_string = "Horizontally";
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_rules = "Two players take turns in placing dominoes on the board. The first player places them vertically and the second horizontally. To place a domino, press the mouse button on a square, drag the mouse to the adjacent square and release the mouse button. The goal is to be the last player to place a domino; the game ends when it becomes obvious who the winner is.";
	game_doc_strategy = "Make parallel columns of dominoes such that you will be able to play in between the columns but the opponent won't.";
}

static int incx[] = { -1, -1, -1, 0, 0, 1, 1, 1};
static int incy[] = { -1, 0, 1, -1, 1, -1, 0, 1};

ResultType stopgate_who_won (Pos *pos, Player player, char ** commp)
{
	char *who_str[2] = {"Vertical won", "Horizontal won"};
	int wscore, bscore, i, j;
	float eval;
	ResultType result;
	result = stopgate_eval (pos, player, &eval);
	if (result == RESULT_WHITE) *commp = who_str[0];
	if (result == RESULT_BLACK) *commp = who_str[1];
	return result;
}

#define EVAL_ISEMPTY(x, y) ((ISINBOARD((x), (y))) && (board[(y) * board_wid + (x)] == STOPGATE_EMPTY))

#define EVAL_OPENSQUARE(x, y) (EVAL_ISEMPTY ((x), (y)) && (EVAL_ISEMPTY ((x)-1,(y)) || EVAL_ISEMPTY ((x)+1, (y))) && (EVAL_ISEMPTY ((x),(y)-1) || EVAL_ISEMPTY ((x), (y)+1)))

enum {
	REGION_WHITE = STOPGATE_NUM_PIECES + 1, 
	REGION_BLACK, 
	REGION_OPEN_X = 1 << 4,
	REGION_OPEN_Y = 1 << 5,
};



static int regions[STOPGATE_BOARD_WID * STOPGATE_BOARD_HEIT];

/* Find which regions are open, which are vertical (REGION_WHITE) and which
   are horizontal (REGION_BLACK)
   clever algo that avoids a queue: every square in an open region must
   be on a straight line from a square which has unfilled nbrs in both directions
   */
static void find_regions (byte *board)
{
	int i, j;
	static int count = 0;
	for (i=0; i<board_wid*board_heit; i++)
		regions[i] = 0;
	for (i=0; i < board_wid; i++)
	for (j=0; j < board_heit; j++)
	{
		int x;
		if (regions [j * board_wid + i] & REGION_OPEN_X)
			continue;
		if (EVAL_OPENSQUARE (i, j))
		{
			for (x=i; EVAL_ISEMPTY (x, j); x++)
				regions [j * board_wid + x] |=  REGION_OPEN_X;
			for (x=i; EVAL_ISEMPTY (x, j); x--)
				regions [j * board_wid + x] |=  REGION_OPEN_X;
		}
	}
	for (i=0; i < board_wid; i++)
	for (j=0; j < board_heit; j++)
	{
		int y;
		if (regions [j * board_wid + i] & REGION_OPEN_Y)
			continue;
		if (EVAL_OPENSQUARE (i, j))
		{
			for (y=j; EVAL_ISEMPTY (i, y); y++)
				regions [y * board_wid + i] |=  REGION_OPEN_Y;
			for (y=j; EVAL_ISEMPTY (i, y); y--)
				regions [y * board_wid + i] |=  REGION_OPEN_Y;
		}
	}
	for (i=0; i < board_wid; i++)
	for (j=0; j < board_heit; j++)
		if (board [j * board_wid + i] == 0 && regions[j * board_wid + i] == 0)
		{
			if (EVAL_ISEMPTY (i,j+1) || EVAL_ISEMPTY (i,j-1))
				regions[j * board_wid + i] = REGION_WHITE;
			else if (EVAL_ISEMPTY (i+1,j) || EVAL_ISEMPTY (i-1,j))
				regions[j * board_wid + i] = REGION_BLACK;
		}
	// TODO: find the lengths of the runs also
}


byte * stopgate_movegen (Pos *pos)
{
	int i, j;
	byte movbuf [512], *movp = movbuf, *movlist;
	byte *board = pos->board;
	Player player = pos->player;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		int incx = (player == WHITE ? 0 : 1), incy = (player == WHITE ? 1 : 0);
		if (board [j * board_wid + i] != STOPGATE_EMPTY) continue;
		if (!ISINBOARD (i + incx, j + incy)) continue;
		if (pos->board [(j + incy) * board_wid + (i + incx)] != STOPGATE_EMPTY) continue;
		if (!EVAL_OPENSQUARE (i, j) && !EVAL_OPENSQUARE (i + incx, j + incy))
			continue;
		*movp++ = i;
		*movp++ = j;
		*movp++ = (player == WHITE ? STOPGATE_NORTH : STOPGATE_EAST);
		*movp++ = i + incx;
		*movp++ = j + incy;
		*movp++ = (player == WHITE ? STOPGATE_SOUTH : STOPGATE_WEST);
		*movp++ = -1;
	}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}


ResultType stopgate_eval (Pos *pos, Player player, float *eval)
{
	int i, j;
	float val = 0;
	gboolean wfound = FALSE, bfound = FALSE, openfound = FALSE;
	int run_len=0;
	int run_eval_w = 0, run_eval_b = 0;
	byte *board = pos->board;
	find_regions (board);
	for (i=0; i<board_wid; i++)
	{
		for (j=0; j<board_heit; j++)
			if (regions[j * board_wid + i] == REGION_WHITE)
				run_len++;
			else
			{
				run_eval_w += 2 * (run_len / 2);
				run_len = 0;
			}
		run_eval_w += 2 * (run_len / 2);
		run_len = 0;
	}
	for (j=0; j<board_heit; j++)
	{
		for (i=0; i<board_wid; i++)
			if (regions[j * board_wid + i] == REGION_BLACK)
				run_len++;
			else
			{
				run_eval_b += 2 * (run_len / 2);
				run_len = 0;
			}
		run_eval_b += 2 * (run_len / 2);
		run_len = 0;
	}
	val = run_eval_w - run_eval_b;
/*	if (++count == 5000)
	{
		count = 0;
		val = run_eval_w - run_eval_b;
		printf ("%d, %d\n", run_eval_w, run_eval_b);
		for (j=board_heit-1; j>=0; j--)
		{
			for (i=0; i<board_wid; i++)
			{
				if (board [j * board_wid + i])
					printf (" #");
				else if (regions[j * board_wid + i] == REGION_WHITE)
					printf (" W");
				else if (regions[j * board_wid + i] == REGION_BLACK)
					printf (" B");
				else printf (" _");
			}
			printf ("\n");
		}
		printf ("\n");
	}	
*/
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (board [j * board_wid + i] != STOPGATE_EMPTY) continue;
		if (regions[j * board_wid + i] == REGION_WHITE)
			wfound = TRUE;
		else if (regions[j * board_wid + i] == REGION_BLACK)
			bfound = TRUE;
		else
		{
		   	if (EVAL_ISEMPTY (i, j+1) || EVAL_ISEMPTY (i, j-1))
			{
				val++;
				wfound = TRUE;
				openfound = TRUE;
			}
			if (EVAL_ISEMPTY (i+1, j) || EVAL_ISEMPTY (i-1, j))
			{
				val--;
				bfound = TRUE;
				openfound = TRUE;
			}
		}
	}
	if (!openfound)
	{
		val += (player == WHITE ? -1 : 1);
		*eval = val;
		return val > 0 ? RESULT_WHITE : RESULT_BLACK;
	}
	if (player == WHITE && !wfound)
	{
		*eval = (val-1);
		return RESULT_BLACK;
	}
	if (player == BLACK && !bfound)
	{
		*eval = (val+1);
		return RESULT_WHITE;
	}
	*eval =  val + 0.01 * random () / RAND_MAX;
	return RESULT_NOTYET;
}

int stopgate_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int **rmovp)
{
	int i, j, sw_len, found=0;
	static byte move[128];
	byte *mp = move;
	int diffx, diffy, dir1 = -1, dir2 = -1;
	if (type == GTKBOARD_BUTTON_PRESS)
	{
		if (pos->board [y * board_wid + x] != STOPGATE_EMPTY)
			return -1;
		stopgate_curx = x;
		stopgate_cury = y;
		return 0;
	}
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (stopgate_curx < 0) return -1;
	diffx = x - stopgate_curx;
	diffy = y - stopgate_cury;
	if (to_play == WHITE && diffx == 0 && diffy == 1)
		dir1 = STOPGATE_NORTH, dir2 = STOPGATE_SOUTH;
	else if (to_play == WHITE && diffx == 0 && diffy == -1)
		dir1 = STOPGATE_SOUTH, dir2 = STOPGATE_NORTH;
	else if (to_play == BLACK && diffx == 1 && diffy == 0)
		dir1 = STOPGATE_EAST, dir2 = STOPGATE_WEST;
	else if (to_play == BLACK && diffx == -1 && diffy == 0)
		dir1 = STOPGATE_WEST, dir2 = STOPGATE_EAST;
	else
	{
		stopgate_curx = stopgate_cury = -1;
		return -1;
	}
	if (pos->board [y * board_wid + x] != STOPGATE_EMPTY)
	{
		stopgate_curx = stopgate_cury = -1;
		return -1;
	}
	*mp++ = stopgate_curx;
	*mp++ = stopgate_cury;
	*mp++ = dir1;
	*mp++ = x;
	*mp++ = y;
	*mp++ = dir2;
	*mp++ = -1;
	*movp = move;
	return 1;
}



