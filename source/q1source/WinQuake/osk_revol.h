/*
Copyright (C) 1997-2001 Id Software, Inc.
Modifications (C) 2009 Heriberto Delgado.

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

// On-screen keyboard common data & functions

#include <gccore.h>
#define BOOL_IMPLEMENTED 1

typedef enum {okl_normal, okl_shift, okl_shiftcaps, okl_caps} osklayout_t;

typedef struct
{
	osklayout_t layout; 
	int	left;			
	int	right;
	int	top;		
	int	bottom; // screen coords
	int key;	// only used if it refers to a specific key being pressed
} oskkey_t;

extern osklayout_t osk_layout;

extern oskkey_t* osk_selected;

extern oskkey_t* osk_shiftpressed;

extern oskkey_t* osk_capspressed;

void OSK_LoadKeys(const u8* keys, int len);

oskkey_t* OSK_KeyAt(int x, int y);

