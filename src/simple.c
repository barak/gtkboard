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
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "game.h"
#include "aaball.h"

#define SIMPLE_CELL_SIZE 55
#define SIMPLE_NUM_PIECES 3 //piece number 2 is the ENTER xpm, 3 is highlighted button

#define SIMPLE_BOARD_WID 7
#define SIMPLE_BOARD_HEIT 4

#define SIMPLE_BUTTON 1
#define ENTER_BUTTON 2
#define HIGHLIGHTED_BUTTON 3
#define SIMPLE_EMPTY 0

#define NO_ROW_ACTIVE -1
#define FIRST_ROW 0
#define SECOND_ROW 1
#define THIRD_ROW 2

#define SELECTED (HIGHLIGHTED_BUTTON << 8 | RENDER_REPLACE)
#define PRINT_POINTER_ADDRESS(pt) fprintf(stderr, "\n Pointer address: 0x%lx",(unsigned long) pt);

#define MACHINE_GOES_GOOD 1
#define HUMAN_GOES_GOOD 0
#define MACHINE_LOST 5
#define HUMAN_LOST 8
#define NO_STATE 2


static char simple_colors[6] = {140, 140, 180, 140, 140, 180};

static char * arrow_blue_return_55_xpm[]=
{
"55 55 2 1",
". c cyan",
"  c none",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                   .............       ",
"                                   .............       ",
"                                   .............       ",
"                                   .............       ",
"                                   .............       ",
"                                   .............       ",
"                    .              .............       ",
"                   ..              .............       ",
"                  ...              .............       ",
"                 ....              .............       ",
"                .....              .............       ",
"               ......              .............       ",
"              .......              .............       ",
"             ........              .............       ",
"            .........              .............       ",
"           ..........              .............       ",
"          ...........              .............       ",
"         .......................................       ",
"        ........................................       ",
"       .........................................       ",
"      ..........................................       ",
"     ...........................................       ",
"    ............................................       ",
"    ............................................       ",
"     ...........................................       ",
"      ..........................................       ",
"       .........................................       ",
"        ........................................       ",
"         .......................................       ",
"          ...........                                  ",
"           ..........                                  ",
"            .........                                  ",
"             ........                                  ",
"              .......                                  ",
"               ......                                  ",
"                .....                                  ",
"                 ....                                  ",
"                  ...                                  ",
"                   ..                                  ",
"                    .                                  ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
"                                                       ",
};

static int simple_init_pos [SIMPLE_BOARD_WID*SIMPLE_BOARD_HEIT] = 
{
	1 , 1 , 1 , 1 , 1 , 1 , 1 ,
	0 , 1 , 1 , 1 , 1 , 1 , 0 ,
	0 , 0 , 1 , 1 , 1 , 0 , 0 ,
	0 , 0 , 0 , 0 , 0 , 0 , 2
};

int simple_game_state;
void simple_init ();
ResultType simple_who_won (Pos *, Player , char **);
unsigned char * simple_get_rgbmap (int, int);
char ** simple_get_pixmap (int, int);
InputType simple_event_handler (Pos *pos, GtkboardEvent *event, MoveInfo *move_info_p);
//static ResultType simple_eval (Pos *, Player, float *);
void simple_search (Pos *pos, byte **move);
gboolean only_one_piece_on_board (Pos *);
//The next three functions compute the best move for the machine
int comprueba (int matriz[3][7], int level);
int compruebael (int matriz[3][7], int level);
int compruebamas (int matriz[3][7], int level);
int vector1[8]={9,9,9,9,9,9,9,9};//la funcion comprueba requiere esta VARIABLE GLOBAL
int htirar=43; //la funcion comprueba necesita esta VARIABLE GLOBAL
int nivel = 1; //level of difficulty
Game Simple = { SIMPLE_CELL_SIZE, SIMPLE_BOARD_WID, SIMPLE_BOARD_HEIT, 
	SIMPLE_NUM_PIECES,
	simple_colors, simple_init_pos, NULL, "Simple Game", "Nimlike games", simple_init};
Game Simple_easy = { SIMPLE_CELL_SIZE, SIMPLE_BOARD_WID, SIMPLE_BOARD_HEIT, 
	SIMPLE_NUM_PIECES,
	simple_colors, simple_init_pos, NULL, "Simple Game Easy", "Nimlike games", simple_init};
Game Simple_medium = { SIMPLE_CELL_SIZE, SIMPLE_BOARD_WID, SIMPLE_BOARD_HEIT, 
	SIMPLE_NUM_PIECES,
	simple_colors, simple_init_pos, NULL, "Simple Game Medium", "Nimlike games", simple_init};
Game Simple_master = { SIMPLE_CELL_SIZE, SIMPLE_BOARD_WID, SIMPLE_BOARD_HEIT, 
	SIMPLE_NUM_PIECES,
	simple_colors, simple_init_pos, NULL, "Simple Game Master", "Nimlike games", simple_init};
//SCORE_FIELD simple_score_fields[] = {SCORE_FIELD_USER, SCORE_FIELD_SCORE, SCORE_FIELD_TIME, SCORE_FIELD_DATE, SCORE_FIELD_NONE};
//char *simple_score_field_names[] = {"User", "Flips", "Time", "Date", NULL};

GameLevel simple_levels[] =
{
	{"Very Easy", &Simple},
	{"Easy", &Simple_easy},
	{"Medium", &Simple_medium},
	{"Master", &Simple_master},
	{ NULL, NULL }
};

void simple_init (Game *game)
{
// HERE WE SETUP ALL THE HOOKS
//	game_getmove = blet_getmove; //deprecated by game_event_handler
	game_levels = simple_levels;
	if (game == &Simple)
		nivel = 1;
	else if (game == &Simple_easy)
		nivel = 2;
	else if (game == &Simple_medium)
		nivel = 3;
	else if (game == &Simple_master)
		nivel = 4;
	//game_eval = simple_eval; //OUR GAME IS SO SIMPLE SO
	//game_movegen = simple_movegen;// WE DONT NEED MINIMAX
	game_search = simple_search;//A function to search and return the best move - for games for which minimax is not appropriate. 
	game_get_pixmap = simple_get_pixmap;
	simple_game_state = NO_STATE;
	//game_get_rgbmap = simple_get_rgbmap;
	/*
	  game_single_player = TRUE;
	  game_score_fields = simple_score_fields;
	  game_score_field_names = simple_score_field_names;
	  game_scorecmp = game_scorecmp_def_iscore;
	*/
	game_who_won = simple_who_won;
	game_white_string = "Human";
	game_black_string = "Machine";
	
	game_event_handler = simple_event_handler;

	game_doc_about_status = STATUS_COMPLETE;
	game_doc_about = 
		"Simple Game\n"
		"URL: "GAME_DEFAULT_URL("Simple Game");
	game_doc_rules = 
		"Simple game has only two rules:\n"
		"- The player who has to remove the last piece looses.\n"
		"- In a turn, you can only remove pieces from one row,\n"
		"but you can remove as many pieces as you want, for "
		" example the entire row.\n\n"
		" Select the pieces you want to remove and click on the\n"
		"blue arrow square to remove them.\n\n"
		"Game algorithm: Daniel Rios\n"
		"Game implementation: Nelson Benitez <gnelsonATcenobioracingDOTcom>";
	game_doc_strategy = 
		"In the very easy level of difficulty, avoid having only two rows with the same number of pieces, "
		"that scenario will end you in a lost game. Higher levels has other scenarios you have to avoid"
		" in order to win.  In the master level of difficulty, you will only have a chance to win if you have"
		" the first turn to play.";
	game_doc_history = 
		"Game algorithm: Daniel Rios\n"
		"Game implementation: Nelson Benitez <gnelsonATcenobioracingDOTcom>\n\n"
		"This game appeared in a TV programme in Spain where people phoned to play the game and win money,\n"
		"my friend Daniel found out all the ways to win and I coded it for fun.";
}

ResultType simple_who_won (Pos *pos, Player player, char **commp)
{
	//This function is called after every move, both for single player and two player games. 
	//The Player argument in single_player_mode is NULL for the machine and 1 for the human player
	static char comment[32];
	const char* nivel_char[4] = {"VERY EASY","EASY","MEDIUM","MASTER"};
	//My function that computes the machine's move also sets the simple_game_state variable that 
	//duplicate the functionality of game_who_won function, so I only attend that variable
	switch (simple_game_state) {
	case MACHINE_GOES_GOOD:
		//snprintf (comment, 32, "You are in the %s level.",nivel_char[nivel-1]);
		//*commp = comment;
		return RESULT_NOTYET;
	case HUMAN_GOES_GOOD:
		snprintf (comment, 32, "Hey, good movement...");
		*commp = comment;
		return RESULT_NOTYET;
	case MACHINE_LOST:
		snprintf (comment, 32, "You won! ,the machine lost.");
		*commp = comment;
		return RESULT_LOST;
        case HUMAN_LOST:
		snprintf (comment, 32, "You lost! ,the machine won.");
		*commp = comment;
		return RESULT_WON;
	case NO_STATE: //we are in HUMAN TO HUMAN game type
		if (only_one_piece_on_board(pos)) {
			snprintf (comment, 32, "%s won! , %s lost.",(player == WHITE) ? "Machine" : "Human",(player == WHITE) ? "Human" : "Machine");
			*commp = comment;
			return RESULT_WON;
		}
	}
	return RESULT_NOTYET;
}

gboolean only_one_piece_on_board(Pos *pos) {
int y,x,count=0;
	for (y=1; y<SIMPLE_BOARD_HEIT; y++){ //nos saltamos la última fila
		for (x=0; x<SIMPLE_BOARD_WID; x++){
			if (pos->board[y * SIMPLE_BOARD_WID + x] == SIMPLE_BUTTON)
				count++;
		}
	}
	if (count <= 1 )//for Human to Human, the one who removes the entire last row (scenario people playing dumb) wins too.
		return TRUE;
	else
		return FALSE;
}
      
InputType simple_event_handler(Pos *pos, GtkboardEvent *event, MoveInfo *move_info_p) {
	struct ITEM {int x; int y; int value;}; //a struct to store the selected items because
	static struct ITEM selected_items[SIMPLE_BOARD_WID];//looping the pos->render is broken
	static char comment[32];
	static byte movement[4*SIMPLE_BOARD_WID+1]; //largest case is the entire upper row
	static int items_selected=0;
	static int active_row;
	static int rmove[4*SIMPLE_BOARD_WID+1];
	byte *pt;
	int *ptr;
	int i;
	static int total_moves=-1,x,y,event_item;
	static gboolean new_turn;
	x = event->x;
	y = event->y;

	if (event->type != GTKBOARD_BUTTON_RELEASE)
		return INPUT_NOTYET;

	event_item = pos->board [y * SIMPLE_BOARD_WID + x];
	new_turn = (total_moves < pos->num_moves); //handles when we are in a new turn
	if (items_selected == 0 ) 
		new_turn = TRUE;
	if ( event_item == SIMPLE_EMPTY) { 
		move_info_p->help_message = "You must click on a valid item."; 
		return INPUT_NOTYET;
	}
	if (new_turn) {
		assert(pos->render[y * SIMPLE_BOARD_WID + x] != SELECTED);//the render info stores in pos->render array
		active_row = NO_ROW_ACTIVE;
		total_moves = pos->num_moves;
		if (event_item == ENTER_BUTTON) { 
			move_info_p->help_message = "You must select at least one item."; 
			return INPUT_ILLEGAL;
		}
		rmove[0] = x;
		rmove[1] = y;
		rmove[2] = SELECTED; // See http://gtkboard.sourceforge.net/doc/doxygen/game_h.html#a103a89
		rmove[3] = -1;
		move_info_p->rmove = rmove;
		active_row = y;
		items_selected++;
		selected_items[x].x = x; //we index by the col number (x)
		selected_items[x].y = y;
		selected_items[x].value = SIMPLE_BUTTON; //SIMPLE_BUTTON -> SELECTED, SIMPLE_EMPTY-> NO SELECTED
		return INPUT_NOTYET;
	} else {
		if (pos->render[y * SIMPLE_BOARD_WID + x] == SELECTED) event_item = HIGHLIGHTED_BUTTON;
	
		switch (event_item) {
		case SIMPLE_BUTTON:
			if (active_row != y) {
				move_info_p->help_message = "You must select items only from one row."; 
				return INPUT_ILLEGAL;
			}
			rmove[0] = x;
			rmove[1] = y;
			rmove[2] = SELECTED; // See http://gtkboard.sourceforge.net/doc/doxygen/game_h.html#a103a89
			rmove[3] = -1;
			move_info_p->rmove = rmove;
			items_selected++;
			selected_items[x].x = x; //we index by the col number (x)
			selected_items[x].y = y;
			selected_items[x].value = SIMPLE_BUTTON; //SIMPLE_BUTTON -> SELECTED, SIMPLE_EMPTY-> NO SELECTED
			return INPUT_NOTYET;
		case HIGHLIGHTED_BUTTON:
			rmove[0] = x;
			rmove[1] = y;
			rmove[2] = RENDER_NONE; // See http://gtkboard.sourceforge.net/doc/doxygen/game_h.html#a103a89
			rmove[3] = -1;
			move_info_p->rmove = rmove;
			items_selected--;
			selected_items[x].value = SIMPLE_EMPTY; //SIMPLE_BUTTON -> SELECTED, SIMPLE_EMPTY-> NO SELECTED
			return INPUT_NOTYET;
		case ENTER_BUTTON:
			pt = movement;
			ptr = rmove;
			for (i=0;i<SIMPLE_BOARD_WID;i++) {
				if (selected_items[i].value == SIMPLE_BUTTON) {
					*pt++ = selected_items[i].x; *pt++ = selected_items[i].y; *pt++ = SIMPLE_EMPTY; 
					*ptr++ = selected_items[i].x; *ptr++ = selected_items[i].y; *ptr++ = RENDER_NONE;
					selected_items[i].value = SIMPLE_EMPTY; //reset at the same time
				}
			}
			*pt++ = -1;// The end of a movelet is a -1
			*ptr++ = -1;
			move_info_p->move = movement;
			move_info_p->rmove = rmove;
			items_selected = 0;//reset cause we going to a new turn
			return INPUT_LEGAL;
		}
	}
	
	return INPUT_NOTYET;
}

char ** simple_get_pixmap (int piece, int color)
{
	int bg, i, fg, rad, grad;
	char *colors;
	static char pixbuf[SIMPLE_CELL_SIZE * (SIMPLE_CELL_SIZE+1)];
	if (piece == ENTER_BUTTON) return arrow_blue_return_55_xpm;
	colors = simple_colors;
	if (color == BLACK) colors += 3;
	for(i=0, bg=0;i<3;i++) 
	{ int col = colors[i]; if (col<0) col += 256; bg += col * (1 << (16-8*i));}
	if (piece == HIGHLIGHTED_BUTTON)
		fg = 0xff << 8;//highlighted piece,never comes here cause highlighted stores in render info
	else
		fg = 0xcc << 8;//normal piece is green
	return pixmap_ball_gen(55, pixbuf, fg, bg, 17.0, 35.0);
}

void board2matriz(Pos *pos,int matriz[3][7]) { 	
 int y,x;
//Reads the board and translate it to a matrix which my old functions work with
//The pos->board[0] is the botton left piece in the board
//convert the board to my matriz:
// X X X X X X X
// - X X X X X -
// - - X X X - -
 for (y=1; y<SIMPLE_BOARD_HEIT; y++){ //skip the bottom row
	for (x=0; x<SIMPLE_BOARD_WID; x++){
		if (pos->board[y * SIMPLE_BOARD_WID + x] == SIMPLE_BUTTON) {
			matriz[(SIMPLE_BOARD_HEIT-1)-y][x] = 1;
		} else {
			matriz[(SIMPLE_BOARD_HEIT-1)-y][x] = 0;
		}
	}
}
//now convert matriz to format:
// X X X X X X X
// X X X X X - -
// X X X - - - -
 for (y=1;y<3;y++) { //skip first row cause its already good
	 for (x=0;x<SIMPLE_BOARD_WID;x++){
		 if (y==1 && x>0)
			 matriz[y][x-1]=matriz[y][x];

		 if (y==2 && x>1)
			 matriz[y][x-2]=matriz[y][x];
	 }
 }
 matriz[1][5]=matriz[1][6]=matriz[2][3]=matriz[2][4]=matriz[2][5]=matriz[2][6]=0;
}

void make_machine_move(byte movement[3*SIMPLE_BOARD_WID+1]) {
	//Move format See: http://gtkboard.sourceforge.net/doc/doxygen/move_h.html#_details
	//See too: http://gtkboard.sourceforge.net/doc/doxygen/struct_MoveInfo.html#m0
	
	byte *pt = movement;
	//PRINT_POINTER_ADDRESS(movement);
	//PRINT_POINTER_ADDRESS(pt);
	int i,y,x=0;
	y = (SIMPLE_BOARD_HEIT-1) - htirar;
	assert(y > 0);
	assert(y < 4);
	for (i=1;i<8;i++) { //la itirar empiezan a partir de vector1[1] incluido.
		if (vector1[i]!=9) {
			if (y==1)
				x=vector1[i]+2;
			if (y==2)
				x=vector1[i]+1;
			if (y==3)
				x=vector1[i];

			 *pt++ = x; *pt++ = y; *pt++ = SIMPLE_EMPTY;
		}  else { 
			 break;
		}
	 }
	*pt++ = -1;
	g_usleep(700000); //sleep 0.7 seconds so user can perceive machine move
}

void simple_search (Pos *pos, byte **move) {
// Coordinates of the board in http://gtkboard.sourceforge.net/doc/doxygen/struct_Pos.html#m0
// Format of the board and how to access squares in the board, see http://gtkboard.sourceforge.net/doc/doxygen/struct_Pos.html#m0
	static int matriz[3][7];
	static byte movement[4*SIMPLE_BOARD_WID+1];
	//PRINT_POINTER_ADDRESS(movement);
	int ret=1;
	int y,x;
	board2matriz(pos,matriz);
	ret=comprueba(matriz,nivel);
	switch (ret){
	case 0:
		simple_game_state = HUMAN_GOES_GOOD;
		make_machine_move(movement);
		*move = movement;
		break;
	case 1:
		simple_game_state = MACHINE_GOES_GOOD;
		make_machine_move(movement);
		*move = movement;
		break;
	case 5:
		simple_game_state = MACHINE_LOST;
		make_machine_move(movement);
		*move = movement;
		break;
	case 8:
		simple_game_state = HUMAN_LOST;
		make_machine_move(movement);
		*move = movement;
		break;
		}
}


// BELOW THIS LINE ARE THE FUNCTIONS THAT LOOKS FOR THE BEST MOVE FOR THE MACHINE
// THEY ARE VERY POOR AND BEGINNER C CODE,YOU BETTER DONT WANT TO LOOK AT THEM
// THEY WERE WRITTEN WHEN I WAS LEARNING C AND I PRACTICED WITH THIS GAME.
// YOU ARE ADVISED ;-)

int comprueba (int matriz[3][7], int level)
{
/*RETORNA 1 si en alguna jugada(de todas las nuestras posibles) el
OTRO le es imposible dejarnos a nosotros en MISMO O 1|1|1 o 1|2|3 etc (dependiendo del nivel de chekeo), y
la jugada en cuestion se haya en las VARIABLES GLOBALES htirar y a partir de vector1[1](incluido)

RETORNA 8 si ganamos la partida porque la ultima casilla quedo en pie para que la tire EL OTRO

RETORNA  5 si perdimos la partida porque nos toco tirar la ultima casilla, y la ultima casilla a tirar
           esta en htirar y vector[1]

RETORNA 0 si para todas nuestras jugadas posibles el OTRO en cualquiera de ellas
puede hacernos MISMO O 1|1|1 o etc (depende del level) y la jugada esta en htirar y vector1[1], 
tambien retorna 0 en el caso que nos quede la ultima casilla por tirar (HEMOS PERDIDO)*/

int w,e,acum,p,q,con=1;
int copia[3][7];
int up,down;//solo pa restablecer la copia de seguridad porke toy kemao
/*****************************************
 INICIALIZACION DE HTIRAR Y VECTOR1
****************************************/
htirar=43;
 for (q=0;q<8;q++)
 {
 vector1[q]=9;
 }
/*****************************************
 FIN INICIALIZACION DE HTIRAR Y VECTOR1
****************************************/
/***************************************
 COMENZAMOS COPIA DE SEGURIDAD DE MATRIZ
****************************************/
for (w=0;w<3;w++)
{
  for (e=0;e<7;e++)
   {
    copia[w][e]=matriz[w][e];
    }
}
/******************************************
 TERMINAMOS LA COPIA DE SEGURIDAD DE MATRIZ
*******************************************/
for (w=0;w<3;w++)
{
 for (e=0;e<7;e++)
 {
  if ((w==1 && e==5) || (w==2 && e==3))
  {
   break;
  }
  if (matriz[w][e]!=0)
  {
   matriz[w][e]=0;
   htirar=w;
   vector1[0]=43;
   vector1[0+con]=e;
   con=con+1;

  if ((p=compruebael(matriz, level))) 
  {
/***************************************
 COMENZAMOS A RESTABLECER COPIA DE SEGURIDAD DE MATRIZ
****************************************/
for (up=0;up<3;up++)
{
  for (down=0;down<7;down++)
   {
    matriz[up][down]=copia[up][down];
    }
}
/******************************************
 TERMINAMOS DE RESTABLECER LA COPIA DE SEGURIDAD DE MATRIZ
*******************************************/

  if (p==8)
  {
    return 8;//esto es que tras nuestra tirada,, al OTRO solo le quedara tirar la ultima casilla 
   }
   return 1; // y la posicion estan en htirar y vector1 GLOBALES
  } //fin del IF p=compruebael
  } //fin del IF !=0
  }//fin del FOR de la e
 if (vector1[0]==43)
 {
 for (up=1;up<8;up++)
 {
  if (vector1[up]!=9)
    {
     matriz[w][vector1[up]]=1;
    }
 }
 con=1;
 htirar=43;
 for (q=0;q<8;q++)
 {
 vector1[q]=9;
 }
 }//fin del IF 43
} //fin del FOR de la w

/***************************************
 COMENZAMOS A RESTABLECER COPIA DE SEGURIDAD DE MATRIZ
****************************************/
for (up=0;up<3;up++)
{
  for (down=0;down<7;down++)
   {
    matriz[up][down]=copia[up][down];
    }
}
/******************************************
 TERMINAMOS DE RESTABLECER LA COPIA DE SEGURIDAD DE MATRIZ
*******************************************/
// A CONTINUACION BUSCAMOS LA PRIMERA CASILLA QUE ENCONTREMOS Y LA ANOTAMOS PARA TIRARLA
// YA QUE NOS DA IGUAL PORQUE NO EXISTE NINGUNA JUGADA SATISFACTORIA Y SALDREMOS CON RETORNO 0
// DE PASO UTILIZAMOS UN ACUMULADOR PARA SABER SI ES LA ULTIMA CASILLA
w=20;
acum=0;
for (up=0;up<3;up++)
 {
  for (down=0;down<7;down++)
  {
   if ((up==1 && down==5) || (up==2 && down==3))
   {
    break; //saltamos para ajustar la matriz al tama¤o de nuestras filas
   }
   if (matriz[up][down]==1)
   {
     if (w==20)
     {
      w=up;   //almacenamos los indices de
      e=down; //la primera casilla que encontramos
     }
    acum+=1;
   }
  } //fin del FOR f
 } //fin del FOR r
htirar=w;     // escribimos en las globales
vector1[0]=43;// la
vector1[1]=e; // posicion de la casilla
//Y AHORA COMPROBAMOS SI ES LA ULTIMA FICHA, PARA SALIR CON RETORNO 5 Y ASI SABER QUE HEMOS PERDIDO
if (acum==1)
{
 return 5; //devolvemos 5 para decir que hemos perdido y la ultima casilla por tirar esta en las globales
}
//Y AHORA LA SALIDA NORMAL EN CASO DE QUE NOS ESTEN GANANDO
return 0; //para todas nuestras jugadas posibles el OTRO consigue hacernos MISMO
} //fin de la DEFINICION de la funcion comprueba


int compruebael (int matriz[3][7], int level)
{
/*RETORNA 1 si tras probar todas las posibilidades del OTRO en ninguna
de ellas consigue hacernos MISMO

RETORNA 0 si al probar todas las posibilidades del OTRO en alguna puede
hacernos MISMO*/
int v,b,a=43,q,con=1;
int heltirar=43; //en este caso son variables LOCALES
int vector1el[8]={9,9,9,9,9,9,9,9}; //en este caso son variables LOCALES
int seguridad[3][7];
int big,small; //solo para restablecer copia de seguridad porke toy kemao de variables
/***************************************
 COMENZAMOS COPIA DE SEGURIDAD DE MATRIZ
****************************************/
for (v=0;v<3;v++)
{
  for (b=0;b<7;b++)
   {
    seguridad[v][b]=matriz[v][b];
    }
}
/******************************************
 TERMINAMOS LA COPIA DE SEGURIDAD DE MATRIZ
*******************************************/
for (v=0;v<3;v++)
{
 for (b=0;b<7;b++)
 {
  if ((v==1 && b==5) || (v==2 && b==3))//los saltos de acuerdo al size de our filas
  {
   break;
  }
  if (matriz[v][b]!=0)
  {
   matriz[v][b]=0;
   heltirar=v;
   vector1el[0]=43;
   vector1el[0+con]=b;
   con=con+1;

  if ((a=compruebamas(matriz, level))==1) //no hace falta poner compruebamas(matriz [3][7])
  {
/***************************************
 COMENZAMOS A RESTABLECER COPIA DE SEGURIDAD DE MATRIZ
****************************************/
for (big=0;big<3;big++)
{
  for (small=0;small<7;small++)
   {
    matriz[big][small]=seguridad[big][small];
    }
}
/******************************************
 TERMINAMOS DE RESTABLECER LA COPIA DE SEGURIDAD DE MATRIZ
*******************************************/
   return 0;
  }
  } //fin del IF !=0
  }//fin del FOR de la e
 if (vector1el[0]==43)
 {
 for (big=0;big<8;big++)
 {
  if (vector1el[big]!=9)
   {
    matriz[v][vector1el[big]]=1;
   }
 }
 con=1;
 heltirar=43;
 for (q=0;q<8;q++)
 {
 vector1el[q]=9;
 }
 }//fin del IF 43
} //fin del FOR de la w

/***************************************
 COMENZAMOS A RESTABLECER COPIA DE SEGURIDAD DE MATRIZ
****************************************/
for (big=0;big<3;big++)
{
  for (small=0;small<7;small++)
   {
    matriz[big][small]=seguridad[big][small];
    }
}
/******************************************
 TERMINAMOS DE RESTABLECER LA COPIA DE SEGURIDAD DE MATRIZ
*******************************************/
if (a==8){
return 8;
}
else {        //esto es para si compruebael recibe una matriz vacia (caso de la ultima casilla por tirar)
  if (a==43) { return 0; } //pues no devolvemos 1, sino 0, que indica que es malo para nosotros, y es lo
     }         //lo que se espera que devuelva para el caso especial de la ultima casilla
return 1;
} //fin de la DEFINICION de la funcion compruebael



int compruebamas(int matriz[3][7], int level)
{
/*RETORNA 1 si de la matriz recibida existen DOS filas con el mismo numero de
 bloques y la TERCERA no tiene ningun bloque, a esta situacion se le denomina
 MISMO o hacer un MISMO. Tambien retorna 1 si existe un 1|1|1 o 1|2|3 o 1|0|0
 o 3|4|5 (predecesor del 1|2|3) o 5|4|1
 NOTA:el que le hace un MISMO a otra persona, esa otra persona ya pierde la partida.

 RETORNA 0 si NO existen SOLO dos filas con el mismo numero de bloques y que
 la tercera no tenga ningun bloque, esto es, si no existe un MISMO, SOLO EN
 EL MISMO 1|1 DEVUELVE TAMBIEN 0 (YA QUE ES BUENO PARA NOSOTROS), tambien
 devuelve 0 si a parte de no haber MISMO no hay tampoco un 1|1|1 ni 1|2|3
 ni 1|0|0 ni 3|4|5*/
int r,f,contador1[3]={0,0,0};
 for (r=0;r<3;r++)
 {
  for (f=0;f<7;f++)
  {
   if ((r==1 && f==5) || (r==2 && f==3))
   {
    break; //saltamos para ajustar la matriz al tama¤o de nuestras filas
   }
   if (matriz[r][f]==1)
   {
    contador1[r]=contador1[r]+1;
   }
  } //fin del FOR f
 } //fin del FOR r
 if (contador1[0]==contador1[1] && contador1[2]==0)
 {
  if (contador1[0]==0 && contador1[1]==0)//ESTO ES QUE NOS DEJA UN 0|0|0 ,QUE SIGNIFICA QUE HEMOS
      {                                  //GANADO PORQUE EL OTRO TIRO LA ULTIMA, AQUI SE DA EL 
       return 8;                         //VALOR DE RETORNO QUE INDICA QUE HEMOS GANADO LA PARTIDA
      }
  if (contador1[0]==1 && contador1[1]==1)
  {          
   return 0; // es un MISMO del tipo 1|1
  }
  else
  {
  return 1; //existen DOS filas con el mismo numero de bloques y la TERCERA no tiene ningun bloque
  }
 }
 else
 {
  if (contador1[2]==contador1[1] && contador1[0]==0)
  {
   if (contador1[2]==1 && contador1[1]==1)
  {
   return 0; //es un MISMO del tipo 1|1
  }
  else
  {
  return 1; //existen DOS filas con el mismo numero de bloques y la TERCERA no tiene ningun bloque
  }
  }
  else
  {
   if (contador1[2]==contador1[0] && contador1[1]==0)
   {
    if (contador1[2]==1 && contador1[0]==1)
  {
   return 0; //es un MISMO del tipo 1|1
  }
  else
  {
  return 1; //existen DOS filas con el mismo numero de bloques y la TERCERA no tiene ningun bloque
  }
   }
  }
 }
 //AHORA COMPROBAMOS SI EXISTE UN 1|2|3 (6 combinaciones)
 //Nota: No ponemos IF anidados porque en cualquiera se sale con return
if (level==2 || level==3 || level==4)//el chekeo 1|2|3 y 1|1|1 se hara si tiene uno de estos niveles
{
 if (contador1[0]==1 && contador1[1]==2 && contador1[2]==3)
 {
  return 1; //existe un 1|2|3
  }
 if (contador1[0]==3 && contador1[1]==2 && contador1[2]==1)
 {
  return 1; //existe un 1|2|3
  }
 if (contador1[0]==2 && contador1[1]==1 && contador1[2]==3)
 {
  return 1; //existe un 1|2|3
  }
 if (contador1[0]==2 && contador1[1]==3 && contador1[2]==1)
 {
  return 1; //existe un 1|2|3
  }
 if (contador1[0]==3 && contador1[1]==1 && contador1[2]==2)
 {
  return 1; //existe un 1|2|3
  }
 if (contador1[0]==1 && contador1[1]==3 && contador1[2]==2)
 {
  return 1; //existe un 1|2|3
  }
 //FIN COMPROBACION DE 1|2|3
 //AHORA COMPROBAMOS SI EXISTE UN 1|1|1
 if (contador1[0]==1 && contador1[1]==1 && contador1[2]==1)
 {
  return 1; //existe un 1|1|1, que nos hace el mismo mal que un MISMO
 }
} //FIN DEL IF LEVEL==1 || LEVEL ==2 ...
 //FIN COMPROBACION DE 1|1|1
 //AHORA COMPROBAMOS SI EXISTE 3|4|5 (PREDECESOR DE 1|2|3) (solo 2 combinaciones
 //porque el 3 solo puede estar en una fila)
if (level==3 || level==4)
{
 if (contador1[0]==5 && contador1[1]==4 && contador1[2]==3)
 {
  return 1;//existe un 3|4|5 (predecesor del 1|2|3)
 }
 if (contador1[0]==4 && contador1[1]==5 && contador1[2]==3)
 {
  return 1;//existe un 3|4|5 (predecesor del 1|2|3)
 }
} //fin del if level==3 || level==4
 //FIN COMPROBACION DE 3|4|5 (PREDECESOR DE 1|2|3)
 //COMPROBAMOS EL 5|4|1
if (level==4)
{
 if (contador1[0]==5 && contador1[1]==4 && contador1[2]==1)
 {
  return 1;//existe un 5|4|1
 }
 if (contador1[0]==4 && contador1[1]==5 && contador1[2]==1)
 {
  return 1;//existe un 5|4|1
 }
}//fin del if level==4
 //TERMINAMOS COMPROBAR 5|4|1
 
 //AHORA COMPROBAMOS SI EXISTE 1|0|0 (son 3 combinaciones)
 if (contador1[0]==1 && contador1[1]==0 && contador1[2]==0)
 {
  return 1; //existe un 1|0|0 que nos perjudica igual k MISMO y 1|1|1 y 1|2|3
 }
 if (contador1[0]==0 && contador1[1]==1 && contador1[2]==0)
 {
  return 1; //existe un 1|0|0 que nos perjudica igual k MISMO y 1|1|1 y 1|2|3
 }
 if (contador1[0]==0 && contador1[1]==0 && contador1[2]==1)
 {
  return 1; //existe un 1|0|0 que nos perjudica igual k MISMO y 1|1|1 y 1|2|3
 }
 //FIN COMPROBACION DE 1|0|0
 
 return 0; //NO existen SOLO dos filas con el mismo numero de bloques
 //ni tampoco un 1|1|1 ni 1|2|3 ni 1|0|0 ni 3|4|5 ni 5|4|1
} //fin de DEFINICION de compruebamas
