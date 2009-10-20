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
#include "stack.h"
#include "game.h"
#include "move.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/** \file stack.c
   \brief implements a stack for navigating undoing and redoing moves

   stack is as follows:
   0 ---> movstack_ptr : "back" list;
   movstack_ptr ---> movstack_max : "forward" list
*/

//! Start offset of the stack in the circular buffer
static int movstack_start = 0;

//! Current position in the stack
static int movstack_ptr = 0;

//! Current size of the stack
static int movstack_max = 0;

//! Maximum size of stack
#define STACK_SIZE 4096

//! Array for moves
static byte *movstack[STACK_SIZE];

//! Array for move inverses. See mov_getinv().
static byte *movinvstack[STACK_SIZE];

void stack_free ()
{
	movstack_free ();
	statestack_free ();
}

void movstack_init ()
{
	// uh?
}

int movstack_get_num_moves()
{
	return movstack_ptr;
}

void movstack_push (byte *board, byte *move)
{
	movstack[(movstack_start + movstack_ptr) % STACK_SIZE] = movdup (move);
	movinvstack[(movstack_start + movstack_ptr) % STACK_SIZE] = mov_getinv (board, move);
	if (movstack_ptr < STACK_SIZE - 1) {
		movstack_ptr++;
		if (movstack_ptr > movstack_max)
			movstack_max = movstack_ptr;
	} else {
		free (movstack[movstack_start]);
		free (movinvstack[movstack_start]);
		movstack_start = (movstack_start + 1) % STACK_SIZE;
	}
}

byte *movstack_pop ()
{
	if (movstack_ptr == 0)
		return NULL;
	movstack_ptr--;
	return movstack[(movstack_start + movstack_ptr) % STACK_SIZE];
}

//! Truncates a stack to the current position. 
/** This will be called when the user makes a move when it is not the final position. */
void movstack_trunc ()
{
	int i;
	assert (movstack_ptr <= movstack_max);
	for (i = movstack_ptr; i < movstack_max; i++)
	{
		int j = (movstack_start + i) % STACK_SIZE;
		free (movstack[j]);
		free (movinvstack[j]);
	}
	movstack_max = movstack_ptr;
}

byte * movstack_forw ()
{
	if (movstack_ptr < movstack_max)
		movstack_ptr++;
	else return NULL;
	return movstack[(movstack_start + movstack_ptr - 1) % STACK_SIZE];
}

byte * movstack_back ()
{
	if (movstack_ptr > 0)
		movstack_ptr--;
	else return NULL;
	return movinvstack[(movstack_start + movstack_ptr) % STACK_SIZE];
}

void movstack_free ()
{
	int i;
	for (i=0; i<movstack_max; i++)
	{
		int j = (movstack_start + i) % STACK_SIZE;
		free (movstack[j]);
		free (movinvstack[j]);
	}
	movstack_max = movstack_ptr = 0;
}


/*
   state stack
*/

static int statestack_start = 0, statestack_ptr = 0, statestack_max = 0;

static void *statestack[STACK_SIZE];

// we create a new copy of state
void statestack_push (void *state)
{
	void *newstate;
	newstate = malloc (game_state_size);
	assert (newstate);
	memcpy (newstate, state, game_state_size);

	statestack[(statestack_start + statestack_ptr) % STACK_SIZE] = newstate;

	if (statestack_ptr < STACK_SIZE - 1) {
		statestack_ptr++;
		if (statestack_ptr > statestack_max)
			statestack_max = statestack_ptr;
	} else {
		free (statestack[statestack_start]);
		statestack_start = (statestack_start + 1) % STACK_SIZE;
	}
}

void *statestack_peek ()
{
	if (statestack_ptr == 0)
		return NULL;
	return statestack[(statestack_start + statestack_ptr - 1) % STACK_SIZE];
}

void *statestack_pop ()
{
	if (statestack_ptr == 0)
		return NULL;
	statestack_ptr--;
	return statestack[(statestack_start + statestack_ptr) % STACK_SIZE];
}

void statestack_trunc ()
{
	int i;
	assert (statestack_ptr <= statestack_max);
	for (i = statestack_ptr; i < statestack_max; i++)
		free (statestack[(statestack_start + i) % STACK_SIZE]);
	statestack_max = statestack_ptr;
}

void * statestack_forw ()
{
	if (statestack_ptr < statestack_max)
		statestack_ptr++;
	else return NULL;
	return statestack[(statestack_start + statestack_ptr - 1) % STACK_SIZE];
}

void * statestack_back ()
{
	if (statestack_ptr > 0)
		statestack_ptr--;
	else return NULL;
	return statestack[(statestack_start + statestack_ptr - 1) % STACK_SIZE];
}

void statestack_free ()
{
	int i;
	for (i=0; i<statestack_max; i++)
		free (statestack[(statestack_start + i) % STACK_SIZE]);
	statestack_max = statestack_ptr = 0;
}

// Local Variables:
// tab-width: 4
// End:
