#ifndef _HIQ_H_
#define _HIQ_H_

#include "game.h"
#include "move.h"

#define HIQ_CELL_SIZE 54
#define HIQ_NUM_PIECES 5

#define HIQ_BOARD_WID 7
#define HIQ_BOARD_HEIT 7

extern char hiq_colors[6];

extern int hiq_initpos [HIQ_BOARD_WID*HIQ_BOARD_HEIT];

extern Game Samegame;

void hiq_init (void);
byte * hiq_movegen (char *, int);
float hiq_eval (byte *, int);

#endif
