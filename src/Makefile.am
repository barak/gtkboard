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
noinst_HEADERS =  othello.h\
	board.h\
	ui_common.h\
	move.h\
	stack.h\
	engine.h\
	aaball.h\
	menu.h\
	memory.h\
	ui.h\
	prefs.h\
	samegame.h\
	fifteen.h\
	tetris.h\
	chess.h\
	antichess.h\
	hiq.h\
	checkers.h\
	plot4.h\
	maze.h\
	dirmaze.h\
	infiltrate.h\
	ataxx.h\
	pentaline.h\
	rgb.h\
	mastermind.h\
	pacman.h
