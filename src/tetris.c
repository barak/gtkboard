#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>

#include "game.h"
#include "aaball.h"

#define TETRIS_CELL_SIZE 20
#define TETRIS_NUM_PIECES 31

#define TETRIS_BOARD_WID 10
#define TETRIS_BOARD_HEIT 22

char tetris_colors[6] = {50, 50, 50, 50, 50, 50};

int * tetris_initpos = NULL;

void tetris_init ();

Game Tetris = { TETRIS_CELL_SIZE, TETRIS_BOARD_WID, TETRIS_BOARD_HEIT, 
	TETRIS_NUM_PIECES,
	tetris_colors,  NULL, NULL, "Tetris", tetris_init};

SCORE_FIELD tetris_score_fields[] = {SCORE_FIELD_USER, SCORE_FIELD_SCORE, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *tetris_score_field_names[] = {"User", "Score", "Date", NULL};

#define TETRIS_EMPTY 0
#define TETRIS_BRICK_4 1 
#define TETRIS_BRICK_22 2 
#define TETRIS_BRICK_121A 3 
#define TETRIS_BRICK_121B 4
#define TETRIS_BRICK_T 5 
#define TETRIS_BRICK_LA 6
#define TETRIS_BRICK_LB 7
#define TETRIS_BRICK_INACTIVE 8
#define TETRIS_BRICK_DYING 9
#define TETRIS_BRICK_MASK 15
#define TETRIS_BRICK_MOTION_MASK 16

static void tetris_setinitpos (Pos *pos);
static char ** tetris_get_pixmap (int idx, int color);
static int tetris_getmove_kb (Pos *cur_pos, int key, Player glob_to_play, 
		byte **move);
static int tetris_animate (Pos *pos, byte **movp);
static ResultType tetris_who_won (Pos *pos, Player to_play, char **commp);
static void *tetris_newstate (Pos *, byte *);
static void tetris_free ();

typedef struct
{
	int score;
}
Tetris_state;

static int num_bricks = 0;
static int level = 1;
static int anim_time_left = 0;
static int anim_time_def = 0;

void tetris_init ()
{
	game_single_player = TRUE;
	game_setinitpos = tetris_setinitpos;
	game_get_pixmap = tetris_get_pixmap;
	game_getmove_kb = tetris_getmove_kb;
	game_animation_time = 50;
	game_animate = tetris_animate;
	game_who_won = tetris_who_won;
	game_stateful = TRUE;
	game_state_size = sizeof (Tetris_state);
	game_newstate = tetris_newstate;
	game_scorecmp = game_scorecmp_def_dscore;
	game_allow_back_forw = FALSE;
	game_scorecmp = game_scorecmp_def_dscore;
	game_score_fields = tetris_score_fields;
	game_score_field_names = tetris_score_field_names;
	game_draw_cell_boundaries = TRUE;
	game_free = tetris_free;
}

void tetris_free ()
{
	num_bricks = 0;
	level = 1;
	anim_time_left = 0;
}

void *tetris_newstate (Pos *pos, byte *move)
	// TODO: implement points for falling
{
	int i, score = 0;
	static Tetris_state state;
	int linepts[5] = {0, 40, 100, 300, 1200};
	for (i=0; move[3*i] >= 0; i++)
		if (move[3*i+2] == 0)
			score++;
	score /= board_wid;
	score = linepts [score];
	state.score = (pos->state ? ((Tetris_state *)pos->state)->score : 0) + score;
	return &state;
}

int tetris_game_over (byte *board)
{
	int i;
	for (i=0; i<board_wid; i++)
		if (board[(board_heit - 2) * board_wid + i] == TETRIS_BRICK_INACTIVE)
			return 1;
	return 0;
}


ResultType tetris_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i;
	int over = tetris_game_over (pos->board);
	snprintf (comment, 32, "%s %s %d", 
			over ? "Game over. " : "", "Score:",
			pos->state ? ((Tetris_state *)pos->state)->score : 0);
	*commp = comment;
	return over ? RESULT_MISC : RESULT_NOTYET;
}

int tetris_fall (byte *pos, byte **movp, int height)
{
	static byte move[32];
	byte *mp = move;
	int moving = 0;
	int canfall = 1;
	int i, j, k;
	for (j=0; j<board_heit; j++)
	for (i=0; i<board_wid; i++)
	{
		if (pos [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
		{
			int tmp;
			if (height > j) height = j;
			moving = 1;
			if (j == 0) canfall = 0;
			for (k=1; k<=height; k++)
			{
				tmp = pos [(j-k) * board_wid + i];
				if (tmp != 0 && !(tmp & TETRIS_BRICK_MOTION_MASK))
					canfall = 0;
			}
		}
	}
	if (moving && canfall)
	{
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
		{
			if (j < board_heit - height)
			if (!(pos [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK) 
				&& (pos [(j+height) * board_wid + i] & TETRIS_BRICK_MOTION_MASK))
			{
				*mp++ = i; *mp++ = j; *mp++ = pos [(j+height) * board_wid + i];
			}
			if (pos [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK 
				&& (j >= board_heit - height || 
					!(pos [(j+height) * board_wid + i] & TETRIS_BRICK_MOTION_MASK)))
			{
				*mp++ = i; *mp++ = j; *mp++ = 0;
			}
		}
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	return -1;
}

int tetris_animate (Pos *pos, byte **movp)
{
	static byte move[1024];
	static int count = 0;
	byte *mp = move;
	byte *board = pos->board;
	int i, j;
	int level = num_bricks / 20 + 1;
	if (level > 10) level = 10;
	anim_time_def = (12 - level) / 2;
	if (anim_time_left) 
	{
		anim_time_left--;
		return 0;
	}

	anim_time_left = 12 - level;
	if (tetris_fall(board, movp, 1) > 0)
		return 1;

	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
		{
			*mp++ = i; *mp++ = j; 
			*mp++ = TETRIS_BRICK_INACTIVE;
		}
	for (j=0; j<board_heit; j++)
	{
		int nfull = 0;
		while (nfull + j < board_heit)
		{
			int full = 1;
			for (i=0; i<board_wid; i++) 
				if (board [(j+nfull) * board_wid + i] == 0) { full = 0; break; }
			if (!full) break;
			nfull++;
		}
		if (nfull > 0)
		{
			for (; j+nfull<board_heit; j++)
			{
				for (i=0; i<board_wid; i++) 
				{
					if (board [j * board_wid + i] != 
							board [(j+nfull) * board_wid + i])
					{ *
						mp++ = i; *mp++ = j; 
						//*mp++ = board [(j+nfull) * board_wid + i]; 
						if ( board [(j+nfull) * board_wid + i] == 0)
							*mp++ = 0;
						else *mp++ =  TETRIS_BRICK_INACTIVE;
					}
				}
			}
			for (; j<board_heit; j++)
			{
				int empty = 1;
				for (i=0; i<board_wid; i++) 
				{
					if (board [j * board_wid + i] != 0)
					{
						*mp++ = i; *mp++ = j; *mp++ = 0;
					}
					if (board [j * board_wid + i] != 0) empty = 0;
				}
				if (empty) break;
			}
			*mp++ = -1;
			*movp = move;
			return 1;
		}
	}
	
	num_bricks ++;

	{
	int idx;
	byte *saved_m = mp;
	// This depends on the #defs!!
	// FIXME: change the shapes so that no brick is more than 2 rows tall
	byte shapes [][4][2] = {
		{ { 3, 1} , {4, 1}, {5, 1}, {6, 1} },
		{ { 4, 1} , {5, 1}, {4, 2}, {5, 2} },
		{ { 4, 1} , {4, 2}, {5, 2}, {5, 3} },
		{ { 5, 2} , {5, 1}, {4, 2}, {4, 3} },
		{ { 4, 1} , {5, 1}, {6, 1}, {5, 2} },
		{ { 4, 3} , {5, 3}, {4, 2}, {4, 1} },
		{ { 4, 3} , {5, 3}, {5, 2}, {5, 1} },
	};

	idx = random() % 7;
	for (i=0; i<4; i++)
	{
		*mp++ = shapes[idx][i][0]; 
		*mp++ = board_heit - shapes[idx][i][1]; 
		if (board [mp[-1] * board_wid + mp[-2]])
		{
			// we need to return the move up to the previous stage
			*saved_m++ = -1;
			*movp = move;
			return 1;
		}
		*mp++ = (idx+1) | TETRIS_BRICK_MOTION_MASK;
	}
	*mp++ = -1;
	*movp = move;
	return 1;
	}
}


int tetris_getmove_kb (Pos *pos, int key, Player glob_to_play, byte **movp)
{
	static byte move[32];
	byte *mp = move;
	int incx;
	int i, j;
	byte *board = pos->board;

	if (key == ' ')
	{
		for (i = board_heit; i>0; i--)
			if (tetris_fall(board, movp, i) > 0)
			{
				anim_time_left = anim_time_def;
				return 1;
			}
		return -1;
	}
	
	if (key == GDK_Down)
	{
		int retval = tetris_fall(board, movp, 1);
		if (retval > 0) anim_time_left = anim_time_def;
		return retval;
	}
	
	if (key == GDK_Up)
	{
		int sumx = 0, sumy = 0, k = 0, incy;
		int thebrick = 0;
		byte newboard [4][2];
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
			{ 
				sumx += i; sumy += j; 
				thebrick = board [j * board_wid + i] | TETRIS_BRICK_MOTION_MASK;
				if (j == 0 || ((board [(j-1) * board_wid + i] != 0) 
					&& !(board [(j-1) * board_wid + i] & TETRIS_BRICK_MOTION_MASK)))
					return -1;
			}
		if (sumy == 0) return -1;
		sumx += 3; incy = sumy % 4 > 0 ? 1 : 0; sumx /= 4; sumy /= 4;
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
			{
				newboard[k][0] = sumx + sumy - j;
				newboard[k][1] = sumy - sumx + i + incy;
				if (newboard[k][0] < 0 || newboard[k][1] < 0 || 
						newboard[k][0] >= board_wid || newboard[k][1] >= board_heit)
					return -1;
				if (board [newboard [k][1] * board_wid + newboard[k][0]]
						== TETRIS_BRICK_INACTIVE)
					return -1;
				k++;
			}
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
		{
			if (!(board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK))
			{
				int found = 0;
				for (k=0; k<4; k++) 
					if (newboard[k][0] == i && newboard[k][1] == j)
					{ found = 1; break; }
				if (found)
				{
					*mp++ = i; *mp++ = j; *mp++ = thebrick;
				}
			}
			if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
			{
				int found = 0;
				for (k=0; k<4; k++) 
					if (newboard[k][0] == i && newboard[k][1] == j)
					{ found = 1; break; }
				if (!found)
				{
					*mp++ = i; *mp++ = j; *mp++ = 0;
				}
			}
		}
		*mp++ = -1;
		*movp = move;
		return 1;
	}
	switch (key)
	{
		case GDK_Left: incx = 1; break;
		case GDK_Right: incx = -1; break;
		default: return -1;
	}
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK)
		{
			if (i - incx < 0 || i - incx >= board_wid) return -1;
			if (board [j * board_wid + i - incx] != 0 && 
					!(board [j * board_wid + i - incx] & TETRIS_BRICK_MOTION_MASK))
				return -1;
		}
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (i+incx >= 0 && i+incx < board_wid)
		if (!(board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK) 
			&& (board [j * board_wid + i+incx] & TETRIS_BRICK_MOTION_MASK))
		{
			*mp++ = i; *mp++ = j; *mp++ = board [j * board_wid + i+incx];
		}
		if (board [j * board_wid + i] & TETRIS_BRICK_MOTION_MASK 
			&& (i+incx < 0 || i+incx >= board_wid || 
				!(board [j * board_wid + i+incx] & TETRIS_BRICK_MOTION_MASK)))
		{
			*mp++ = i; *mp++ = j; *mp++ = 0;
		}
	}
	*mp++ = -1;
	*movp = move;
	return 1;
}

void tetris_setinitpos (Pos *pos)
{
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		pos->board [j * board_wid + i] = 0;
}


char ** tetris_get_pixmap (int idx, int color)
{
	int i;
	static char *pixmap [TETRIS_CELL_SIZE + 2];
	char *line = "                    ";
	//pixmap = g_new(char *, TETRIS_CELL_SIZE + 2);
	pixmap[0] = "20 20 1 1";
	switch (idx & TETRIS_BRICK_MASK)
	{
		case TETRIS_BRICK_4: pixmap[1] = "  c blue"; break;
		case TETRIS_BRICK_22: pixmap[1] = "  c red"; break;
		case TETRIS_BRICK_121A: pixmap[1] = "  c yellow"; break;
		case TETRIS_BRICK_121B: pixmap[1] = "  c magenta"; break;
		case TETRIS_BRICK_T: pixmap[1] = "  c green"; break;
		case TETRIS_BRICK_LA: pixmap[1] = "  c pink"; break;
		case TETRIS_BRICK_LB: pixmap[1] = "  c orange"; break;
		case TETRIS_BRICK_INACTIVE: pixmap[1] = "  c gray"; break;
		default: return NULL;
	}
	for (i=0; i<TETRIS_CELL_SIZE; i++) pixmap[2+i] = line;
	return pixmap;
}

