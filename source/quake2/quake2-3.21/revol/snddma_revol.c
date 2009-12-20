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

// snd_revol.c -- sound driver for the Nintendo Wii using devkitPPC / libogc
// Implements the remaining functions required by the software mixer
// Based on snddma_null.c

#include <gccore.h>
#include <asndlib.h>
#define BOOL_IMPLEMENTED 1

#include "../client/client.h"
#include "../client/snd_loc.h"

volatile int snd_current_sample_pos;

void SNDDMA_Loopback(s32 voice)
{
	ASND_AddVoice(voice, (unsigned short*)dma.buffer + snd_current_sample_pos, dma.samples >> 2);
	snd_current_sample_pos += (dma.samples >> 3);
	if(snd_current_sample_pos >= dma.samples)
		snd_current_sample_pos = 0;
}

qboolean SNDDMA_Init(void)
{
	ASND_Init();
	ASND_Pause(0);

	dma.samplebits = 16;
	dma.speed = 22050;
	dma.channels = 2;
	dma.samples = 32768;
	dma.samplepos = 0;
	dma.submission_chunk = 1;
	dma.buffer = malloc(65536);
	if(dma.buffer == 0)
		Com_Error(ERR_DROP, "SNDDMA_Init: can't allocate %i in dma.buffer\n", 65536);

	ASND_SetVoice(1, VOICE_STEREO_16BIT, dma.speed, 0, dma.buffer, dma.samples >> 2, 255, 255, SNDDMA_Loopback);
	snd_current_sample_pos = (dma.samples >> 3);

	return true;
}

int	SNDDMA_GetDMAPos(void)
{
	dma.samplepos = snd_current_sample_pos;
	return dma.samplepos;
}

void SNDDMA_Shutdown(void)
{
	ASND_End();
	free(dma.buffer);
}

void SNDDMA_BeginPainting (void)
{
}

void SNDDMA_Submit(void)
{
}
