

#include "SDL_mixer.h"
#include "mikmod.h"

int playing_music=0;

static void snd_thread(void *data)
{
	printf("sndserver activo\n");//, thd_get_current()->tid);

	for(;;)
	{
		if(playing_music)
		{
			if (Player_Active())
			{
		//		printf("sndserver reproduciendo\n");
		 		MikMod_Update();
			}
		}
		thd_sleep(5);
	}
}


int Mix_OpenAudio(int frequency, uint16 format, int channels, int chunksize)
{
//    snd_stream_init();
	printf("FAKE Mixer initializing audio\n");
    /* register all the drivers */
    MikMod_RegisterAllDrivers();

    /* register all the module loaders */
    MikMod_RegisterAllLoaders();

    /* initialize the library */
    md_mode |= DMODE_SOFT_MUSIC;
    if (MikMod_Init("")) {
        printf("Could not initialize sound, reason: %s\n", MikMod_strerror(MikMod_errno));
		return 1;
    }
	thd_create(0, snd_thread, NULL);
	return 0;
}


void Mix_CloseAudio(void)
{

}


int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
{
	snd_sfx_play(chunk, 255, 128);
	return 0;
}


int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
	snd_sfx_play(chunk, 255, 128);
	return 0;
}

int Mix_HaltChannel(int channel)
{

	return 0;
}


int Mix_Volume(int channel, int volume)
{

	return 0;
}


Mix_Chunk *Mix_LoadWAV(const char *file)
{
	Mix_Chunk snd;
	printf("FAKE Mixer loading sound %s\n", file);
	snd = snd_sfx_load(file);
	if(snd)	return snd;
	else    return 0;
	
}


/*Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc)
{

return 0;
}*/


int Mix_VolumeChunk(Mix_Chunk *chunk, int volume)
{

	return 0;
}


Mix_Music *Mix_LoadMUS(const char *file)
{

	MODULE *module;
    /* load module */
	printf("FAKE Mixer loading music %s\n", file);
    module = Player_Load(file, 64, 0);
    if (module) {
		return (Mix_Music *)module;
    }else
	{
        printf("Could not load module, reason: %s\n", MikMod_strerror(MikMod_errno));
	}
	return 1;

}


int Mix_PlayMusic(Mix_Music *music, int loops)
{
	MODULE *module;
	module=(MODULE *)music;
	if(loops){
		module->wrap=1;
		module->loop=1;
	}
	Player_Start(module);
	playing_music=1;
	return 0;
}


int Mix_HaltMusic(void)
{
        Player_Stop();
		playing_music=0;
		return 0;
}


void Mix_FreeMusic(Mix_Music *music)
{
        Player_Free(music);
}
