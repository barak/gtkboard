#ifndef _SOUND_H_
#define _SOUND_H_

#include "config.h"

typedef enum 
{
	SOUND_PROGRAM_START,
	SOUND_ILLEGAL_MOVE,
	SOUND_WON,
	SOUND_LOST,
	SOUND_USER_MOVE,
	SOUND_MACHINE_MOVE,
	SOUND_GAME_START,
	SOUND_GAME_OVER,
	SOUND_HIGHSCORE,
}SoundEvent;

void sound_init();
void sound_stop ();
void sound_play (SoundEvent event);
void sound_set_enabled (gboolean enabled);
gboolean sound_get_enabled ();
void sound_set_enabled (gboolean enabled);
void sound_enable_pref_cb (gchar *key, gchar *value);

#endif
