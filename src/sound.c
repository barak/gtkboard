#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "sound.h"

#ifdef HAVE_SDL

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>

Uint8 *sound_data;
int sound_len, sound_cur;

gboolean sound_active = FALSE;

void sound_mix_cb (void *unused, Uint8 *stream, int len)
{
	int amount;

	if (!sound_active) return;

	amount = (sound_len - sound_cur);
	if (amount > len) amount = len;

	SDL_MixAudio(stream, &sound_data[sound_cur], amount, SDL_MIX_MAXVOLUME);
	sound_cur += amount;
}

#endif

void sound_init()
{
#ifdef HAVE_SDL
	SDL_AudioSpec fmt;
	static gboolean first = TRUE;
	if (!first) return;
	first = FALSE;

	/* Set 16-bit stereo audio at 22Khz */
	fmt.freq = 22050;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = 512;
	fmt.callback = sound_mix_cb;
	fmt.userdata = NULL;

	/* Open the audio device and start playing sound! */
	if (SDL_OpenAudio(&fmt, NULL) < 0 ) 
	{
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		return;
	}

	sound_active = TRUE;
	sound_data = NULL;
	sound_len = sound_cur = 0;
#endif
}

void sound_exit ()
{
#ifdef HAVE_SDL
	if (!sound_active) return;
	SDL_CloseAudio();
	if (sound_data) free (sound_data);
	sound_active = FALSE;
#endif
}

void sound_play_real (char *file, gboolean reset)
{
#ifdef HAVE_SDL
	SDL_AudioSpec wave;
	SDL_AudioCVT cvt;
	Uint8 *tmp;

//	if (sound_cur < sound_len) return;

	sound_active = FALSE;

	if (reset)
	{
		if (sound_data) free (sound_data);
		sound_cur = 0;

		if (SDL_LoadWAV(file, &wave, &tmp, &sound_len) == NULL )
		{
			fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
			return;
		}
		SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
				AUDIO_S16, 2, 22050);
		sound_data = malloc(sound_len * cvt.len_mult);
		memcpy (sound_data, tmp, sound_len);
		cvt.buf = sound_data;
		cvt.len = sound_len;
		SDL_ConvertAudio(&cvt);
		SDL_FreeWAV(tmp);
	}
	else
		sound_cur = 0;

	sound_active = TRUE;
	SDL_PauseAudio (0);
#endif
}

void sound_play (SoundEvent event)
{
	char *file;
	SoundEvent prev_event = -1;
	switch (event)
	{
		case SOUND_PROGRAM_START:
			file = NULL;
			break;
		case SOUND_ILLEGAL_MOVE:
			file = "/usr/share/sounds/error.wav";
			break;
		case SOUND_HIGHSCORE:
			file = "/usr/lib/openoffice/share/gallery/sounds/applause.wav";
			break;
		case SOUND_WON:
			file = "/usr/share/sounds/KDE_Beep_RimShot.wav";
			break;
		case SOUND_USER_MOVE:
			file = "/usr/share/sounds/KDE_Click.wav";
			break;
		case SOUND_MACHINE_MOVE:
			file = "/usr/share/sounds/email.wav";
			break;
		default:
			file = NULL;
	}
	if (event == prev_event)
	{
		sound_init ();
		sound_play_real (file, FALSE);
	}
	if (file)
	{
		sound_init ();
		sound_play_real (file, TRUE);
	}
}
