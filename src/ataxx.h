#ifndef _ATAXX_H_
#define _ATAXX_H_

#include "game.h"
#include "move.h"

#define ATAXX_CELL_SIZE 55
#define ATAXX_NUM_PIECES 2

#define ATAXX_BOARD_WID 7
#define ATAXX_BOARD_HEIT 7

#define ATAXX_EMPTY 0
#define ATAXX_WP 1
#define ATAXX_BP 2

extern char ataxx_colors[6];

extern int ataxx_initpos [ATAXX_BOARD_WID*ATAXX_BOARD_HEIT];

#define ATAXX_PIXMAP_SIZE (ATAXX_CELL_SIZE+3)

//extern char *ataxx_pixmaps[(ATAXX_NUM_PIECES+1)*ATAXX_PIXMAP_SIZE];

extern Game Ataxx;

#endif
