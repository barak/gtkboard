#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "sound.h"

#ifdef HAVE_SDL

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

Mix_Music *music = NULL;

#endif

char *sound_dir = NULL;

static void find_sound_dir ()
{
	// TODO: eventually we will allow the user to set the sound directory in the prefs file
	sound_dir = g_strdup_printf ("%s/sounds/gtkboard", DATADIR);
	if (!g_file_test (sound_dir, G_FILE_TEST_IS_DIR))
	{
		fprintf (stderr, "Sound directory %s not found\n", sound_dir);
		g_free (sound_dir);
		sound_dir = NULL;
	}
}

void sound_init()
{
#ifdef HAVE_SDL
	int audio_rate = 22050;
	Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
	int audio_channels = 2;
	int audio_buffers = 4096;

	static gboolean first = TRUE;
	if (!first) return;
	first = FALSE;

	find_sound_dir ();
	SDL_Init(SDL_INIT_AUDIO);

	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
	{
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		exit(1);
	}

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
	if (!sound_dir) return;
	sound_file = g_strdup_printf ("%s/%s", sound_dir, file);
	sound_play_real (sound_file);
	g_free (sound_file);
}
