AM_CFLAGS = -Wall -Wno-unused -DDATADIR=\"$(datadir)\"

INCLUDES = @GTK_CFLAGS@ @GNOME_CFLAGS@

LDADD = @GTK_LIBS@ @GNOME_LIBS@

bin_PROGRAMS = gtkboard

#we'll install this when we're more stable
#gtkboardincludedir=$(includedir)/gtkboard-1.0
#gtkboardinclude_HEADERS =   \
#    game.h


gtkboard_SOURCES = 	\
	board.c\
	menu.c\
	prefs.c\
	sound.c\
	ui.c\
	\
	aaball.c\
	ab.c\
	engine.c\
	hash.c\
	move.c\
	stack.c\
	\
	antichess.c\
	ataxx.c\
	blet.c\
	breakthrough.c\
	checkers.c\
	chess.c\
	cpento.c\
	dirmaze.c\
	dotsandboxes.c\
	eightqueens.c\
	fifteen.c\
	flw.c\
	hiq.c\
	infiltrate.c\
	knights.c\
	kttour.c\
	mastermind.c\
	maze.c\
	memory.c\
	ninemm.c\
	othello.c\
	pacman.c\
	pentaline.c\
	plot4.c\
	quarto.c\
	rgb.c\
	samegame.c\
	stopgate.c\
	tetris.c\
	towers.c\
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
	sound.h\
	ui_common.h\
	ui.h\
	\
	flwords.h
