#ifndef _PLOT4_H_
#define _PLOT4_H_

#include "game.h"
#include "move.h"

#define PLOT4_CELL_SIZE 55
#define PLOT4_NUM_PIECES 3

#define PLOT4_BOARD_WID 7
#define PLOT4_BOARD_HEIT 6

extern char plot4_colors[6];

//extern int plot4_initpos [PLOT4_BOARD_WID*PLOT4_BOARD_HEIT];

#define PLOT4_PIXMAP_SIZE (PLOT4_CELL_SIZE+3)

//extern char *plot4_pixmaps[(PLOT4_NUM_PIECES+1)*PLOT4_PIXMAP_SIZE];

extern Game Plot4;

void plot4_init (void);
byte * plot4_movegen (Pos *, Player);
float plot4_eval (Pos *, Player);

#endif
