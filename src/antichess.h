#ifndef _ANTICHESS_H_
#define _ANTICHESS_H_

#include "game.h"

#define ANTICHESS_CELL_SIZE 54
#define ANTICHESS_NUM_PIECES 12

#define ANTICHESS_BOARD_WID 8
#define ANTICHESS_BOARD_HEIT 8

#define ANTICHESS_WK 1
#define ANTICHESS_WQ 2
#define ANTICHESS_WR 3
#define ANTICHESS_WB 4
#define ANTICHESS_WN 5
#define ANTICHESS_WP 6
#define ANTICHESS_BK 7
#define ANTICHESS_BQ 8
#define ANTICHESS_BR 9
#define ANTICHESS_BB 10
#define ANTICHESS_BN 11
#define ANTICHESS_BP 12

#define ANTICHESS_ISWHITE(x) (x >= 1 && x <= 6)
#define ANTICHESS_ISBLACK(x) (x >= 7 && x <= 12)


extern int antichess_initpos [ANTICHESS_BOARD_WID * ANTICHESS_BOARD_HEIT];

extern Game Antichess;

#endif
