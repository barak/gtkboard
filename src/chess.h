#ifndef _CHESS_H_
#define _CHESS_H_

#include "game.h"

#define CHESS_CELL_SIZE 54
#define CHESS_NUM_PIECES 12

#define CHESS_BOARD_WID 8
#define CHESS_BOARD_HEIT 8

#define CHESS_WK 1
#define CHESS_WQ 2
#define CHESS_WR 3
#define CHESS_WB 4
#define CHESS_WN 5
#define CHESS_WP 6
#define CHESS_BK 7
#define CHESS_BQ 8
#define CHESS_BR 9
#define CHESS_BB 10
#define CHESS_BN 11
#define CHESS_BP 12

#define CHESS_ISWHITE(x) (x >= 1 && x <= 6)
#define CHESS_ISBLACK(x) (x >= 7 && x <= 12)


extern int chess_initpos [CHESS_BOARD_WID * CHESS_BOARD_HEIT];

extern Game Chess;

#endif
