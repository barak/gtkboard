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
	assert (movstack_ptr < STACK_SIZE - 1);
	movstack[movstack_ptr] = movdup (move);
	movinvstack[movstack_ptr] = mov_getinv (board, move);
	movstack_ptr++;
	if (movstack_ptr > movstack_max)
		movstack_max = movstack_ptr;
}

byte *movstack_pop ()
{
	if (movstack_ptr == 0)
		return NULL;
	return movstack[--movstack_ptr];
}

//! Truncates a stack to the current poisition. 
/** This will be called when the user makes a move when it is not the final poisition. */
void movstack_trunc ()
{
	int i;
	assert (movstack_ptr <= movstack_max);
	for (i = movstack_ptr; i < movstack_max; i++)
	{
		free (movstack[i]);
		free (movinvstack[i]);
	}
	movstack_max = movstack_ptr;
}

byte * movstack_forw ()
{
	if (movstack_ptr < movstack_max)
		movstack_ptr++;
	else return NULL;
	return movstack[movstack_ptr-1];
}

byte * movstack_back ()
{
	if (movstack_ptr > 0)
		movstack_ptr--;
	else return NULL;
	return movinvstack[movstack_ptr];
}

void movstack_free ()
{
	int i;
	for (i=0; i<movstack_max; i++)
	{
		free (movstack[i]);
		free (movinvstack[i]);
	}
	movstack_max = movstack_ptr = 0;
}


/*
   state stack
*/

static int statestack_ptr = 0, statestack_max = 0;

static void *statestack[STACK_SIZE];

// we create a new copy of state
void statestack_push (void *state)
{
	void *newstate;
	assert (statestack_ptr < STACK_SIZE - 1);
	newstate = malloc (game_state_size);
	assert (newstate);
	memcpy (newstate, state, game_state_size);
	statestack[statestack_ptr] = newstate;
	statestack_ptr++;
	if (statestack_ptr > statestack_max)
		statestack_max = statestack_ptr;
}

void *statestack_peek ()
{
	if (statestack_ptr == 0)
		return NULL;
	return statestack[statestack_ptr-1];
}

void *statestack_pop ()
{
	if (statestack_ptr == 0)
		return NULL;
	return statestack[--statestack_ptr];
}

void statestack_trunc ()
{
	int i;
	assert (statestack_ptr <= statestack_max);
	for (i = statestack_ptr; i < statestack_max; i++)
		free (statestack[i]);
	statestack_max = statestack_ptr;
}

void * statestack_forw ()
{
	if (statestack_ptr < statestack_max)
		statestack_ptr++;
	else return NULL;
	return statestack[statestack_ptr-1];
}

void * statestack_back ()
{
	if (statestack_ptr > 0)
		statestack_ptr--;
	else return NULL;
	return statestack[statestack_ptr-1];
}

void statestack_free ()
{
	int i;
	for (i=0; i<statestack_max; i++)
		free (statestack[i]);
	statestack_max = statestack_ptr = 0;
}
