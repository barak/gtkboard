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
#include <stdlib.h>
#include <math.h>

#include "game.h"
#include "aaball.h"

#define CHECKERS_CELL_SIZE 40
#define CHECKERS_NUM_PIECES 4

#define CHECKERS_BOARD_WID 8
#define CHECKERS_BOARD_HEIT 8

#define CHECKERS_WK 1
#define CHECKERS_WP 2
#define CHECKERS_BK 3
#define CHECKERS_BP 4

#define CHECKERS_ISKING(x) (x == 1 || x == 3)
#define CHECKERS_ISPAWN(x) (x == 2 || x == 4)

#define CHECKERS_ISWHITE(x) (x >= 1 && x <= 2)
#define CHECKERS_ISBLACK(x) (x >= 3 && x <= 4)

char checkers_colors[] = 
	{200, 200, 200, 
	180, 180, 180};
	
int	checkers_init_pos[] = 
{
	 0 , 4 , 0 , 4 , 0 , 4 , 0 , 4 ,
	 4 , 0 , 4 , 0 , 4 , 0 , 4 , 0 ,
	 0 , 4 , 0 , 4 , 0 , 4 , 0 , 4 ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	 2 , 0 , 2 , 0 , 2 , 0 , 2 , 0 ,
	 0 , 2 , 0 , 2 , 0 , 2 , 0 , 2 ,
	 2 , 0 , 2 , 0 , 2 , 0 , 2 , 0 ,
};

static int checkers_max_moves = 200;


void checkers_init ();
int checkers_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
ResultType checkers_who_won (Pos *, Player, char **);
byte *checkers_movegen (Pos *);
ResultType checkers_eval (Pos *, Player, float *);
char ** checkers_get_pixmap (int idx, int color);
void checkers_reset_uistate ();
	
Game Checkers = 
	{ CHECKERS_CELL_SIZE, CHECKERS_BOARD_WID, CHECKERS_BOARD_HEIT, 
	CHECKERS_NUM_PIECES,
	checkers_colors, checkers_init_pos, NULL, "Checkers", NULL,
	checkers_init};

void checkers_init ()
{
	game_getmove = checkers_getmove;
	game_movegen = checkers_movegen;
	game_who_won = checkers_who_won;
	game_eval = checkers_eval;
	game_get_pixmap = checkers_get_pixmap;
	game_reset_uistate = checkers_reset_uistate;
	game_file_label = FILERANK_LABEL_TYPE_ALPHA;
	game_rank_label = FILERANK_LABEL_TYPE_NUM | FILERANK_LABEL_DESC;
	game_allow_flip = TRUE;
	game_doc_about_status = STATUS_UNPLAYABLE;
	game_doc_about = 
		"Checkers\n"
		"Two player game\n"
		"Status: Partially implemented (currently unplayable)\n"
		"URL: "GAME_DEFAULT_URL("checkers");
}

ResultType checkers_who_won (Pos *pos, Player player, char **commp)
{
	static char comment[32];
	char *who_str [2] = { "white won", "black won"};
	int found_w = 0, found_b = 0;
	int i;
	for (i=0; i<board_wid * board_heit; i++)
		if (CHECKERS_ISWHITE (pos->board[i])) found_w = 1;
		else if (CHECKERS_ISBLACK (pos->board[i])) found_b = 1;
	if (!found_b)
	{
		strncpy (comment, who_str[0], 31);
		*commp = comment;
		return RESULT_WHITE;
	}
	if (!found_w)
	{
		strncpy (comment, who_str[1], 31);
		*commp = comment;
		return RESULT_BLACK;
	}
	return RESULT_NOTYET;
}

byte * checkers_movegen (Pos *pos)
{
	int i, j, diffx, diffy;
	byte movbuf [256];
	byte *movlist, *mp = movbuf;
	byte *board = pos->board;
	Player player = pos->player;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (player == BLACK && !CHECKERS_ISBLACK(board [j * board_wid + i]))
			continue;
		if (player == WHITE && !CHECKERS_ISWHITE(board [j * board_wid + i]))
			continue;
		for (diffx = -1; diffx <= 1; diffx += 2)
		for (diffy = -1; diffy <= 1; diffy += 2)
		{
			if (CHECKERS_ISPAWN (board [j * board_wid + i])
					&& 	diffy != (player == WHITE ? 1 : -1))
				continue;
			if (!ISINBOARD(i+diffx, j+diffy)) continue;
			if (board [(j + diffy) * board_wid + (i + diffx)] != 0)
				continue;
			*mp++ = i; *mp++ = j; *mp++ = 0;
			*mp++ = i + diffx; *mp++ = j + diffy; 
			if ((player == WHITE && (j + diffy) == board_heit - 1) 
					|| (player == BLACK && (j + diffy) == 0))
				*mp++ = (player == WHITE ? CHECKERS_WK : CHECKERS_BK);
			else
				*mp++ = board [j * board_wid + i];
			*mp++ = -1;
		}
		for (diffx = -2; diffx <= 2; diffx += 4)
		for (diffy = -2; diffy <= 2; diffy += 4)
		{
			int val;
			if (CHECKERS_ISPAWN (board [j * board_wid + i])
					&& 	diffy != (player == WHITE ? 2 : -2))
				continue;
			if (!ISINBOARD(i+diffx, j+diffy)) continue;
			if (board [(j + diffy) * board_wid + (i + diffx)] != 0)
				continue;
			val = board [(j + diffy/2) * board_wid + i + diffx/2];
			if ((player == WHITE && !CHECKERS_ISBLACK(val)) || 
					(player == BLACK && !CHECKERS_ISWHITE (val)))
				continue;
			*mp++ = i; *mp++ = j; *mp++ = 0;
			*mp++ = i + diffx; *mp++ = j + diffy; 
			if ((player == WHITE && (j + diffy) == board_heit - 1) 
					|| (player == BLACK && (j + diffy) == 0))
				*mp++ = (player == WHITE ? CHECKERS_WK : CHECKERS_BK);
			else
				*mp++ = board [j * board_wid + i];
			*mp++ = i + diffx/2; *mp++ = j + diffy/2; *mp++ = 0;
			*mp++ = -1;
		}
		
			
	}
	if (mp == movbuf)
		*mp++ = -1;
	*mp++ = -2;
	movlist = (byte *) (malloc (mp - movbuf));
	memcpy (movlist, movbuf, (mp - movbuf));
	return movlist;
}

ResultType checkers_eval (Pos *pos, Player to_play, float *eval)
{
	float sum = 0;
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		switch (pos->board [j * board_wid + i])
		{
			case CHECKERS_WK: sum += (5 - fabs ((i-3.5) * (j-3.5)) / 10); break;
			case CHECKERS_WP: sum += (1 + j / 10.0); break;
			case CHECKERS_BK: sum -= (5 - fabs ((i-3.5) * (j-3.5)) / 10); break;
			case CHECKERS_BP: sum -= (1 + (board_heit - 1 - j) / 10.0); break;
		}
	}
	*eval = sum;
	return RESULT_NOTYET;

}

static int oldx = -1, oldy = -1;

void checkers_reset_uistate ()
{
	oldx = -1, oldy = -1;
}

int checkers_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int ** rmovep)
{
	static byte move[10];
	byte *mp = move;
	int diffx, diffy;
	byte *board = pos->board;
	if (type != GTKBOARD_BUTTON_RELEASE) return 0;
	if (oldx < 0)
	{
		int val = board [y * board_wid + x];
		if ((CHECKERS_ISWHITE(val) && !(to_play == WHITE)) ||
		(CHECKERS_ISBLACK(val) && !(to_play == BLACK)))
			return -1;
		oldx = x; oldy = y;
		return 0;
	}

	if (x == oldx && y == oldy)
	{
		oldx = -1; oldy = -1; return 0;
	}
	
	diffx = x - oldx;
	if (abs (diffx) == 1) 
	{
		diffy = y - oldy;
		if (abs (diffy) != 1)
		{ oldx = oldy = -1; return -1;}
		if (!CHECKERS_ISKING(board [oldy * board_wid + oldx])
				&& diffy != (to_play == WHITE ? 1 : -1))
		{ oldx = oldy = -1; return -1;}
		if (board [y * board_wid + x] != 0)
		{ oldx = oldy = -1; return -1;}
		*mp++ = oldx; *mp++ = oldy; *mp++ = 0;
		*mp++ = x; *mp++ = y; 
		if ((to_play == WHITE && y == board_heit - 1) 
				|| (to_play == BLACK && y == 0))
			*mp++ = (to_play == WHITE ? CHECKERS_WK : CHECKERS_BK);
		else
			*mp++ = board [oldy * board_wid + oldx];
		*mp++ = -1;
		*movp = move;
		oldx = oldy = -1;
		return 1;
	}
	if (abs (diffx) == 2)
	{
		int val;
		diffy = y - oldy;
		if (abs (diffy) != 2)
		{ oldx = oldy = -1; return -1;}
		if (!CHECKERS_ISKING(board [oldy * board_wid + oldx])
				&& diffy != (to_play == WHITE ? 2 : -2))
		{ oldx = oldy = -1; return -1;}
		if (board [y * board_wid + x] != 0)
		{ oldx = oldy = -1; return -1;}
		val = board [(y-diffy/2) * board_wid + (x-diffx/2)];
		if ((!CHECKERS_ISWHITE(val) && (to_play == BLACK)) ||
		(!CHECKERS_ISBLACK(val) && (to_play == WHITE)))
		{ oldx = oldy = -1; return -1;}
		*mp++ = oldx; *mp++ = oldy; *mp++ = 0;
		*mp++ = oldx+diffx/2; *mp++ = oldy+diffy/2; *mp++ = 0;
		*mp++ = x; *mp++ = y; 
		if ((to_play == WHITE && y == board_heit - 1)
				|| (to_play == BLACK && y == 0))
			*mp++ = (to_play == WHITE ? CHECKERS_WK : CHECKERS_BK);
		else
			*mp++ = board [oldy * board_wid + oldx];
		*mp++ = -1;
		*movp = move;
		oldx = oldy = -1;
		return 1;
	}
	{ oldx = oldy = -1; return -1;}
}

char ** checkers_get_pixmap (int idx, int color)
{
	int bg;
	int i;
	static char pixbuf[CHECKERS_CELL_SIZE * (CHECKERS_CELL_SIZE+1)];
	for(i=0, bg=0;i<3;i++) 
	{ int col = checkers_colors[i+3]; 
		if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	return pixmap_ball_gen (CHECKERS_CELL_SIZE, pixbuf,
			CHECKERS_ISWHITE(idx) ? 0xffffff : 0x0000ff, bg, 
			(idx == CHECKERS_WP || idx == CHECKERS_BP) ? 8 : 12, 24);
}
