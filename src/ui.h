#ifndef _GTKBOARD_H_
#define _GTKBOARD_H_

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include "game.h"

// FIXME: confusing name. Make this 2 macros: human_to_move and machine_to_move
#define player_to_play (state_player == WHITE ? ui_white : ui_black)

enum {NONE, HUMAN, MACHINE};

extern Game *games[];

extern Pos cur_pos;

extern int num_games;

extern FILE *move_fin, *move_fout;

extern int state_player;

extern gboolean engine_flag;

extern gboolean ui_stopped;
extern gboolean ui_cheated;
extern gboolean ui_gameover;
extern gboolean state_gui_active;

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
void game_setinitpos_def (Pos *);

#endif
