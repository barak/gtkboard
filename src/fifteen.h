#ifndef _FIFTEEN_H_
#define _FIFTEEN_H_

#include "game.h"
#include "move.h"

#define FIFTEEN_CELL_SIZE 60
#define FIFTEEN_NUM_PIECES 16

#define FIFTEEN_BOARD_WID 4
#define FIFTEEN_BOARD_HEIT 4

extern char fifteen_colors[6];

extern Game Fifteen;

void fifteen_init (void);

#endif
