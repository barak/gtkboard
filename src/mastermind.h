#ifndef _MASTERMIND_H_
#define _MASTERMIND_H_

#include "game.h"
#include "move.h"

#define MASTERMIND_CELL_SIZE 40
#define MASTERMIND_NUM_PIECES 26

#define MASTERMIND_BOARD_WID 8
#define MASTERMIND_BOARD_HEIT 11

extern char mastermind_colors[9];

extern Game Mastermind;

void mastermind_init (void);
byte * mastermind_movegen (Pos *, int);
float mastermind_eval (Pos *, int);

#endif
