#ifndef _MAZE_H_
#define _MAZE_H_

#include "game.h"
#include "move.h"

#define MAZE_CELL_SIZE 8
#define MAZE_NUM_PIECES 127

#define MAZE_BOARD_WID 60
#define MAZE_BOARD_HEIT 60

extern char maze_colors[6];

extern Game Maze;

void maze_init (void);

#endif
