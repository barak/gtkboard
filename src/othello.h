#ifndef _OTHELLO_H_
#define _OTHELLO_H_

#include "game.h"

#define OTHELLO_CELL_SIZE 55
#define OTHELLO_NUM_PIECES 2

#define OTHELLO_BOARD_WID 8
#define OTHELLO_BOARD_HEIT 8

extern char othello_colors[];

//extern int othello_initpos []; 

#define OTHELLO_PIXMAP_SIZE (OTHELLO_CELL_SIZE+3)

//extern char *** othello_pixmaps;

extern Game Othello;

#endif
