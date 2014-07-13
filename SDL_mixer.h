
#ifndef _FAKE_MIXER_H
#define _FAKE_MIXER_H

#include "kos.h"
#include <SDL/SDL.h>

#define MIX_DEFAULT_FORMAT	0
#define MIX_MAX_VOLUME		255
#define Mix_GetError		SDL_GetError
#define Mix_Music			void
#define Mix_Chunk			sfxhnd_t

//extern struct SDL_RWops;

#ifdef __cplusplus
extern "C" {
#endif

int Mix_OpenAudio(int frequency, uint16 format, int channels, int chunksize);
void Mix_CloseAudio(void);

int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);
int Mix_HaltChannel(int channel);
int Mix_Volume(int channel, int volume);

Mix_Chunk *Mix_LoadWAV(const char *file);
//Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
int Mix_VolumeChunk(Mix_Chunk *chunk, int volume);


Mix_Music *Mix_LoadMUS(const char *file);
int Mix_PlayMusic(Mix_Music *music, int loops);
int Mix_HaltMusic(void);
void Mix_FreeMusic(Mix_Music *music);

#ifdef __cplusplus
}
#endif

#endif
