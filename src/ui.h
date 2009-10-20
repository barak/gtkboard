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
#ifndef _GTKBOARD_H_
#define _GTKBOARD_H_

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include "game.h"

// FIXME: confusing name. Make this 2 macros: human_to_move and machine_to_move
#define player_to_play (cur_pos.player == WHITE ? ui_white : ui_black)

enum {NONE, HUMAN, MACHINE};

extern Game *games[];

extern Pos cur_pos;

extern const int num_games;

extern FILE *move_fin, *move_fout;

extern int state_player;

extern gboolean engine_flag;

extern gboolean ui_stopped;
extern gboolean ui_cheated;
extern gboolean ui_gameover;
extern gboolean state_gui_active;
extern gboolean game_file_label, game_rank_label;

extern Game *opt_game;
extern FILE *opt_infile;
extern int opt_delay;
extern int opt_quiet;
extern int ui_white;
extern int ui_black;
extern int opt_white;
extern int opt_black;
extern int opt_verbose;

gboolean impl_check ();
void ui_send_make_move ();
void set_game_params ();
void reset_game_params ();
void ui_terminate_game ();
void ui_start_game ();
void start_game_num (int);
void ui_check_who_won ();
void ui_cancel_move ();
void game_set_init_pos_def (Pos *);

#endif
