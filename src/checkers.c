#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "checkers.h"
#include "move.h"
#include "aaball.h"

char checkers_colors[] = 
	{200, 200, 200, 
	180, 180, 180};
	
int	checkers_initpos[] = 
{
	 0 , 4 , 0 , 4 , 0 , 4 , 0 , 4 ,
	 4 , 0 , 4 , 0 , 4 , 0 , 4 , 0 ,
	 0 , 4 , 0 , 4 , 0 , 4 , 0 , 4 ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	 2 , 0 , 2 , 0 , 2 , 0 , 2 , 0 ,
	 0 , 2 , 0 , 2 , 0 , 2 , 0 , 2 ,
	 2 , 0 , 2 , 0 , 2 , 0 , 2 , 0 ,
};

static int checkers_max_moves = 200;


void checkers_init ();
int checkers_getmove (Pos *, int, int, int, Player, byte **);
ResultType checkers_who_won (Pos *, Player, char **);
byte *checkers_movegen (Pos *, Player );
float checkers_eval (Pos *, Player);
char ** checkers_get_pixmap (int idx, int color);
	
Game Checkers = 
	{ CHECKERS_CELL_SIZE, CHECKERS_BOARD_WID, CHECKERS_BOARD_HEIT, 
	CHECKERS_NUM_PIECES,
	checkers_colors, checkers_initpos, NULL, "Checkers",
	checkers_init};

void checkers_init ()
{
	game_getmove = checkers_getmove;
	game_movegen = checkers_movegen;
	game_who_won = checkers_who_won;
	game_eval = checkers_eval;
	game_get_pixmap = checkers_get_pixmap;
}

ResultType checkers_who_won (Pos *pos, Player player, char **commp)
{
	static char comment[32];
	char *who_str [2] = { "white won", "black won"};
	int found_w = 0, found_b = 0;
	int i;
	*commp = comment;
	for (i=0; i<board_wid * board_heit; i++)
		if (CHECKERS_ISWHITE (pos->board[i])) found_w = 1;
		else if (CHECKERS_ISBLACK (pos->board[i])) found_b = 1;
	if (!found_b)
	{
		strncpy (comment, who_str[0], 31);
		return RESULT_WHITE;
	}
	if (!found_w)
	{
		strncpy (comment, who_str[1], 31);
		return RESULT_BLACK;
	}
	return RESULT_NOTYET;
}

byte * checkers_movegen (Pos *pos, Player player)
{
	int i, j, diffx, diffy;
	byte movbuf [256];
	byte *movlist, *mp = movbuf;
	byte *board = pos->board;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		if (player == BLACK && !CHECKERS_ISBLACK(board [j * board_wid + i]))
			continue;
		if (player == WHITE && !CHECKERS_ISWHITE(board [j * board_wid + i]))
			continue;
		for (diffx = -1; diffx <= 1; diffx += 2)
		for (diffy = -1; diffy <= 1; diffy += 2)
		{
			if (CHECKERS_ISPAWN (board [j * board_wid + i])
					&& 	diffy != (player == WHITE ? 1 : -1))
				continue;
			if (!ISINBOARD(i+diffx, j+diffy)) continue;
			if (board [(j + diffy) * board_wid + (i + diffx)] != 0)
				continue;
			*mp++ = i; *mp++ = j; *mp++ = 0;
			*mp++ = i + diffx; *mp++ = j + diffy; 
			if ((player == WHITE && (j + diffy) == board_heit - 1) 
					|| (player == BLACK && (j + diffy) == 0))
				*mp++ = (player == WHITE ? CHECKERS_WK : CHECKERS_BK);
			else
				*mp++ = board [j * board_wid + i];
			*mp++ = -1;
		}
		for (diffx = -2; diffx <= 2; diffx += 4)
		for (diffy = -2; diffy <= 2; diffy += 4)
		{
			int val;
			if (CHECKERS_ISPAWN (board [j * board_wid + i])
					&& 	diffy != (player == WHITE ? 2 : -2))
				continue;
			if (!ISINBOARD(i+diffx, j+diffy)) continue;
			if (board [(j + diffy) * board_wid + (i + diffx)] != 0)
				continue;
			val = board [(j + diffy/2) * board_wid + i + diffx/2];
			if ((player == WHITE && !CHECKERS_ISBLACK(val)) || 
					(player == BLACK && !CHECKERS_ISWHITE (val)))
				continue;
			*mp++ = i; *mp++ = j; *mp++ = 0;
			*mp++ = i + diffx; *mp++ = j + diffy; 
			if ((player == WHITE && (j + diffy) == board_heit - 1) 
					|| (player == BLACK && (j + diffy) == 0))
				*mp++ = (player == WHITE ? CHECKERS_WK : CHECKERS_BK);
			else
				*mp++ = board [j * board_wid + i];
			*mp++ = i + diffx/2; *mp++ = j + diffy/2; *mp++ = 0;
			*mp++ = -1;
		}
		
			
	}
	if (mp == movbuf)
		*mp++ = -1;
	*mp++ = -2;
	movlist = (byte *) (malloc (mp - movbuf));
	memcpy (movlist, movbuf, (mp - movbuf));
	return movlist;
}

float checkers_eval (Pos *pos, Player to_play)
{
	float sum = 0;
	int i, j;
	for (i=0; i<board_wid; i++)
	for (j=0; j<board_heit; j++)
	{
		switch (pos->board [j * board_wid + i])
		{
			case CHECKERS_WK: sum += (5 - fabs ((i-3.5) * (j-3.5)) / 10); break;
			case CHECKERS_WP: sum += (1 + j / 10.0); break;
			case CHECKERS_BK: sum -= (5 - fabs ((i-3.5) * (j-3.5)) / 10); break;
			case CHECKERS_BP: sum -= (1 + (board_heit - 1 - j) / 10.0); break;
		}
	}
	return sum;

}

int checkers_getmove (Pos *pos, int x, int y, int type, Player to_play, 
		byte **movp)
{
	static byte move[10];
	byte *mp = move;
	static int oldx = -1, oldy = -1;
	int diffx, diffy;
	byte *board = pos->board;
	if (type != GTKBOARD_BUTTON_RELEASE) return 0;
	if (oldx < 0)
	{
		int val = board [y * board_wid + x];
		if ((CHECKERS_ISWHITE(val) && !(to_play == WHITE)) ||
		(CHECKERS_ISBLACK(val) && !(to_play == BLACK)))
			return -1;
		oldx = x; oldy = y;
		return 0;
	}

	if (x == oldx && y == oldy)
	{
		oldx = -1; oldy = -1; return 0;
	}
	
	diffx = x - oldx;
	if (abs (diffx) == 1) 
	{
		diffy = y - oldy;
		if (abs (diffy) != 1)
		{ oldx = oldy = -1; return -1;}
		if (!CHECKERS_ISKING(board [oldy * board_wid + oldx])
				&& diffy != (to_play == WHITE ? 1 : -1))
		{ oldx = oldy = -1; return -1;}
		if (board [y * board_wid + x] != 0)
		{ oldx = oldy = -1; return -1;}
		*mp++ = oldx; *mp++ = oldy; *mp++ = 0;
		*mp++ = x; *mp++ = y; 
		if ((to_play == WHITE && y == board_heit - 1) 
				|| (to_play == BLACK && y == 0))
			*mp++ = (to_play == WHITE ? CHECKERS_WK : CHECKERS_BK);
		else
			*mp++ = board [oldy * board_wid + oldx];
		*mp++ = -1;
		*movp = move;
		oldx = oldy = -1;
		return 1;
	}
	if (abs (diffx) == 2)
	{
		int val;
		diffy = y - oldy;
		if (abs (diffy) != 2)
		{ oldx = oldy = -1; return -1;}
		if (!CHECKERS_ISKING(board [oldy * board_wid + oldx])
				&& diffy != (to_play == WHITE ? 2 : -2))
		{ oldx = oldy = -1; return -1;}
		if (board [y * board_wid + x] != 0)
		{ oldx = oldy = -1; return -1;}
		val = board [(y-diffy/2) * board_wid + (x-diffx/2)];
		if ((!CHECKERS_ISWHITE(val) && (to_play == BLACK)) ||
		(!CHECKERS_ISBLACK(val) && (to_play == WHITE)))
		{ oldx = oldy = -1; return -1;}
		*mp++ = oldx; *mp++ = oldy; *mp++ = 0;
		*mp++ = oldx+diffx/2; *mp++ = oldy+diffy/2; *mp++ = 0;
		*mp++ = x; *mp++ = y; 
		if ((to_play == WHITE && y == board_heit - 1)
				|| (to_play == BLACK && y == 0))
			*mp++ = (to_play == WHITE ? CHECKERS_WK : CHECKERS_BK);
		else
			*mp++ = board [oldy * board_wid + oldx];
		*mp++ = -1;
		*movp = move;
		oldx = oldy = -1;
		return 1;
	}
	{ oldx = oldy = -1; return -1;}
}

char ** checkers_get_pixmap (int idx, int color)
{
	int bg;
	int i;
	static char pixbuf[CHECKERS_CELL_SIZE * (CHECKERS_CELL_SIZE+1)];
	for(i=0, bg=0;i<3;i++) 
	{ int col = checkers_colors[i+3]; 
		if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	return pixmap_ball_gen (CHECKERS_CELL_SIZE, pixbuf,
			CHECKERS_ISWHITE(idx) ? 0xffffff : 0x0000ff, bg, 
			(idx == CHECKERS_WP || idx == CHECKERS_BP) ? 8 : 12, 24);
}
