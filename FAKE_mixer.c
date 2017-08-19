#include "SDL_mixer.h"
#include "mm.h"

//SDL_Mixer
static void (*music_finished_hook)(void) = NULL;
static void (*channel_finished_hook)(int channel) = NULL;
static void (*mix_music)(void *udata) = NULL;
static void *music_data = NULL;

int loaded=0;
int gfrequency=0;
int gchannels=0;
int voicevolume=256;
int musicvolume=128;
int v1;

static void *snd_thread(void *data)
{

	for(;;)
	{
		MikMod_Update();
		thd_sleep(5);
	}
	
	return NULL;
}


int Mix_OpenAudio(int frequency, uint16 format, int channels, int chunksize)
{
    /* register all the drivers */
    MikMod_RegisterAllDrivers();

    /* register all the module loaders */
    MikMod_RegisterAllLoaders();

    /* initialize the library */
    md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
    
    if (MikMod_Init("")) {
        printf("Could not initialize sound, reason: %s\n", MikMod_strerror(MikMod_errno));
		return 1;
    }
    
    /* One for music and one for sfx */
    MikMod_SetNumVoices(-1,8);
    MikMod_EnableOutput();
    
    loaded = 1;
	gfrequency = frequency;
	gchannels = channels;
	
	thd_create(0, snd_thread, NULL);
	return 0;
}


void Mix_CloseAudio(void)
{
	if (loaded == 1){
		MikMod_DisableOutput();
		MikMod_Exit();
		loaded = 0;
	}
}

void Mix_FreeChunk(Mix_Chunk *chunk)
{
	Sample_Free((SAMPLE *)chunk);
}

int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
{
	return Mix_PlayChannelTimed(channel, chunk, loops, -1);
}

int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks)
{
	SAMPLE *sample;
	sample=(SAMPLE *)chunk;
	
	//OK, Mikmod will play any loaded wav at max sound at fisrt!
	//SO, must change internal sample volume before playing... dirty!
	sample->volume = voicevolume;
	
    v1 = Sample_Play(sample, 0, 0);
	return 0;
}

int Mix_HaltChannel(int channel)
{
	Player_Stop();
    
	if (channel_finished_hook){
		channel_finished_hook(channel);
	}
	
	return 0;
}

//0-256
int Mix_Volume(int channel, int v)
{
	voicevolume = v;
	if (Voice_Stopped(v1) == 0) Voice_SetVolume(v1, v);
	return v;
}

void Mix_Resume(int channel)
{
	return;
}

void Mix_Pause(int channel)
{
	return;
}

Mix_Chunk *Mix_LoadWAV(char *file)
{
	SAMPLE *sample;
    /* load module */
    sample = Sample_Load(file);
    if (sample) {
		return (Mix_Chunk *) sample;
    }else
	{
        printf("Could not load sample, reason: %s\n", MikMod_strerror(MikMod_errno));
	}
	return NULL;
}

//0-256
int Mix_VolumeChunk(Mix_Chunk *chunk, int volume)
{
	voicevolume = volume;
	if (Voice_Stopped(v1) == 0) Voice_SetVolume(v1, volume);
	return volume;
}

Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc)
{
	SAMPLE *sample;
   	struct MREADER* reader = _mm_new_rwops_reader (src);
   	if (reader) {
   		sample = Sample_LoadGeneric(reader);
		if(reader) free(reader);
		return (Mix_Music *) sample;
   	}
   	else
	{
        printf("Could not load sample, reason: %s\n", MikMod_strerror(MikMod_errno));
	}
	return NULL;
}

//This one is for wavs!
int Mix_Playing(int channel)
{
	if (Voice_Stopped(v1) == 0) return 1;
	else return 0;
}

//Music part!
int Mix_PlayingMusic(void)
{
	return Player_Active();
}

Mix_Music *Mix_LoadMUS(const char *file)
{
	MODULE *module;
    /* load module */
    module = Player_Load((char *)(file), 64, 0);
    if (module) {
		return (Mix_Music *) module;
    }else
	{
        printf("Could not load module, reason: %s\n", MikMod_strerror(MikMod_errno));
	}
	return NULL;

}

/* SDL_RWops compatibility */
Mix_Music *Mix_LoadMUS_RW(SDL_RWops *rw)
{
	//Sanity check: only MOD!
	if (detect_music_type(rw) != MUS_MOD){
		printf("Error: format not supported.\n");
		return NULL;
	}
	
	MODULE *module;
   	struct MREADER* reader = _mm_new_rwops_reader (rw);
   	if (reader) {
   		module = Player_LoadGeneric(reader,64,0);
		if(reader) free(reader);
		return (Mix_Music *) module;
   	}
   	else
	{
        printf("Could not load module, reason: %s\n", MikMod_strerror(MikMod_errno));
	}
	return NULL;
}

//-1 plays music forever.
//0 should play 0 times, but when using hooks, there are no args, so it sends 0!
//TODO: Positive will play only one.
int Mix_PlayMusic(Mix_Music *music, int loops)
{
	if (music == NULL){
		//Error, no music!
		return -1;
	}
	
	MODULE *module;
	module=(MODULE *)music;
	if(loops == -1){
		module->wrap=1;
		module->loop=1;
	}
	
	//Set music volume before playing
	module->volume = musicvolume;
	Player_Start(module);
	
	if (mix_music){
		mix_music(music_data);
	}
	
	return 0;
}

int Mix_HaltMusic(void)
{
    Player_Stop();
    
	if (music_finished_hook){
		music_finished_hook();
	}
	
	return 0;
}

void Mix_FreeMusic(Mix_Music *music)
{
	Player_Stop(); //Just in case!
	MODULE *module; //Casting needed
	module = (MODULE *)music;
	
    Player_Free(module);
}

int Mix_SetMusicPosition(double position)
{
	Player_SetPosition(position);
	return 0;
}

int Mix_AllocateChannels(int numchans)
{
	return numchans; //LOL!
}

int Mix_ReserveChannels(int num)
{
	return num;
}

int Mix_QuerySpec(int *frequency, Uint16 *format, int *channels)
{
	*frequency = gfrequency;
	*channels = gchannels;
	*format = 0x8010;  /** Signed 16-bit samples */
	return loaded;
}

int Mix_SetReverseStereo(int channel, int flip)
{
	return 1;
}

int Mix_SetDistance(int channel, Uint8 distance)
{
	return 1;
}

int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance) 
{
	return 1;
}

int Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
	return 1;
}

//Do a normal stop
int Mix_FadeOutMusic(int ms)
{
	return Mix_HaltMusic();
}

//Do a normal play
int Mix_FadeInMusic(Mix_Music *music, int loops, int ms)
{
	return Mix_PlayMusic(music, loops);
}

void Mix_ResumeMusic(void)
{
	if (Player_Paused()){
		Player_TogglePause();
	}
}

void Mix_PauseMusic(void)
{
	if (!Player_Paused()){
		Player_TogglePause();
	}
}

//0-128
int Mix_VolumeMusic(int volume)
{
	musicvolume = volume;
	if (Player_Active()) Player_SetVolume(musicvolume);
	return volume;
}

//This sets up a function to be called when music playback is halted.
void Mix_HookMusicFinished(void (*music_finished)()) 
{
	SDL_LockAudio();
	music_finished_hook = music_finished;
	SDL_UnlockAudio();
}

void Mix_ChannelFinished(void (*channel_finished)(int channel)) 
{
	SDL_LockAudio();
	channel_finished_hook = channel_finished;
	SDL_UnlockAudio();
}

void Mix_HookMusic(void (*mix_func)(void *udata), void *arg)
{
	SDL_LockAudio();
	if ( mix_func != NULL ) {
		music_data = arg;
		mix_music = mix_func;
	} else {
		music_data = NULL;
		mix_music = NULL;
	}
	SDL_UnlockAudio();
}

//Range 32-255
int Mix_GetTempo(Mix_Music *music)
{
	MODULE *module;
	module=(MODULE *)music;
	if (module != NULL){
		return module->bpm;
	}
	return -1;
}

void Mix_SetTempo(Mix_Music *music, int tempo)
{
	Player_SetTempo(tempo);
}

int Mix_SetMusicCMD(const char *command)
{
	return 0;
}

/* MUS_MOD can't be auto-detected. If no other format was detected, MOD is
 * assumed and MUS_MOD will be returned, meaning that the format might not
 * actually be MOD-based.
 *
 * Returns MUS_NONE in case of errors. */
Mix_MusicType detect_music_type(SDL_RWops *rw)
{
	Uint8 magic[5];
	Uint8 moremagic[9];

	int start = SDL_RWtell(rw);
	if (SDL_RWread(rw, magic, 1, 4) != 4 || SDL_RWread(rw, moremagic, 1, 8) != 8 ) {
		printf("Couldn't read from RWops\n");
		return MUS_NONE;
	}
	SDL_RWseek(rw, start, RW_SEEK_SET);
	magic[4]='\0';
	moremagic[8] = '\0';

	/* WAVE files have the magic four bytes "RIFF"
	   AIFF files have the magic 12 bytes "FORM" XXXX "AIFF" */
	if (((strcmp((char *)magic, "RIFF") == 0) && (strcmp((char *)(moremagic+4), "WAVE") == 0)) ||
		(strcmp((char *)magic, "FORM") == 0)) {
		printf("Detected WAV from RW\n");
		return MUS_WAV;
	}

	/* Ogg Vorbis files have the magic four bytes "OggS" */
	if (strcmp((char *)magic, "OggS") == 0) {
		printf("Detected OGG from RW\n");
		return MUS_OGG;
	}

	/* FLAC files have the magic four bytes "fLaC" */
	if (strcmp((char *)magic, "fLaC") == 0) {
		printf("Detected FLAC from RW\n");
		return MUS_FLAC;
	}

	/* MIDI files have the magic four bytes "MThd" */
	if (strcmp((char *)magic, "MThd") == 0) {
		printf("Detected MIDI from RW\n");
		return MUS_MID;
	}

	if (detect_mp3(magic)) {
		printf("Detected MP3 from RW\n");
		return MUS_MP3;
	}

	/* Assume MOD format.
	 *
	 * Apparently there is no way to check if the file is really a MOD,
	 * or there are too many formats supported by MikMod/ModPlug, or
	 * MikMod/ModPlug does this check by itself. */
	return MUS_MOD;
}

int detect_mp3(Uint8 *magic)
{
	if ( strncmp((char *)magic, "ID3", 3) == 0 ) {
		return 1;
	}

	/* Detection code lifted from SMPEG */
	if(((magic[0] & 0xff) != 0xff) || // No sync bits
	   ((magic[1] & 0xf0) != 0xf0) || //
	   ((magic[2] & 0xf0) == 0x00) || // Bitrate is 0
	   ((magic[2] & 0xf0) == 0xf0) || // Bitrate is 15
	   ((magic[2] & 0x0c) == 0x0c) || // Frequency is 3
	   ((magic[1] & 0x06) == 0x00)) { // Layer is 4
		return(0);
	}
	return 1;
}