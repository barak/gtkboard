#ifndef _CHECKERS_H_
#define _CHECKERS_H_

#include "game.h"

#define CHECKERS_CELL_SIZE 40
#define CHECKERS_NUM_PIECES 4

#define CHECKERS_BOARD_WID 8
#define CHECKERS_BOARD_HEIT 8

#define CHECKERS_WK 1
#define CHECKERS_WP 2
#define CHECKERS_BK 3
#define CHECKERS_BP 4

#define CHECKERS_ISKING(x) (x == 1 || x == 3)
#define CHECKERS_ISPAWN(x) (x == 2 || x == 4)

#define CHECKERS_ISWHITE(x) (x >= 1 && x <= 2)
#define CHECKERS_ISBLACK(x) (x >= 3 && x <= 4)


extern int checkers_initpos [CHECKERS_BOARD_WID * CHECKERS_BOARD_HEIT];

extern Game Checkers;

#endif
