CFLAGS = -Wall -g -O3 -Wno-unused 

INCLUDES = @GTK_CFLAGS@

LDADD = @GTK_LIBS@

bin_PROGRAMS = gtkboard

gtkboard_SOURCES = 	othello.c\
	ui.c\
	board.c\
	prefs.c\
	menu.c\
	hash.c\
	move.c\
	stack.c\
	engine.c\
	minimax.c\
	ab.c\
	samegame.c\
	rgb.c\
	aaball.c\
	memory.c\
	tetris.c\
	chess.c\
	antichess.c\
	hiq.c\
	checkers.c\
	plot4.c\
	maze.c\
	dirmaze.c\
	infiltrate.c\
	ataxx.c\
	pentaline.c\
	mastermind.c\
	fifteen.c\
	game.h\
	pacman.c

#NOTE: we want to install game.h
noinst_HEADERS =  board.h\
	ui_common.h\
	move.h\
	stack.h\
	engine.h\
	aaball.h\
	menu.h\
	ui.h\
	prefs.h
