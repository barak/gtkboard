#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "sound.h"
#include "prefs.h"

#ifdef HAVE_SDL

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

extern gboolean opt_verbose;

Mix_Music *music = NULL;

#endif

char *sound_dir = NULL;

gboolean sound_initialized = FALSE;
gboolean sound_enabled = TRUE;

void sound_set_enabled (gboolean enabled)
{
	sound_enabled = enabled;
	prefs_set_config_val ("enable_sound", enabled ? "true" : "false");
}

gboolean sound_get_enabled ()
{
	return sound_enabled;
}

// callback when reading the value from the config file
void sound_enable_pref_cb (gchar *key, gchar *value)
{
	sound_enabled = prefs_get_bool_val (value);
}

// default list of directories to search for sound. This is obviously stupid, but I don't have a better idea.
static char * default_sound_dirs[] = 
{
	"/usr/share/sounds/gtkboard",
	"/usr/local/share/sounds/gtkboard",
	"/opt/share/sounds/gtkboard",
	"sounds",
	"../sounds",
	NULL
};

static void find_sound_dir ()
{
	int i;
	sound_dir = prefs_get_config_val ("sound_dir");
	if (!sound_dir)
		sound_dir = g_strdup_printf ("%s/sounds/gtkboard", DATADIR);
#if GLIB_MAJOR_VERSION > 1
	if (!g_file_test (sound_dir, G_FILE_TEST_IS_DIR))
		g_free (sound_dir);
	for (i=0; default_sound_dirs[i]; i++)
	{ 
		sound_dir = default_sound_dirs[i];
		if (g_file_test (sound_dir, G_FILE_TEST_IS_DIR))
		{
			if (opt_verbose) fprintf (stderr, "Using sound directory %s\n", sound_dir);
			return;
		}
	}
	fprintf (stderr, "Sound directory not found, sounds will be disabled\n");
	sound_dir = NULL;
	
#endif
}

void sound_init()
{
#ifdef HAVE_SDL
	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16;
	int audio_channels = 2;
	int audio_buffers = 512;

	static gboolean first = TRUE;
	if (!first) return;
	first = FALSE;

	find_sound_dir ();
	SDL_Init(SDL_INIT_AUDIO);

	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
	{
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		return;
	}
	sound_initialized = TRUE;
#endif
}

void sound_stop ()
{
#ifdef HAVE_SDL
	if (music)
	{
		Mix_HaltMusic ();
		Mix_FreeMusic (music);
		music = NULL;
	}
#endif
}

void sound_play_real (char *file)
{
#ifdef HAVE_SDL
	SDL_AudioSpec wave;
	SDL_AudioCVT cvt;
	Uint8 *tmp;

	sound_stop ();
	
	music = Mix_LoadMUS (file);
	if (!music)
	{
		fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
		return;
	}
	Mix_PlayMusic(music, 0);

#endif
}

void sound_play (SoundEvent event)
{
	gchar *sound_file;
	gchar *file;
	if (!sound_enabled) return;
	switch (event)
	{
		case SOUND_PROGRAM_START:
			file = NULL;
			break;
		case SOUND_ILLEGAL_MOVE:
			file = "illegal_move.ogg";
			break;
		case SOUND_HIGHSCORE:
			file = "highscore.ogg";
			break;
		case SOUND_WON:
			file = "won.ogg";
			break;
		case SOUND_LOST:
			file = "lost.ogg";
			break;
		case SOUND_USER_MOVE:
			file = "user_move.ogg";
			break;
		case SOUND_MACHINE_MOVE:
			file = "machine_move.ogg";
			break;
		default:
			file = NULL;
	}
	if (!file) return;
	sound_init ();
	if (!sound_initialized) return;
	if (!sound_dir) return;
	sound_file = g_strdup_printf ("%s/%s", sound_dir, file);
	sound_play_real (sound_file);
	g_free (sound_file);
}
