#ifndef _PACMAN_H_
#define _PACMAN_H_

#include "game.h"
#include "move.h"

#define PACMAN_CELL_SIZE 25
#define PACMAN_NUM_PIECES 48

#define PACMAN_BOARD_WID 26
#define PACMAN_BOARD_HEIT 25

extern char pacman_colors[6];

#define PACMAN_PIXMAP_SIZE (PACMAN_CELL_SIZE+3)

extern Game Pacman;

void pacman_init (void);
byte * pacman_movegen (char *, int);
float pacman_eval (byte *, int);

#endif
