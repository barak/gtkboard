#ifndef _TETRIS_H_
#define _TETRIS_H_

#include "game.h"
#include "move.h"

#define TETRIS_CELL_SIZE 20
#define TETRIS_NUM_PIECES 31

#define TETRIS_BOARD_WID 10
#define TETRIS_BOARD_HEIT 22

extern char tetris_colors[6];

extern Game Tetris;

void tetris_init (void);

#endif
