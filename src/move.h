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
#ifndef _MOVE_H_
#define _MOVE_H_

/** \file move.h
  \brief Operations on moves.

  A move is an array of <tt>byte</tt>s, which are simply <tt>gint8</tt>s.
  A move is a sequence of movelets terminated by -1. A movelet is a sequence
  of 3 bytes which describes what happens to a particular square in the board.
  Movelet format is "x y val" where val is the new value of the square (x, y).
  Thus the move corresponding to "1. e4" in chess is "4 1 0 4 3 6 -1" 
  (i.e, square (4, 0), which means e2 becomes empty, and square (4, 3), which is
  e4, becomes 6, assuming that white pawn is represented by 6. Note that
  row and col numbers start from 0.) When the move is written to file or sent
  to the engine, it will be converted to a human friendly format.
*/

#include <stdio.h>
#include <glib.h>

#ifndef byte 
#define byte gint8
#endif


//! Make a copy of the move
/** The new move is malloc()d and so must be free()d by the caller.*/
byte *movdup (byte *);

//! Compares two moves byte by byte. Literal not semantic comparison. Returns TRUE if the moves are equal
gboolean movcmp_literal (byte *move1, byte *move2);

//! Makes the move src to the move dest.
/** Memory is assumed to have already been allocated for dest. */
int movcpy (byte *dest, byte *src);

//! Returns the inverse of a move in the given board
/** The move format does not specify the old value of the squares. So
it is not possible to go from the new #Pos to the old Pos. To be able to
do this we need to get the "inverse" of the move using the current board*/
byte *mov_getinv (byte *, byte *);

//! Applies the move to the board.
void move_apply (byte *, byte *);

//! Writes a move to the pipe in human friendly format
void move_fwrite (byte *, FILE *);

//! Parses a string into a move
byte * move_read (char *);

//! Reads a move from the pipe.
byte *move_fread (FILE *);

//! Same as move_fread(), except it expects the stream to begin with "ACK ". Returns NULL otherwise. 
byte *move_fread_ack (FILE *);

//! Reads a line from the stream.
char *line_read (FILE *);

//! Same as move_fwrite(), but precedes line with "ACK ".
void move_fwrite_ack (byte *, FILE *);

//! Writes "NAK " followed by arbitrary error message.
void move_fwrite_nak (char *, FILE *);

//! Returns the next move in a movlist.
/** A movlist is also an array of <tt>byte</tt>s. It is a sequence of moves terminated by -2 */
byte *movlist_next (byte *);
#endif
