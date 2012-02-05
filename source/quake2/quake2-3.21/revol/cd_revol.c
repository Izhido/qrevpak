/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#define BOOL_IMPLEMENTED 1

#include "../client/client.h"

extern cvar_t *s_volume;

extern int sound_started;

typedef struct trackinfo_s
{
	char* track;
	struct trackinfo_s *prev, *next;
} trackinfo_t;

static char         trackDir[MAX_OSPATH];
static char         tracks[MAX_OSPATH];
static qboolean     cdValid = false;
static qboolean	    playing = false;
static qboolean	    wasPlaying = false;
static qboolean	    initialized = false;
static qboolean	    enabled = false;
static qboolean     playLooping = false;
static byte 	    remap[100];
static byte		    playTrack;
static byte		    maxTrack;
static byte*        trackData = NULL;
static int          trackDataLength = 0;
static int          trackDataSize = 0;
static trackinfo_t* trackList = NULL;

cvar_t *cd_nocd;
cvar_t *cd_loopcount;
cvar_t *cd_looptrack;
cvar_t *cd_tracks;
cvar_t *cd_nofirst;


int		loopcounter;


void CDAudio_Pause(void);

static void CDAudio_AddTrack(trackinfo_t* nod, char* track)
{
	int ord;
	trackinfo_t* tnew;

	ord = strcmp(track, nod->track);
	if(ord != 0)
	{
		if(((ord < 0) && (nod->prev == NULL)) || ((ord > 0) && (nod->next == NULL)))
		{
			tnew = malloc(sizeof(trackinfo_t));
			if(tnew == 0)
				Com_Error(ERR_DROP, "CDAudio_AddTrack: can't allocate %i for track info\n", sizeof(trackinfo_t));

			tnew->track = malloc(strlen(track) + 1);
			if(tnew->track == 0)
				Com_Error(ERR_DROP, "CDAudio_AddTrack: can't allocate %i for track name\n", strlen(track) + 1);

			strcpy(tnew->track, track);
			tnew->prev = NULL;
			tnew->next = NULL;
			if(ord < 0)
			{
				nod->prev = tnew;
			} else
			{
				nod->next = tnew;
			};
		} else if(ord < 0)
		{
			CDAudio_AddTrack(nod->prev, track);
		} else
		{
			CDAudio_AddTrack(nod->next, track);
		};
	};
}


static trackinfo_t* CDAudio_GetTrack(trackinfo_t* nod, int index, int* lastfound)
{
	trackinfo_t* ret = NULL;

	if(nod->prev != NULL)
	{
		ret = CDAudio_GetTrack(nod->prev, index, lastfound);
	};

	if(ret != NULL)
		return ret;

	(*lastfound)++;
	if(index == (*lastfound))
	{
		return nod;
	};

	if(nod->next != NULL)
	{
		ret = CDAudio_GetTrack(nod->next, index, lastfound);
	};

	return ret;
}


static void CDAudio_DeleteTracks(trackinfo_t* nod)
{
	if(nod->next != NULL)
	{
		CDAudio_DeleteTracks(nod->next);
		free(nod->next);
	};
	if(nod->next != NULL)
	{
		CDAudio_DeleteTracks(nod->prev);
		free(nod->prev);
	};
	free(nod->track);
}


static void CDAudio_Eject(void)
{
}


static void CDAudio_CloseDoor(void)
{
}


static int CDAudio_GetAudioDiskInfo(void)
{
	char* t;

	cdValid = false;

	if(trackList != NULL)
	{
		CDAudio_DeleteTracks(trackList);
		free(trackList);
		trackList = NULL;
	};
	maxTrack = 0;

	sprintf(trackDir, cd_tracks->string, FS_Gamedir());
	sprintf(tracks, "%s/*.ogg", trackDir);
	t = Sys_FindFirst(tracks, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);
	if(t == NULL)
	{
		Sys_FindClose();
		return -1;
	}

	cdValid = true;

	do
	{
		if(trackList == NULL)
		{
			trackList = malloc(sizeof(trackinfo_t));
			if(trackList == 0)
				Com_Error(ERR_DROP, "CDAudio_GetAudioDiskInfo: can't allocate %i for track info\n", sizeof(trackinfo_t));

			trackList->track = malloc(strlen(t) + 1);
			if(trackList->track == 0)
				Com_Error(ERR_DROP, "CDAudio_GetAudioDiskInfo: can't allocate %i for track name\n", sizeof(strlen(t) + 1));

			strcpy(trackList->track, t);
			trackList->prev = NULL;
			trackList->next = NULL;
		} else
		{
			CDAudio_AddTrack(trackList, t);
		};
		maxTrack++;
		t = Sys_FindNext(0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM);
	} while(t != NULL);

	if(cd_nofirst->value)
		maxTrack++;

	Sys_FindClose();

	return 0;
}


void CDAudio_Play2(int track, qboolean looping)
{
	int i, lf;
	trackinfo_t* tnod;
	FILE* trackFile;
	int newTrackDataLength;
	byte* newTrackData;

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
		Com_DPrintf("CDAudio: Bad track number %u.\n", track);
		return;
	}

	i = track;

	if(cd_nofirst->value)
		i--;
	
	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop();
	}

	lf = -1;
	tnod = CDAudio_GetTrack(trackList, i - 1, &lf); 

	if(tnod == NULL)
	{
		Com_Printf("CDAudio: Can't locate track %i\n", track);
		return;
	}

	trackFile = fopen(tnod->track, "rb");
	if(trackFile == NULL)
	{
		Com_Printf("CDAudio: Can't open track %i\n", track);
		return;
	};

	fseek(trackFile, 0, SEEK_END);
	newTrackDataLength = ftell(trackFile);
	fseek(trackFile, 0, SEEK_SET);

	if(newTrackDataLength <= 0)
	{
		fclose(trackFile);
		Com_Printf("CDAudio: Can't open track %i\n", track);
		return;
	};

	if(trackData == NULL)
	{
		trackData = malloc(newTrackDataLength);
		if(trackData == 0)
		{
			fclose(trackFile);
			Com_Error(ERR_DROP, "CDAudio_GetAudioDiskInfo: can't allocate %i for track data\n", trackDataLength);
		};
		trackDataSize = newTrackDataLength;
	} else if(trackDataSize < newTrackDataLength)
	{
		newTrackData = realloc(trackData, newTrackDataLength);
		if(newTrackData == NULL)
		{
			free(trackData);
			trackData = malloc(newTrackDataLength);
			if(trackData == 0)
			{
				fclose(trackFile);
				Com_Error(ERR_DROP, "CDAudio_GetAudioDiskInfo: can't reallocate %i for track data\n", trackDataLength);
			};
		} else
		{
			trackData = newTrackData;
		};
		trackDataSize = newTrackDataLength;
	};
	trackDataLength = newTrackDataLength;

	fread(trackData, 1, trackDataLength, trackFile);
	fclose(trackFile);

	if(PlayOgg(trackData, trackDataLength, 0, OGG_ONE_TIME))
	{
		Com_Printf("CDAudio: Can't play track %i\n", track);
		return;
	};

	playLooping = looping;
	playTrack = track;
	playing = true;

	//Com_Printf("Now playing: %s\n", tnod->track);

	if ( Cvar_VariableValue( "cd_nocd" ) )
		CDAudio_Pause ();
}


void CDAudio_Play(int track, qboolean looping)
{
	// set a loop counter so that this track will change to the
	// looptrack later
	loopcounter = 0;
	CDAudio_Play2(track, looping);
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
					Com_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = atoi(Cmd_Argv (n+1));
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
			Com_Printf("No CD in player.\n");
			return;
		}
	}

	if (Q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play(atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play(atoi(Cmd_Argv (2)), true);
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
		Com_Printf("%u tracks\n", maxTrack);
		if (playing)
			Com_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Com_Printf("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		return;
	}
}


void CDAudio_Update(void)
{
	if ( cd_nocd->value != !enabled )
	{
		if ( cd_nocd->value )
		{
			CDAudio_Stop();
			enabled = false;
		}
		else
		{
			enabled = true;
			CDAudio_Resume ();
		}
	};

	if (!enabled)
		return;

	if(StatusOgg() == OGG_STATUS_EOF)
	{
		if(playing)
		{
			playing = false;
			if (playLooping)
			{
				// if the track has played the given number of times,
				// go to the ambient track
				if (++loopcounter >= cd_loopcount->value)
					CDAudio_Play2(cd_looptrack->value, true);
				else
					CDAudio_Play2(playTrack, true);
			}
		};
	};
}


int CDAudio_Init(void)
{
	int n;

	cd_nocd = Cvar_Get ("cd_nocd", "0", CVAR_ARCHIVE );
	cd_loopcount = Cvar_Get ("cd_loopcount", "4", 0);
	cd_looptrack = Cvar_Get ("cd_looptrack", "11", 0);
	cd_tracks = Cvar_Get ("cd_tracks", "%s/cdtracks", CVAR_ARCHIVE);
	cd_nofirst = Cvar_Get ("cd_nofirst", "1", CVAR_ARCHIVE);
	if ( cd_nocd->value)
		return -1;

	if(!sound_started)
		return -1;

	for (n = 0; n < 100; n++)
		remap[n] = n;
	initialized = true;
	enabled = true;

	Cmd_AddCommand ("cd", CD_f);

	Com_Printf("CD Audio Initialized\n");

	return 0;
}


void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	free(trackData);
	CDAudio_Stop();
}