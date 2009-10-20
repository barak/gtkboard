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

#define SAMEGAME_NUM_ANIM 8

#define SAMEGAME_CELL_SIZE 40
#define SAMEGAME_NUM_PIECES (3*SAMEGAME_NUM_ANIM)

#define SAMEGAME_BOARD_WID 15
#define SAMEGAME_BOARD_HEIT 10

#define SAMEGAME_EMPTY 0
#define SAMEGAME_RP 1
#define SAMEGAME_BP 2
#define SAMEGAME_GP 3

char samegame_colors[6] = {50, 50, 50, 50, 50, 50};

void samegame_init ();

Game Samegame = { SAMEGAME_CELL_SIZE, SAMEGAME_BOARD_WID, SAMEGAME_BOARD_HEIT, 
	SAMEGAME_NUM_PIECES, 
	samegame_colors, NULL, /*samegame_pixmaps*/ NULL, 
	"Samegame", "Arcade",
	samegame_init};

typedef struct
{
	int score;
} Samegame_state;

SCORE_FIELD samegame_score_fields[] = {SCORE_FIELD_RANK, SCORE_FIELD_USER, SCORE_FIELD_SCORE, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *samegame_score_field_names[] = {"Rank", "User", "Score", "Time", "Date", NULL};

static void recursive_delete (byte *board, int x, int y, int val);
static void pull_down (byte *board);
static byte * synth_move (byte *newboard, byte *board);

static int samegame_animate (Pos *pos, byte **movp);
static int samegame_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static char **samegame_get_pixmap (int , int);
static void *samegame_newstate (Pos *, byte *);
static ResultType samegame_who_won (Pos *, Player, char **);
static void samegame_search (Pos *pos, byte **movp);
static void samegame_getxy (byte *board, int *x, int *y);

static int anim_curx=-1, anim_cury=-1;

void samegame_set_init_pos (Pos *pos)
{
	int i;
	for (i=0; i<SAMEGAME_BOARD_WID * SAMEGAME_BOARD_HEIT; i++)
		pos->board[i] = ((random () % 3) + 1) * SAMEGAME_NUM_ANIM;
}

void samegame_init ()
{
	game_single_player = 1;
	game_getmove = samegame_getmove;
	game_set_init_pos = samegame_set_init_pos;
	game_get_pixmap = samegame_get_pixmap;
	game_search = samegame_search;
	game_animate = samegame_animate;
	game_animation_time = 80;
	game_animation_use_movstack = FALSE;
	game_stateful = TRUE;
	game_state_size = sizeof (Samegame_state);
	game_newstate = samegame_newstate;
	game_who_won = samegame_who_won;
	game_scorecmp = game_scorecmp_def_dscore;
	game_score_fields =  samegame_score_fields;
	game_score_field_names = samegame_score_field_names;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Samegame\n"
		"Single player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL("samegame");
	game_doc_rules = 
		"  * Two or more balls of the same color that form a connected region constitute a block.\n"
		"  * Hovering the mouse over a ball will highlight the block that the ball belongs to.\n" 
		"  * Clicking on a ball removes that ball's block.\n"
		"  * You get (n-2)^2 points for removing a block with n balls in it.\n" 
		"  * You get a 1000 point bonus if you remove all the balls.\n"
		"  * The goal is to maximize your score.\n"
		"  * As balls are removed, balls above fall down to take their place. If a column is removed, the column to its right moves to the left.\n"	;
	game_doc_strategy = 
		"  * Try to identify the color which occurs the most number of times.\n"
		"  * Remove balls of the other 2 colors until all the balls of the most frequent color (or as many of them as possible) form a single block.\n"
		"  * Remove this block. Now do the same with the remaining two colors.\n"
		"  * Avoid getting into a situation in which there is a single ball of some color. If this happens you won't be able to clear the entire board.\n"
		;
}


int samegame_over (byte *board)
{
	int i, j, k;
	int incx[2] = {1, 0};
	int incy[2] = {0, 1};
	for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			for (k=0; k<2; k++)
			{
				int x = i + incx[k], y = j + incy[k];
				if (board [j * board_wid + i] == 0) continue;
				if (x < 0 || x > board_wid - 1 || y < 0 || y > board_heit - 1)
					continue;
				if (board [y * board_wid + x] == board [j * board_wid + i])
					return FALSE;
			}
	return TRUE;
}

ResultType samegame_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	gboolean over = samegame_over (pos->board);
	char *scorestr = over ? "Game over. Score:" : "Score:";
	snprintf (comment, 32, "%s %d", scorestr,
			pos->state ? ((Samegame_state *)pos->state)->score : 0);
	*commp = comment;
	return over ? RESULT_WON : RESULT_NOTYET;
}


static Samegame_state state = {0};

void *samegame_newstate (Pos *pos, byte *move)
{
	int i, score = 0;
	gboolean bonus = FALSE;
	for (i=0; move[3*i] >= 0; i++)
	{
		if (move[3*i] == 0 && move[3*i + 1] == 0 && move[3*i + 2] == 0)
			bonus = TRUE;
		if (move[3*i+2] == 0)
			score++;
		else if (pos->board[move[3*i+1] * board_wid + move[3*i]] == 0)
			score--;
	}
	score -= 2; score *= score;
	if (score < 0) score = 0;
	state.score = (pos->state ? ((Samegame_state *)pos->state)->score : 0) + score;
	if (bonus)
		state.score += 1000;
	return &state;
}

static byte animgen_buf [SAMEGAME_BOARD_WID * SAMEGAME_BOARD_HEIT];

static void recursive_animgen (byte *board, int x, int y, int val, int newval, byte **mp)
{
	int tmp;
	if (x < 0 || y < 0 || x >= board_wid || y >= board_heit)
		return;
	if (animgen_buf[y * board_wid + x] != 0)
		return;
	if (board [y * board_wid + x] == 0) return;
	if ((board [y * board_wid + x]-1)/SAMEGAME_NUM_ANIM != val)
		return;
	animgen_buf [y * board_wid + x] = 1;
	**mp = x; (*mp)++; **mp = y; (*mp)++;
/*	tmp = board[y * board_wid + x];
	if (tmp % SAMEGAME_NUM_ANIM == 0)
		tmp -= SAMEGAME_NUM_ANIM;
	g_assert (tmp >= 0);*/
	**mp = newval; (*mp)++;
	recursive_animgen (board, x - 1, y   , val, newval, mp);
	recursive_animgen (board, x + 1, y   , val, newval, mp);
	recursive_animgen (board, x    , y - 1, val, newval, mp);
	recursive_animgen (board, x    , y + 1, val, newval, mp);
}

int samegame_animate (Pos *pos, byte **movp)
{
	static byte move[1024];
	static int animdir = 1;
	int i, j;
	byte *mp = move;
	byte *board = pos->board;
	memset (animgen_buf, 0, SAMEGAME_BOARD_WID * SAMEGAME_BOARD_HEIT);
	if (anim_curx >= 0 && anim_cury >= 0 
			&& board [anim_cury * board_wid + anim_curx] != 0)
	{
		int val = board[anim_cury * board_wid + anim_curx];
		if (val % SAMEGAME_NUM_ANIM == 0) animdir = -1;
		if (val % SAMEGAME_NUM_ANIM == 1) animdir = 1;
		recursive_animgen (board, anim_curx, anim_cury, 
			(val - 1) / SAMEGAME_NUM_ANIM, val + animdir, &mp);
		if (mp - move <= 3)
			mp = move;
	}
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (animgen_buf[j * board_wid + i] == 0 
				&& board [j * board_wid + i] % SAMEGAME_NUM_ANIM != 0)
		{
			// don't let any balls settle at less than max intensity
			// after they have finished animating
			int newval = board [j * board_wid + i] + SAMEGAME_NUM_ANIM - 
				board [j * board_wid + i] % SAMEGAME_NUM_ANIM;
			*mp++ = i;
			*mp++ = j;
			*mp++ = newval;
		}
	}
	*mp = -1;
	if (mp - move < 3) return -1;
	*movp = move;
	return 1;
}

static int getmove_real (Pos *pos, int x, int y, byte **movp)
{
	byte *newboard, val;
	if ((val = pos->board[y * board_wid + x]) == 0)	
		return -1;
	/* do we have at least 1 neighbor */
	do
	{
		if (x > 0 && val == pos->board[y * board_wid + x - 1])
			break;
		if (x < board_wid -1 && val == pos->board[y * board_wid + x + 1 ])
			break;
		if (y > 0 && val == pos->board[(y-1) * board_wid + x])
			break;
		if (y < board_heit && val == pos->board[(y+1) * board_wid + x])
			break;
		return -1;
	} while (0);
		
	newboard = (byte *) malloc (board_wid * board_heit);
	assert (newboard);
	memcpy (newboard, pos->board, board_wid * board_heit);
	recursive_delete (newboard, x, y, newboard[y * board_wid + x]);
	pull_down (newboard);
	if (movp)
		*movp = synth_move (newboard, pos->board);
	free (newboard);
	
	return 1;
}

int samegame_getmove 
	(Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
{
	if (type == GTKBOARD_MOTION_NOTIFY)
	{
		anim_curx = x;
		anim_cury = y;
	}
	if (type == GTKBOARD_LEAVE_NOTIFY)
	{
		anim_curx = -1;
		anim_cury = -1;
	}
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;

	return getmove_real (pos, x, y, movp);
}

static void recursive_delete (byte *board, int x, int y, int val)
{
	if (x < 0 || y < 0 || x >= board_wid || y >= board_heit)
		return;
	if (board[y * board_wid + x] != val)
		return;
	board[y * board_wid + x] = 0;
	recursive_delete (board, x - 1, y   , val);
	recursive_delete (board, x + 1, y   , val);
	recursive_delete (board, x    , y - 1, val);
	recursive_delete (board, x    , y + 1, val);
}

static void pull_down (byte *board)
{
	int i, j, k, cnt;
	byte *tempcol;
	tempcol = (byte *) malloc (board_heit);
	assert (tempcol);

	/* for each column apply gravity */
	for (i=0; i<board_wid; i++)
	{
		for (j=0, k=0; j<board_heit; j++)
		{
			if (board[j * board_wid + i])
				tempcol[k++] = board [j * board_wid + i];
		}
		for (; k<board_heit; k++)
			tempcol[k] = 0;
		for (j=0; j<board_heit; j++)
			board[j * board_wid + i] = tempcol [j];
	}

	for (cnt = 0; cnt < board_wid; cnt++) // ugly, but works
	{
		/* if a column is empty move the col to its right in its place */
		for (i=0; i<board_wid - 1; i++)
		{
			for (j=0; j<board_heit; j++)
				if (board[j * board_wid + i])
					break;
			if (j == board_heit)	/* col is empty */
				for (j=0; j<board_heit; j++)
				{
					board [j * board_wid + i]	= board [j * board_wid + i + 1];
					board [j * board_wid + i + 1] = 0;
				}
		}
	}
	free (tempcol);
}

static byte * synth_move (byte *newboard, byte *board)
{
	static byte movbuf [1024];
	int i, j, m = 0;
	for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (newboard [j * board_wid + i] != board [j * board_wid + i])
			{
				movbuf[m++] = i;
				movbuf[m++] = j;
				movbuf[m++] = newboard [j * board_wid + i];
			}
	movbuf[m] = -1;
	return movbuf;
}

char ** samegame_get_pixmap (int idx, int color)
{
	int bg, fg = 0, i, tmp;
	static char pixbuf[SAMEGAME_CELL_SIZE*(SAMEGAME_CELL_SIZE+1)];
	static gboolean first = TRUE;
	for(i=0, bg=0;i<3;i++) 
	{ int col = samegame_colors[i]; 
		if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	idx--;
	tmp = idx%SAMEGAME_NUM_ANIM;
	if (tmp == SAMEGAME_NUM_ANIM-1) fg = 255;
	else
		fg = 256 * (SAMEGAME_NUM_ANIM-1 + tmp) / (2*(SAMEGAME_NUM_ANIM-1));
	switch (idx/SAMEGAME_NUM_ANIM + 1)
	{
		case SAMEGAME_RP: fg *= 0x010000; break;
		case SAMEGAME_GP: fg *= 0x000100; break;
		case SAMEGAME_BP: fg *= 0x000001; break;
	}
	if (first)
	{
		first = FALSE;
		return pixmap_ball_gen (SAMEGAME_CELL_SIZE, pixbuf, fg, bg, 
			SAMEGAME_CELL_SIZE * 2/7, 24.0);
	}
	return pixmap_header_gen (SAMEGAME_CELL_SIZE, pixbuf, fg, bg);
}

void samegame_search (Pos *pos, byte **movp)
{
	int x, y;
	int retval;
	samegame_getxy (pos->board, &x, &y);
	retval = getmove_real (pos, x, y, movp);
	assert (retval > 0);
}

void samegame_getxy (byte *board, int *x, int *y)
{
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		int val = board [j * board_wid + i];
		if (val == SAMEGAME_EMPTY) continue;
		if ((i > 0 && board [j * board_wid + i-1] == val) || 
				(j > 0 && board [(j-1) * board_wid + i] == val))
		{
			*x = i;
			*y = j;
			return;
		}
	}
}
