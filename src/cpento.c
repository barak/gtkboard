/*  This file is a part of gtkboard, a board games system.
    Copyright (C) 2003, Arthur J. O'Dwyer <ajo@andrew.cmu.edu>

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

#include "game.h"
#include "../pixmaps/alpha.xpm"
#include "../pixmaps/cpento.xpm"

#define CPENTO_CELL_SIZE  36
#define CPENTO_BOARD_WID  15
#define CPENTO_BOARD_HEIT 9

#define CPENTO_NUM_PIECES 17
#define CPENTO_EMPTY 0
#define CPENTO_TILE_F 1
#define CPENTO_TILE_I 2
#define CPENTO_TILE_L 3
#define CPENTO_TILE_N 4
#define CPENTO_TILE_P 5
#define CPENTO_TILE_T 6
#define CPENTO_TILE_U 7
#define CPENTO_TILE_V 8
#define CPENTO_TILE_W 9
#define CPENTO_TILE_X 10
#define CPENTO_TILE_Y 11
#define CPENTO_TILE_Z 12
#define CPENTO_TILE_SIMPLE 13
#define CPENTO_TILE_RARROW 14
#define CPENTO_TILE_LARROW 15
#define CPENTO_TILE_FLIPLR 16
#define CPENTO_PIECE_BALL 17



static char cpento_colors[6];
static int cpento_initpos[CPENTO_BOARD_WID*CPENTO_BOARD_HEIT];
static char **cpento_pixmaps[CPENTO_NUM_PIECES];
static void cpento_init(void);

static int cpento_getmove(Pos *, int, int,
                   GtkboardEventType,
                   Player, byte **, int **);

Game CapturePento = {
    CPENTO_CELL_SIZE,
    CPENTO_BOARD_WID, CPENTO_BOARD_HEIT,
    CPENTO_NUM_PIECES,
    cpento_colors, cpento_initpos,
    cpento_pixmaps,
    "Capture Pentominoes",
    cpento_init
};

static char cpento_colors[6] = { 0,120,0, 10,130,10 };


static int cpento_initpos[CPENTO_BOARD_WID*CPENTO_BOARD_HEIT] =
{
    1, 2, 3, 4, 5, 6,13,13,13,13,13,13,13,13,13,
    7, 8, 9,10,11,12,13, 0, 0, 0, 0, 0, 0, 0,13,
   13,13,13,13,13,13,13, 0, 0, 0, 0, 0, 0, 0,13,
   13, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0, 0,13,
   13, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0, 0,13,
   15, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0, 0, 0,13,
   13, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0, 0,13,
   13, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0, 0,13,
   13,13,13,16,13,13,13,13,13,13,13,13,13,13,13,
};

static char *ball_grey_36_36_xpm[] = {
"36 36 2 1",
"  c #000000",
"X c #FF00FF",
"....................................",
"....................................",
".................X..................",
"............XXXXXXXXXXX.............",
"..........XXXXXXXXXXXXXXX...........",
"........XXXXXXXXXXXXXXXXXXX.........",
".......XXXXXXXXXXXXXXXXXXXXX........",
"......XXXXXXXXXXXXXXXXXXXXXXX.......",
".....XXXXXXXXXXXXXXXXXXXXXXXXX......",
".....XXXXXXXXXXXXXXXXXXXXXXXXX......",
"....XXXXXXXXXXXXXXXXXXXXXXXXXXX.....",
"....XXXXXXXXXXXXXXXXXXXXXXXXXXX.....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"...XXXXXXXXXXXXXXXXXXXXXXXXXXXXX....",
"....XXXXXXXXXXXXXXXXXXXXXXXXXXX.....",
"....XXXXXXXXXXXXXXXXXXXXXXXXXXX.....",
".....XXXXXXXXXXXXXXXXXXXXXXXXX......",
".....XXXXXXXXXXXXXXXXXXXXXXXXX......",
"......XXXXXXXXXXXXXXXXXXXXXXX.......",
".......XXXXXXXXXXXXXXXXXXXXX........",
"........XXXXXXXXXXXXXXXXXXX.........",
"..........XXXXXXXXXXXXXXX...........",
"............XXXXXXXXXXX.............",
".................X..................",
"....................................",
"....................................",
"....................................",
};


static char **cpento_pixmaps[CPENTO_NUM_PIECES] =
{
    char_F_grey_36_36_xpm,
    char_I_grey_36_36_xpm,
    char_L_grey_36_36_xpm,
    char_N_grey_36_36_xpm,
    char_P_grey_36_36_xpm,
    char_T_grey_36_36_xpm,
    char_U_grey_36_36_xpm,
    char_V_grey_36_36_xpm,
    char_W_grey_36_36_xpm,
    char_X_grey_36_36_xpm,
    char_Y_grey_36_36_xpm,
    char_Z_grey_36_36_xpm,
    cpento_fourway_36_36_xpm,
    char_R_grey_36_36_xpm,
    char_L_grey_36_36_xpm,
    char_F_grey_36_36_xpm,
    ball_grey_36_36_xpm,
};


static void cpento_init(void)
{
    game_single_player = 1;
    game_getmove = cpento_getmove;
    game_who_won = NULL;
    game_get_pixmap = NULL;
    game_scorecmp = game_scorecmp_def_iscore;
    game_doc_about =
    "Capture Pentominoes\n"
    "Two player game\n"
    "Status: Not implemented\n"
    "URL: " GAME_DEFAULT_URL("cpento")
    ;
    game_doc_rules =
    "Capture Pentominoes rules\n"
    "\n"
    "The objective is to be the last player able to move.\n"
    "\n"
    "Each move consists of selecting a piece from the pool"
    " and placing it on some empty region of the board."
    " If the newly placed piece touches a second piece on"
    " three or more sides, that second piece is captured."
    " A captured piece may be removed from the board by"
    " its captor at any time.\n"
    "\n"
    "At the moment, this is just an interface demo.  There's"
    " no actual game implemented.\n"
    ;
    game_doc_strategy =
    "No strategy yet."
    ;
}


static char *cpento_piece_list[][5] = {
  {
    ".....",
    "..XX.",
    ".XX..",
    "..X..",
    ".....",
  },
  {
    "..X..",
    "..X..",
    "..X..",
    "..X..",
    "..X..",
  },
  {
    "..X..",
    "..X..",
    "..X..",
    "..XX.",
    ".....",
  },
  {
    ".....",
    "..X..",
    "..XX.",
    "...X.",
    "...X.",
  },
  {
    ".....",
    "..XX.",
    "..XX.",
    "..X..",
    ".....",
  },
  {
    ".....",
    ".XXX.",
    "..X..",
    "..X..",
    ".....",
  },
  {
    ".....",
    ".....",
    ".X.X.",
    ".XXX.",
    ".....",
  },
  {
    ".....",
    ".X...",
    ".X...",
    ".XXX.",
    ".....",
  },
  {
    ".....",
    ".X...",
    ".XX..",
    "..XX.",
    ".....",
  },
  {
    ".....",
    "..X..",
    ".XXX.",
    "..X..",
    ".....",
  },
  {
    ".....",
    "..X..",
    ".XX..",
    "..X..",
    "..X..",
  },
  {
    ".....",
    ".XX..",
    "..X..",
    "..XX.",
    ".....",
  },
};


static void cpento_add_move(byte *move, int x, int y, int m)
{
    int i;
    for (i=0; move[i] != -1; i += 3)
    {
        if (move[i] == x && move[i+1] == y) {
            move[i+2] = m;
            return;
        }
    }
    move[i] = x;
    move[i+1] = y;
    move[i+2] = m;
    move[i+3] = -1;
    return;
}


static void cpento_clear_left(byte *move)
{
    int i, j, m;

    /* Clear the left side of the board */
    for (i=1; i < 6; ++i)
    for (j=1; j < 6; ++j)
    {
        cpento_add_move(move, i, j, 0);
    }
}

static void cpento_orient(int x, int y, int *nx, int *ny, int orient)
{
    switch (orient)
    {
        case 0:  *nx =   x; *ny =   y; return;
        case 1:  *nx = 4-y; *ny =   x; return;
        case 2:  *nx = 4-x; *ny = 4-y; return;
        case 3:  *nx =   y; *ny = 4-x; return;
        case 4:  *nx = 4-x; *ny =   y; return;
        case 5:  *nx =   y; *ny =   x; return;
        case 6:  *nx =   x; *ny = 4-y; return;
        case 7:  *nx = 4-y; *ny = 4-x; return;
    }
}

static void cpento_place_left(byte *move, char *pento[5], int orient)
{
    int i, j;
    cpento_clear_left(move);
    for (i=0; i < 5; ++i) {
        for (j=0; j < 5; ++j) {
            int X, Y;
            cpento_orient(i, j, &X, &Y, orient);
            if (pento[i][j] == 'X')
              cpento_add_move(move, X+1, Y+1, CPENTO_PIECE_BALL);
        }
    }
}


static void cpento_place_right(byte *move, char *pento[5],
                               int orient, int x, int y)
{
    int i, j;
    for (i=0; i < 5; ++i) {
        for (j=0; j < 5; ++j) {
            int X, Y;
            cpento_orient(i, j, &X, &Y, orient);
            if (pento[i][j] == 'X')
              cpento_add_move(move, X+x-2, Y+y-2, CPENTO_PIECE_BALL);
        }
    }
}


static int cpento_fits_right(byte *move, char *pento[5],
                             int orient, int x, int y, Pos *pos)
{
    int i, j;
    for (i=0; i < 5; ++i) {
        for (j=0; j < 5; ++j) {
            int X, Y;
            cpento_orient(i, j, &X, &Y, orient);
            if (pento[i][j] == 'X') {
                if (X+x-2 < 1 || X+x-2 > CPENTO_BOARD_WID-2)
                  return 0;
                if (Y+y-2 < 1 || Y+y-2 > CPENTO_BOARD_HEIT-2)
                  return 0;
                if (pos->board[(Y+y-2)*board_wid + (X+x-2)] != CPENTO_EMPTY)
                  return 0;
            }
        }
    }
    return 1;
}


static int cpento_getmove(Pos *pos, int x, int y,
                          GtkboardEventType type,
                          Player to_play, byte **movp,
                          int **renderp)
{
    static byte move[2000]; /* theoretical max length: 151 */
    static int state = 0;
    static int orient = 0;
    static int pento = 0;
    int tile;

    /* States */
    /* 0 = no pentomino selected */
    /* 1 = pentomino selected */
    /* Orientations */
    /* 0 = neutral */
    /* 1 = rot 90 right */
    /* 2 = rot 180 right */
    /* 5 = flipped, then rot 90 */
    /* 7 = flipped, then rot 270 */
    /* Pentos run 0..11 */
 
    static const int larrow_effects[] =
      { 1, 2, 3, 0, 7, 4, 5, 6 };
    static const int rarrow_effects[] =
      { 3, 0, 1, 2, 5, 6, 7, 4 };
    static const int fliplr_effects[] =
      { 4, 5, 6, 7, 0, 1, 2, 3 };

    if (type != GTKBOARD_BUTTON_RELEASE)
      return 0;

    move[0] = -1;

    tile = pos->board[y * board_wid + x];

    if (tile >= CPENTO_TILE_F && tile <= CPENTO_TILE_Z)
    {
        /* Select a new pentomino; get rid of the old one */
        pento = (tile - CPENTO_TILE_F);
        orient = 0;
        cpento_place_left(move, cpento_piece_list[pento], orient);
        state = 1;
        if (movp)
          *movp = move;
        return 1;
    }
    else if ((tile == CPENTO_TILE_LARROW) && (state == 1))
    {
        orient = larrow_effects[orient];
        cpento_place_left(move, cpento_piece_list[pento], orient);
        if (movp)
          *movp = move;
        return 1;
    }
    else if ((tile == CPENTO_TILE_RARROW) && (state == 1))
    {
        orient = rarrow_effects[orient];
        cpento_place_left(move, cpento_piece_list[pento], orient);
        if (movp)
          *movp = move;
        return 1;
    }
    else if ((tile == CPENTO_TILE_FLIPLR) && (state == 1))
    {
        orient = fliplr_effects[orient];
        cpento_place_left(move, cpento_piece_list[pento], orient);
        if (movp)
          *movp = move;
        return 1;
    }
    else if (tile != CPENTO_EMPTY)
    {
        return -1;
    }
    else if ((x > 6) && (state == 1))
    {
        cpento_clear_left(move);
        if (cpento_fits_right(move, cpento_piece_list[pento], orient, x, y, pos))
        {
            cpento_place_right(move, cpento_piece_list[pento], orient, x, y);
            state = 0;
            if (movp)
              *movp = move;
            return 1;
        }
        else {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}

