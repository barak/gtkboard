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
#ifndef _GAME_H_
#define _GAME_H_

#include<glib.h>

/** \file game.h
  \brief Header file that must be included by all games.

  This is the only file that the games need to include.
*/

//! Both moves and positions are arrays of <tt>byte</tt>s
#ifndef byte 
#define byte gint8
#endif

//! Used in a two player game to represent whose move it is.
/* White is always the player that moves first. Internally they
 are always called white and black even if the game wants the user to
 see them as red and blue, or Bob and Sue etc.*/
typedef enum {WHITE, BLACK} Player;

//! Used for representing the type of user input in game_getmove() and game_getmove_kb()
/** This layer of abstraction exists so that to write a game it will not be necessary to have the g[dt]k headers */
typedef enum 
{
	GTKBOARD_BUTTON_PRESS = 1, 
	GTKBOARD_BUTTON_RELEASE, 
	GTKBOARD_MOTION_NOTIFY,
	GTKBOARD_LEAVE_NOTIFY,
	GTKBOARD_KEY_PRESS,
	GTKBOARD_KEY_RELEASE,
	GTKBOARD_GAME_START,
	GTKBOARD_HUMAN_MOVE,
	GTKBOARD_MACHINE_MOVE,
	GTKBOARD_HISTORY_MOVE
} GtkboardEventType;

//! Abstraction of a user event
typedef struct 
{
	//! Type of event
	GtkboardEventType type;

	//! (For mouse events) x coordinate of the *cell* -- ranges from 0 to board_wid
	int x;

	// y coordinate of the cell
	int y;

	//! Actual x coordinate
	int pixel_x;

	//! Actual y coordinate
	int pixel_y;

	//! The key that was pressed
	int key;

	//! The move (when type is GTKBOARD_{HUMAN,MACHINE,HISTORY}_MOVE)
	byte *move;
} GtkboardEvent;

//! Used for representing the result of game_who_won()
typedef enum 
{
	//! White won
	RESULT_WHITE, 
	//! Black won
	RESULT_BLACK, 
	//! Draw
	RESULT_TIE, 
	//! Game not over yet
	RESULT_NOTYET,
	//! User completed the game successfully (single player game)
	RESULT_WON,
	//! User failed to complete game (single player game)
   /**	eg: exhausted all rows in mastermind (mastermind_who_won()) */
	RESULT_LOST,
	//! Anything else
	RESULT_MISC
} ResultType;

//! The return type of game_event_handler()
typedef enum
{
	INPUT_ILLEGAL = -1,
	INPUT_NOTYET = 0,
	INPUT_LEGAL = 1
} InputType;

//! The move and all the info. associated with it
typedef struct
{
	//! The move. 
	/** This is a sequence of the form 
	[x_1, y_1, v_1, x_2, y_2, v_2, ... x_n, y_n, v_n, -1] where [x_i, y_i, v_i]
	says that the value of the square (x_i, y_i) should change to v_i. */
	byte *move;

	//! (Only client side) the rendering info. associated with this move
	int *rmove;
	//! Any game-specific custom information associated with this move.
	/** The purpose of having this is that it will be sent from UI to engine and engine to UI */
	gchar *custom;

	//! (Only client side) Help message
	gchar *help_message;

	//! The human-readable version of the move eg: "e4" in chess
	gchar *human_readable;
}MoveInfo;

//! the completion status of the game
/** 
  This is the type of #game_doc_about_status.
  At the moment it only matters if the status is STATUS_COMPLETE or not 
 */
typedef enum
{
	//! don't use this
	STATUS_NONE,

	//! unplayable
	STATUS_UNPLAYABLE,

	//! partial
	STATUS_PARTIAL,

	//! complete
	STATUS_COMPLETE
} CompletionStatus;

//! values of #game_file_label and #game_rank_label
typedef enum {
	FILERANK_LABEL_TYPE_NONE, 
	FILERANK_LABEL_TYPE_NUM,
	FILERANK_LABEL_TYPE_ALPHA,
	FILERANK_LABEL_TYPE_ALPHA_CAPS
} FILERANK_LABEL_TYPE;

#define FILERANK_LABEL_TYPE_MASK 0x3

//! if #game_file_label or #game_rank_label is ORed with this the order of file/rank labels will be reversed
#define FILERANK_LABEL_DESC (1 << 2)

//! The return value of game_eval() should be larger than GAME_EVAL_INFTY in absolute value to indicate that the game is over.
#define GAME_EVAL_INFTY (1.0e10)

//! Indicates whether the square (x, y) is legal
#define ISINBOARD(x,y) ((x)>=0 && (y)>=0 && (x)<board_wid && (y)< board_heit)

//! Home page for the game <tt>x</tt>
#define GAME_DEFAULT_URL(x) "http://gtkboard.sourceforge.net/games"

struct _Game;

//! The Game struct gives essential information about the game
/** Only information that <b>must</b> be provided by every game
 is declared here. Of course, there are many other variables and 
 functions that can be used to customize the game. These must be
 set in the function game_init(). The good thing is that you can
 get your game running first and only use new features as you need
 them.*/
typedef struct _Game
{
	//! The size of each square on the board in pixels
	int cell_size;

	//! The number of rows in the board
	int board_wid;

	//! The number of columns in the board
	int board_heit;

	//! The number of types of pieces that the game uses 
	/** (12 for chess (chess.c), for instance -- 6 white and 6 black). 
	  The maximum value is 127.
	 In several games, like mastermind (mastermind.c), the value of num_pieces
	 is more than the actual number of pieces that the user 
	 sees. This is because several logical pieces are mapped to the same
	 image and the value of the piece is used to encode some state information
	 about that square.*/
	int num_pieces;

	//! An array which gives the colors for the squares of the board
	char *colors;

	//! The initial position
	int * init_pos;

	//! An array of pixmaps representing the pieces
	char *** pixmaps;

	//! Name of the game
	char *name;

	//! Which group does this game belong to. 
	/** In the menu, the game will be nested within this group. Use NULL for no group. */
	char *group;
	
	//! A pointer to the function that will be called when initializing the game
	void (*game_init) (struct _Game *);
}Game;

//! How to render a square
typedef enum 
{
	//! Just use the default pixmap
	RENDER_NONE, 
	//! Draw a colored border around the square. See #game_highlight_colors
	RENDER_HIGHLIGHT1, RENDER_HIGHLIGHT2, RENDER_HIGHLIGHT3,
	//! Shade the square. Not yet implemented
	RENDER_SHADE1, RENDER_SHADE2, RENDER_SHADE3,
	//! Make the square look like a button
	RENDER_BUTTONIZE,
	//! Hide the square (show the background color)
	RENDER_HIDE,
	//! Use a different pixmap. 
	/** The pixmap must be specified in the 8-15th bits, 
	  i.e, if you want to use the pixmap of the piece p,
	  then you must set the render value to p << 8 | RENDER_REPLACE */
	RENDER_REPLACE,
} RenderType;

//! A struct describing a position in a game.
typedef struct
{
	//! Which game is going on
	Game *game;
	
	//! An array representing the pieces of each square.
	/** The size of the array is #board_wid * #board_heit.
	  For each pair (x, y), board[y * board_wid + x] is a value between 0
	  and num_pieces inclusive which gives the piece at the square (x, y).
	  0 always indicates an empty square. The origin of the coordinates is
	  at the bottom left. */
	byte *board;

	//! Additional information about how to render the square
	/** For example, highlight, shade, hide etc. See #RenderType*/
	/* A note on why this is int* while board is byte*: the latter will
	 be used by the engine, and so size is important. But render can be 
	 big because it will be used only by the engine which will have only
	 once instance of Pos*/
	int *render;

	//! Which player has the move. 
	Player player;

	//! State information required to completely describe the position
	/** Some games are <i>stateful</i>, which means that the position can not
	  be completely described by the state of the board alone. 
	  In chess (chess.c), for example, we need to keep track of whether
	  either player can castle, etc. The variable state points to a 
	  struct which is defined by the game. It is modified using the function
	  game_newstate().*/
	void *state;

	//! Client-side state information () (currently unused)
	void *ui_state;

	//! The number of moves that have been made to reach the current position.
	/** In two-player games, it represents the number of ply.*/
	int num_moves;

	//! (engine only) If this position has been generated during search, how deep from the root node is it.
	int search_depth;
}Pos;

//! If you have implemented more than one evaluation function then you put them in an array of structs of type HeurTab. Its unlikely that you'll need to know about this. See #game_htab for more details.
typedef struct
{
	//! The user-visible name of the evaluation function.
	char *name;

	//! A pointer to the function. See game_eval().
	float (*eval_fun) (Pos *, int);

	//! A description of the function so that the user can know what its about.
	char *comment;

	//! Currently unused.
	char *args;
}HeurTab;

typedef struct
{
	//! The name to show in the Levels menu
	char *name;

	//! Pointer to the Game (Each level is treated as a separate Game)
	Game *game;
}GameLevel;

//! A pointer to the game's evaluation function. 
/** Only for two player games. It <b>must</b> be implemented if you want
  the computer to be able to play the game. */
extern ResultType (*game_eval) (Pos *pos, Player player, float *eval);

//! A pointer to the game's incremental evaluation function. 
/** Only for two player games. This is an advanced feature: if you feel
 that being forced to look at the whole board for each call to game_eval
 is inefficient, you can write an incremental evaluation function which
 takes a position and the move being made and returns the <i>difference</i> in
 the eval of the original and final positions. Note that you still need
 to implement game_eval even if you implement this function. Since
 premature optimization is the root of all evil, it is highly recommended
 that you get your game working and stable before you think of implementing
 this function :)*/
extern ResultType (*game_eval_incr) (Pos *pos, byte *move, float *eval);

//! Should we use the incr eval function
extern gboolean (*game_use_incr_eval) (Pos *pos);

//! A function to search and return the best move - for games for which minimax is not appropriate
extern void (*game_search) (Pos *pos, byte **move);

//! A pointer to the game's move generation function.
/** Only for two player games. It <b>must</b> be implemented if you want
  the computer to be able to play the game. 

 It returns a list of moves possible in a given position. See move.h
 for documentation of the MOVLIST format. Plot4's movegen function 
 (plot4_movegen()) is a good example of a simple movegen function.

 The move list (array) should be malloc'd inside this function 
 and will be freed by the caller.
 */
extern byte * (*game_movegen) (Pos *);


//! This takes a mouse click and returns the move that it corresponds to.
/**	@param pos 
	@param x x coordinate of the square that was clicked
	@param y y coordinate of the square that was clicked
	@param type type of event
	@param to_play whose turn is it
	@param movp a pointer to store the move in if the move is valid
	@param returns: >0 if move is valid, 0 if more information is needed, -1 if move is illegal // TODO: there should be an enum for this

	pentaline_getmove() is a good example of a minimal getmove function.
 
 */
extern int (*game_getmove) (Pos *pos, int x, int y, GtkboardEventType type, Player to_play, byte ** movp, int **renderp);

//! The all-in-one function that makes game_getmove and game_getmove_kb deprecated
/** */
extern InputType (*game_event_handler) (Pos *pos, GtkboardEvent *event, MoveInfo *move_info_p);

//! Takes a keypress and returns the move that it corresponds to.
/**	@param pos
	@param key the key that the user pressed
	@param to_play
	@param movp a pointer to the move. The game must allocate memory for this (statically).
	@param rmovp pointer to rendering change
*/
extern int (*game_getmove_kb) (Pos *pos, int key, byte ** movp, int **rmovp);

//! Checks if the game is over, and if so, who has won
/** This function is called after every move, both for single player and two player games. 
 @param scorep pointer to the "score". If game_who_won sets this, the score will be displayed in the score field of the statusbar.
*/
extern ResultType (*game_who_won) (Pos *pos, Player player, char ** scorep);

//! Pointer to function which sets the game's initial position.
/** In some games such as maze (maze.c), the initial position is not constant but randomly generated. Such functions use game_set_init_pos. The function is expected to set the value of pos->board.*/
extern void (*game_set_init_pos) (Pos *pos);

//! Sets the initial state of the rendering hints
extern void (*game_set_init_render) (Pos *pos);

//! The rendering hints associated with the move
extern void (*game_get_render) (Pos *pos, byte *move, int **rmovp);

//! Returns the pixmap for a piece.
/** In many games, the pixmaps are generated at runtime (see aaball.c). Such games use this function. The second argument color is 0 or 1 depending on whether the piece will be shown on a light square or a dark square. If your pixmap is antialiased you need this.*/
extern char ** (*game_get_pixmap) (int piece, int color);

//! Same as game_get_pixmap() but returns a rgbmap instead of pixmap.
extern guchar * (*game_get_rgbmap) (int piece, int color);

//! Pointer to animation callback which will be called periodically
extern int (*game_animate) (Pos *pos, byte ** movp);

//! Pointer to function which will compute the new state from the current position and the move
/** The returned state should be a pointer to a statically declared structure. */
extern void * (*game_newstate) (Pos *pos, byte * move);

//! Called at the end of every game.
extern void (*game_free) ();

//! The default value of game_set_init_pos()
//extern void game_set_init_pos_def (Pos *);

//! This is called after each move the user completes
/** The user may have made some clicks which do not complete a move and then
 clicked the back button, for example. Then the game must forget the saved clicks.*/
extern void (*game_reset_uistate) ();

//! Globals for convenience.
extern int board_wid, board_heit, cell_size, num_pieces;

//! Are we a single player game or a two-player game? DEFAULT: FALSE.
extern int game_single_player;

//! Is the user allowed to undo move and still get on the highscores (only for single player games; default: FALSE)
extern gboolean game_allow_undo;

//! Determines how frequently to call the game's animation callback function (game_animate()). Default: 0.
extern int game_animation_time;

//! Whether or not to consider animations "moves". Default: TRUE.
extern gboolean game_animation_use_movstack;

//! Are we a stateful game. Default: FALSE
extern gboolean game_stateful;

//! Should the lines between the rows and columns be drawn. Default: FALSE.
/** Example of game which draws boundaries: pentaline (pentaline.c)
    Example of game which doesn't draw boundaries: memory (memory.c) */
extern gboolean game_draw_cell_boundaries;

//! Colors to use for highlighting squares.
extern char *game_highlight_colors;

//! Should we allow the user to move back and forward in the game. Default: TRUE
/** You should not set this to FALSE unless you have a compelling reason to do so.
  Don't worry about the user cheating and getting a highscore that they don't deserve -- if the user ever clicks back, highscores will be disabled for that game :-) Currently only tetris (tetris.c) sets this to FALSE, because of some complex client-server related issues.*/
extern gboolean game_allow_back_forw;

//! Should the user's clock start ticking as soon as the game is selected. Default: FALSE.
/** Doesn't make a lot of sense for two player games. In games like maze (maze.c), the user can solve the maze without even making a move, so it would be unfair to let them look at the maze without starting the clock. */
extern gboolean game_start_immediately;

//! (Only for two player games) Is Settings->Flip Board active. For single player games it is always inactive.
extern gboolean game_allow_flip;

//! How to display names of rows and columns. This should be of type FILERANK_LABEL_TYPE, optionally ORed with FILERANK_LABEL_DESC
extern gboolean game_file_label, game_rank_label;
	
//! Size of the Pos::state structure
/** For stateful games, you need to specify the size of the state structure (as defined by the sizeof operator.) */
extern int game_state_size;

extern GameLevel *game_levels;

//! Array of structs representing evaluation functions.
extern HeurTab *game_htab;

//! The text to be shown in the About dialog for the game (Help->GameName->About).
extern gchar *game_doc_about;
//! The text to be shown in the Rules dialog for the game (Help->GameName->Rules).
extern gchar *game_doc_rules;
//! The text to be shown in the Strategy dialog for the game (Help->GameName->Strategy).
extern gchar *game_doc_strategy;
//! The text to be shown in the History dialog for the game (Help->GameName->History).
extern gchar *game_doc_history;

//! Completion status of the game
extern CompletionStatus game_doc_about_status;

//! User visible labels for white and black
extern gchar *game_white_string, *game_black_string;


//! Background image for the board
extern char ** game_bg_pixmap;


//! The columns that will be shown in the highscores.
/** SCORE_FIELD_NONE acts as a NULL terminator for the #game_score_fields array.
 Note that highscores make sense only for single player games.*/
typedef enum {
	SCORE_FIELD_NONE, 
	SCORE_FIELD_RANK, SCORE_FIELD_USER, SCORE_FIELD_SCORE, SCORE_FIELD_TIME, SCORE_FIELD_DATE, 
	SCORE_FIELD_MISC1, SCORE_FIELD_MISC2
} SCORE_FIELD;

//! Used to override the default highscore columns.
/** By default, the columns shown in the highscores are User, Score, Time and Date. This sequence can be overridden by specifying a game_score_fields array terminated by #SCORE_FIELD_NONE. #SCORE_FIELD_MISC1 and #SCORE_FIELD_MISC2 are used to specify some field which is none of the 5 available by default. The MISC functionality is currently unimplemented */
extern SCORE_FIELD * game_score_fields;

//! The names of the column titles in the highscores.
/** If you set #game_score_fields you also have to set game_score_field_names which gives the titles to use for the respective columns. */
extern gchar **game_score_field_names;

//! Pointer to the comparison function used to order highscores.
/** Highscores work by applying the comparison function on the 
  score string returned by game_who_won(). Three defaults are 
  availble (see below), one of which will likely fit what you want.
  If not you can write your own function and set this pointer to it.*/
extern int (*game_scorecmp) (gchar *, int, gchar*, int);

//! Default highscore comparison function: decreasing order of Score field.
extern int (*game_scorecmp_def_dscore) (gchar *, int, gchar*, int);
//! Default highscore comparison function: increasing order of Score field.
extern int (*game_scorecmp_def_iscore) (gchar *, int, gchar*, int);
//! Default highscore comparison function: increasing order of Time field.
extern int (*game_scorecmp_def_time) (gchar *, int, gchar*, int);

#endif
