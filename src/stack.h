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
#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>
#include <glib.h>

#ifndef byte 
#define byte gint8
#endif

void stack_free ();

void movstack_init ();
int movstack_get_num_moves ();
void movstack_push (byte *, byte *);
byte *movstack_pop ();
void movstack_trunc ();
byte * movstack_forw ();
byte * movstack_back ();
void movstack_free ();

void statestack_push (void *state);
void *statestack_peek ();
void *statestack_pop ();
void statestack_trunc ();
void *statestack_forw ();
void *statestack_back ();
void statestack_free ();
#endif
