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

#include "game.h"
#include "aaball.h"

#define ATAXX_CELL_SIZE 55
#define ATAXX_NUM_PIECES 2

#define ATAXX_BOARD_WID 7
#define ATAXX_BOARD_HEIT 7

#define ATAXX_EMPTY 0
#define ATAXX_WP 1
#define ATAXX_BP 2

#define ATAXX_MOVEGEN_PLAUSIBLE 1

static char ataxx_colors[6] = {140, 160, 140, 200, 200, 200};

static int ataxx_init_pos [ATAXX_BOARD_WID*ATAXX_BOARD_HEIT] = 
{
	1 , 0 , 0 , 0 , 0 , 0 , 2 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	2 , 0 , 0 , 0 , 0 , 0 , 1 ,
};

void ataxx_init ();

Game Ataxx = { ATAXX_CELL_SIZE, ATAXX_BOARD_WID, ATAXX_BOARD_HEIT, 
	ATAXX_NUM_PIECES,
	ataxx_colors, ataxx_init_pos, NULL, "Ataxx", NULL, ataxx_init};

ResultType ataxx_eval (Pos *, Player, float *);
byte *ataxx_movegen (Pos *);

static int ataxx_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType ataxx_who_won (Pos *, Player , char **);
unsigned char * ataxx_get_rgbmap (int, int);
void ataxx_reset_uistate ();


static int ataxx_max_moves = 200;

void ataxx_init ()
{
	game_eval = ataxx_eval;
	game_movegen = ataxx_movegen;
	game_getmove = ataxx_getmove;
	game_who_won = ataxx_who_won;
	game_get_rgbmap = ataxx_get_rgbmap;
	game_white_string = "Red";
	game_black_string = "Blue";
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	game_reset_uistate = ataxx_reset_uistate;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Ataxx\n"
		"Two player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL("ataxx");
	game_doc_rules = 
		"  - The objective of the game is to get as many balls of your color as possible.\n"
		"  - In each move you must click an existing ball of your color followed by an empty square.\n"
		"  - The new square should be at a distance of at most 2 from the first square (a diagonal counts as one unit).\n"
		"  - If the distance is two the first square becomes empty, but not if the distance is 1.\n"
		"  - In either case all balls adjacent to the new square, if they are the opponent's color, get converted to your color.\n"
		"  - If one player has no moves the player with more balls wins.\n";
}

ResultType ataxx_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i, wscore = 0, bscore = 0, who_idx;
	char *who_str [3] = { "Red won", "Blue won", "its a tie" };
	byte *move = ataxx_movegen (pos);
	for (i=0; i<board_wid * board_heit; i++)
		if (pos->board[i] == ATAXX_WP)
			wscore++;
		else if (pos->board[i] == ATAXX_BP)
			bscore++;
	if (move[0] != -2)
	{
		free (move);
		if (pos->num_moves > ataxx_max_moves)
		{
			fprintf (stderr, "max moves reached\n");
			snprintf (comment, 32, "%s", who_str[2]);
			*commp = comment;
			return RESULT_TIE;
		}
		else
		{
			snprintf (comment, 32, "%d : %d", wscore, bscore);
			*commp = comment;
			return RESULT_NOTYET;
		}
	}
	free (move);
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


ResultType ataxx_eval (Pos *pos, Player to_play, float *eval)
{
	int i;
	int wcount, bcount;
	for (i=0, wcount=0, bcount=0; i<board_wid*board_heit; i++)
	{
		if (pos->board[i] == ATAXX_WP) wcount++;
		if (pos->board[i] == ATAXX_BP) bcount++;
	}
	*eval = wcount-bcount;
	if (!wcount || !bcount) *eval *= GAME_EVAL_INFTY;
	if (!wcount) return RESULT_BLACK;
	if (!bcount) return RESULT_WHITE;
	return RESULT_NOTYET;
}

byte *ataxx_movegen (Pos *pos)
	/* to keep things from getting out of hand, we'll generate only 
	   _plausible_ moves: find the max #flips possible and generate
	   only those moves that lead to at least max-1 flips */
{
	int incx[] = { -1, -1, -1, 0, 0, 1, 1, 1};
	int incy[] = { -1, 0, 1, -1, 1, -1, 0, 1};
	int incx2[] = { -2, -2, -2, -2, -2, -1, -1, 0, 0, 1, 1, 2, 2, 2, 2, 2};
	int incy2[] = { -2, -1, 0, 1, 2, -2, 2, -2, 2, -2, 2, -2, -1, 0, 1, 2};
	int x, y, i, j, newx, newy;
	Player player = pos->player;
	byte our = (player == WHITE ? ATAXX_WP : ATAXX_BP);
	byte other = (player == WHITE ? ATAXX_BP : ATAXX_WP);
	byte *board = pos->board;
#ifdef ATAXX_MOVEGEN_PLAUSIBLE
	int max_nbrs;
#endif
	int found = 0;
	byte movbuf [4096];
	byte *movp = movbuf;
	byte *movlist;
	byte *nbrs;
   	nbrs = (byte *) malloc (board_wid * board_heit * sizeof (byte));
	assert (nbrs);
	for (i=0; i<board_wid * board_heit; i++)
		nbrs[i] = 0;
	for (x=0; x<board_wid; x++)
		for (y=0; y<board_heit; y++)
		{
			if (board [y * board_wid + x] != other)
				continue;
			for (i=0; i<8; i++)
			{
				newx = x + incx[i];
				newy = y + incy[i];
				if (newx >= 0 && newy >= 0 
						&& newx < board_wid && newy < board_heit)
					nbrs [newy * board_wid + newx] ++;
			}
		}
#ifdef ATAXX_MOVEGEN_PLAUSIBLE
	max_nbrs=0;
	for (x=0; x<board_wid; x++)
		for (y=0; y<board_heit; y++)
		if (board[y * board_wid + x] == ATAXX_EMPTY 
				&& nbrs[y * board_wid + x] > max_nbrs)
		{
			found=0;
			for (j=0; j<8; j++)
			{
				newx = x + incx[j];
				newy = y + incy[j];
				if (newx >= 0 && newy >= 0 
						&& newx < board_wid && newy < board_heit
						&& board [newy * board_wid + newx] == our)
				{
					max_nbrs = nbrs[y * board_wid + x];
					found=1;
					break;
				}
			}
			/*if (found) continue;
			nbrs [y * board_wid + x]--;
			if (nbrs[y * board_wid + x] <= max_nbrs) continue;
			for (j=0; j<16; j++)
			{
				newx = x + incx2[j];
				newy = y + incy2[j];
				if (newx >= 0 && newy >= 0 
						&& newx < board_wid && newy < board_heit
						&& board [newy * board_wid + newx] == our)
				{
					max_nbrs = nbrs[y * board_wid + x];
					break;
				}
			}*/
		} 
#endif
	for (x=0; x<board_wid; x++)
		for (y=0; y<board_heit; y++)
		{
			found=0;
			if (board [y * board_wid + x] != ATAXX_EMPTY)
				continue;
#ifdef ATAXX_MOVEGEN_PLAUSIBLE
			if (nbrs [y * board_wid + x] < max_nbrs - 1)
				continue;
#endif
			for (i=0; i<8; i++)
			{
				newx = x + incx[i];
				newy = y + incy[i];
				if (newx >= 0 && newy >= 0 
						&& newx < board_wid && newy < board_heit)
					if (board [newy * board_wid + newx] == our)
					{
						/* found a same col neighbor */
						found=1;
						break;
					}
			}
			if (found)
			{
				*movp++ = x;
				*movp++ = y;
				*movp++ = our;
				for (i=0; i<8; i++)
				{
					newx = x + incx[i];
					newy = y + incy[i];
					if (newx >= 0 && newy >= 0 
							&& newx < board_wid && newy < board_heit)
						if (board [newy * board_wid + newx] == other)
						{
							*movp++ = newx;
							*movp++ = newy;
							*movp++ = our;
						}
				}
				*movp++ = -1;
			}	
			else
			{
				for (i=0; i<16; i++)
				{
					newx = x + incx2[i];
					newy = y + incy2[i];
					if (!(newx >= 0 && newy >= 0 
							&& newx < board_wid && newy < board_heit))
						continue;
					if (board [newy * board_wid + newx] != our)
						continue;
					*movp++ = x;
					*movp++ = y;
					*movp++ = our;
					*movp++ = newx;
					*movp++ = newy;
					*movp++ = ATAXX_EMPTY;
					for (j=0; j<8; j++)
					{
						newx = x + incx[j];
						newy = y + incy[j];
						if (newx >= 0 && newy >= 0 
								&& newx < board_wid && newy < board_heit)
							if (board [newy * board_wid + newx] == other)
							{
								*movp++ = newx;
								*movp++ = newy;
								*movp++ = our;
							}
					}
					*movp++ = -1;
				}
			}
		}
	*movp++ = -2;
	movlist = (byte *) (malloc (movp - movbuf));
	memcpy (movlist, movbuf, (movp - movbuf));
	free (nbrs);
	return movlist;
	
}

static int  oldx = -1, oldy = -1;

void ataxx_reset_uistate ()
{
	oldx = -1, oldy = -1;
}

int ataxx_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int **rmovep)
{
	static byte move[32];
	static int rmove[4];
	byte *mptr = move;
	int *rp = rmove;
	int diffx, diffy;
	int other, i;
	int incx[] = { -1, -1, -1, 0, 0, 1, 1, 1};
	int incy[] = { -1, 0, 1, -1, 1, -1, 0, 1};
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (oldx == -1)
	{
		if (pos->board [y * board_wid + x] != (to_play == WHITE ? ATAXX_WP : ATAXX_BP))
			return -1;
		oldx = x; oldy = y;
		*rp++ = x;
		*rp++ = y;
		*rp++ = RENDER_HIGHLIGHT1;
		*rp++ = -1;
		*rmovep = rmove;
		return 0;
	}

	if (x == oldx && y == oldy) 
	{ 
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		oldx = -1; oldy = -1; return 0; 
	} 
	if (pos->board [y * board_wid + x] != ATAXX_EMPTY) 
	{ 
		*rp++ = oldx; *rp++ = oldy; *rp++ = RENDER_NONE; *rp++ = -1; *rmovep = rmove;
		return oldx = oldy = -1; 
	}
	diffx = abs (x - oldx); diffy = abs (y - oldy);
	if (diffx > 2 || diffy > 2) { return oldx = oldy = -1; }
	if (diffx > 1 || diffy > 1)
	{ *mptr++ = oldx; *mptr++ = oldy; *mptr++ = ATAXX_EMPTY; }
	other = (to_play == WHITE ? ATAXX_BP : ATAXX_WP);
	for (i=0; i<8; i++)
	{
		int newx = x + incx[i], newy = y + incy[i];
		if (newx < 0 || newy < 0 || newx >= board_wid || newy >= board_heit)
			continue;
		if (pos->board[newy * board_wid + newx] == other)
		{
			*mptr++ = newx; *mptr++ = newy; 
			*mptr++ = (to_play == WHITE ? ATAXX_WP : ATAXX_BP);
		}
	}
	{ *mptr++ = x; *mptr++ = y; *mptr++ = 
		(to_play == WHITE ? ATAXX_WP : ATAXX_BP); }
	*mptr = -1;
	if (movp)
		*movp = move;	
	*rp++ = oldx;
	*rp++ = oldy;
	*rp++ = RENDER_NONE;
	*rp++ = -1;
	*rmovep = rmove;
	oldx = -1; oldy = -1;
	return 1;
}

unsigned char * ataxx_get_rgbmap (int idx, int color)
{
	int fg, bg, i;
	char *colors;
	static char rgbbuf[3 * ATAXX_CELL_SIZE * ATAXX_CELL_SIZE];
	colors = ataxx_colors;
	fg = (idx == ATAXX_WP ? 0xee << 16 : 0xee);
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	rgbmap_ball_shadow_gen(55, rgbbuf, fg, bg, 17.0, 35.0, 3);
	return rgbbuf;
}

