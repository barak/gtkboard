#ifndef _INFILTRATE_H_
#define _INFILTRATE_H_

#include "game.h"

#define INFILTRATE_CELL_SIZE 40
#define INFILTRATE_NUM_PIECES 2

#define INFILTRATE_BOARD_WID 7
#define INFILTRATE_BOARD_HEIT 7

#define INFILTRATE_WP 1
#define INFILTRATE_BP 2

extern int infiltrate_initpos [INFILTRATE_BOARD_WID * INFILTRATE_BOARD_HEIT];

extern Game Infiltrate;

#endif
