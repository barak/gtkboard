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
#include <gdk/gdkkeysyms.h>


#include "game.h"
#include "../pixmaps/alpha.xpm"
#include "flwords.h"


#define FLW_CELL_SIZE 36

#define FLW_NUM_PIECES 26

#define FLW_BOARD_WID 4
#define FLW_BOARD_HEIT 6

#define FLW_LEN 4

char flw_colors[9] = {0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0, 0, 0xff};
char flw_highlight_colors[9] = {0xff, 0, 0, 0, 0, 0, 0, 0, 0};

static char **flw_pixmaps [] = 
{
	char_A_grey_36_36_xpm,
	char_B_grey_36_36_xpm,
	char_C_grey_36_36_xpm,
	char_D_grey_36_36_xpm,
	char_E_grey_36_36_xpm,
	char_F_grey_36_36_xpm,
	char_G_grey_36_36_xpm,
	char_H_grey_36_36_xpm,
	char_I_grey_36_36_xpm,
	char_J_grey_36_36_xpm,
	char_K_grey_36_36_xpm,
	char_L_grey_36_36_xpm,
	char_M_grey_36_36_xpm,
	char_N_grey_36_36_xpm,
	char_O_grey_36_36_xpm,
	char_P_grey_36_36_xpm,
	char_Q_grey_36_36_xpm,
	char_R_grey_36_36_xpm,
	char_S_grey_36_36_xpm,
	char_T_grey_36_36_xpm,
	char_U_grey_36_36_xpm,
	char_V_grey_36_36_xpm,
	char_W_grey_36_36_xpm,
	char_X_grey_36_36_xpm,
	char_Y_grey_36_36_xpm,
	char_Z_grey_36_36_xpm,
};

SCORE_FIELD flw_score_fields[] = {SCORE_FIELD_USER, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *flw_score_field_names[] = {"User", "Time", "Date", NULL};

static int flw_wordcmp (const void *p1, const void *p2)
{
	return strcmp ((char *)p1, *(char **)p2);
}

void flw_find_chain (char chain[FLW_LEN+1][FLW_LEN+1])
{
	int i, j, idx, found;
	int done[FLW_LEN];
	char word[FLW_LEN+1];
	do
	{
		strncpy (word, flwords[random() % num_flwords], FLW_LEN+1);
		sprintf (chain[0], "%s", word);
		for (i=0, memset (done, 0, FLW_LEN * sizeof (int)); i<FLW_LEN; i++)
		{
			char orig;
			do {idx = random() % FLW_LEN;} while (done[idx]); done[idx] = 1;
			orig= word[idx];
			for (j=0, found=0; j<100; j++)
			{
				if ((word[idx] = random() % 26 + 'a') == orig) continue;
				if (bsearch (word, flwords, num_flwords, sizeof (flwords[0]), flw_wordcmp))
				{
					sprintf (chain[i+1], "%s", word);
					found = 1; 
					break;
				}
			}
			if (!found)
				break;
		}
	}
	while (!found);
}

static void flw_init ();
int flw_getmove (Pos *, int, int, GtkboardEventType, Player, byte **, int **);
int flw_getmove_kb (Pos *, int, byte **, int **);
void flw_free ();
ResultType flw_who_won (Pos *, Player , char **);

Game Flw = { FLW_CELL_SIZE, FLW_BOARD_WID, FLW_BOARD_HEIT, 
	FLW_NUM_PIECES, 
	flw_colors, NULL, flw_pixmaps, 
	"Four Letter Words", 
	"Word games",
	flw_init};


static char flw_chain[FLW_LEN+1][FLW_LEN+1];


static void flw_set_init_pos (Pos *pos)
{
	int i, j;
	for (i=0; i<board_wid * board_heit; i++)
		pos->board[i] = 0;
	flw_find_chain (flw_chain);
	for (i=0; i<FLW_LEN; i++)
	{
		pos->board[(board_heit - 1) * board_wid + i] = flw_chain[0][i] - 'a' + 1;
		pos->board[i] = flw_chain[FLW_LEN][i] - 'a' + 1;
	}
}

static void flw_init ()
{
	game_single_player = TRUE;
	game_set_init_pos = flw_set_init_pos;
	game_draw_cell_boundaries = TRUE;
	game_free = flw_free;
	game_getmove = flw_getmove;
	game_getmove_kb = flw_getmove_kb;
	game_who_won = flw_who_won;
	game_start_immediately = TRUE;
	game_scorecmp = game_scorecmp_def_iscore;
	game_score_fields = flw_score_fields;
	game_score_field_names = flw_score_field_names;
	game_highlight_colors = flw_highlight_colors;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Four letter words\n"
		"Single player game\n"
		"Status: Partially implemented (playable)\n"
		"URL: "GAME_DEFAULT_URL("flw");
	game_doc_rules = 
		" This is a simple game in which the objective is to change the word at the top to the word at the bottom by changing one letter at a time. All intermediate words must be legal.\n"
		" To start playing, hit enter. This will make a copy of the top word on the second row. Click on the letter you want to change and change it by typing the new letter. Now hit enter. Repeat until all the rows are filled.";
}

static int flw_curx = -1, flw_cury = -1;

ResultType flw_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32] = "You won.";
	int i;
	for (i=0; i<FLW_LEN; i++)
		if (pos->board [i] != pos->board [board_wid + i])
			return RESULT_NOTYET;
	*commp = comment;
	return RESULT_WON;
}

static int flw_get_cur_row (byte *board)
{
	int j;
	for (j = board_heit - 1; j>=0 && board[j * board_wid]; j--)
		;
	if (j < 0) j = 0;
	return j+1;
}

void flw_get_render (Pos *pos, byte *move, int type, int **rmovp)
{
	static int rmove[7];
	int *rp = rmove;
	int i, j;
	int oldx = 0;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (pos->render [j * board_wid + i] == RENDER_HIGHLIGHT1)
		{
			assert (rp == rmove);
			*rp++ = i;
			*rp++ = j;
			*rp++ = 0;
			oldx = i;
		}
	*rp++ = oldx;
	*rp++ = flw_get_cur_row (pos->board);
	*rp++ = RENDER_HIGHLIGHT1;
	*rp++ = -1;
	*rmovp = rmove;
}


int flw_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp, int ** rmovep)
{
	static int rmove[7];
	int *rp = rmove;
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (y == 0 || y == board_heit - 1) return 0;
	if (pos->board [y * board_wid + x] == 0) return 0;
	if (y != 1 && pos->board [(y-1) * board_wid + x] != 0) return 0;
	if (flw_curx == x && flw_cury == y) return 0;
	*rp++ = flw_curx; *rp++ = flw_cury; *rp++ = 0;
	*rp++ = x; *rp++ = y; *rp++ = RENDER_HIGHLIGHT1;
	*rp++ = -1;
	*rmovep = rmove;
	flw_curx = x;
	flw_cury = y;
	return 0;
}

int flw_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp)
{
	static int rmove[7];
	int *rp = rmove;
	static byte move[10];
	byte *mp = move;
	int i, j, cury;
	char word[FLW_LEN+1], prevword[FLW_LEN+1];
	if (flw_cury >= 0)
	{
		if (!(pos->board [flw_cury * board_wid] &&
				(pos->board [(flw_cury-1) * board_wid] == 0 || flw_cury == 1)))
		{
			assert (0);
			// find the current row
			/*for (j = board_heit - 1; j>=0 && pos->board[j * board_wid]; j--)
				;
			if (j < 0) j = 0;
			flw_cury = j+1;
			flw_curx = 0;*/
			flw_cury = flw_get_cur_row (pos->board);
			flw_curx = 0;
		}
	}
	if (key == GDK_Return)
	{
		if ((cury = flw_cury) < 0)
			cury = board_heit - 1;
		for (i=0; i<FLW_LEN; i++)
		{
			word[i] = pos->board [cury * board_wid + i] + 'a' - 1;
			if (flw_cury >= 0)
				prevword[i] = pos->board [(cury+1) * board_wid + i] + 'a' - 1;
		}
		word[FLW_LEN] = '\0';
		prevword[FLW_LEN] = '\0';
		if (!bsearch (word, flwords, num_flwords, sizeof (flwords[0]), flw_wordcmp))
			return -1;
		if (flw_cury >= 0)
		{
			int diff_cnt_prev = 0, diff_cnt_last = 0;
			for (i=0; i<FLW_LEN; i++)
			{
				if (word[i] != prevword[i])
					diff_cnt_prev++;
				if (word[i] != flw_chain[FLW_LEN+1][i])
					diff_cnt_last++;
			}
			if (diff_cnt_prev != 1)
				return -1;
			if (cury == 1 && diff_cnt_last != 1)
				return -1;	
		}
		if (flw_cury >= 0)
		{
			*rp++ = flw_curx; *rp++ = flw_cury; *rp++ = 0;
		}
		if (flw_cury < 0)
		{
			flw_curx = 0;
			flw_cury = board_heit - 1;
		}
		flw_cury --;
		*rp++ = flw_curx; *rp++ = flw_cury; *rp++ = RENDER_HIGHLIGHT1;
		*rp++ = -1;
		*rmovp = rmove;
		for (i=0; i<FLW_LEN; i++)
		{
			*mp++ = i; *mp++ = flw_cury; *mp++ = word[i] - 'a' + 1;
		}
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	if (flw_cury < 0 || flw_cury == board_heit - 1)
		return -1;
	if (key == GDK_Right)
	{
		*rp++ = flw_curx; *rp++ = flw_cury; *rp++ = 0;
		if (++flw_curx == FLW_LEN) flw_curx = 0;
		*rp++ = flw_curx; *rp++ = flw_cury; *rp++ = RENDER_HIGHLIGHT1;
		*rp++ = -1;
		*rmovp = rmove;
		return 0;
	}
	if (key == GDK_Left)
	{
		*rp++ = flw_curx; *rp++ = flw_cury; *rp++ = 0;
		if (--flw_curx < 0) flw_curx = FLW_LEN - 1;
		*rp++ = flw_curx; *rp++ = flw_cury; *rp++ = RENDER_HIGHLIGHT1;
		*rp++ = -1;
		*rmovp = rmove;
		return 0;
	}
	if (key >= GDK_A && key <= GDK_Z)
		key = key + GDK_a - GDK_A;
	if (key >= GDK_a && key <= GDK_z)
	{
			*mp++ = flw_curx; *mp++ = flw_cury; *mp++ = key - GDK_a + 1;
			*mp++ = -1;
			*movp = move;
			return 1;
	}
	return -1;
}

void flw_free ()
{
	flw_curx = flw_cury = -1;
}

