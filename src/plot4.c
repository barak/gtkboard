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

/** \file plot4.c */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "game.h"
#include "aaball.h"

#define PLOT4_CELL_SIZE 55
#define PLOT4_NUM_PIECES 3

#define PLOT4_BOARD_WID 7
#define PLOT4_BOARD_HEIT 6

char plot4_colors[6] = {160, 140, 100, 160, 140, 100};

int * plot4_init_pos = NULL;

#define PLOT4_WP 1
#define PLOT4_BP 2
#define PLOT4_EMPTY 3


void plot4_init ();

Game Plot4 = { PLOT4_CELL_SIZE, PLOT4_BOARD_WID, PLOT4_BOARD_HEIT, 
	PLOT4_NUM_PIECES,
	plot4_colors,  NULL, NULL, "Plot 4", "k-in-a-row", plot4_init};



static int eval_runs (Pos *, int);
static int find_runs (byte *, int, int, int , int, int, int);
static int plot4_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType plot4_who_won (Pos *, Player , char **);
static void plot4_set_init_pos (Pos *pos);
static char ** plot4_get_pixmap (int, int);
static byte * plot4_movegen (Pos *);
static ResultType plot4_eval (Pos *, Player, float *);


static const int RUN_WT = 20;


void plot4_init ()
{
	game_eval = plot4_eval;
	game_movegen = plot4_movegen;
	game_getmove = plot4_getmove;
	game_who_won = plot4_who_won;
	game_set_init_pos = plot4_set_init_pos;
	game_get_pixmap = plot4_get_pixmap;
	game_white_string = "Green";
	game_black_string = "Yellow";
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Plot4\n"
		"Two player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL ("plot4");
	game_doc_rules = 
		"Two players alternate in placing balls of either color on a 7x6 board. Not exactly placing, because the balls have gravity and fall down to the lowest unoccupied square on the column. The goal is to get as many 4-in-a-row's as possible. A 5-in-a-row counts as two, 6 as 3, and 7 as 4.\n";
}

void plot4_set_init_pos (Pos *pos)
{
	int i;
	for (i=0; i<board_wid * board_heit; i++)
		pos->board [i] = PLOT4_EMPTY;
}

ResultType plot4_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	char *who_str [3] = { "Green won", "Yellow won", "Its a tie" };
	int i, wscore, bscore, who_idx;
	wscore = eval_runs (pos, WHITE) / RUN_WT;
	bscore = eval_runs (pos, BLACK) / RUN_WT * -1;
	*commp = comment;
	for (i=0; i<board_wid * board_heit; i++)
		if (pos->board[i] == PLOT4_EMPTY) 
		{
			snprintf (comment, 32, "%d : %d", wscore, bscore);
			return RESULT_NOTYET;
		}
	if (wscore > bscore) who_idx = 0;
	else if (wscore < bscore) who_idx = 1;
	else who_idx = 2;
	snprintf (comment, 32, "%s (%d : %d)", who_str[who_idx], wscore, bscore);
	if (wscore > bscore)
		return RESULT_WHITE;
	if (wscore < bscore)
		return RESULT_BLACK;
	return RESULT_TIE;
}

int plot4_islegal (byte *board, int x, int y)
	/* check bounds
	   check if (x,y) is empty
	   check if (x, y-1) is not empty */
{
	return (x >= 0 && x < board_wid && y >= 0 && y < board_heit
			&& (board[y * board_wid + x] == PLOT4_EMPTY) && 
			((y == 0) || (board [(y-1) * board_wid + x] != PLOT4_EMPTY)));
}

int plot4_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
	/* translate a sequence of mouse clicks into a cbgf move.
	   pos is the current position, x and y are the square which was
	   clicked, type is the event type: MOUSE_PRESSED, MOUSE_RELEASED
	   to_play is who has the move, movp is used to return the move

	   the return value is 1 if the clicks were successfully translated
	   into a move; -1 if the move is illegal, and 0 if further clicks
	   are required to determine the move. In the latter case, this 
	   function is responsible for keeping track of the current state.
	 */
{
	static byte move[4];
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	/* OK to click anywhere in the column */
	for (y = 0; y < board_heit; y++)
		if (pos->board[y * board_wid + x] == PLOT4_EMPTY)
			break;
	if (y == board_heit)
		return -1;
	move[0] = x;
	move[1] = y;
	move[2] = to_play == WHITE ? PLOT4_WP : PLOT4_BP;
	move[3] = -1;
	if (movp)
		*movp = move;	
	return 1;
}

ResultType plot4_eval (Pos *pos, Player to_play, float *eval)
	/* always eval from POV of white */
{
	/* TODO : add some weights */
	*eval = eval_runs (pos, -1);
	return RESULT_NOTYET;
}



static int eval_runs (Pos *pos, int forwhom)
	/* sum of all the runs of forwhom (both if -1) from POV of WHITE */
{
	int sum = 0;
	int i, board_min, min;
	byte *board = pos->board;
	board_min = board_wid < board_heit ? board_wid : board_heit;
	for (i=0; i<board_wid; i++)
		sum += find_runs (board, i, 0, 0, 1, board_heit, forwhom);
	for (i=0; i<board_heit; i++)
		sum += find_runs (board, 0, i, 1, 0, board_wid, forwhom);
	for (i=0; i<board_wid; i++)
	{
		min = (board_wid - i) < board_min ? (board_wid - i) : board_min;
		sum += find_runs (board, i, 0, 1, 1, min, forwhom);
		sum += find_runs (board, board_wid - i - 1, 0, -1, 1, min, forwhom);
	}
	for (i=1; i<board_heit; i++)
	{
		min = (board_heit - i) < board_min ? (board_heit - i) : board_min;
		sum += find_runs (board, 0, i, 1, 1, min, forwhom);
		sum += find_runs (board, board_wid - 1, i, -1, 1, min, forwhom);
	}
	return sum;
}

static int find_runs (byte *board, int x0, int y0, int dx, int dy, int len,
		int forwhom)
	/* OK this function is very ugly. if forwhom is -1 it returns an
	* eval of the given line. If not it returns a count forwhom's 
	* score on that line, multiplied by RUN_WT */
{
	int sum = 0, i, cur = 0; /* cur = current run length */
	int x = x0, y = y0;
	char prev = -1, cell;
	int lopen = 0, ropen = 0; /* can we extend this run in either direction*/
	for (i=0; i<len; i++, x += dx, y += dy, prev = cell)
	{
		cell = board[y * board_wid + x];
		if (cell == PLOT4_EMPTY)
		{
			cur=0;
			lopen = 1;
			continue;
		}
		if (cell != prev)
			cur = 0;
		cur ++;
		if (i < len - 1 && 
				board [(y + dy) * board_wid + (x + dx)] == PLOT4_EMPTY)
			ropen = 1;
		else ropen = 0;
		if (cell == PLOT4_WP)
		{
			if (forwhom == -1)
				sum += ((ropen + lopen) * (cur * cur));
			if (forwhom != BLACK && cur >= 4)
				sum += RUN_WT;
		}
		if (cell == PLOT4_BP)
		{
			if (forwhom == -1)
				sum -= ((ropen + lopen) * (cur * cur));
			if (forwhom != WHITE && cur >= 4)
				sum -= RUN_WT;
		}
		lopen = 0;
	}
	return sum;
}

byte * plot4_movegen_single (char *pos, int player, int reset)
	/* return a move terminated by -1 */
	/* NULL ==> no more moves */
{
	static int row;
	static byte movbuf[4];
	if (reset)
		row = 0;
	while (row < board_wid)
	{
		int j;
		for (j=board_heit-1; j>=0; j--)
			if (pos[j * board_wid + row] == PLOT4_EMPTY)
			{
				movbuf[0] = row;
				movbuf[1] = j;
				movbuf[2] = (player == WHITE ? PLOT4_WP : PLOT4_BP);
				movbuf[3] = -1;
				row++;
				return movbuf;
			}
		row++;
	}
	/*movbuf[0] = -1;*/
	row = 0;
	return NULL;
}

//! movegen function
byte *plot4_movegen (Pos *pos)
{
	byte movbuf[256];
	byte *movp = movbuf;
	byte *movlist;
	int i, j;
	for (i=0; i<board_wid; i++)
	{
		for (j=0; j<board_heit; j++)
			if (pos->board[j * board_wid + i] == PLOT4_EMPTY)
			{
				*movp++ = i;
				*movp++ = j;
				*movp++ = (pos->player == WHITE ? PLOT4_WP : PLOT4_BP);
				*movp++ = -1;
				break;
			}
	}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}

char ** plot4_get_pixmap (int idx, int color)
{
	int fg, bg, i;
	char *colors = plot4_colors;
	static char pixbuf[PLOT4_CELL_SIZE*(PLOT4_CELL_SIZE)+1];
	if (idx == PLOT4_WP) fg = 0xee << 8;
	else if (idx == PLOT4_BP) fg = (0xee << 16) + (0xee << 8);
	else fg = 0xd7d7d7;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	return pixmap_ball_gen(55, pixbuf, fg, bg, 17.0, 30.0);
}

