CFLAGS = -Wall -g -O3 -Wno-unused

INCLUDES = @GTK_CFLAGS@

LDADD = @GTK_LIBS@

bin_PROGRAMS = gtkboard

#we'll install this when we're more stable
#gtkboardincludedir=$(includedir)/gtkboard-1.0
#gtkboardinclude_HEADERS =   \
#    game.h


gtkboard_SOURCES = 	\
	board.c\
	menu.c\
	prefs.c\
	ui.c\
	\
	aaball.c\
	ab.c\
	engine.c\
	hash.c\
	minimax.c\
	move.c\
	stack.c\
	\
	antichess.c\
	ataxx.c\
	checkers.c\
	chess.c\
	dirmaze.c\
	fifteen.c\
	flw.c\
	hiq.c\
	infiltrate.c\
	mastermind.c\
	maze.c\
	memory.c\
	othello.c\
	pacman.c\
	pentaline.c\
	plot4.c\
	rgb.c\
	samegame.c\
	tetris.c\
	wordtris.c

noinst_HEADERS =  \
	aaball.h\
	board.h\
	engine.h\
	game.h\
	menu.h\
	move.h\
	prefs.h\
	stack.h\
	ui_common.h\
	ui.h\
	\
	flwords.h
