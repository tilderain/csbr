/* SOUND.C
  PXT/SS/Org sound interface
*/
#include <stdio.h>
#include <string.h>

#include "../nx.h"
#include "../settings.h"
#include "pxt.h"

#ifdef _WIN32
	#include "Windows.h"

	HMODULE hModule = NULL;
	typedef int (__stdcall *pstartOrganya)(HWND);

	pstartOrganya startOrganya;
	ploadOrganya loadOrganya;
	psetOrganyaPosition setOrganyaPosition;
	pgetOrganyaPosition getOrganyaPosition;
	pplayOrganyaMusic playOrganyaMusic;
	pchangeOrganyaVolume changeOrganyaVolume;
	pstopOrganyaMusic stopOrganyaMusic;
	pendOrganya endOrganya;
#endif

#include "sound.h"
#include "sound.fdh"


#define MUSIC_OFF		0
#define MUSIC_ON		1
#define MUSIC_BOSS_ONLY	2
static int lastsong = 0;		// this holds the previous song, for <RMU
static int cursong = 0;

// there are more than this around 9b; those are drums and are loaded by the org module
#define NUM_SOUNDS		0x75
#define ORG_VOLUME		75

const char *org_names[] =
{
	NULL,
	"egg", "safety", "gameover", "gravity", "grasstown", "meltdown2", "eyesofflame",
	"gestation", "town", "fanfale1", "balrog", "cemetary", "plant", "pulse", "fanfale2",
	"fanfale3", "tyrant", "run", "jenka1", "labyrinth", "access", "oppression", "geothermal",
	"theme", "oside", "heroend", "scorching", "quiet", "lastcave", "balcony", "charge",
	"lastbattle", "credits", "zombie", "breakdown", "hell", "jenka2", "waterway", "seal",
	"toroko", "white", "azarashi", NULL
};

static const char bossmusic[] = { 4, 7, 10, 11, 15, 16, 17, 18, 21, 22, 31, 33, 35, 0 };

static const char *pxt_dir = "./pxt/";
static const char *org_dir = "./org/";
static const char *sndcache = "sndcache.pcm";
static const char *org_wavetable = "wavetable.dat";

bool sound_init(void)
{
	if (SSInit()) return 1;
	if (pxt_init()) return 1;
	if (pxt_LoadSoundFX(pxt_dir, sndcache, NUM_SOUNDS)) return 1;

#ifndef _WIN32
	if (org_init(org_wavetable, pxt_dir, ORG_VOLUME))
	{
		staterr("Music failed to initialize");
		return 1;
	}
#elif _WIN32
	 hModule = LoadLibrary("Org001.dll");
  
	if(hModule == NULL) {
		staterr("failed to load organya library!\n");
		return 1;
	} else {
		stat("loaded organya library\n");
	}
	
	
	HWND handle = GetActiveWindow();
	
	startOrganya = (pstartOrganya)GetProcAddress(hModule, "StartOrganya");
	loadOrganya = (ploadOrganya)GetProcAddress(hModule, "LoadOrganya");
	setOrganyaPosition = (psetOrganyaPosition)GetProcAddress(hModule, "SetOrganyaPosition");
	getOrganyaPosition = (pgetOrganyaPosition)GetProcAddress(hModule, "GetOrganyaPosition");
	playOrganyaMusic = (pplayOrganyaMusic)GetProcAddress(hModule, "PlayOrganyaMusic");
	changeOrganyaVolume = (pchangeOrganyaVolume)GetProcAddress(hModule, "ChangeOrganyaVolume");
	stopOrganyaMusic = (pstopOrganyaMusic)GetProcAddress(hModule, "StopOrganyaMusic");
	endOrganya = (pendOrganya)GetProcAddress(hModule, "EndOrganya");
	
	startOrganya(handle);
#endif

	return 0;
}

void sound_close(void)
{
	pxt_freeSoundFX();
	SSClose();
}

/*
void c------------------------------() {}
*/

void sound(int snd)
{
	if (!settings->sound_enabled)
		return;
	
	pxt_Stop(snd);
	pxt_Play(-1, snd, 0);
}

void sound_loop(int snd)
{
	if (!settings->sound_enabled)
		return;
	
	pxt_Play(-1, snd, -1);
}

void sound_stop(int snd)
{
	pxt_Stop(snd);
}

bool sound_is_playing(int snd)
{
	return pxt_IsPlaying(snd);
}


void StartStreamSound(int freq)
{
	// pxt_ChangePitch(SND_STREAM1, some_formula);
	// pxt_ChangePitch(SND_STREAM2, some_other_formula);
	sound_loop(SND_STREAM1);
	sound_loop(SND_STREAM2);
}

void StartPropSound(void)
{
	sound_loop(SND_PROPELLOR);
}

void StopLoopSounds(void)
{
	sound_stop(SND_STREAM1);
	sound_stop(SND_STREAM2);
	sound_stop(SND_PROPELLOR);
}

/*
void c------------------------------() {}
*/

void music(int songno)
{
	if (songno == cursong)
		return;
	
	lastsong = cursong;
	cursong = songno;
	
	stat(" >> music(%d)", songno);
	
	if (songno != 0 && !should_music_play(songno, settings->music_enabled))
	{
		stat("Not playing track %d because music_enabled is %d", songno, settings->music_enabled);
#ifndef _WIN32
		org_stop();
#elif _WIN32
		stopOrganyaMusic();
#endif
		return;
	}
	
	start_track(songno);
}


bool should_music_play(int songno, int musicmode)
{
	if (game.mode == GM_TITLE || game.mode == GM_CREDITS)
		return true;
	
	switch(musicmode)
	{
		case MUSIC_OFF: return false;
		case MUSIC_ON:  return true;
		case MUSIC_BOSS_ONLY:
			return music_is_boss(songno);
	}
	
	return false;
}

bool music_is_boss(int songno)
{
	if (strchr(bossmusic, songno))
		return true;
	else
		return false;
}

void music_set_enabled(int newstate)
{
	stat("music_set_enabled(%d)", newstate);
	
	settings->music_enabled = newstate;
	bool play = should_music_play(cursong, newstate);
	
	if (play)
		start_track(cursong);
	else
#ifndef _WIN32
		org_stop();
#elif _WIN32
		stopOrganyaMusic();
#endif

}

static void start_track(int songno)
{
char fname[MAXPATHLEN];

	if (songno == 0)
	{
#ifndef _WIN32
		org_stop();
#elif _WIN32
		stopOrganyaMusic();
#endif
		return;
	}

#ifndef _WIN32
		org_stop();
#elif _WIN32
		stopOrganyaMusic();
#endif

	strcpy(fname, org_dir);
	strcat(fname, org_names[songno]);
	strcat(fname, ".org");
	
#ifndef _WIN32
	if (!org_load(fname))
	{
		org_start(0);
#elif _WIN32
	if (loadOrganya(fname))
	{
		playOrganyaMusic();
		changeOrganyaVolume(ORGDLL_VOLUME);
#endif
	}
}

int music_cursong()		{ return cursong; }
int music_lastsong() 	{ return lastsong; }


