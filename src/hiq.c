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

#define HIQ_CELL_SIZE 54
#define HIQ_NUM_PIECES 5

#define HIQ_BOARD_WID 7
#define HIQ_BOARD_HEIT 7


#define abs(x) ((x) < 0 ? -(x):(x))

char hiq_colors[6] = {170, 170, 170, 170, 170, 170};

int hiq_init_pos [HIQ_BOARD_WID*HIQ_BOARD_HEIT] = 
{
	 3 , 3 , 1 , 1 , 1 , 3 , 3 ,
	 3 , 3 , 1 , 1 , 1 , 3 , 3 ,
	 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
	 1 , 1 , 1 , 4 , 1 , 1 , 1 ,
	 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
	 3 , 3 , 1 , 1 , 1 , 3 , 3 ,
	 3 , 3 , 1 , 1 , 1 , 3 , 3 ,
};

#define HIQ_EMPTY 0
#define HIQ_RP 1
#define HIQ_BP 2
#define HIQ_UNUSED 3
#define HIQ_HOLE 4

static char * grey_square_54_xpm [] =
{
"54 54 1 1",
". c #d7d7d7",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
"......................................................",
};


void hiq_init ();

Game Hiq = { HIQ_CELL_SIZE, HIQ_BOARD_WID, HIQ_BOARD_HEIT, 
	HIQ_NUM_PIECES, 
	hiq_colors, hiq_init_pos, NULL, "Hiq", "Logic puzzles",
	hiq_init};

typedef struct
{
	int score;
}Hiq_state;

InputType hiq_event_handler
	(Pos *pos,  GtkboardEvent *event, MoveInfo *move_info_p);
ResultType hiq_who_won (Pos *, Player , char **);
char ** hiq_get_pixmap (int , int); 
void hiq_reset_uistate ();

void hiq_init ()
{
	game_single_player = 1;
//	game_getmove = hiq_getmove;
	game_event_handler = hiq_event_handler;
	game_who_won = hiq_who_won;
	game_get_pixmap = hiq_get_pixmap;
	game_who_won = hiq_who_won;
	game_scorecmp = game_scorecmp_def_iscore;
	game_reset_uistate = hiq_reset_uistate;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Hiq\n"
		"Single player game\n"
		"Status: Fully implemented\n"
		"URL: "GAME_DEFAULT_URL ("hiq");
	game_doc_rules = 
		"The objective is to eliminate as many balls as possible.\n"
		"\n"
		"Each move consists of clicking on a filled square and then clicking on an empty square with exactly one square in between, which must be filled. The ball on the original square goes to the empty square and the middle ball disappears. Diagonal moves are not allowed.\n"
		"\n"
		"If you have managed to leave only a single ball remaining then try the problem of leaving only a single ball on the center square.\n"; 
	game_doc_strategy =
		"Get the balls out of the edges as soon as possible.";
}

char ** hiq_get_pixmap (int idx, int color)
{
	int bg, i, fg, rad, grad;
	char *colors;
	static char pixbuf[HIQ_CELL_SIZE * (HIQ_CELL_SIZE+1)];
	if (idx == HIQ_UNUSED) return grey_square_54_xpm;
	colors = hiq_colors;
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	fg = (idx == HIQ_RP ? 0xff << 16 : 0xd0d0d0);
	rad = (idx == HIQ_RP ? 16 : 18);
	grad = (idx == HIQ_RP ? 24 : 36);
	return pixmap_ball_gen(54, pixbuf, fg, bg, rad, grad);
}

ResultType hiq_who_won (Pos *pos, Player player, char **commp)
{
	static char comment [32];
	int i, j, k;
	int found = 0, count = 0;
	int incx [] = { 0, 0, -1, 1 };
	int incy [] = { -1, 1, 0, 0 };
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (pos->board [j * board_heit + i] != HIQ_RP) continue;
		count++;
		for (k=0; k<4; k++)
		{
			int x = i + incx[k];
			int y = j + incy[k];
			if (x < 0 || y < 0 || x >= board_wid || y >= board_heit) continue;
			if (pos->board [y * board_heit + x] != HIQ_RP) continue;
			x += incx[k];
			y += incy[k];
			if (x < 0 || y < 0 || x >= board_wid || y >= board_heit) continue;
			if (pos->board [y * board_heit + x] != HIQ_HOLE) continue;
			found = 1;
		}
	}
	if (!found)
	{
		snprintf (comment, 32, "%s %d %s left", 
				count == 1 ? "Congrats!" : "Game over.", 
				count, count == 1 ? "ball" : "balls");
		*commp = comment;
		return RESULT_WON;
	}
	snprintf (comment, 32, "%d balls left", 32 - pos->num_moves);
	*commp = comment;
	return RESULT_NOTYET;
}

static int  oldx = -1, oldy = -1;

void hiq_reset_uistate ()
{
	oldx = oldy = -1;
}

InputType hiq_event_handler
	(Pos *pos,  GtkboardEvent *event, MoveInfo *move_info_p)
{
	static byte move[10];
	static int rmove[10];
	int diffx, diffy, x, y;
	x = event->x;
	y = event->y;
	if (event->type != GTKBOARD_BUTTON_RELEASE)
		return INPUT_NOTYET;
	if (oldx == -1)
	{
		if (hiq_init_pos[y * board_wid + x] == HIQ_UNUSED 
				|| pos->board [y * board_wid + x] == HIQ_HOLE) 
		{ 
			move_info_p->help_message = "You must click on a ball.";
			return INPUT_ILLEGAL;
		}
		oldx = x; oldy = y;
		rmove[0] = x;
		rmove[1] = y;
		rmove[2] = RENDER_HIGHLIGHT1;
		rmove[3] = -1;
		move_info_p->rmove = rmove;
		return INPUT_NOTYET;
	}

	rmove[0] = oldx;
	rmove[1] = oldy;
	rmove[2] = RENDER_NONE;
	rmove[3] = -1;
	move_info_p->rmove = rmove;
	if (hiq_init_pos[y * board_wid + x] == HIQ_UNUSED) 
	{ 
		oldx = oldy = -1;
		move_info_p->help_message = "You must jump over a ball.";
		return INPUT_ILLEGAL;
	}

	if (x == oldx && y == oldy)
	{
		oldx = -1; oldy = -1; return INPUT_NOTYET;
	}
	
	if (pos->board [y * board_wid + x] != HIQ_HOLE) 
	{
		oldx = oldy = -1; 
		move_info_p->help_message = "You must click on an empty square.";
		return INPUT_ILLEGAL;
	}
	diffx = abs (x - oldx); diffy = abs (y - oldy);
	if ((diffx != 2 && diffy != 2) || diffx * diffy != 0) 
	{ 
		oldx = oldy = -1; 
		move_info_p->help_message = "You must jump over a ball.";
		return INPUT_ILLEGAL;
	}
	move[0] = oldx; move[1] = oldy; move[2] = HIQ_HOLE;
	move[3] = x; move[4] = y; move[5] = pos->board [oldy * board_wid + oldx];
	move[6] = (x+oldx)/2; move[7] = (y+oldy)/2; move[8] = HIQ_HOLE;
	move[9] = -1;
	oldx = -1; oldy = -1;

	move_info_p->move = move;	
	return INPUT_LEGAL;
}

