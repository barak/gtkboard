#ifndef _HYPERMAZE_H_
#define _HYPERMAZE_H_

#include "game.h"
#include "move.h"

#define HYPERMAZE_CELL_SIZE 20
#define HYPERMAZE_NUM_PIECES 10

#define HYPERMAZE_BOARD_WID 25
#define HYPERMAZE_BOARD_HEIT 25

extern char hypermaze_colors[6];

extern Game Hypermaze;

void hypermaze_init (void);

#endif
