#ifndef _PENTALINE_H_
#define _PENTALINE_H_

#include "game.h"
#include "move.h"

#define PENTALINE_CELL_SIZE 40
#define PENTALINE_NUM_PIECES 2

#define PENTALINE_BOARD_WID 12
#define PENTALINE_BOARD_HEIT 12

extern char pentaline_colors[9];

extern Game Pentaline;

void pentaline_init (void);
byte * pentaline_movegen (Pos *, Player);
float pentaline_eval (Pos *, Player);

#endif
