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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <glib.h>

#include "move.h"

/** \file hash.c
   \brief hash table which implements transposition tables.

   A hash entry stores the following information: 
   value of the node
   depth to which it has been explored
   verification code (secondary hash value)
   
   if there is a collision and the table is less than (say) 75% full, search
   sequentially till an empty cell is found
   otherwise just kick out the previous entry
   (if depth of the current entry is smaller than the earlier entry, search
   sequentially till a shallower node is found and kick it out)

   When used with DFID, if l levels were completed on the previous move, l-2
   levels (ply) will be completed almost instantly on this move. Even if we are
   unable to complete level l on this move, it mattereth not, for the 
   partial information will be there in the hash table and will be useful 
   for the next move.
 */

#ifndef uint
#define uint unsigned
#endif

#ifndef byte
#define byte gint8
#endif

// FIXME: write a function called debug or something instead of doing it this way
extern int opt_verbose;

/* TODO: use a secondary hash fn of 16 bits to use as increment in case
   of collision. Also use it for 16 of the 48 bits of the check field */

/* TODO: hash the state also. games like chess will need it */

typedef struct
{
	guint32 check;	/* should this be 32 or 64 ? */
				/* hmm.. maybe 48: that would fit nicely into 3 words */
	float eval;
	int num_moves:16;
	int depth:8;
	int free:1;
	int stale:1;
	byte *best_move;
} hash_t;

static int hash_table_size = 1 << 16;
static int hash_table_max = 3 * 1 << 14;
static int num_hash_coeffts = 1024;
static uint *hash_coeffts = NULL;
static hash_t *hash_table = NULL;
static int hash_filled = 0;
static int hash_eval_hits = 0, hash_eval_misses = 0;
static int hash_move_hits = 0, hash_move_misses = 0;

static void hash_init ()
	/* malloc the stuff */
	// FIXME: Do we need to free this?
{
	int i;
	if (hash_coeffts) return;
	hash_coeffts = (uint *) malloc (num_hash_coeffts * sizeof (uint));
	assert (hash_coeffts);
	for (i=0; i<num_hash_coeffts; i++)
		hash_coeffts[i] = (random() << 1u) + 1;
	hash_table = (hash_t *) malloc (hash_table_size * sizeof (hash_t));
	assert (hash_table);
	for (i=0; i<hash_table_size; i++)
		hash_table[i].free = 1;
}

static uint get_hash (byte *pos, int poslen)
{
	int i;
	uint hash = 0;
	if (!hash_coeffts)
		hash_init ();
	for (i = 0; i < poslen; i++)
		hash += (pos[i] * hash_coeffts[i%num_hash_coeffts]);
	return hash;
}

static uint get_check (byte *pos, int poslen)
	/* just another hash function */
{
	int i;
	uint check = 0;
	if (!hash_coeffts)
		hash_init ();
	for (i=0; i < poslen; i++)
		check += (pos[i] * hash_coeffts
				[num_hash_coeffts-1-(i%num_hash_coeffts)]);
	return check;
}

// FIXME: hash should work with state as well
void hash_insert (byte *pos, int poslen, int num_moves, int depth, float eval, 
		byte *move)
{
	uint start = get_hash (pos, poslen) % hash_table_size;
	uint check = get_check (pos, poslen);
	int idx, cnt = 0;
	for (idx=start; cnt <= 10; idx = (idx+1) % hash_table_size, cnt++)
	{
		if (hash_table[idx].free)
		{
			if (hash_filled >= hash_table_max)
				continue;
			hash_filled++;
			break;
		}
		if (hash_table[idx].stale)
			break;
		/* even if the same position is already there it should be 
			overwritten with the new depth */
		if (hash_table[idx].check == check)
			break;
		if (hash_filled >= hash_table_max && depth > hash_table[idx].depth)
			break;
	}
	if (hash_table[idx].free == 0 && hash_table[idx].best_move)
		free (hash_table[idx].best_move);
	hash_table[idx].free = 0;
	hash_table[idx].check = check;
	hash_table[idx].num_moves = num_moves;
	hash_table[idx].eval = eval;
	hash_table[idx].depth = depth;
	hash_table[idx].stale = 0;
	hash_table[idx].best_move = (move ? movdup (move) : NULL);
}

int hash_get_eval (byte *pos, int poslen, int num_moves, int depth, float *evalp)
	/* get the eval of a pos if it is there in the hash table 
	   retval = was it found
	   eval = answer*/
{
	uint idx = get_hash (pos, poslen) % hash_table_size;
	uint check = get_check (pos, poslen);
	for (; hash_table[idx].free == 0; idx = (idx+1) % hash_table_size)
	{
		if (hash_table[idx].check == check)	/* found it */
		{
			/* don't compare 2 evals at different depths*/
			if (hash_table[idx].depth == depth 
					&& hash_table[idx].num_moves == num_moves)
			{
				if (evalp)
					*evalp = hash_table[idx].eval;
				hash_table[idx].stale = 0;
				hash_eval_hits++;
				return 1;
			}
			break;
		}
	}
	/* found a free slot before encountering this pos */
	hash_eval_misses++;
	return 0;
}

byte * hash_get_move (byte *pos, int poslen, int num_moves)
{
	uint idx = get_hash (pos, poslen) % hash_table_size;
	uint check = get_check (pos, poslen);
	for (; hash_table[idx].free == 0; idx = (idx+1) % hash_table_size)
	{
		if (hash_table[idx].check == check)	/* found it */
		{
			if (hash_table[idx].num_moves == num_moves)
			{
				hash_table[idx].stale = 0;
				hash_move_hits++;
				return hash_table[idx].best_move;
			}
			break;
		}
	}
	/* found a free slot before encountering this pos */
	hash_move_misses++;
	return NULL;
}

void hash_clear ()
{
	int i;
	for (i=0; i<hash_table_size; i++)
	{
		if (hash_table[i].free == 0 && hash_table[i].best_move)
			free (hash_table[i].best_move);
		hash_table[i].free = 1;
	}
	hash_filled = 0;
}

void hash_print_stats ()
{
	int i, stale=0;
	if (opt_verbose) printf ("hashtable: size=%d \tfilled=%d \teval_hits=%d \teval_misses=%d \tmove_hits=%d \tmove_misses=%d \t",
			hash_table_size, hash_filled, 
			hash_eval_hits, hash_eval_misses, hash_move_hits, hash_move_misses);
	hash_eval_hits = hash_eval_misses = hash_move_hits = hash_move_misses = 0;
	for (i=0; i<hash_table_size; i++)
	{
		if (!hash_table[i].free && hash_table[i].stale)
			stale++;
		hash_table[i].stale = 1;
	}
	if (opt_verbose) printf ("stale=%d\n", stale);
}
