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
#include "../pixmaps/ninemm_bg.xpm"

#define NINEMM_CELL_SIZE 36
#define NINEMM_NUM_PIECES 2

#define NINEMM_BOARD_WID 7
#define NINEMM_BOARD_HEIT 7

char ninemm_colors[6] = {200, 200, 200, 140, 140, 140};

static char *ninemm_wp_xpm[] = 
{
	"36 36 2 1",
	"  c none",
	". c blue",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
};

static char *ninemm_bp_xpm[] = 
{
	"36 36 2 1",
	"  c none",
	". c red",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"        ....................        ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
	"                                    ",
};

static char ** ninemm_pixmaps[] = { ninemm_wp_xpm, ninemm_bp_xpm };

int ninemm_init_pos [NINEMM_BOARD_WID*NINEMM_BOARD_HEIT] = {0};

#define NINEMM_EMPTY 0
#define NINEMM_WP 1
#define NINEMM_BP 2


void ninemm_init ();
int ninemm_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
void ninemm_reset_uistate ();

Game Ninemm = { NINEMM_CELL_SIZE, NINEMM_BOARD_WID, NINEMM_BOARD_HEIT, 
	NINEMM_NUM_PIECES, 
	ninemm_colors, ninemm_init_pos, ninemm_pixmaps, "Nine Men's Morris", NULL,
	ninemm_init};

static int ninemm_allowed [] = 
{
	1, 0, 0, 1, 0, 0, 1,
	0, 1, 0, 1, 0, 1, 0,
	0, 0, 1, 1, 1, 0, 0,
	1, 1, 1, 0, 1, 1, 1,
	0, 0, 1, 1, 1, 0, 0,
	0, 1, 0, 1, 0, 1, 0,
	1, 0, 0, 1, 0, 0, 1,

};

static int ninemm_mills [][3][3] = 
{
	/* triples of squares that form mills 
	   The four rotations of these will produce all the mills */
	{ {0, 0}, {3, 0}, {6, 0} },
	{ {1, 1}, {3, 1}, {5, 1} },
	{ {2, 2}, {3, 0}, {4, 2} },
	{ {0, 0}, {1, 1}, {2, 2} },
};

void ninemm_init ()
{
	game_white_string = "Blue";
	game_black_string = "Red";
	game_bg_pixmap = ninemm_bg_xpm;
	game_getmove = ninemm_getmove;
	game_reset_uistate = ninemm_reset_uistate;
	game_doc_about_status = STATUS_UNPLAYABLE;
	game_doc_about = 
		"Ninemm\n"
		"Two player game\n"
		"Status: Partially implemented\n"
		"URL: "GAME_DEFAULT_URL ("ninemm");
}

static int curx = -1, cury = -1;

// does the move (x, y) complete a mill for player
static gboolean makes_mill (byte *board, Player player, int x, int y)
{
	int i, j;
	int thepiece = player == WHITE ? NINEMM_WP : NINEMM_BP;
	for (i=x-2; i<=x; i++)
	{
		gboolean found = TRUE;
		if (i < 0 || i >= board_wid-2)
			continue;
		for (j=i; j<=i+2; j++)
			if (j != x && board [y * board_wid + j] != thepiece)
				found = FALSE;
		if (found) return TRUE;
	}
	
	for (i=y-2; i<=y; i++)
	{
		gboolean found = TRUE;
		if (i < 0 || i >= board_heit-2)
			continue;
		for (j=i; j<=i+2; j++)
			if (j != y && board [j * board_wid + x] != thepiece)
				found = FALSE;
		if (found) return TRUE;
	}

	return FALSE;
}

int ninemm_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte **movp, int ** rmovep)
{
	int val;
	static byte move[16];
	static int rmove[16];
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (!ninemm_allowed [y * board_wid + x]) return -1;
	if (curx < 0)
	{
		if (pos->board [y * board_wid + x] != NINEMM_EMPTY)
			return -1;
		if (makes_mill (pos->board, to_play, x, y))
		{
			curx = x;
			cury = y;
			rmove[0] = x;
			rmove[1] = y;
			rmove[2] = RENDER_REPLACE | 
				((to_play == WHITE ? NINEMM_WP : NINEMM_BP) << 8);
			rmove[3] = -1;
			*rmovep = rmove;
			return 0;
		}
		else
		{
			move[0] = x;
			move[1] = y;
			move[2] = (to_play == WHITE ? NINEMM_WP : NINEMM_BP);
			move[3] = -1;
			*movp = move;
			return 1;			
		}
	}
	if (pos->board [y * board_wid + x] != (to_play == WHITE ? NINEMM_BP : NINEMM_WP))
		return -1;
	rmove[0] = curx;
	rmove[1] = cury;
	rmove[2] = 0;
	rmove[3] = -1;
	*rmovep = rmove;
	move[0] = x;
	move[1] = y;
	move[2] = 0;
	move[3] = curx;
	move[4] = cury;
	move[5] = (to_play == WHITE ? NINEMM_WP : NINEMM_BP);
	move[6] = -1;
	*movp = move;	
	curx = -1;
	cury = -1;
	return 1;
}

void ninemm_reset_uistate ()
{
	curx = cury = -1;
}
