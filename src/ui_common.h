#ifndef _UI_COMMON_H_
#define _UI_COMMON_H_

/** \file ui_common.h
  \brief function declarations common to all ui modules
  */

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

//! The main application window
extern GtkWidget *main_window;

//! Show a message to the user
void sb_message (char *msg, gboolean serious);

//! Inform user of an error
void sb_error (char *msg, gboolean serious);

//! Converts a time in milliseconds into human-parsable format
gchar *sb_ftime(int temps);
	
#endif
