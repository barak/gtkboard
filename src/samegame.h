#ifndef _SAMEGAME_H_
#define _SAMEGAME_H_

#include "game.h"

#define SAMEGAME_NUM_ANIM 8

#define SAMEGAME_CELL_SIZE 40
#define SAMEGAME_NUM_PIECES (3*SAMEGAME_NUM_ANIM)

#define SAMEGAME_BOARD_WID 15
#define SAMEGAME_BOARD_HEIT 10

#define SAMEGAME_RP 1
#define SAMEGAME_BP 2
#define SAMEGAME_GP 3

extern char samegame_colors[6];

extern int samegame_initpos [SAMEGAME_BOARD_WID*SAMEGAME_BOARD_HEIT];

extern Game Samegame;

void samegame_init (void);

#endif
