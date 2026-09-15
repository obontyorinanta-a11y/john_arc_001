#ifndef AUDIO_HPP
#define AUDIO_HPP

// -----------------------------------------------------------------
//  Audio.hpp - music and sound effects.
//
//  iGraphics has no audio of its own, so this uses the Windows
//  multimedia API directly:
//     music  -> mciSendString, which can stream and loop an MP3
//     effect -> PlaySound, which plays a short WAV without blocking
//  They are separate subsystems, so an effect does not cut the music.
//
//  winmm.lib is already linked by a #pragma inside glut.h, and
//  mmsystem.h arrives with windows.h, so nothing has to be added to
//  the project settings.
// -----------------------------------------------------------------

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "Config.hpp"

int gMusicOn = 1;
int gSoundOn = 1;
char gMusicAlias[32] = "";

// ---- music (MP3 through MCI) ----

void musicStop()
{
	if (gMusicAlias[0] == 0) return;
	char cmd[128];
	sprintf(cmd, "close %s", gMusicAlias);
	mciSendStringA(cmd, NULL, 0, NULL);
	gMusicAlias[0] = 0;
}

void musicPlay(const char* file, const char* alias, int loop)
{
	char cmd[512];

	musicStop();
	if (!gMusicOn) return;

	sprintf(cmd, "open \"%s\" type mpegvideo alias %s", file, alias);
	if (mciSendStringA(cmd, NULL, 0, NULL) != 0) {
		// no MP3 decoder or file missing - the game carries on in silence
		return;
	}
	strcpy(gMusicAlias, alias);

	sprintf(cmd, "play %s%s", alias, loop ? " repeat" : "");
	mciSendStringA(cmd, NULL, 0, NULL);
}

void musicToggle()
{
	gMusicOn = !gMusicOn;
	if (!gMusicOn) musicStop();
}

// ---- effects (WAV through PlaySound) ----

void sfx(const char* name)
{
	char path[256];
	if (!gSoundOn) return;
	sprintf(path, "Audios/sfx/%s.wav", name);
	PlaySoundA(path, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

#endif
