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
// in_revol.c -- input handler for the Nintendo Wii using devkitPPC / libogc
// (based on in_null.c)

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/usbmouse.h>
#define BOOL_IMPLEMENTED 1

#include "../client/client.h"

extern GXRModeObj* sys_rmode;

typedef struct
{
	int x; 
	int y;
} incursorcoords_t;

// mouse variables
cvar_t	*m_filter;

cvar_t	*in_mouse;

cvar_t	*in_wmote;

cvar_t	*in_gcpad;

cvar_t	*in_joystick;

cvar_t	*in_osk;

qboolean	mouseactive;	// false when not focus app

qboolean	mouseinitialized;

qboolean	mlooking;

qboolean	wmoteactive;	// false when not focus app

qboolean	wmoteinitialized;

qboolean	gcpadactive;	// false when not focus app

qboolean	gcpadinitialized;

incursorcoords_t current_pos;

int			mouse_x, mouse_y, old_mouse_x, old_mouse_y;

int			window_center_x, window_center_y;

qboolean	in_appactive;

int			wmote_x, wmote_y, old_wmote_x, old_wmote_y;

int			wmote_adjust_x, wmote_adjust_y;

u8			in_previous_mouse_buttons;

int			gcpad_x, gcpad_y, old_gcpad_x, old_gcpad_y;

void IN_StartupMouse (void)
{
	cvar_t		*cv;

	cv = Cvar_Get ("in_initmouse", "1", CVAR_NOSET);
	if ( !cv->value ) 
		return; 

	if (MOUSE_Init() != 0)
	{
		Sys_ConsoleOutput("Mouse not found\n");
		return;
	};

	mouseinitialized = true;
}

void IN_StartupWmote (void)
{
	cvar_t		*cv;

	cv = Cvar_Get ("in_initwmote", "1", CVAR_NOSET);
	if ( !cv->value ) 
		return; 

	wmote_adjust_x = 0;
	wmote_adjust_y = 0;

	wmoteinitialized = true;
}

void IN_StartupGCPad (void)
{
	cvar_t		*cv;

	cv = Cvar_Get ("in_initgcpad", "1", CVAR_NOSET);
	if ( !cv->value ) 
		return; 

	gcpadinitialized = true;
}

void IN_MLookDown (void)
{
	mlooking = true; 
}

void IN_MLookUp (void) 
{
	mlooking = false;
	if (!freelook->value && lookspring->value)
			IN_CenterView ();
}

qboolean IN_GetMouseCursorPos(incursorcoords_t* p)
{
	qboolean valid;
	mouse_event m;

	valid = false;
	if(MOUSE_GetEvent(&m))
	{
		p->x = window_center_x + m.rx;
		p->y = window_center_y + m.ry;
		if((p->x < 0)||(p->x > sys_rmode->viWidth)||(p->y < 0)||(p->y > sys_rmode->viHeight))
		{
			p->x = window_center_x;
			p->y = window_center_y;
		}
		if((in_previous_mouse_buttons & 0x01) != (m.button & 0x01))
		{
			Key_Event(K_MOUSE1, ((m.button & 0x01) == 0x01), Sys_Milliseconds());
		};
		if((in_previous_mouse_buttons & 0x02) != (m.button & 0x02))
		{
			Key_Event(K_MOUSE2, ((m.button & 0x02) == 0x02), Sys_Milliseconds());
		};
		if((in_previous_mouse_buttons & 0x04) != (m.button & 0x04))
		{
			Key_Event(K_MOUSE3, ((m.button & 0x04) == 0x04), Sys_Milliseconds());
		};
		in_previous_mouse_buttons = m.button;
		valid = true;
	};
	return valid;
}

void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;

	if (!mouseactive)
		return;

	// find mouse movement
	if (!IN_GetMouseCursorPos (&current_pos))
		return;

	mx = current_pos.x - window_center_x;
	my = current_pos.y - window_center_y;

#if 0
	if (!mx && !my)
		return;
#endif

	if (m_filter->value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	mouse_x *= sensitivity->value;
	mouse_y *= sensitivity->value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe->value && mlooking ))
		cmd->sidemove += m_side->value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw->value * mouse_x;

	if ( (mlooking || freelook->value) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch->value * mouse_y;
	}
	else
	{
		cmd->forwardmove -= m_forward->value * mouse_y;
	}
}

qboolean IN_GetWmoteCursorPos(incursorcoords_t* p)
{
	ir_t w;
	qboolean valid;

	valid = false;
	WPAD_ScanPads();
	WPAD_IR(WPAD_CHAN_0, &w);
	if(w.valid)
	{
		p->x = w.x - wmote_adjust_x;
		p->y = w.y - wmote_adjust_y;
		valid = true;
	};
	return valid;
}

void IN_SetWmoteCursorPos(int x, int y)
{
	ir_t w;

	WPAD_ScanPads();
	WPAD_IR(WPAD_CHAN_0, &w);
	if(w.valid)
	{
		wmote_adjust_x = w.x - x;
		wmote_adjust_y = w.y - y;
	};
}

void IN_WmoteMove (usercmd_t *cmd)
{
	int		wx, wy;

	if (!wmoteactive)
		return;

	// find Wii Remote movement
	if (!IN_GetWmoteCursorPos (&current_pos))
		return;

	wx = current_pos.x - window_center_x;
	wy = current_pos.y - window_center_y;

#if 0
	if (!wx && !wy)
		return;
#endif

	if (m_filter->value)
	{
		wmote_x = (wx + old_wmote_x) * 0.5;
		wmote_y = (wy + old_wmote_y) * 0.5;
	}
	else
	{
		wmote_x = wx;
		wmote_y = wy;
	}

	old_wmote_x = wx;
	old_wmote_y = wy;

	wmote_x *= wmotespeed->value * 3;
	wmote_y *= wmotespeed->value * 3;
	
// add Wii Remote X/Y movement to cmd
	cl.viewangles[YAW] -= m_yaw->value * wmote_x;

	cl.viewangles[PITCH] += m_pitch->value * wmote_y;

	// force the Wii Remote to the center, so there's room to move
	// (by offsetting from the current location, in order to keep
	//  using the same code that uses the mouse)
	if (wx || wy)
		IN_SetWmoteCursorPos (window_center_x, window_center_y);

}

void IN_NunchukMove (usercmd_t *cmd)
{
	expansion_t e;

	WPAD_ScanPads();
	WPAD_Expansion(WPAD_CHAN_0, &e);
	
	cmd->sidemove += m_side->value * (e.nunchuk.js.pos.x - e.nunchuk.js.center.x) * wmotespeed->value;
	cmd->forwardmove -= m_forward->value * (e.nunchuk.js.center.y - e.nunchuk.js.pos.y) * wmotespeed->value;
}

qboolean IN_GetGCPadCursorPos(incursorcoords_t* p)
{
	qboolean valid;

	valid = false;
	PAD_ScanPads();
	p->x = window_center_x + PAD_SubStickX(0);
	p->y = window_center_y + PAD_SubStickY(0);
	if((p->x != window_center_x)||(p->y != window_center_y))
		valid = true;
	return valid;
}

void IN_GCPadMainStickMove (usercmd_t *cmd)
{
	PAD_ScanPads();
	cmd->sidemove += m_side->value * PAD_StickX(0) * gcpadspeed->value;
	cmd->forwardmove += m_forward->value * PAD_StickY(0) * gcpadspeed->value;
}

void IN_GCPadMove (usercmd_t *cmd)
{
	int		gx, gy;

	if (!gcpadactive)
		return;

	// find C-stick movement
	if (!IN_GetGCPadCursorPos (&current_pos))
		return;

	gx = current_pos.x - window_center_x;
	gy = current_pos.y - window_center_y;

#if 0
	if (!gx && !gy)
		return;
#endif

	if (m_filter->value)
	{
		gcpad_x = (gx + old_gcpad_x) * 0.5;
		gcpad_y = (gy + old_gcpad_y) * 0.5;
	}
	else
	{
		gcpad_x = gx;
		gcpad_y = gy;
	}

	old_gcpad_x = gx;
	old_gcpad_y = gy;

	gcpad_x *= gcpadspeed->value;
	gcpad_y *= gcpadspeed->value;
	
// add C-stick X/Y movement to cmd
	cl.viewangles[YAW] -= m_yaw->value * gcpad_x;

	cl.viewangles[PITCH] += m_pitch->value * gcpad_y;
}

void IN_Init (void)
{
	// mouse variables
	m_filter				= Cvar_Get ("m_filter",					"0",		0);
    in_mouse				= Cvar_Get ("in_mouse",					"1",		CVAR_ARCHIVE);

	// Wii Remote variables
	in_wmote				= Cvar_Get ("in_wmote",					"1",		CVAR_ARCHIVE);

	// Gamecube controller variables
	in_gcpad				= Cvar_Get ("in_gcpad",					"1",		CVAR_ARCHIVE);

	// joystick variables
	in_joystick				= Cvar_Get ("in_joystick",				"0",		CVAR_ARCHIVE);

	// on-screen keyboard variables
	in_osk					= Cvar_Get ("in_osk",					"0",		CVAR_ARCHIVE);

	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);

	IN_StartupMouse ();
	IN_StartupWmote ();
	IN_StartupGCPad ();

	IN_Activate(true);
}

void IN_Commands (void)
{
}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove (cmd);
	IN_WmoteMove (cmd);
	IN_NunchukMove (cmd);
	IN_GCPadMove (cmd);
	IN_GCPadMainStickMove (cmd);
}

void IN_Activate (qboolean active)
{
	in_appactive = active;
	mouseactive = !active;		// force a new window check or turn off
	wmoteactive = !active;
	gcpadactive = !active;
}

void IN_ActivateMouse (void)
{
	int		width, height;

	if (!mouseinitialized)
		return;
	if (!in_mouse->value)
	{
		mouseactive = false;
		return;
	}
	if (mouseactive)
		return;

	mouseactive = true;

	width = sys_rmode->viWidth;
	height = sys_rmode->viHeight;

	window_center_x = width/2;
	window_center_y = height/2;
}

void IN_ActivateWmote (void)
{
	int		width, height;

	if (!wmoteinitialized)
		return;
	if (!in_wmote->value)
	{
		wmoteactive = false;
		return;
	}
	if (wmoteactive)
		return;

	wmoteactive = true;

	width = sys_rmode->viWidth;
	height = sys_rmode->viHeight;

	window_center_x = width/2;
	window_center_y = height/2;

	IN_SetWmoteCursorPos (window_center_x, window_center_y);
}

void IN_ActivateGCPad (void)
{
	int		width, height;

	if (!gcpadinitialized)
		return;
	if (!in_gcpad->value)
	{
		gcpadactive = false;
		return;
	}
	if (gcpadactive)
		return;

	gcpadactive = true;

	width = sys_rmode->viWidth;
	height = sys_rmode->viHeight;

	window_center_x = width/2;
	window_center_y = height/2;
}

void IN_DeactivateMouse (void)
{
	if (!mouseinitialized)
		return;
	if (!mouseactive)
		return;

	mouseactive = false;
}

void IN_DeactivateWmote (void)
{
	if (!wmoteinitialized)
		return;
	if (!wmoteactive)
		return;

	wmoteactive = false;
}

void IN_DeactivateGCPad (void)
{
	if (!gcpadinitialized)
		return;
	if (!gcpadactive)
		return;

	gcpadactive = false;
}

void IN_MouseFrame(void)
{
	if (!mouseinitialized)
		return;

	if (!in_mouse || !in_appactive)
	{
		IN_DeactivateMouse ();
		return;
	}

	if ( !cl.refresh_prepped
		|| cls.key_dest == key_console
		|| cls.key_dest == key_menu)
	{
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableValue ("vid_fullscreen") == 0)
		{
			IN_DeactivateMouse ();
			return;
		}
	}

	if(!MOUSE_IsConnected())
	{
		IN_DeactivateMouse ();
		return;
	};

	IN_ActivateMouse ();
}

void IN_WmoteFrame(void)
{
	u32 k;

	if (!wmoteinitialized)
		return;

	if (!in_wmote || !in_appactive)
	{
		IN_DeactivateWmote ();
		return;
	}

	if ( !cl.refresh_prepped
		|| cls.key_dest == key_console
		|| cls.key_dest == key_menu)
	{
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableValue ("vid_fullscreen") == 0)
		{
			IN_DeactivateWmote ();
			return;
		}
	}

	WPAD_ScanPads();
	k = WPAD_ButtonsHeld(WPAD_CHAN_0);
	if((k & WPAD_BUTTON_A) != WPAD_BUTTON_A)
	{
		IN_DeactivateWmote ();
		return;
	};

	IN_ActivateWmote ();
}

void IN_GCPadFrame(void)
{
	if (!gcpadinitialized)
		return;

	if (!in_gcpad || !in_appactive)
	{
		IN_DeactivateGCPad ();
		return;
	}

	if ( !cl.refresh_prepped
		|| cls.key_dest == key_console
		|| cls.key_dest == key_menu)
	{
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableValue ("vid_fullscreen") == 0)
		{
			IN_DeactivateGCPad ();
			return;
		}
	}

	IN_ActivateGCPad ();
}

void IN_Frame (void)
{
	IN_MouseFrame();
	IN_WmoteFrame();
	IN_GCPadFrame();
}

void IN_Shutdown (void)
{
	IN_DeactivateGCPad ();
	IN_DeactivateWmote ();
	IN_DeactivateMouse ();
}

