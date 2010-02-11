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

extern s32 sys_mouse_valid;

extern mouse_event sys_mouse_event;

typedef struct
{
	int x; 
	int y;
} incursorcoords_t;

// mouse variables
cvar_t	*m_filter;

cvar_t	*in_mouse;

// on-screen keyboard variables
cvar_t	*in_osk;

// Wii Remote look mode variables
cvar_t	*in_wlook;

// Wii Remote variables
cvar_t	*in_wmote;

cvar_t	*in_wmotemovscale;

cvar_t	*in_wmotevangscale;

cvar_t	*in_wmotemovmin;

// Gamecube controller variables
cvar_t	*in_gcpad;

cvar_t	*in_gcpadmovscale;

cvar_t	*in_gcpadmovmin;

// Classic controller variables
cvar_t	*in_clsct;

cvar_t	*in_clsctmovscale;

cvar_t	*in_clsctvangscale;

cvar_t	*in_clsctmovmin;

// Joystick variables
cvar_t	*in_joystick;

qboolean	mouseactive;	// false when not focus app

qboolean	mouseinitialized;

qboolean	mlooking;

qboolean	wmoteactive;	// false when not focus app

qboolean	wmoteinitialized;

qboolean	gcpadactive;	// false when not focus app

qboolean	gcpadinitialized;

qboolean	clsctactive;	// false when not focus app

qboolean	clsctinitialized;

incursorcoords_t current_pos;

int			mouse_x, mouse_y, old_mouse_x, old_mouse_y;

int			window_center_x, window_center_y;

qboolean	in_appactive;

int			wmote_x, wmote_y, old_wmote_x, old_wmote_y;

int			wmote_adjust_x, wmote_adjust_y;

int			wmote_validcount, wmote_curr_x, wmote_curr_y, wmote_prev_x, wmote_prev_y;

u8			in_previous_mouse_buttons;

int			gcpad_x, gcpad_y, old_gcpad_x, old_gcpad_y;

int			clsct_x, clsct_y, old_clsct_x, old_clsct_y;

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

void IN_StartupClsCt (void)
{
	cvar_t		*cv;

	cv = Cvar_Get ("in_initclsct", "1", CVAR_NOSET);
	if ( !cv->value ) 
		return; 

	clsctinitialized = true;
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
	p->x = window_center_x + sys_mouse_event.rx;
	p->y = window_center_y + sys_mouse_event.ry;
	if((p->x < 0)||(p->x > sys_rmode->viWidth)||(p->y < 0)||(p->y > sys_rmode->viHeight))
	{
		p->x = window_center_x;
		p->y = window_center_y;
	}
	return true;
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

void IN_CalcWmoteOutMove(ir_t w)
{
	int dif_l;
	int dif_r;
	int dif_t;
	int dif_b;

	wmote_prev_x = wmote_curr_x;
	wmote_prev_y = wmote_curr_y;
	if(w.smooth_valid)
	{
		wmote_curr_x = w.sx;
		wmote_curr_y = w.sy;
	} else
	{
		dif_l = wmote_curr_x;
		dif_r = sys_rmode->viWidth - 1 - wmote_curr_x;
		dif_t = wmote_curr_y;
		dif_b = sys_rmode->viHeight - 1 - wmote_curr_y;
		if(dif_l < dif_r)
		{
			if(dif_t < dif_b)
			{
				if(dif_l < dif_t)
				{
					if(dif_l < 20)
					{
						wmote_curr_x = dif_l - 20;
					} else
					{
						wmote_curr_x = 0;
					};
				} else
				{
					if(dif_t < 15)
					{
						wmote_curr_y = dif_t - 15;
					} else
					{
						wmote_curr_y = 0;
					};
				};
			} else
			{
				if(dif_l < dif_b)
				{
					if(dif_l < 20)
					{
						wmote_curr_x = dif_l - 20;
					} else
					{
						wmote_curr_x = 0;
					};
				} else
				{
					if(dif_b < 15)
					{
						wmote_curr_y = sys_rmode->viHeight + 14 - dif_b;
					} else
					{
						wmote_curr_y = sys_rmode->viHeight - 1;
					};
				};
			};
		} else
		{
			if(dif_t < dif_b)
			{
				if(dif_r < dif_t)
				{
					if(dif_r < 20)
					{
						wmote_curr_x = sys_rmode->viWidth + 19 - dif_r;
					} else
					{
						wmote_curr_x = sys_rmode->viWidth - 1;
					};
				} else
				{
					if(dif_t < 15)
					{
						wmote_curr_y = dif_t - 15;
					} else
					{
						wmote_curr_y = 0;
					};
				};
			} else
			{
				if(dif_r < dif_b)
				{
					if(dif_r < 20)
					{
						wmote_curr_x = sys_rmode->viWidth + 19 - dif_r;
					} else
					{
						wmote_curr_x = sys_rmode->viWidth - 1;
					};
				} else
				{
					if(dif_b < 15)
					{
						wmote_curr_y = sys_rmode->viHeight + 14 - dif_b;
					} else
					{
						wmote_curr_y = sys_rmode->viHeight - 1;
					};
				};
			};
		};
	};
}

qboolean IN_GetWmoteCursorPos(incursorcoords_t* p)
{
	ir_t w;
	int wx;
	int wy;

	WPAD_IR(WPAD_CHAN_0, &w);
	if(w.valid)
	{
		wmote_prev_x = wmote_curr_x;
		wmote_prev_y = wmote_curr_y;
		wmote_curr_x = w.x;
		wmote_curr_y = w.y;
		if(wmote_validcount == 0)
		{
			IN_SetWmoteCursorPos (window_center_x, window_center_y);
		};
		wmote_validcount++;
	} else
	{
		if(wmote_validcount == 0)
		{
			wx = wmote_curr_x + wmote_curr_x - wmote_prev_x;
			wy = wmote_curr_y + wmote_curr_y - wmote_prev_y;
			wmote_prev_x = wmote_curr_x;
			wmote_prev_y = wmote_curr_y;
			wmote_curr_x = wx;
			wmote_curr_y = wy;
		} else if(wmote_validcount == 1)
		{
			IN_CalcWmoteOutMove(w);
		} else
		{
			wx = wmote_curr_x + wmote_curr_x - wmote_prev_x;
			wy = wmote_curr_y + wmote_curr_y - wmote_prev_y;
			if((wx > 0)&&(wy > 0)&&(wx < sys_rmode->viWidth)&&(wy < sys_rmode->viHeight))
			{
				IN_CalcWmoteOutMove(w);
			} else
			{
				wmote_prev_x = wmote_curr_x;
				wmote_prev_y = wmote_curr_y;
				wmote_curr_x = wx;
				wmote_curr_y = wy;
			};
		};
		wmote_validcount = 0;
	};
	if((((in_wlook->value != 0)&&(wmotelookbinv->value == 0))
	  ||((in_wlook->value == 0)&&(wmotelookbinv->value != 0)))
	  &&(in_osk->value == 0))
	{
		p->x = wmote_curr_x - wmote_adjust_x;
		p->y = wmote_curr_y - wmote_adjust_y;
		return true;
	};
	return false;
}

void IN_SetWmoteCursorPos(int x, int y)
{
	wmote_adjust_x = wmote_curr_x - x;
	wmote_adjust_y = wmote_curr_y - y;
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

	wmote_x *= wmotespeed->value * in_wmotevangscale->value;
	wmote_y *= wmotespeed->value * in_wmotevangscale->value;
	
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
	int nx;
	int ny;

	WPAD_Expansion(WPAD_CHAN_0, &e);
	if(e.type != WPAD_EXP_NUNCHUK)
		return;

	nx = e.nunchuk.js.pos.x - e.nunchuk.js.center.x;
	ny = e.nunchuk.js.pos.y - e.nunchuk.js.center.y;
	if((nx > -in_wmotemovmin->value)&&(nx < in_wmotemovmin->value))
		nx = 0;
	if((ny > -in_wmotemovmin->value)&&(ny < in_wmotemovmin->value))
		ny = 0;
	if((nx == 0)&&(ny == 0))
		return;

	cmd->sidemove += m_side->value * nx * in_wmotemovscale->value;
	cmd->forwardmove += m_forward->value * ny * in_wmotemovscale->value;
}

qboolean IN_GetGCPadCursorPos(incursorcoords_t* p)
{
	PAD_ScanPads();
	p->x = window_center_x + PAD_SubStickX(0);
	p->y = window_center_y + PAD_SubStickY(0);
	if((p->x != window_center_x)||(p->y != window_center_y))
		return true;

	return false;
}

void IN_GCPadMainStickMove (usercmd_t *cmd)
{
	int gx;
	int gy;

	PAD_ScanPads();

	gx = PAD_StickX(0);
	gy = PAD_StickY(0);
	if((gx > -in_gcpadmovmin->value)&&(gx < in_gcpadmovmin->value))
		gx = 0;
	if((gy > -in_gcpadmovmin->value)&&(gy < in_gcpadmovmin->value))
		gy = 0;
	if((gx == 0)&&(gy == 0))
		return;

	cmd->sidemove += m_side->value * gx * in_gcpadmovscale->value;
	cmd->forwardmove += m_forward->value * gy * in_gcpadmovscale->value;
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

qboolean IN_GetClsCtCursorPos(incursorcoords_t* p)
{
	expansion_t e;

	WPAD_Expansion(WPAD_CHAN_0, &e);
	if(e.type != WPAD_EXP_CLASSIC)
		return false;		

	p->x = window_center_x + (e.classic.rjs.pos.x - e.classic.rjs.center.x);
	p->y = window_center_y + (e.classic.rjs.pos.y - e.classic.rjs.center.y);
	if((p->x == window_center_x)&&(p->y == window_center_y))
		return false;

	return true;
}

void IN_ClsCtLeftStickMove (usercmd_t *cmd)
{
	expansion_t e;
	int cx;
	int cy;

	WPAD_Expansion(WPAD_CHAN_0, &e);
	if(e.type != WPAD_EXP_CLASSIC)
		return;

	cx = e.classic.ljs.pos.x - e.classic.ljs.center.x;
	cy = e.classic.ljs.pos.y - e.classic.ljs.center.y;
	if((cx > -in_clsctmovmin->value)&&(cx < in_clsctmovmin->value))
		cx = 0;
	if((cy > -in_clsctmovmin->value)&&(cy < in_clsctmovmin->value))
		cy = 0;
	if((cx == 0)&&(cy == 0))
		return;

	cmd->sidemove += m_side->value * cx * in_clsctmovscale->value;
	cmd->forwardmove += m_forward->value * cy * in_clsctmovscale->value;
}

void IN_ClsCtMove (usercmd_t *cmd)
{
	int		cx, cy;

	if (!clsctactive)
		return;

	// find Right Stick movement
	if (!IN_GetClsCtCursorPos (&current_pos))
		return;

	cx = current_pos.x - window_center_x;
	cy = current_pos.y - window_center_y;

#if 0
	if (!cx && !cy)
		return;
#endif

	if (m_filter->value)
	{
		clsct_x = (cx + old_clsct_x) * 0.5;
		clsct_y = (cy + old_clsct_y) * 0.5;
	}
	else
	{
		clsct_x = cx;
		clsct_y = cy;
	}

	old_clsct_x = cx;
	old_clsct_y = cy;

	clsct_x *= clsctspeed->value * in_clsctvangscale->value;
	clsct_y *= clsctspeed->value * in_clsctvangscale->value;
	
// add Right Stick X/Y movement to cmd
	cl.viewangles[YAW] -= m_yaw->value * clsct_x;

	cl.viewangles[PITCH] += m_pitch->value * clsct_y;
}

void IN_Init (void)
{
	// mouse variables
	m_filter				= Cvar_Get ("m_filter",					"0",		0);
    in_mouse				= Cvar_Get ("in_mouse",					"1",		CVAR_ARCHIVE);

	// on-screen keyboard variables
	in_osk					= Cvar_Get ("in_osk",					"0",		CVAR_ARCHIVE);

	// Wii Remote look mode variables
	in_wlook				= Cvar_Get ("in_wlook",					"0",		CVAR_ARCHIVE);

	// Wii Remote variables
	in_wmote				= Cvar_Get ("in_wmote",					"1",		CVAR_ARCHIVE);
	in_wmotemovscale		= Cvar_Get ("in_wmotemovscale",			"3",		CVAR_ARCHIVE);
	in_wmotevangscale		= Cvar_Get ("in_wmotevangscale",		"3",		CVAR_ARCHIVE);
	in_wmotemovmin			= Cvar_Get ("in_wmotemovmin",			"4",		CVAR_ARCHIVE);

	// Gamecube controller variables
	in_gcpad				= Cvar_Get ("in_gcpad",					"1",		CVAR_ARCHIVE);
	in_gcpadmovscale		= Cvar_Get ("in_gcpadmovscale",			"3",		CVAR_ARCHIVE);
	in_gcpadmovmin			= Cvar_Get ("in_gcpadmovmin",			"0",		CVAR_ARCHIVE);

	// Classic controller variables
	in_clsct				= Cvar_Get ("in_clsct",					"1",		CVAR_ARCHIVE);
	in_clsctmovscale		= Cvar_Get ("in_clsctmovscale",			"9",		CVAR_ARCHIVE);
	in_clsctvangscale		= Cvar_Get ("in_clsctvangscale",		"9",		CVAR_ARCHIVE);
	in_clsctmovmin			= Cvar_Get ("in_clsctmovmin",			"0",		CVAR_ARCHIVE);

	// joystick variables
	in_joystick				= Cvar_Get ("in_joystick",				"0",		CVAR_ARCHIVE);

	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);

	IN_StartupMouse ();
	IN_StartupWmote ();
	IN_StartupGCPad ();
	IN_StartupClsCt ();

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
	IN_ClsCtMove (cmd);
	IN_ClsCtLeftStickMove (cmd);
}

void IN_Activate (qboolean active)
{
	in_appactive = active;
	mouseactive = !active;		// force a new window check or turn off
	wmoteactive = !active;
	gcpadactive = !active;
	clsctactive = !active;
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

void IN_ActivateClsCt (void)
{
	int		width, height;

	if (!clsctinitialized)
		return;
	if (!in_clsct->value)
	{
		clsctactive = false;
		return;
	}
	if (clsctactive)
		return;

	clsctactive = true;

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

void IN_DeactivateClsCt (void)
{
	if (!clsctinitialized)
		return;
	if (!clsctactive)
		return;

	clsctactive = false;
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

	if(!sys_mouse_valid)
	{
		IN_DeactivateMouse ();
		return;
	};

	IN_ActivateMouse ();
}

void IN_WmoteFrame(void)
{
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

void IN_ClsCtFrame(void)
{
	if (!clsctinitialized)
		return;

	if (!in_clsct || !in_appactive)
	{
		IN_DeactivateClsCt ();
		return;
	}

	if ( !cl.refresh_prepped
		|| cls.key_dest == key_console
		|| cls.key_dest == key_menu)
	{
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableValue ("vid_fullscreen") == 0)
		{
			IN_DeactivateClsCt ();
			return;
		}
	}

	IN_ActivateClsCt ();
}

void IN_Frame (void)
{
	IN_MouseFrame();
	IN_WmoteFrame();
	IN_GCPadFrame();
	IN_ClsCtFrame();
}

void IN_Shutdown (void)
{
	IN_DeactivateClsCt ();
	IN_DeactivateGCPad ();
	IN_DeactivateWmote ();
	IN_DeactivateMouse ();
}

