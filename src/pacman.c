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
#include <math.h>
#include <gdk/gdkkeysyms.h>

#include "game.h"
#include "aaball.h"

#define PACMAN_CELL_SIZE 25
#define PACMAN_NUM_PIECES 48

#define PACMAN_BOARD_WID 26
#define PACMAN_BOARD_HEIT 25

char pacman_colors[6] = {100, 150, 200, 100, 150, 200};

int * pacman_init_pos = NULL;

//#define PACMAN_WALL 1
#define PACMAN_FOOD 1
#define PACMAN_POWER 2
#define PACMAN_EMPTY 3
#define PACMAN_IS_EDIBLE(x) (x>=1 && x<=3)
#define PACMAN_PAC_MIN 28
#define PACMAN_PAC_MAX 32
#define PACMAN_PAC_UP 28
#define PACMAN_PAC_DOWN 29
#define PACMAN_PAC_RIGHT 30
#define PACMAN_PAC_LEFT 31
#define PACMAN_IS_PAC(x) ((x) >= PACMAN_PAC_MIN && (x) < PACMAN_PAC_MAX)
//#define PACMAN_GHOST_MASK 4
#define PACMAN_IS_GHOST(x) ((x) >= 4 && x < 20)
#define PACMAN_GET_GHOST(x) ((x)-(x)%4)
#define PACMAN_GET_GHOST_NUM(x) (PACMAN_GET_GHOST(x)/4-1)
/*#define PACMAN_GHOST_INKY 5
#define PACMAN_GHOST_PINKY 6
#define PACMAN_GHOST_BLINKY 7
#define PACMAN_GHOST_SUE 8*/
//#define PACMAN_IS_GHOST(x) ((x) >= PACMAN_GHOST_INKY && (x) <= PACMAN_GHOST_SUE)

#define PACMAN_WALL_MASK 32
#define PACMAN_WALL_UP    8
#define PACMAN_WALL_DOWN  4
#define PACMAN_WALL_LEFT  2
#define PACMAN_WALL_RIGHT 1

int pacman_maze[PACMAN_BOARD_HEIT][PACMAN_BOARD_WID] = 
{
	{32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32},
	{32, 2, 1, 1, 1, 1, 1,32, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,32, 1, 1, 1, 1, 1, 2,32},
	{32, 1,32,32,32,32, 1,32, 1,32,32,32,32,32,32,32,32, 1,32, 1,32,32,32,32, 1,32},
	{32, 1,32,32,32,32, 1,32, 1,32,32,32,32,32,32,32,32, 1,32, 1,32,32,32,32, 1,32},
	{32, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,32},
	{32,32,32, 1,32,32, 1,32,32,32,32, 1,32,32, 1,32,32,32,32, 1,32,32, 1,32,32,32},
	{32,32,32, 1,32,32, 1, 1, 1, 1, 1, 1,32,32, 1, 1, 1, 1, 1, 1,32,32, 1,32,32,32},
	{32, 1, 1, 1,32,32,32,32, 1,32,32,32,32,32,32,32,32, 1,32,32,32,32, 1, 1, 1,32},
	{32, 1, 1, 1,32,32,32,32, 1,32,32,32,32,32,32,32,32, 1,32,32,32,32, 1, 1, 1,32},
	{32,32,32, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,32,32,32},
	{32,32,32, 1,32,32,32,32, 1,32,32,32,32,32,32, 1,32, 1,32,32,32,32, 1,32,32,32},
	{32,32,32, 1,32,32,32,32, 1,32, 7, 3,15, 3, 3, 3,32, 1,32,32,32,32, 1,32,32,32},
	{32,32,32, 1,32,32, 1, 1, 1,32, 3,11, 3,19, 3, 3,32, 1, 1, 1,32,32, 1,32,32,32},
	{32,32,32, 1,32,32, 1,32, 1,32,32,32,32,32,32,32,32, 1,32, 1,32,32, 1,32,32,32},
	{32, 1, 1, 1, 1, 1, 1,32, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,32, 1, 1, 1, 1, 1, 1,32},
	{32,32,32, 1,32,32,32,32,32,32,32, 1,32,32, 1,32,32,32,32,32,32,32, 1,32,32,32},
	{32,32,32, 1, 1, 1, 1, 1, 1, 1, 1, 1,32,32, 1, 1, 1, 1, 1, 1, 1, 1, 1,32,32,32},
	{32,32,32, 1,32,32,32,32, 1,32,32,32,32,32,32,32,32, 1,32,32,32,32, 1,32,32,32},
	{32,32,32, 1,32,32,32,32, 1,32,32,32,32,32,32,32,32, 1,32,32,32,32, 1,32,32,32},
	{32, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,28, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,32},
	{32, 1,32,32,32, 1,32,32,32,32,32, 1,32,32, 1,32,32,32,32,32, 1,32,32,32, 1,32},
	{32, 1,32,32,32, 1,32,32, 1, 1, 1, 1,32,32, 1, 1, 1, 1,32,32, 1,32,32,32, 1,32},
	{32, 1,32,32,32, 1,32,32, 1,32,32,32,32,32,32,32,32, 1,32,32, 1,32,32,32, 1,32},
	{32, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,32},
	{32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32},
};

static char * brown_square_25_xpm[]=
{
"25 25 2 1",
"  c none",
". c #443333",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",
".........................",

};


void pacman_init ();

Game Pacman = { PACMAN_CELL_SIZE, PACMAN_BOARD_WID, PACMAN_BOARD_HEIT, 
	PACMAN_NUM_PIECES,
	pacman_colors,  NULL, NULL, "Pacman", "Arcade", pacman_init};


static void pacman_set_init_pos (Pos *pos);
static char ** pacman_get_pixmap (int idx, int color);
static int pacman_getmove_kb (Pos *cur_pos, int key, byte **move, int **);
static int pacman_animate (Pos *pos, byte **movp);


void pacman_init ()
{
	game_single_player = TRUE;
	game_set_init_pos = pacman_set_init_pos;
	game_get_pixmap = pacman_get_pixmap;
	game_getmove_kb = pacman_getmove_kb;
	game_animation_time = 100;
	game_animate = pacman_animate;
	game_doc_about_status = STATUS_UNPLAYABLE;
	game_doc_about = 
		"Pacman\n"
		"Single player game\n"
		"Status: Partially implemented (currently unplayable)\n"
		"URL: "GAME_DEFAULT_URL ("pacman");
}

void pacman_get_cur_pos (byte *pos, int *x, int *y)
{
	int i, j;
	for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			if (PACMAN_IS_PAC(pos [j * board_wid + i]))
			{ *x = i; *y = j; return;
			}
}

static short dist[PACMAN_BOARD_WID][PACMAN_BOARD_HEIT];

// FIXME: avoid recursion
void pacman_recursive_dist (byte *pos, int x, int y, int val)
{
	if (x < 0 || y < 0 || x >= board_wid || y >= board_heit) return;
	if (pos[y * board_wid + x] & PACMAN_WALL_MASK) return;
	if (dist[x][y] >= 0 && dist[x][y] < val) return;
	dist[x][y] = val;
	pacman_recursive_dist (pos, x+1, y, val+1);
	pacman_recursive_dist (pos, x-1, y, val+1);
	pacman_recursive_dist (pos, x, y+1, val+1);
	pacman_recursive_dist (pos, x, y-1, val+1);
}

void pacman_set_dist (byte *pos)
{
	int i, j;
	int curx, cury;
	pacman_get_cur_pos (pos, &curx, &cury);
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		dist[i][j] = -1;
	dist[curx][cury] = 1;
	pacman_recursive_dist (pos, curx, cury, 0);
}

int pacman_animate (Pos *pos, byte **movp)
{
	static int count = 0;
	static byte move[32];
	byte *mp = move;
	int curx = -1, cury = -1;
	int incx = 0, incy = 0;
	int i, j, k, x, y;
	int oldboard;
	// FIXME: do this using stateful
	static int prevx[4] = {0, 0, 0, 0}, prevy[4] = {0, 0, 0, 0};
	// make sure the ghosts dont step on each others toes
	int taken[4] = {-1, -1, -1, -1}, t=0, gid;
	byte *board = pos->board;
	pacman_set_dist (board);
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (!PACMAN_IS_GHOST(board[j * board_wid + i]))
			continue;
		gid = PACMAN_GET_GHOST_NUM(board [j * board_wid + i]);
		for (k=0; k<10; k++)
		{
			int rnd = random()%4;
			switch(rnd)
			{
				case 0: incx = 0; incy = 1; break;
				case 1: incx = 0; incy = -1; break;
				case 2: incx = 1; incy = 0; break;
				case 3: incx = -1; incy = 0; break;
			}
#if 0
			// make the ghosts move straight as long as boardsible
			if (k < 5 && (incx != prevx[gid] || incy != prevy[gid])) continue;
#endif
			x = i+incx, y = j+incy;
			// make the ghosts head straight for pacman
			if (k < 5 && dist[x][y] > dist[i][j]) continue;
			if (!PACMAN_IS_EDIBLE(board [y * board_wid + x])) continue;
			{ int found = 0, s;
			for (s=0; s<t; s++) if (y * board_wid + x == taken[s])
				found = 1;
			if (found) break;
			}
			prevx[gid] = incx; prevy[gid] = incy;
			taken[t++] = y * board_wid + x;
			*mp++ = i; *mp++ = j; 
			*mp++ = board [j * board_wid + i] - 
				PACMAN_GET_GHOST (board [j * board_wid + i]);
			*mp++ = x; *mp++ = y; 
			*mp++ = board [y * board_wid + x] +
				PACMAN_GET_GHOST (board [j * board_wid + i]);
			*movp = move;
			break;
		}
	}
	pacman_get_cur_pos (board, &curx, &cury);
	g_assert (curx >= 0 && cury >= 0);
	switch ((oldboard = board [cury * board_wid + curx]))
	{
		case PACMAN_PAC_UP: incx = 0; incy = 1; break;
		case PACMAN_PAC_DOWN: incx = 0; incy = -1; break;
		case PACMAN_PAC_RIGHT: incx = 1; incy = 0; break;
		case PACMAN_PAC_LEFT: incx = -1; incy = 0; break;
	}
	if (incx == 0 && incy == 0)   return -1;
	x = curx + incx; y = cury + incy;
	if (x >= 0 && y >= 0 && x < board_wid && y < board_heit &&
			!(board [y * board_wid + x] & PACMAN_WALL_MASK))
	{
		*mp++ = curx; *mp++ = cury; *mp++ = PACMAN_EMPTY;
		*mp++ = x; *mp++ = y; *mp++ = oldboard; *mp++ = -1;
		*movp = move;
	}
	*mp = -1;
	return mp > move ? 1 : -1;
	//return -1;
	

}

// TODO
int pacman_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp)
{
	static byte move[10];
	int curx = -1, cury = -1;
	int newpos;
	pacman_get_cur_pos (pos->board, &curx, &cury);
	g_assert (curx >= 0 && cury >= 0);
	switch (key)
	{
		case GDK_Up: newpos = PACMAN_PAC_UP; break;
		case GDK_Down: newpos = PACMAN_PAC_DOWN; break;
		case GDK_Right: newpos = PACMAN_PAC_RIGHT; break;
		case GDK_Left: newpos = PACMAN_PAC_LEFT; break;
		default: return -1;
	}
	move[0] = curx; move[1] = cury; 
	move[2] = newpos;
	move[3] = -1;
	*movp = move;
	return 1;
}

void pacman_set_init_pos (Pos *pos)
{
	int i, j, k;
	int incx[4] = {0, 0, 1, -1};
	int incy[4] = {1, -1, 0, 0};
	int wallmask[4] = {PACMAN_WALL_UP, PACMAN_WALL_DOWN, 
		PACMAN_WALL_RIGHT, PACMAN_WALL_LEFT};
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		board [j * board_wid + i] = pacman_maze[board_heit - 1 - j][i];
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (board [j * board_wid + i] & PACMAN_WALL_MASK)
		{
			for (k=0; k<4; k++)
			{
				int x = i + incx[k];
				int y = j + incy[k];
				if (x < 0 || y < 0 || x >= board_wid || y>= board_heit)
					continue;
				if (board [y * board_wid + x] & PACMAN_WALL_MASK)
					board [j * board_wid + i] |= wallmask[k];
			}
		}
	}
}

static float get_angle (float x, float y)
{
	float ang;
	if (x >= 0 && y == 0) return 0;
	if (x < 0 && y == 0) return M_PI;
	if (x == 0 && y > 0) return M_PI/2;
	if (x == 0 && y < 0) return 3 * M_PI/2;
	if (x > 0) { ang = atan(y/x); if (ang < 0) ang += 2 * M_PI; return ang;}
	return M_PI + atan(y/x);
}

static char **pacman_pixmap_ghost_gen(char *pixbuf)
	// FIXME: ghosts look like flowers :(
{
	int i, j;
	static char *pixmap[PACMAN_CELL_SIZE+3];
	pixmap[0] = "25 25 2 1";
	pixmap[1] = "  c none";
	pixmap[2] = ". c #dddddd";
	for (i=0; i<PACMAN_CELL_SIZE; i++)
	{
		pixmap[i+3] = pixbuf + i * (PACMAN_CELL_SIZE+1);
		pixmap[i+3][PACMAN_CELL_SIZE] = 0;
	}
	for (i=0; i<PACMAN_CELL_SIZE; i++)
	for (j=0; j<PACMAN_CELL_SIZE; j++)
	{
		int x = i - PACMAN_CELL_SIZE/2, y = j - PACMAN_CELL_SIZE/2;
		float rad = x * x + y * y;
		float val = cos (3 * get_angle (x, y));
		val = val * val * PACMAN_CELL_SIZE * PACMAN_CELL_SIZE/6;
		if (rad - val < PACMAN_CELL_SIZE*PACMAN_CELL_SIZE/10)
			pixmap[i+3][j] = '.';
		else
			pixmap[i+3][j] = ' ';
	}
	return pixmap;
}

char ** pacman_pixmap_wall_gen(int idx, char *pixbuf)
{
	int i, j;
	char **pixmap;
	char *colors;
	int fg, bg;
	if (!(idx & PACMAN_WALL_MASK))
		return NULL;
	colors = pacman_colors;
	fg = (200 << 16) + 200;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	pixmap = g_new(char *, PACMAN_CELL_SIZE + 3);
	for (i=0; i<3; i++)
		pixmap[i] = brown_square_25_xpm[i];
	for (i=0; i<PACMAN_CELL_SIZE; i++)
	{
		//pixmap[i+3] = g_new(char, PACMAN_CELL_SIZE+1);
		pixmap[i+3] = pixbuf + i * (PACMAN_CELL_SIZE+1);
		pixmap[i+3][PACMAN_CELL_SIZE] = 0;
	}
	for (i=0; i<PACMAN_CELL_SIZE; i++)
	for (j=0; j<PACMAN_CELL_SIZE; j++)
	{
		pixmap[i+3][j] = ' ';
		if ((i >= PACMAN_CELL_SIZE/3 && i < 2*PACMAN_CELL_SIZE/3)
		 && (j >= PACMAN_CELL_SIZE/3 && j < 2*PACMAN_CELL_SIZE/3))
			pixmap[i+3][j] = '.';
		if ((i >= PACMAN_CELL_SIZE*2/3)
		 && (j >= PACMAN_CELL_SIZE/3 && j < 2*PACMAN_CELL_SIZE/3)
		 && idx & PACMAN_WALL_DOWN)
			pixmap[i+3][j] = '.';
		if ((i < PACMAN_CELL_SIZE/3)
		 && (j >= PACMAN_CELL_SIZE/3 && j < 2*PACMAN_CELL_SIZE/3)
		 && idx & PACMAN_WALL_UP)
			pixmap[i+3][j] = '.';
		if ((i >= PACMAN_CELL_SIZE/3 && i < 2*PACMAN_CELL_SIZE/3)
		 && (j >= PACMAN_CELL_SIZE*2/3)
		 && idx & PACMAN_WALL_RIGHT)
			pixmap[i+3][j] = '.';
		if ((i >= PACMAN_CELL_SIZE/3 && i < 2*PACMAN_CELL_SIZE/3)
		 && (j < PACMAN_CELL_SIZE/3)
		 && idx & PACMAN_WALL_LEFT)
			pixmap[i+3][j] = '.';
	}
	return pixmap;
}

char ** pacman_pixmap_pac_gen(float dir, float gap, char *pixbuf)
{
	char **pixmap;
	float ang;
	float x0, y0;
	int i, j;
	pixmap = g_new(char *, 5 + PACMAN_CELL_SIZE);
	pixmap[0] = "25 25 4 1";
	pixmap[1] = "  c none";
	pixmap[2] = "b c black";
	pixmap[3] = "y c yellow";
	pixmap[4] = "r c red";
	ang = dir * M_PI / 2;
	x0 = cos(ang);
	y0 = sin(ang);
	for (i=0; i<PACMAN_CELL_SIZE; i++)
	{
		pixmap[i+5] = pixbuf + i * (PACMAN_CELL_SIZE+1);
		pixmap[i+5][PACMAN_CELL_SIZE] = 0;
		for (j=0; j<PACMAN_CELL_SIZE; j++)
		{
			float x, y, tmp;
			pixmap[i+5][j] = ' ';
			x = i - PACMAN_CELL_SIZE/2, y = j - PACMAN_CELL_SIZE/2;
			if (x * x + y * y > PACMAN_CELL_SIZE * PACMAN_CELL_SIZE/6)
				continue;
			tmp =  sqrt(x*x+y*y);
			if (tmp > 0) { 	x /= tmp;	y /= tmp; }
			if (x0 * x + y0 * y < cos (gap * M_PI / 180))
				pixmap[i+5][j] = 'y';			
			if (x*x+y*y==0) 
				pixmap[i+5][j] = 'y';			
		}
	}
	return pixmap;
}

char ** pacman_get_pixmap (int idx, int color)
{
	int fg, bg, i;
	char *colors;
	static char pixbuf[PACMAN_CELL_SIZE*(PACMAN_CELL_SIZE+1)];
	if (idx & PACMAN_WALL_MASK)
		return pacman_pixmap_wall_gen(idx, pixbuf);
	colors = pacman_colors;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	if (idx == PACMAN_FOOD)
	{
		fg = 200 << 8;
		return pixmap_ball_gen(PACMAN_CELL_SIZE, pixbuf, fg, bg, 4.0, 30.0);
	}
	else if (idx == PACMAN_EMPTY)
	{
		fg = 200 << 8;
		return pixmap_ball_gen(PACMAN_CELL_SIZE, pixbuf, fg, bg, 0, 1);
	}	
	else if (idx == PACMAN_POWER)
	{
		fg = 200 << 16;
		return pixmap_ball_gen(PACMAN_CELL_SIZE, pixbuf, fg, bg, 6.0, 30.0);
	}
	else if (idx == PACMAN_PAC_UP)
	{
		return pacman_pixmap_pac_gen(2, 45, pixbuf);
	}
	else if (idx == PACMAN_PAC_DOWN)
	{
		return pacman_pixmap_pac_gen(0, 45, pixbuf);
	}
	else if (idx == PACMAN_PAC_RIGHT)
	{
		return pacman_pixmap_pac_gen(1, 45, pixbuf);
	}
	else if (idx == PACMAN_PAC_LEFT)
	{
		return pacman_pixmap_pac_gen(3, 45, pixbuf);
	}
	else if (PACMAN_IS_GHOST(idx))
	{
		//fg = 50;
		return pacman_pixmap_ghost_gen (pixbuf);
			//pixmap_ball_gen(PACMAN_CELL_SIZE, pixbuf, fg, bg, 6.0, 24.0);
	}
	return NULL;
}

