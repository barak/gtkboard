AC_PREREQ([2.67])
AC_INIT([gtkboard],[0.11pre0])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(gtkboard,0.11pre0)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_ARG_ENABLE(gtk2,
		[  --disable-gtk2      Don't look for GTK2 libraries],
		enable_gtk2=$enableval,
		enable_gtk2="yes")

found_gtk2=0;

if test "$enable_gtk2" = "yes" ; then
	dnl check if user has gtk2
	PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.0.0, found_gtk2=1, found_gtk2=0)
fi


AC_ARG_ENABLE(sdl,
        [  --disable-sdl      Don't look for the SDL library],
        enable_sdl=$enableval,
        enable_sdl="yes")

if test "$enable_sdl" = "yes" ; then
	dnl Check for SDL
#	SDL_VERSION=1.2.0
#	AM_PATH_SDL($SDL_VERSION,
#			AC_DEFINE([HAVE_SDL], [], [Do we have the SDL library]), 
#			AC_MSG_WARN([*** SDL version $SDL_VERSION not found. Sound will be disabled])
#	)
	AC_CHECK_LIB( SDL_mixer, Mix_OpenAudio, 
		have_SDL_mixer=yes, have_SDL_mixer=no)

	if test "$have_SDL_mixer" = "yes" ; then 
#		CFLAGS="$CFLAGS -DHAVE_SDL" 
		AC_DEFINE([HAVE_SDL], [], [Do we have the SDL library])
	    LIBS="$LIBS -lSDL_mixer -lSDL"
	else
	    echo "*** SDL_mixer not found.  Configuring without audio support."
	fi

	
#	CFLAGS="$CFLAGS $SDL_CFLAGS"
#	LIBS="$LIBS $SDL_LIBS -lSDL_mixer"
fi

AC_CHECK_LIB([gmodule-2.0], [g_module_symbol], [LIBS="$LIBS -lgmodule-2.0"], AC_MSG_WARN([Cannot find gmodule-2.0 library]))

AC_CHECK_LIB([m], [sincosf], [LIBS="$LIBS -lm"], AC_MSG_WARN([Cannot find math library]))

gnome=false

GNOME_CFLAGS=""
GNOME_LIBS=""

AC_ARG_ENABLE(gnome,[  --enable-gnome    Turn on gnome ],[
	if test "x$enableval" = "xyes"; then
		gnome=true
	fi
])

if test "$gnome" = true ; then
	PKG_CHECK_MODULES(GNOME, [
			libgnome-2.0 >= 2.0.0
			libgnomeui-2.0 >= 2.0.0
	])
	GNOME_CFLAGS="$GNOME_CFLAGS -DHAVE_GNOME=1"
fi

AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)
	
AM_CONDITIONAL(WITH_GNOME, test "x$gnome" = "xtrue")

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL

dnl Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd memset mkdir sqrt strcasecmp strncasecmp strpbrk strstr strtol])

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 pixmaps/Makefile
                 sounds/Makefile
                 src/Makefile])
AC_OUTPUT

echo " "

if test "$enable_gtk2" = "yes" ; then
	AC_MSG_RESULT([    GTK2 enabled.])
else
	AC_MSG_RESULT([    GTK2 disabled.])
fi

if test "$enable_sdl" = "yes" ; then
	AC_MSG_RESULT([     SDL enabled.])
else
	AC_MSG_RESULT([     SDL disabled.])
fi
if test "$enable_gnome" = "yes" ; then
	AC_MSG_RESULT([   GNOME enabled.])
else
	AC_MSG_RESULT([   GNOME disabled.])
fi
