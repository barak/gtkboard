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

#define CPENTO_NUM_PIECES 32
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
#define CPENTO_PIECE_LEG_UP 18
#define CPENTO_PIECE_LEG_DOWN 19
#define CPENTO_PIECE_PIPE_VERT 20
#define CPENTO_PIECE_LEG_RIGHT 21
#define CPENTO_PIECE_BEND_UR 22
#define CPENTO_PIECE_BEND_DR 23
#define CPENTO_PIECE_T_LEFT 24
#define CPENTO_PIECE_LEG_LEFT 25
#define CPENTO_PIECE_BEND_UL 26
#define CPENTO_PIECE_BEND_DL 27
#define CPENTO_PIECE_T_RIGHT 28
#define CPENTO_PIECE_PIPE_HORIZ 29
#define CPENTO_PIECE_T_DOWN 30
#define CPENTO_PIECE_T_UP 31
#define CPENTO_PIECE_FOURWAY 32


static char cpento_colors[6];
static int cpento_initpos[CPENTO_BOARD_WID*CPENTO_BOARD_HEIT];
static char **cpento_pixmaps[CPENTO_NUM_PIECES];
static void cpento_init();

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
	"Nimlike games",
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
    cpento_leg_u_36_36_xpm,
    cpento_leg_d_36_36_xpm,
    cpento_pipe_ud_36_36_xpm,
    cpento_leg_r_36_36_xpm,
    cpento_bend_ur_36_36_xpm,
    cpento_bend_dr_36_36_xpm,
    cpento_t_closed_l_36_36_xpm,
    cpento_leg_l_36_36_xpm,
    cpento_bend_ul_36_36_xpm,
    cpento_bend_dl_36_36_xpm,
    cpento_t_closed_r_36_36_xpm,
    cpento_pipe_lr_36_36_xpm,
    cpento_t_closed_d_36_36_xpm,
    cpento_t_closed_u_36_36_xpm,
    cpento_fourway_36_36_xpm,
};


static void cpento_init()
{
    game_single_player = 1;
    game_getmove = cpento_getmove;
    game_who_won = NULL;
    game_get_pixmap = NULL;
    game_scorecmp = game_scorecmp_def_iscore;
	game_doc_about_status = STATUS_UNPLAYABLE;
    game_doc_about =
    "Capture Pentominoes\n"
    "Two player game\n"
    "Status: Not implemented\n"
    "URL: " GAME_DEFAULT_URL("cpento")
    ;
    game_doc_rules =
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
    ".X...",
    ".XX..",
    "..X..",
    "..X..",
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
    ".X.X.",
    ".XXX.",
    ".....",
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

/* From picture coords to display coords */
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

/* From display coords to picture coords */
static void cpento_rev_orient(int x, int y, int *nx, int *ny, int orient)
{
    switch (orient)
    {
        case 0:  *nx =   x; *ny =   y; return;
        case 1:  *nx =   y; *ny = 4-x; return;
        case 2:  *nx = 4-x; *ny = 4-y; return;
        case 3:  *nx = 4-y; *ny =   x; return;
        case 4:  *nx = 4-x; *ny =   y; return;
        case 5:  *nx =   y; *ny =   x; return;
        case 6:  *nx =   x; *ny = 4-y; return;
        case 7:  *nx = 4-y; *ny = 4-x; return;
    }
}

static int cpento_tile_orient(int x, int y, int orient, char **pento)
{
    int tx, ty;
    int top=0, left=0, right=0, bottom=0;
    int results[16] =
    {
       0,
       CPENTO_PIECE_LEG_UP,
       CPENTO_PIECE_LEG_DOWN,
       CPENTO_PIECE_PIPE_VERT,
       CPENTO_PIECE_LEG_RIGHT,
       CPENTO_PIECE_BEND_UR,
       CPENTO_PIECE_BEND_DR,
       CPENTO_PIECE_T_LEFT,
       CPENTO_PIECE_LEG_LEFT,
       CPENTO_PIECE_BEND_UL,
       CPENTO_PIECE_BEND_DL,
       CPENTO_PIECE_T_RIGHT,
       CPENTO_PIECE_PIPE_HORIZ,
       CPENTO_PIECE_T_DOWN,
       CPENTO_PIECE_T_UP,
       CPENTO_PIECE_FOURWAY
    };

    cpento_rev_orient(x, y, &tx, &ty, orient);
    if (pento[tx][ty] != 'X')
      return 0;
    cpento_rev_orient(x, y+1, &tx, &ty, orient);
    top = (y < 4 && pento[tx][ty] == 'X');
    cpento_rev_orient(x, y-1, &tx, &ty, orient);
    bottom = (y > 0 && pento[tx][ty] == 'X');
    cpento_rev_orient(x+1, y, &tx, &ty, orient);
    right = (x < 4 && pento[tx][ty] == 'X');
    cpento_rev_orient(x-1, y, &tx, &ty, orient);
    left = (x > 0 && pento[tx][ty] == 'X');
    return results[top | bottom<<1 | right<<2 | left<<3];
}

static void cpento_place_left(byte *move, char *pento[5],
                              int piece_letter, int orient)
{
    int i, j;
    cpento_clear_left(move);
    for (i=0; i < 5; ++i) {
        for (j=0; j < 5; ++j) {
            int X, Y;
            cpento_orient(i, j, &X, &Y, orient);
            if (pento[i][j] == 'X')
              cpento_add_move(move, X+1, Y+1, 
                  cpento_tile_orient(X, Y, orient, pento));
        }
    }
}


static void cpento_place_right(byte *move, char *pento[5],
                               int piece_letter, int orient,
                               int x, int y)
{
    int i, j;
    for (i=0; i < 5; ++i) {
        for (j=0; j < 5; ++j) {
            int X, Y;
            cpento_orient(i, j, &X, &Y, orient);
            if (pento[i][j] == 'X')
              cpento_add_move(move, X+x-2, Y+y-2,
                  cpento_tile_orient(X, Y, orient, pento));
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
        cpento_place_left(move, cpento_piece_list[pento], tile, orient);
        state = 1;
        if (movp)
          *movp = move;
        return 1;
    }
    else if ((tile == CPENTO_TILE_LARROW) && (state == 1))
    {
        orient = larrow_effects[orient];
        cpento_place_left(move, cpento_piece_list[pento], pento, orient);
        if (movp)
          *movp = move;
        return 1;
    }
    else if ((tile == CPENTO_TILE_RARROW) && (state == 1))
    {
        orient = rarrow_effects[orient];
        cpento_place_left(move, cpento_piece_list[pento], pento, orient);
        if (movp)
          *movp = move;
        return 1;
    }
    else if ((tile == CPENTO_TILE_FLIPLR) && (state == 1))
    {
        orient = fliplr_effects[orient];
        cpento_place_left(move, cpento_piece_list[pento], pento, orient);
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
            cpento_place_right(move, cpento_piece_list[pento], pento, orient, x, y);
            state = 0;
            if (movp)
              *movp = move;
            return 1;
        }
        else {
            int possibles = 0;
            int fit = 0;
            if (cpento_fits_right(move, cpento_piece_list[pento], orient, 
                x-1, y, pos))
              ++possibles, fit = 0 * 3 + 1;
            if (cpento_fits_right(move, cpento_piece_list[pento], orient, 
                x+1, y, pos))
              ++possibles, fit = 2 * 3 + 1;
            if (cpento_fits_right(move, cpento_piece_list[pento], orient, 
                x, y-1, pos))
              ++possibles, fit = 1 * 3 + 0;
            if (cpento_fits_right(move, cpento_piece_list[pento], orient, 
                x, y+1, pos))
              ++possibles, fit = 1 * 3 + 2;

            if (possibles != 1)
              return -1;

            cpento_place_right(move, cpento_piece_list[pento], pento, orient,
                x+(fit/3)-1, y+(fit%3)-1);
            state = 0;
            if (movp)
              *movp = move;
            return 1;
        }
    }
    else
    {
        return -1;
    }
}

