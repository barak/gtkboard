/*  This file is a part of gtkboard, a board games system.
    Copyright (C) 2003, Arvind Narayanan <arvindn@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

*/
#include "move.h"
#include "game.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

extern int board_wid, board_heit;

// FIXME: this is ugly
/*extern void board_refresh_cell (int, int);
extern void board_set_cell (int, int, byte);
*/
	
int movcpy (byte *dest, byte *src)
{
	int i;
	for (i=0; i%3 || src[i] != -1; i++)
		dest[i] = src[i];
	dest[i] = -1;
	return i;
}

gboolean movcmp_literal (byte *move1, byte *move2)
{
	int i;
	assert (move1);
	assert (move2);
	for (i=0; ; i++)
	{
		if (move1[i] == -1 && move2[i] == -1) return TRUE;
		if (move1[i] != move2[i]) return FALSE;
	}
}

byte *movlist_next (byte *move)
{
	while (*move != -1)
		move += 3;
	return move + 1;
}

void move_apply (byte *board, byte *move)
{
	int i, x, y;
	if (!move) return;
	for (i=0; move[3*i] != -1; i++)
	{
		x = move[3*i]; y = move[3*i+1];
		board [y * board_wid + x] = move [3*i+2];
	}
}



void move_fwrite (byte *move, FILE *fout)
{
	int i;
	if (!move)
	{
		fprintf (stderr, "move_fwrite got NULL arg\n");
		return;
	}
	/* write in human friendly format -- start row and col num from 1 not 0*/
	for (i=0; move[3*i] != -1; i++)
		fprintf (fout, "%d %d %d ", move[3*i] + 1, 
			move[3*i+1] + 1, move[3*i+2]);
	fprintf (fout, "\n");	
	fflush (fout);
}

void move_fwrite_ack (byte *move, FILE *fout)
{
	fprintf (fout, "ACK ");
	move_fwrite (move, fout);
}

void move_fwrite_nak (char *str, FILE *fout)
{
	fprintf (fout, "NAK ");
	if (str)
		fprintf (fout, "%s", str);
	fprintf (fout, "\n");
	fflush (fout);
}

byte *move_read (char *line)
{
	static byte movbuf[1024];
	byte *new, *tmp;
	int nc = 0;
	tmp = line;
	while (1)
	{
		movbuf[nc] = (byte) strtol (tmp, (char **) &new, 10);
		if (new == tmp)
			break;
		tmp = new;
		if (nc % 3 == 0)
		{
			assert (movbuf[nc] >= 1 && movbuf[nc] <= board_wid);
			movbuf[nc]--;
		}
		else if (nc % 3 == 1)
		{
			assert (movbuf[nc] >= 1 && movbuf[nc] <= board_heit);
			movbuf[nc]--;
		}
		nc++;
	}
	assert (nc % 3 == 0);
	movbuf[nc] = -1;
	return movbuf;
}

static byte linebuf [4096];

byte *move_fread (FILE *fin)
{
	fgets (linebuf, 4096, fin);
	return move_read (linebuf);
}

byte *move_fread_ack (FILE *fin)
{
	fgets (linebuf, 4096, fin);
	//printf ("%s\n", linebuf);
	if (strncasecmp (linebuf, "ACK", 3))
		return NULL;
	return move_read (linebuf + 4);
}

char *line_read (FILE *fin)
{
	fgets (linebuf, 4096, fin);
	return linebuf;
}


byte *movdup (byte *move)
{
	byte *newmov;
	int len;
	for (len=0; move[3*len] != -1; len++)
		;
	newmov = (byte *) malloc (3 * len + 1);
	memcpy (newmov, move, 3 * len + 1);
	return newmov;
}

byte *mov_getinv (byte *board, byte *move)
{
	int i;
	byte *inv = movdup (move);
	for (i=0; move[3*i] != -1; i++)
	{
		int x = move [3*i], y = move [3*i+1];
		inv [3*i+2] = board [y * board_wid + x];
	}
	return inv;
}


