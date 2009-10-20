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
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "game.h"

//! A struct representing a command that the engine understands.
typedef struct
{
	//! The string
	char * proto_str;
	//! Has this command been implemented
	int isimpl;
	//! Function which will execute this command
	void (*impl_func) (char *);
} Command;

extern Command  commands[];

ResultType engine_eval (Pos *, /*Player,*/ float *);

//! Functions that do the actual thinking must periodically call this function.
/** It checks if new commands have arrived. */
void engine_poll ();


#endif
