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


#define WORDTRIS_CELL_SIZE 36

#define WORDTRIS_NUM_PIECES 26

#define WORDTRIS_BOARD_WID 4
#define WORDTRIS_BOARD_HEIT 12

#define WORDTRIS_LEN 4

#define WORDTRIS_EMPTY 0

char wordtris_colors[9] = {0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0, 0, 0xff};

static char **wordtris_pixmaps [] = 
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

typedef struct
{
	int score;
} Wordtris_state;


static int wordtris_wordcmp (const void *p1, const void *p2)
{
	return strcmp ((char *)p1, *(char **)p2);
}

static void wordtris_init ();
int wordtris_getmove (Pos *, int, int, GtkboardEventType, Player, byte **);
int wordtris_getmove_kb (Pos *, int, Player, byte **);
void wordtris_free ();
ResultType wordtris_who_won (Pos *, Player , char **);
int wordtris_animate (Pos *pos, byte **movp);
static void *wordtris_newstate (Pos *, byte *);

Game Wordtris = { WORDTRIS_CELL_SIZE, WORDTRIS_BOARD_WID, WORDTRIS_BOARD_HEIT, 
	WORDTRIS_NUM_PIECES, 
	wordtris_colors, NULL, wordtris_pixmaps, 
	"Wordtris", 
	wordtris_init};

/* This list was produced using

  $  egrep -x [a-z]{4} /usr/share/dict/words | perl -e 'while (<>) {foreach $i ((0 .. 3)) {$count{substr($_,$i,1)}++}} foreach $c (("a" .. "z")) {print $count{$c}, "\n"}'

  Perl totally rocks
*/

static float charcounts[26] = 
{
	629,
	223,
	205,
	302,
	746,
	152,
	186,
	182,
	395,
	30,
	167,
	460,
	234,
	338,
	503,
	272,
	6,
	395,
	622,
	404,
	276,
	63,
	173,
	24,
	125,
	28,
};


static void wordtris_setinitpos (Pos *pos)
{
	int i, j;
	const char *word = flwords [rand() % num_flwords];
	for (i=0; i<board_wid * board_heit; i++)
		pos->board[i] = 0;
	for (i=0; i<WORDTRIS_LEN; i++)
		pos->board [i] = word[i] - 'a' + 1;
}

static void wordtris_init ()
{
	game_single_player = TRUE;
	game_setinitpos = wordtris_setinitpos;
	game_free = wordtris_free;
	game_getmove = wordtris_getmove;
	game_getmove_kb = wordtris_getmove_kb;
	game_who_won = wordtris_who_won;
	game_scorecmp = game_scorecmp_def_dscore;
	game_animation_time = 2000;
	game_animate =  wordtris_animate;
	game_stateful = TRUE;
	game_state_size = sizeof (Wordtris_state);
	game_newstate = wordtris_newstate;
	game_doc_about = 
		"Wordtris\n"
		"Single player game\n"
		"Status: Partially implemented (playable)\n"
		"URL: "GAME_DEFAULT_URL("wordtris");
	game_doc_rules = 
		" Wordtris rules\n\n"
		"Press Ctrl+G to start the game.\n\n"
		" Click one of the letters of the word at the bottom and type the letter on one of the falling blocks to change that letter to the new letter. The new word must be legal.\n"
		"\n"
		"You get a point for every new word you make. Currently the game ends when a block falls to the bottom row. In the future this will change so that you have a fixed number of lives.";
}

static int wordtris_curx = 0, wordtris_cury = 0;

ResultType wordtris_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i;
	*commp = comment;
	for (i=0; i<WORDTRIS_LEN; i++)
		if (pos->board [board_wid + i] != WORDTRIS_EMPTY)
		{
			snprintf (comment, 32, "Game over. Score: %d", 
					pos->state ? ((Wordtris_state *)pos->state)->score: 0);
			return RESULT_WON;
		}
	snprintf (comment, 32, "Score: %d", 
					pos->state ? ((Wordtris_state *)pos->state)->score: 0);
	return RESULT_NOTYET;
}

void *wordtris_newstate (Pos *pos, byte *move)
{
	static Wordtris_state state;
	int i, score = 0;
	for (i=0; move[3*i] >= 0; i++)
	{
		if (move[3*i+1] == 0)
			score = 1;
	}
	state.score = (pos->state ? ((Wordtris_state *)pos->state)->score : 0) + score;
	return &state;
}

int wordtris_getmove (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, 
		byte **movp)
{
	if (type != GTKBOARD_BUTTON_RELEASE)
		return 0;
	if (y != 0) return 0;
	wordtris_curx = x;
	return 0;
}

gboolean wordtris_findletter (byte *board, int letter, int *x, int *y)
{
	int i, j;
	int minx = -1, miny = board_heit;
	for (i=0; i < WORDTRIS_LEN; i++)
	{
		for (j=1; j<board_heit; j++)
		{
			int val = board [j * board_wid + i];
			if (val == letter)
			{
				if (j < miny)
				{
					minx = i;
					miny = j;
				}
			}
#ifdef ONLY_LOWEST
			else if (val != WORDTRIS_EMPTY)
				break;
#endif
		}
	}
	if (miny >= board_heit) return FALSE;
	*x = minx;
	*y = miny;
	return TRUE;
}

int wordtris_getmove_kb (Pos *pos, int key, Player glob_to_play, byte **movp)
{
	static byte move[10];
	byte *mp = move;
	if (key == GDK_Right)
	{
		if (++wordtris_curx == WORDTRIS_LEN) wordtris_curx = 0;
		return 0;
	}
	if (key == GDK_Left)
	{
		if (--wordtris_curx < 0) wordtris_curx = WORDTRIS_LEN - 1;
		return 0;
	}
	if (key >= GDK_A && key <= GDK_Z)
		key = key + GDK_a - GDK_A;
	if (key >= GDK_a && key <= GDK_z)
	{
		int i, x, y;
		char word [WORDTRIS_LEN+1];
		gboolean found = wordtris_findletter (pos->board, key - GDK_a + 1, &x, &y);
		if (!found) return -1;
		if (wordtris_curx < 0) return -1;
		for (i=0; i<WORDTRIS_LEN; i++)
			word[i] = pos->board [i] - 1 + 'a';
		word [WORDTRIS_LEN] = '\0';
		if (word [wordtris_curx] == key - GDK_a + 'a') return -1;
		word [wordtris_curx] = key - GDK_a + 'a';
		if (!bsearch (word, flwords, num_flwords, sizeof (flwords[0]), wordtris_wordcmp))
			return -1;
		*mp++ = wordtris_curx; *mp++ = wordtris_cury; *mp++ = key - GDK_a + 1;
		*mp++ = x; *mp++ = y; *mp++ = WORDTRIS_EMPTY;
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	return -1;
}

void wordtris_free ()
{
	wordtris_curx = wordtris_cury = 0;
}

int wordtris_get_rand_char ()
{
	int i, sum, thresh;
	for (i=0, sum=0; i<26; i++)
		sum += charcounts[i];
	thresh = rand() % sum;
	for (i=0, sum=0; i<26; i++)
		if ((sum += charcounts[i]) > thresh)
			return i;
	assert (0);
}

int wordtris_animate (Pos *pos, byte **movp)
{
	static byte move[1024];
	int i, j;
	byte *mp = move;
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=board_heit-1; j>=2; j--)
	{
		int val;
		if ((val = pos->board [j * board_wid + i]) != WORDTRIS_EMPTY)
		{
			*mp++ = i;
			*mp++ = j;
			*mp++ = WORDTRIS_EMPTY;
			*mp++ = i;
			*mp++ = j-1;
			*mp++ = val;
		}
	}
	while (1)
	{
		i = rand() % WORDTRIS_LEN;
		if (pos->board [(board_heit - 1) * board_wid + i])
			continue;
		*mp++ = i;
		*mp++ = board_heit - 1;
		*mp++ = wordtris_get_rand_char() + 1;
		break;
	}
	*mp = -1;
	*movp = move;
	return 1;
}



