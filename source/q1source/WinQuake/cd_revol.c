/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <gccore.h>

#include "oggplayer.h"

#include "quakedef.h"

extern	cvar_t	bgmvolume;

cvar_t cd_tracks = {"cd_tracks", "%s/cdtracks", true};

cvar_t cd_nofirst = {"cd_nofirst", "1", true};

static char trackDir[MAX_OSPATH];
static char tracks[MAX_OSPATH];
static qboolean cdValid = false;
static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	initialized = false;
static qboolean	enabled = false;
static qboolean playLooping = false;
static float	cdvolume;
static byte 	remap[100];
static byte		playTrack;
static byte		maxTrack;
static byte* trackData = NULL;
static int trackDataLength = 0;


static void CDAudio_CloseDoor(void)
{
}


static void CDAudio_Eject(void)
{
}


static int CDAudio_GetAudioDiskInfo(void)
{

	cdValid = false;

	sprintf(trackDir, cd_tracks.string, com_gamedir);
	sprintf(tracks, "%s/*.ogg", trackDir);
	if(!Sys_FindFirst(tracks, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
	{
		Sys_FindClose();
		return -1;
	}

	cdValid = true;
	maxTrack = 1;

	while(Sys_FindNext(0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
	{
		maxTrack++;
	};

	if(cd_nofirst.value)
		maxTrack++;

	Sys_FindClose();

	return 0;
}


void CDAudio_SetVolume (float newvolume)
{
	if (!initialized || !enabled)
		return;

	SetVolumeOgg((int)(newvolume * 255.0));
}


void CDAudio_Play(byte track, qboolean looping)
{
	char* trackFile;
	int trackHdl;
	int m;

	if (!enabled)
		return;
	
	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
			return;
	}

	track = remap[track];

	if (track < 1 || track > maxTrack)
	{
		Con_DPrintf("CDAudio: Bad track number %u.\n", track);
		return;
	}

	m = track;

	if(cd_nofirst.value)
		m--;

	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop();
	}

	trackFile = Sys_FindFirst(tracks, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);
	m--;
	while((trackFile != NULL) && (m > 0))
	{
		trackFile = Sys_FindNext(0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);
		m--;
	};

	Sys_FindClose();

	if(trackFile == NULL)
	{
		Con_Printf("CDAudio: Can't locate track %i\n", track);
		return;
	}

	free(trackData);

	trackDataLength = Sys_FileOpenRead(trackFile, &trackHdl);
	if(trackDataLength <= 0)
	{
		trackDataLength = 0;
		Con_Printf("CDAudio: Can't open track %i\n", track);
	};

	trackData = Sys_Malloc(trackDataLength, "CDAudio_Play");
	Sys_FileRead(trackHdl, trackData, trackDataLength);
	Sys_FileClose(trackHdl);

	if(PlayOgg(trackData, trackDataLength, 0, looping ? OGG_INFINITE_TIME : OGG_ONE_TIME))
	{
		Con_Printf("CDAudio: Can't play track %i\n", track);
		return;
	};

	playLooping = looping;
	playTrack = track;
	playing = true;

	Con_Printf("Now playing: %s\n", trackFile);

	if (cdvolume == 0.0)
		CDAudio_Pause ();
}


void CDAudio_Stop(void)
{
	if (!enabled)
		return;
	
	if (!playing)
		return;

	StopOgg();

	wasPlaying = false;
	playing = false;
}


void CDAudio_Pause(void)
{
	if (!enabled)
		return;

	if (!playing)
		return;

	PauseOgg(1);

	wasPlaying = playing;
	playing = false;
}


void CDAudio_Resume(void)
{
	if (!enabled)
		return;
	
	if (!cdValid)
		return;

	if (!wasPlaying)
		return;

	PauseOgg(0);

	playing = true;
}


void CDAudio_Update(void)
{
	if (!enabled)
		return;

	if (bgmvolume.value != cdvolume)
	{
		cdvolume = bgmvolume.value;
		CDAudio_SetVolume (cdvolume);
		if (cdvolume)
		{
			if(!playing)
				CDAudio_Resume ();
		}
		else
		{
			if(playing)
				CDAudio_Pause ();
		}
	}
}


static void CD_f (void)
{
	char	*command;
	int		ret;
	int		n;

	if (Cmd_Argc() < 2)
		return;

	command = Cmd_Argv (1);

	if (Q_strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (Q_strcasecmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (Q_strcasecmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (Q_strcasecmp(command, "remap") == 0)
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (remap[n] != n)
					Con_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = Q_atoi(Cmd_Argv (n+1));
		return;
	}

	if (Q_strcasecmp(command, "close") == 0)
	{
		CDAudio_CloseDoor();
		return;
	}

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
		{
			Con_Printf("No CD in player.\n");
			return;
		}
	}

	if (Q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play((byte)Q_atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play((byte)Q_atoi(Cmd_Argv (2)), true);
		return;
	}

	if (Q_strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (Q_strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (Q_strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (Q_strcasecmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (Q_strcasecmp(command, "info") == 0)
	{
		Con_Printf("%u tracks\n", maxTrack);
		if (playing)
			Con_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Con_Printf("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Con_Printf("Volume is %f\n", cdvolume);
		return;
	}
}


int CDAudio_Init(void)
{
	int n;

	if (cls.state == ca_dedicated)
		return -1;

	if (COM_CheckParm("-nocdaudio"))
		return -1;

	if(!snd_initialized)
		return -1;

	if(fakedma)
		return -1;

	for (n = 0; n < 100; n++)
		remap[n] = n;
	initialized = true;
	enabled = true;

	Cvar_RegisterVariable(&cd_tracks);
	Cvar_RegisterVariable(&cd_nofirst);

	Cmd_AddCommand ("cd", CD_f);

	Con_Printf("CD Audio Initialized\n");

	return 0;
}


void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
}