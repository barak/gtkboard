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
#include <gdk/gdkkeysyms.h>

#include "game.h"
#include "aaball.h"

#define OTHELLO_CELL_SIZE 55
#define OTHELLO_NUM_PIECES 2

char othello_colors[6] = {200, 200, 200, 140, 140, 140};

int othello_init_pos [8*8] = 
{
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 0 , 2 , 1 , 0 , 0 , 0  ,
	0 , 0 , 0 , 1 , 2 , 0 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 , 0  ,
};

int othello6x6_init_pos [6*6] = 
{
	0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 2 , 1 , 0 , 0  ,
	0 , 0 , 1 , 2 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0  ,
	0 , 0 , 0 , 0 , 0 , 0  ,
};

#define OTHELLO_WP 1
#define OTHELLO_BP 2
#define OTHELLO_EMPTY 0


int othello_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
int othello_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp);
void othello_init ();
ResultType othello_who_won (Pos *, Player, char **);
ResultType othello_eval (Pos *, Player, float *);
ResultType othello_eval_incr (Pos *, byte *, float *);
byte * othello_movegen (Pos *);
char ** othello_get_pixmap (int, int);
guchar *othello_get_rgbmap (int, int);
gboolean othello_use_incr_eval (Pos *pos);

Game Othello = { OTHELLO_CELL_SIZE, 8, 8,
	OTHELLO_NUM_PIECES, 
	othello_colors, othello_init_pos, NULL, "Othello", "Othello", othello_init};

Game Othello6x6 = { OTHELLO_CELL_SIZE, 6, 6,
	OTHELLO_NUM_PIECES, 
	othello_colors, othello6x6_init_pos, NULL, "Othello 6x6", "Othello", othello_init};


void othello_init (Game *game)
{
	game_getmove = othello_getmove;
	game_getmove_kb = othello_getmove_kb;
	game_who_won = othello_who_won;
	game_eval = othello_eval;
	game_eval_incr = othello_eval_incr;
	game_use_incr_eval = othello_use_incr_eval;
	game_movegen = othello_movegen;
	game_get_rgbmap = othello_get_rgbmap;
	game_white_string = "Red";
	game_black_string = "Blue";
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	
	game_doc_about_status = STATUS_COMPLETE;
	if (game == &Othello)
		game_doc_about =
			"Othello\n"
			"Two player game\n"
			"Status: Fully implemented\n"
			"URL: "GAME_DEFAULT_URL ("othello");
	else
		game_doc_about =
			"Othello 6x6\n"
			"Two player game\n"
			"Status: Fully implemented\n"
			"URL: "GAME_DEFAULT_URL ("othello6x6");
	game_doc_rules = 
		"Two players take turns in placing balls of either color. The objective is to get as many balls of your color as possible.\n"
		"\n"
		"When you place a ball in such a way that two of your balls sandwich one or more of the opponent's balls along a line (horizontal, vertical, or diagonal), then the sandwiched balls change to your color. You must move in such a way that at least one switch happens.\n"
		"\n"
		"If you don't have a move, you pass by hitting space or clicking on an empty square."
		;
	game_doc_strategy = 
		"The number of balls of either color at a given time is, paradoxically, a _very_ poor indicator of who has the advantage. This is because balls can flip color en masse and wildly, especially during the last few moves.\n"
		"\n"
		"Indeed, in the beginning you should try to minimize the number of balls you have. The key is mobility. You must strive to maximize your number of legal moves so that you can try to force your opponent into making bad moves.\n"
		"\n"
		"The corners are key squares, because corner balls never flip. If your opponent blunders into giving you a corner before the final stages of the game, you are practically assured of a win.\n";
}

char ** othello_get_pixmap (int idx, int color)
{
	int fg, bg, i;
	char *colors;
	static char pixbuf[OTHELLO_CELL_SIZE*(OTHELLO_CELL_SIZE)+1];
	colors = othello_colors;
	fg = (idx == OTHELLO_WP ? 0xee << 16 : 0xee);
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	return pixmap_ball_gen(55, pixbuf, fg, bg, 17.0, 30.0);
}

guchar *othello_get_rgbmap (int idx, int color)
{
	static guchar rgbbuf[3*OTHELLO_CELL_SIZE*OTHELLO_CELL_SIZE];
	int fg, bg, i;
	char *colors;
	colors = othello_colors;
	fg = (idx == OTHELLO_WP ? 0xee << 16 : 0xee);
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	rgbmap_ball_shadow_gen(OTHELLO_CELL_SIZE, rgbbuf, fg, bg, 17.0, 30.0, 3);
	return rgbbuf;
}

static int incx[] = { -1, -1, -1, 0, 0, 1, 1, 1};
static int incy[] = { -1, 0, 1, -1, 1, -1, 0, 1};

static int get_sandwich_len (Pos *pos, int x0, int y0, int dx, int dy, byte player)
{
	int x = x0 + dx, y = y0+dy, len=0;
	byte state;
	for (; x >= 0 && y >= 0 && x < board_wid && y < board_heit; 
			x+=dx, y+=dy, len++)
	{
		if ((state = pos->board[y * board_wid + x]) == 0)
			return -1;
		else if (state == player)
			return len;
	}
	return -1;
}

// does player have a move in this position
static gboolean hasmove (Pos *pos, Player player)
{
	int i, x, y, found = FALSE;
	for (x=0; x<board_wid && !found; x++)
		for (y=0; y<board_heit && !found; y++)
		{
			if (pos->board [y * board_wid + x] != OTHELLO_EMPTY)
				continue;
			for (i=0; i<8; i++)
			{
				if (get_sandwich_len (pos, x, y ,incx[i], incy[i], 
						player == WHITE ? OTHELLO_WP : OTHELLO_BP) > 0)
				{
					found = TRUE;
					break;
				}
			}
		}
	return found;
}

int othello_getmove_kb (Pos *pos, int key,  byte **movp, int **rmovp)
{
	static byte move[1];
	if (key != GDK_space) return -1;
	if (hasmove (pos, pos->player)) return -1;
	move[0] = -1;
	*movp = move;
	return 1;	
}
	
int othello_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int ** rmovep)
{
	int i, j, sw_len, found=0;
	static byte move[128];
	byte *temp = move;
	byte our   = to_play == WHITE ? OTHELLO_WP : OTHELLO_BP;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (pos->board [y * board_wid + x] != OTHELLO_EMPTY)
		return -1;
	for (i=0; i<8; i++)
	{
		sw_len = get_sandwich_len (pos, x, y ,incx[i], incy[i], 
				to_play == WHITE ? OTHELLO_WP : OTHELLO_BP);
		if (sw_len > 0) found = 1;
		for (j=1; j<=sw_len; j++)
		{
			*temp++ = x + incx[i] * j;
			*temp++ = y + incy[i] * j;
			*temp++ = our;
		}		
	}
	if (!found)
	{
		if (hasmove (pos, to_play)) return -1;
		*temp++ = -1;
		*movp = move;
	}
	*temp++ = x;
	*temp++ = y;
	*temp++ = our;
	*temp++ = -1;
	*movp = move;
	return 1;
}

ResultType othello_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i, wscore = 0, bscore = 0, who_idx ,x, y;
	char *who_str [3] = { "Red won", "Blue won", "its a tie" };
	int found = 0;
	for (i=0; i<board_wid * board_heit; i++)
		if (pos->board[i] == OTHELLO_WP)
			wscore++;
		else if (pos->board[i] == OTHELLO_BP)
			bscore++;
	if (! (wscore == 0 || bscore == 0 
				|| wscore + bscore == board_wid * board_heit))
	{
		snprintf (comment, 32, "%d : %d", wscore, bscore);
		*commp = comment;
		return RESULT_NOTYET;
	}
	if (wscore > bscore) who_idx = 0;
	else if (wscore < bscore) who_idx = 1;
	else who_idx = 2;
	snprintf (comment, 32, "%s (%d : %d)", who_str [who_idx], wscore, bscore);
	*commp = comment;
	if (wscore > bscore)
		return RESULT_WHITE;
	if (wscore < bscore)
		return RESULT_BLACK;
	return RESULT_TIE;
}

byte * othello_movegen (Pos *pos)
{
	int i, j, x, y, sw_len;
	byte movbuf [4096];
	byte *movlist, *movp = movbuf;
	Player player = pos->player;
	byte our = player == WHITE ? OTHELLO_WP : OTHELLO_BP;
	gboolean game_over = TRUE;
	for (x=0; x<board_wid; x++)
		for (y=0; y<board_wid; y++)
		{
			gboolean found = FALSE;
			if (pos->board [y * board_wid + x] != OTHELLO_EMPTY)
				continue;
			game_over = FALSE;
			for (i=0; i<8; i++)
			{
				sw_len = get_sandwich_len (pos, x, y ,incx[i], incy[i], 
						player == WHITE ? OTHELLO_WP : OTHELLO_BP);
				if (sw_len > 0) found = TRUE;
				for (j=1; j<=sw_len; j++)
				{
					*movp++ = x + incx[i] * j;
					*movp++ = y + incy[i] * j;
					*movp++ = our;
				}		
			}
			if (!found)
				continue;
			*movp++ = x;
			*movp++ = y;
			*movp++ = our;
			*movp++ = -1;
		}
	/* if we want to pass, we must return an empty move, NOT no move */
	if (movp == movbuf && !game_over)
		*movp++ = -1;
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	return movlist;
}

static float othello_eval_count (Pos *pos)
{
	int i, sum=0;
	for (i=0; i<board_wid * board_heit; i++)
		if (pos->board [i] == OTHELLO_WP)
			sum++;
		else if (pos->board [i] == OTHELLO_BP)
			sum--;
	return sum;
}

static int othello_eval_mobility_count (Pos *pos, int color)
{
	int i, x, y, found, sum = 0;
	byte our = (color == WHITE ? OTHELLO_WP : OTHELLO_BP);
	for (x=0; x<board_wid; x++)
		for (y=0; y<board_heit; y++)
		{
			if (pos->board [y * board_wid + x] != our)
				continue;
			found = 0;
			for (i=0; i<8; i++)
				if (get_sandwich_len (pos, x, y ,incx[i], incy[i], our) > 0)
				{
					found = 1;
					break;
				}
			if (found)
				sum++;
		}
	return sum;
}

static float othello_eval_mobility (Pos *pos)
{
	return othello_eval_mobility_count (pos, WHITE) - 
		othello_eval_mobility_count (pos, BLACK);
}

//! Faster approxmiation for mobility
static float othello_eval_liberty (Pos *pos)
{
	int i, j, k;
	int liberty = 0;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (pos->board [j * board_wid + i] != OTHELLO_EMPTY)
			continue;
		for (k=0; k<8; k++)
		{
			int x = i + incx[k], y = j + incy[k];
			int val;
			if (!ISINBOARD (x, y)) continue;
			if ((val = pos->board [y * board_wid + x]) == OTHELLO_EMPTY)
				continue;
			liberty += (val == OTHELLO_WP ? -1 : 1);
		}
	}
	return liberty;
}

static int othello_eval_num_moves (Pos *pos)
{
	int i, sum=0;
	for (i=0; i<board_wid * board_heit; i++)
		if (pos->board [i] != OTHELLO_EMPTY)
			sum++;
	return sum;
}

enum { SAFE, UNSAFE, UNKNOWN };

static byte * safe_cached;


/* TODO recursion sucks. reimplement this */
static int othello_eval_is_safe (Pos *pos, int x, int y, byte our)
{
	if (x < 0 || y < 0 || x >= board_wid || y >= board_heit)
		return SAFE;
	if (pos->board [y * board_wid + x] != our)
		{ return (safe_cached [y * board_wid + x] = UNSAFE);}
	if (safe_cached [y * board_wid + x] != UNKNOWN)
		return safe_cached [y * board_wid + x];

	/* crucial to avoid infinite recursion */
	safe_cached [y * board_wid + x] = UNSAFE;
	if (othello_eval_is_safe (pos, x - 1, y - 1, our) == UNSAFE
			&& othello_eval_is_safe (pos, x + 1, y + 1, our) == UNSAFE)
		{ return (safe_cached [y * board_wid + x] = UNSAFE);}
	if (othello_eval_is_safe (pos, x + 1, y - 1, our) == UNSAFE
			&& othello_eval_is_safe (pos, x - 1, y + 1, our) == UNSAFE)
		{ return (safe_cached [y * board_wid + x] = UNSAFE);}
	if (othello_eval_is_safe (pos, x - 1, y, our) == UNSAFE
			&& othello_eval_is_safe (pos, x + 1, y, our) == UNSAFE)
		{ return (safe_cached [y * board_wid + x] = UNSAFE);}
	if (othello_eval_is_safe (pos, x, y - 1, our) == UNSAFE
			&& othello_eval_is_safe (pos, x, y + 1, our) == UNSAFE)
		{ return (safe_cached [y * board_wid + x] = UNSAFE);}
	return (safe_cached [y * board_wid + x] = SAFE);
}

static float othello_eval_safe (Pos *pos)
{
	int i, x, y, sum=0;
	if (pos->board [0 * board_wid + 0] == OTHELLO_EMPTY &&
		pos->board [0 * board_wid + board_wid - 1] == OTHELLO_EMPTY &&
		pos->board [(board_heit - 1) * board_wid + 0] == OTHELLO_EMPTY &&
		pos->board [(board_heit - 1) * board_wid + board_wid - 1] == OTHELLO_EMPTY )
		return 0;
	safe_cached = (byte *) malloc (board_wid * board_heit);
	assert (safe_cached);
	for (i=0; i<board_wid * board_heit; i++)
		safe_cached [i] = UNKNOWN;
	
	for (x=0; x<board_wid; x++)
		for (y=0; y<board_heit; y++)
			if (pos->board [y * board_wid + x] == OTHELLO_WP &&
					othello_eval_is_safe (pos, x, y, OTHELLO_WP) == SAFE)
				sum++;
			else if (pos->board [y * board_wid + x] == OTHELLO_BP &&
					othello_eval_is_safe (pos, x, y, OTHELLO_BP) == SAFE)
				sum --;
	free (safe_cached);
	return sum;		
}

/*static float othello_eval_dangerous (Pos *pos)
{
	int i, j;
	float danger = 0;
	for (i=1; i<=6; i+=5)
	for (j=1; j<=6; j+=5)
	{
		if (pos->board[j * board_wid + i] == OTHELLO_WP) danger++;
		else if (pos->board[j * board_wid + i] == OTHELLO_BP) danger--;
	}
	for (i=1; i<=6; i+=5)
	for (j=0; j<=7; j+=7)
	{
		if (pos->board[j * board_wid + i] == OTHELLO_WP) danger += 0.3;
		else if (pos->board[j * board_wid + i] == OTHELLO_BP) danger -= 0.3;
	}
	for (i=0; i<=7; i+=7)
	for (j=1; j<=6; j+=5)
	{
		if (pos->board[j * board_wid + i] == OTHELLO_WP) danger += 0.3;
		else if (pos->board[j * board_wid + i] == OTHELLO_BP) danger -= 0.3;
	}
	return -danger;		
}*/

static float othello_eval_material (Pos *pos)
{
	int i, material = 0;
	for (i=0; i<board_wid * board_heit; i++)
	{
		if (pos->board[i] == OTHELLO_WP)
			material++;
		else if (pos->board[i] == OTHELLO_BP)
			material--;
	}
	return material;
}

static float othello_eval_weights (Pos *pos)
{
	static int weights8 [4][4] = 
	{
		{ 500, -240, 85, 69 },
		{ 0  , -130, 49, 23 },
		{ 0  ,    0,  1,  9 },
		{ 0  ,    0,  0, 32 },
	};
	
	// FIXME: find a decent weight set
	static int weights6 [3][3] = 
	{
		{ 500, -200, 80},
		{ 0  , -300, 50},
		{ 0  ,    0, 20},
	};

	int i, j, wtsum = 0;

	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		int val = pos->board[j * board_wid + i];
		int x = i, y = j;
		if (val == OTHELLO_EMPTY)
			continue;

		if (pos->game == &Othello)
		{
			if (x > 3) x = 7-x;
			if (y > 3) y = 7-y;
		}
		else //if (pos->game == Othello6x6)
		{
			if (x > 2) x = 5-x;
			if (y > 2) y = 5-y;
		}
		
		if (x > y)  {int tmp = y; y = x; x = tmp;}
		
		wtsum += (pos->game == &Othello ? weights8 [x][y] : weights6[x][y])  
			* (val == OTHELLO_WP ? 1 : -1);
	}
	return wtsum;	
}

ResultType othello_eval (Pos *pos, Player player, float *eval)
{
	int i;
	gboolean found;

	if (pos->num_moves >= board_wid * board_heit)
	{
		for (i=0, found=FALSE; i<board_wid*board_heit; i++)
			if (pos->board [i] == OTHELLO_EMPTY) {found = TRUE; break;}
		if (!found)
		{
			*eval = othello_eval_material (pos);
			*eval *= GAME_EVAL_INFTY;
			if (*eval > 0) return RESULT_WHITE;
			if (*eval < 0) return RESULT_BLACK;
			return RESULT_NOTYET;
		}
	}

	*eval = 
		10 * othello_eval_liberty (pos) 
		//10 * othello_eval_mobility (pos) 
		+ 100 * othello_eval_safe (pos) 
		+ othello_eval_weights (pos);
	return RESULT_NOTYET;
}

ResultType othello_eval_incr (Pos *pos, byte *move, float *eval)
{
	int i;
	for (i=0; move[3*i] != -1; i++)
		;
	if (i == 0) 
		*eval = 0;
	else
		*eval = (pos->player == WHITE ? (2 * i - 1) : - (2 * i - 1));
	return RESULT_NOTYET;
}

gboolean othello_use_incr_eval (Pos *pos)
{
	// TODO: use different threshold for Othello6x6
	return pos->num_moves > 50 ? TRUE : FALSE;
}
