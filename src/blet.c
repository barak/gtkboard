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

#define BLET_CELL_SIZE 55
#define BLET_NUM_PIECES 2

#define BLET_BOARD_WID 8
#define BLET_BOARD_HEIT 8

#define BLET_EMPTY 0
#define BLET_RP 1
#define BLET_GP 2

static char blet_colors[6] = {140, 140, 180, 140, 140, 180};

static int blet_init_pos [BLET_BOARD_WID*BLET_BOARD_HEIT] = 
{
	1 , 2 , 1 , 2 , 1 , 2 , 1 , 2 ,
	2 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 2 ,
	2 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 2 ,
	2 , 0 , 0 , 0 , 0 , 0 , 0 , 1 ,
	1 , 0 , 0 , 0 , 0 , 0 , 0 , 2 ,
	2 , 1 , 2 , 1 , 2 , 1 , 2 , 1 ,
};

SCORE_FIELD blet_score_fields[] = {SCORE_FIELD_USER, SCORE_FIELD_SCORE, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *blet_score_field_names[] = {"User", "Flips", "Time", "Date", NULL};

void blet_init ();

Game Blet = { BLET_CELL_SIZE, BLET_BOARD_WID, BLET_BOARD_HEIT, 
	BLET_NUM_PIECES,
	blet_colors, blet_init_pos, NULL, "Blet", "Logic puzzles", blet_init};

static int blet_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
static ResultType blet_who_won (Pos *, Player , char **);
unsigned char * blet_get_rgbmap (int, int);
InputType blet_event_handler (Pos *pos, GtkboardEvent *event, MoveInfo *move_info_p);

void blet_init ()
{
//	game_getmove = blet_getmove;
	game_get_rgbmap = blet_get_rgbmap;
	game_single_player = TRUE;
	game_who_won = blet_who_won;
	game_scorecmp = game_scorecmp_def_iscore;
	game_score_fields = blet_score_fields;
	game_score_field_names = blet_score_field_names;
	game_event_handler = blet_event_handler;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Blet\n"
		"Single player game\n"
		"Status: Partially implemented\n"
		"URL: "GAME_DEFAULT_URL("blet");
	game_doc_rules = 
		"If a ball is surrounded by balls of the opposite color, you can click on the middle ball to flip all three.\n\n"
		"The goal is to get 23 green balls in as few flips as possible\n";
	game_doc_strategy = 
		"Blet was invented by Villegas, Sadun and Voloch. A research paper showing the optimal strategy as well as an online version of the game (which requires a tcl browser plugin) can be found at: http://www.ma.utexas.edu/users/voloch/blet.html";
}

static ResultType blet_who_won (Pos *pos, Player player, char **commp)
{
	static char comment[32];
	int ngreen, i;
	gboolean over;
	for (i=0, ngreen = 0; i < board_wid * board_heit; i++)
		if (pos->board[i] == BLET_GP) ngreen++;
	over = (ngreen == 23 ? TRUE : FALSE);
   	if (over) snprintf (comment, 32, "You won! Flips: %d", pos->num_moves);
	else snprintf (comment, 32, "Green: %d; Flips: %d", ngreen, pos->num_moves);
	*commp = comment;
	return over ? RESULT_WON : RESULT_NOTYET;
}

static int incx[] = { -1, 1, 0, 0};
static int incy[] = { 0, 0, 1, -1};

#define FLIP(x) ((x)==BLET_RP?BLET_GP:BLET_RP)

InputType blet_event_handler (Pos *pos, GtkboardEvent *event, MoveInfo *move_info_p)
{
	static byte move[32];
	byte *mp = move;
	int val, k, x, y;
	if (event->type != GTKBOARD_BUTTON_RELEASE)
		return INPUT_NOTYET;
	x = event->x;
	y = event->y;
	if (x > 0 && x < board_wid - 1 && y > 0 && y < board_heit - 1)
	{
		move_info_p->help_message = "You must click on one of the balls.";
		return INPUT_ILLEGAL;
	}
	val = pos->board [y * board_wid + x];
	for (k=0; k < 4; k++)
	{
		int newx = x + incx[k], newy = y + incy[k];
		if (!ISINBOARD(newx, newy)) continue;
	   	if (pos->board[newy * board_wid + newx] == val)
		{	
			move_info_p->help_message = "Both neighbors must be of the opposite color.";
			return INPUT_ILLEGAL;
		}
		if (pos->board[newy * board_wid + newx] == BLET_EMPTY) continue;
		*mp++ = newx; *mp++ = newy; *mp++ = val;
	}
	*mp++ = x; *mp++ = y; *mp++ = FLIP(val);
	*mp++ = -1;
	move_info_p->move = move;
	return INPUT_LEGAL;
}


unsigned char * blet_get_rgbmap (int idx, int color)
{
	int fg, bg, i;
	char *colors;
	static char rgbbuf[3 * BLET_CELL_SIZE * BLET_CELL_SIZE];
	colors = blet_colors;
	fg = (idx == BLET_GP ? 0xcc << 8 : 0xee << 16);
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	rgbmap_ball_shadow_gen(55, rgbbuf, fg, bg, 17.0, 35.0, 3);
	return rgbbuf;
}

