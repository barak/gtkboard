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

#define MAZE_CELL_SIZE 8
#define MAZE_NUM_PIECES 127

#define MAZE_BOARD_WID 60
#define MAZE_BOARD_HEIT 60

char maze_colors[6] = {100, 150, 200, 100, 150, 200};

int * maze_init_pos = NULL;

static int maze_maze[MAZE_BOARD_WID][MAZE_BOARD_HEIT] = {{0}};

#define MAZE_WALL 2
#define MAZE_CUR 1

#define CORNER_SIZE 3

void maze_init ();

SCORE_FIELD maze_score_fields[] = {SCORE_FIELD_USER, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
char *maze_score_field_names[] = {"User", "Time", "Date", NULL};


Game Maze = { MAZE_CELL_SIZE, MAZE_BOARD_WID, MAZE_BOARD_HEIT, 
	MAZE_NUM_PIECES, maze_colors,  NULL, NULL, "Maze", "Maze", maze_init};


static void maze_set_init_pos (Pos *pos);
static char ** maze_get_pixmap (int idx, int color);
static int maze_getmove_kb (Pos *cur_pos, int key, byte **move, int **);
ResultType maze_who_won (Pos *, Player, char **);

void maze_init ()
{
	game_single_player = TRUE;
	game_set_init_pos = maze_set_init_pos;
	game_get_pixmap = maze_get_pixmap;
	game_getmove_kb = maze_getmove_kb;
	game_who_won = maze_who_won;
	game_start_immediately = TRUE;
	game_scorecmp = game_scorecmp_def_time;
	game_score_fields = maze_score_fields;
	game_score_field_names = maze_score_field_names;
	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Maze\n"
		"Single player game\n"
		"Status: Partially implemented (playable)\n"
		"URL: "GAME_DEFAULT_URL ("maze");
	game_doc_rules = 
		"Your objective is to lead the man (or mouse or whatever) trapped in the maze from the lower left corner to the upper right.\n"
		"\n"
		"The maze is randomly generated, and is currently not very good at generating particularly hard-to-solve mazes.\n";
}


ResultType maze_who_won (Pos *pos, Player to_play, char **commp)
{
	static char comment[32];
	int i, j;
	gboolean over = FALSE;
	for (i=0; i<CORNER_SIZE; i++)
	for (j=0; j<CORNER_SIZE; j++)
		if (pos->board [(board_heit - 1 - j) * board_wid + (board_wid - 1 - i)] == MAZE_CUR)
			over = TRUE;
//	gboolean over = (pos->board [board_wid * board_heit - 1] == MAZE_CUR);
	snprintf (comment, 32, "%sMoves: %d", 
			over ? "You won. " : "",
			pos->num_moves);
	*commp = comment;
	return over ? RESULT_WON : RESULT_NOTYET;
}

void maze_get_cur_pos (byte *pos, int *x, int *y)
{
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (pos [j * board_wid + i] == MAZE_CUR)
		{ *x = i; *y = j; return;
		}
}

int maze_getmove_kb (Pos *pos, int key, byte **movp, int **rmovp)
{
	static byte move[10];
	int curx = -1, cury = -1;
	int incx = -1, incy = -1;
	int x, y;
	maze_get_cur_pos (pos->board, &curx, &cury);
	g_assert (curx >= 0 && cury >= 0);
	switch (key)
	{
		case GDK_Up: incx = 0; incy = 1; break;
		case GDK_Down: incx = 0; incy = -1; break;
		case GDK_Right: incx = 1; incy = 0; break;
		case GDK_Left: incx = -1; incy = 0; break;
		default: return -1;
	}
	x = curx + incx;
	y = cury + incy;
	if (x < 0 || y < 0 || x >= board_wid || y >= board_heit) return -1;
	if (pos->board[y * board_wid + x] == MAZE_WALL) return -1;
	move[0] = curx; move[1] = cury; move[2] = 0;
	move[3] = x; move[4] = y; move[5] = MAZE_CUR;
	move[6] = -1;
	*movp = move;
	return 1;
}


int maze_checknbrs(int x, int y)
{
	int i, j;
	int incx[] = {1, 1, 1, 0, -1, -1, -1, 0};
	int incy[] = {1, 0, -1, -1, -1, 0, 1, 1};
	if (x < 0 || y < 0 || x >= board_wid || y >= board_heit)
		return 0;
	for (i=1; i<=7; i+=2)
	{
		int found = 0;
		for (j=0; j<3; j++)
		{
			int k = (i+j)%8;
			int newx = x + incx[k], newy = y + incy[k];
			if (newx < 0 || newy < 0 || newx >= board_wid || newy >= board_heit)
				continue;
			if (maze_maze[newx][newy] == MAZE_WALL)
				found = 1;
		}
		if (!found) return 0;
	}
	return 1;	
}

static void recursive_pathgen (byte *board, int x, int y, int val)
{
	static int incx[4] = {-1, 0, 0, 1};
	static int incy[4] = {0, -1, 1, 0};

	int i;
	if (!ISINBOARD(x, y)) return;
	if (board[y * board_wid + x] == val) return;
	if (board[y * board_wid + x] == MAZE_WALL) return;
	board [y * board_wid + x] = val;
	for (i=0; i<4; i++)
		recursive_pathgen (board, x+incx[i], y+incy [i], val);
}


static void mazegen ()
{
	int x, y, npts = 0;
	for (x=0; x<board_wid; x++)
	for (y=0; y<board_heit; y++)
		maze_maze[x][y] = MAZE_WALL;
	while (npts < board_wid * board_heit * 0.58)
	{
		x = random() % board_wid;
		y = random() % board_heit;
		if (maze_maze[x][y] == 0) continue;
		if (maze_checknbrs(x, y))
		{
			maze_maze[x][y] = 0;
			npts++;
		}
	}
	{
	int incx[] = {1, 0, -1, 0};
	int incy[] = {0, -1, 0, 1};
	int k;
	// make the 4 edges into walls
	for (y=0; y<board_heit; y++) 
	{
		maze_maze[0][y] = MAZE_WALL;
		maze_maze[board_heit-1][y] = MAZE_WALL;
	}
	for (x=0; x<board_wid; x++) 
	{
		maze_maze[x][0] = MAZE_WALL;
		maze_maze[x][board_wid-1] = MAZE_WALL;
	}
	// if all squares are surrounded then make it a wall
	for (x=1; x<board_wid-1; x++)
	for (y=1; y<board_heit-1; y++)
	{
		int found = 0;
		for (k=0; k<4; k++)
		{
			int newx = x + incx[k], newy = y + incy[k]; 
			if (maze_maze[newx][newy] != MAZE_WALL)
			{
				found = 1;
				break;
			}
		}
		if (!found)
			maze_maze [x][y] = MAZE_WALL;
	}
	}
}

void maze_set_init_pos (Pos *pos)
{
	int i, j, k;
	int incx[4] = {0, 0, 1, -1};
	int incy[4] = {1, -1, 0, 0};
	do
	{
		mazegen();
		for (i=0; i<board_wid; i++)
		for (j=0; j<board_heit; j++)
			pos->board [j * board_wid + i] = maze_maze[i][j];
		for (i=0; i<CORNER_SIZE; i++)
		for (j=0; j<CORNER_SIZE; j++)
			pos->board [j * board_wid + i] = 0;
		for (i=board_wid-CORNER_SIZE; i<board_wid; i++)
		for (j=board_heit-CORNER_SIZE; j<board_heit; j++)
			pos->board [j * board_wid + i] = 0;
		recursive_pathgen (pos->board, 0, 0, -1);
	}
	while (pos->board [board_wid * board_heit - 1] != -1);
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
		if (pos->board [j * board_wid + i] == -1)
			pos->board [j * board_wid + i] = 0;
	pos->board[0] = MAZE_CUR;
	
	/*while(1)
	{
		i = random()%board_wid;
		j = random()%board_heit;
		if (pos->board [j*board_wid+i] == 0)
		{
			pos->board [j * board_wid + i] = 1;
			break;
		}
	}*/
}

char ** maze_pixmap_square_gen (int idx, char *col)
{
	int i, j;
	char **pixmap;
	char *line = "        ";
	pixmap = g_new(char *, MAZE_CELL_SIZE + 2);
	pixmap[0] = "8 8 1 1";
	pixmap[1] = g_strdup_printf (" c %s", col);
	for (i=0; i<MAZE_CELL_SIZE; i++) pixmap[i+2] = line; return pixmap;
}

char ** maze_get_pixmap (int idx, int color)
{
	int fg, bg, i;
	char *colors = maze_colors;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	if (idx == MAZE_WALL)
		return maze_pixmap_square_gen(idx, "#443333");
	return maze_pixmap_square_gen(idx, "#ffff00");
	//else return pixmap_ball_gen(MAZE_CELL_SIZE, 0xc0c040, bg, 2, 50);
}

