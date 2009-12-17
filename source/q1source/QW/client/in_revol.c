// in_revol.c -- input handler for the Nintendo Wii using devkitPPC / libogc
// (based on in_null.c)

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/usbmouse.h>

#include "quakedef.h"

extern GXRModeObj* sys_rmode;

typedef struct
{
	int x; 
	int y;
} incursorcoords_t;

cvar_t m_filter = {"m_filter","0"};

cvar_t in_osk = {"in_osk","0"};

qboolean	mouseinitialized;

qboolean	mouseactive;

qboolean	wmoteinitialized;

qboolean	wmoteactive;

qboolean	gcpadinitialized;

qboolean	gcpadactive;

incursorcoords_t current_pos;

int			window_center_x, window_center_y;

int			wmote_adjust_x, wmote_adjust_y;

int			mouse_x, mouse_y, old_mouse_x, old_mouse_y;

int			wmote_x, wmote_y, old_wmote_x, old_wmote_y;

int			gcpad_x, gcpad_y, old_gcpad_x, old_gcpad_y;

u8			in_previous_mouse_buttons;

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
			Key_Event(K_MOUSE1, ((m.button & 0x01) == 0x01));
		};
		if((in_previous_mouse_buttons & 0x02) != (m.button & 0x02))
		{
			Key_Event(K_MOUSE2, ((m.button & 0x02) == 0x02));
		};
		if((in_previous_mouse_buttons & 0x04) != (m.button & 0x04))
		{
			Key_Event(K_MOUSE3, ((m.button & 0x04) == 0x04));
		};
		in_previous_mouse_buttons = m.button;
		valid = true;
	};
	return valid;
}

qboolean IN_GetWmoteCursorPos(incursorcoords_t* p)
{
	u32 k;
	ir_t w;
	qboolean valid;

	valid = false;
	WPAD_ScanPads();
	k = WPAD_ButtonsHeld(WPAD_CHAN_0);
	if((k & WPAD_BUTTON_A) == WPAD_BUTTON_A)
	{
		WPAD_IR(WPAD_CHAN_0, &w);
		if(w.valid)
		{
			p->x = w.x - wmote_adjust_x;
			p->y = w.y - wmote_adjust_y;
			valid = true;
		};
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

void IN_ActivateMouse (void)
{
	int		width, height;

	if (!mouseinitialized)
		return;
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

void IN_StartupMouse (void)
{
	if (MOUSE_Init() != 0)
	{
		Sys_Printf("Mouse not found\n");
		return;
	};

	mouseinitialized = true;

	IN_ActivateMouse();
}

void IN_StartupWmote (void)
{
	wmote_adjust_x = 0;
	wmote_adjust_y = 0;

	wmoteinitialized = true;

	IN_ActivateWmote();
}

void IN_StartupGCPad (void)
{
	gcpadinitialized = true;

	IN_ActivateGCPad();
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

	if (m_filter.value)
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

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;

	if (in_mlook.state & 1)
		V_StopPitchDrift ();
		
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

void IN_WmoteMove (usercmd_t *cmd)
{
	int		wx, wy;

	if (!wmoteactive)
		return;

	// find Wii Remote movement (if not available, reset to center before exiting)
	if (!IN_GetWmoteCursorPos (&current_pos))
	{
		IN_SetWmoteCursorPos (window_center_x, window_center_y);
		return;
	};

	wx = current_pos.x - window_center_x;
	wy = current_pos.y - window_center_y;

	if (m_filter.value)
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

	wmote_x *= wmotespeed.value * 3;
	wmote_y *= wmotespeed.value * 3;
	
// add Wii Remote X/Y movement to cmd
	cl.viewangles[YAW] -= m_yaw.value * wmote_x;

	V_StopPitchDrift ();
		
	cl.viewangles[PITCH] += m_yaw.value * wmote_y;
	if (cl.viewangles[PITCH] > 80)
		cl.viewangles[PITCH] = 80;
	if (cl.viewangles[PITCH] < -70)
		cl.viewangles[PITCH] = -70;

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
	
	cmd->sidemove += m_side.value * (e.nunchuk.js.pos.x - e.nunchuk.js.center.x) * wmotespeed.value;
	cmd->forwardmove -= m_forward.value * (e.nunchuk.js.center.y - e.nunchuk.js.pos.y) * wmotespeed.value;
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

	if (m_filter.value)
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

	gcpad_x *= gcpadspeed.value;
	gcpad_y *= gcpadspeed.value;
	
// add Wii Remote X/Y movement to cmd
	cl.viewangles[YAW] -= m_yaw.value * gcpad_x;

	V_StopPitchDrift ();
		
	cl.viewangles[PITCH] += m_yaw.value * gcpad_y;
	if (cl.viewangles[PITCH] > 80)
		cl.viewangles[PITCH] = 80;
	if (cl.viewangles[PITCH] < -70)
		cl.viewangles[PITCH] = -70;
}

void IN_GCPadMainStickMove (usercmd_t *cmd)
{
	PAD_ScanPads();
	cmd->sidemove += m_side.value * PAD_StickX(0) * gcpadspeed.value;
	cmd->forwardmove += m_forward.value * PAD_StickY(0) * gcpadspeed.value;
}

void IN_Init (void)
{
	// mouse variables
	Cvar_RegisterVariable (&m_filter);
	Cvar_RegisterVariable(&in_osk);

	IN_StartupMouse ();
	IN_StartupWmote ();
	IN_StartupGCPad ();
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

/*
===========
IN_ModeChanged
===========
*/
void IN_ModeChanged (void)
{
}

void IN_Shutdown (void)
{
	IN_DeactivateGCPad ();
	IN_DeactivateWmote ();
	IN_DeactivateMouse ();
}

