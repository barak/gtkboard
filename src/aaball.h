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
#ifndef _AABALL_H_
#define _AABALL_H_

char ** pixmap_ball_gen(int, char *, int, int, float, float);
char ** pixmap_header_gen(int, char *, int, int);
char ** pixmap_die_gen(int, char *, int, int, float, float, int);

void rgbmap_ball_gen(int, unsigned char *, int, int, float, float);
void rgbmap_ball_gen_nobg(int, unsigned char *, int, int, float, float);
void rgbmap_ball_shadow_gen(int, unsigned char *, int, int, float, float, int);
void rgbmap_square_gen (int, unsigned char *, int, int, float);

#endif
