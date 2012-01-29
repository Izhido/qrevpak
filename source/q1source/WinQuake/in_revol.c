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
// in_revol.c -- input handler for the Nintendo Wii using devkitPPC / libogc
// (based on in_null.c)

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/usbmouse.h>

#include "quakedef.h"

extern GXRModeObj* sys_rmode;

extern s32 sys_mouse_valid;

extern mouse_event sys_mouse_event;

typedef struct
{
	int x; 
	int y;
} incursorcoords_t;

cvar_t m_filter = {"m_filter","0"};

cvar_t in_osk = {"in_osk","0"};

cvar_t in_wlook = {"in_wlook","0"};

cvar_t in_wmotemovscale = {"in_wmotemovscale","3"};

cvar_t in_wmotevangscale = {"in_wmotevangscale","3"};

cvar_t in_wmotemovmin = {"in_wmotemovmin","4"};

cvar_t in_gcpadmovscale = {"in_gcpadmovscale","3"};

cvar_t in_gcpadmovmin = {"in_gcpadmovmin","0"};

cvar_t in_clsctmovscale = {"in_clsctmovscale","9"};

cvar_t in_clsctvangscale = {"in_clsctvangscale","9"};

cvar_t in_clsctmovmin = {"in_clsctmovmin","0"};

qboolean	mouseinitialized;

qboolean	mouseactive;

qboolean	wmoteinitialized;

qboolean	wmoteactive;

qboolean	gcpadinitialized;

qboolean	gcpadactive;

qboolean	clsctinitialized;

qboolean	clsctactive;

incursorcoords_t current_pos;

int			window_center_x, window_center_y;

int			wmote_adjust_x, wmote_adjust_y;

int			mouse_x, mouse_y, old_mouse_x, old_mouse_y;

int			wmote_x, wmote_y, old_wmote_x, old_wmote_y;

int			wmote_validcount, wmote_curr_x, wmote_curr_y, wmote_prev_x, wmote_prev_y;

int			gcpad_x, gcpad_y, old_gcpad_x, old_gcpad_y;

int			clsct_x, clsct_y, old_clsct_x, old_clsct_y;

qboolean IN_GetMouseCursorPos(incursorcoords_t* p)
{
	if(!sys_mouse_valid)
		return false;

	p->x = window_center_x + sys_mouse_event.rx;
	p->y = window_center_y + sys_mouse_event.ry;
	if((p->x < 0)||(p->x > sys_rmode->viWidth)||(p->y < 0)||(p->y > sys_rmode->viHeight))
	{
		p->x = window_center_x;
		p->y = window_center_y;
	}

	return true;
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

void IN_SetWmoteCursorPos(int x, int y)
{
	wmote_adjust_x = wmote_curr_x - x;
	wmote_adjust_y = wmote_curr_y - y;
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
	if((((in_wlook.value != 0)&&(wmotelookbinv.value == 0))
	  ||((in_wlook.value == 0)&&(wmotelookbinv.value != 0)))
	  &&(in_osk.value == 0))
	{
		p->x = wmote_curr_x - wmote_adjust_x;
		p->y = wmote_curr_y - wmote_adjust_y;
		return true;
	};
	return false;
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

void IN_ActivateClsCt (void)
{
	int		width, height;

	if (!clsctinitialized)
		return;
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

void IN_StartupClsCt (void)
{
	clsctinitialized = true;

	IN_ActivateClsCt();
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

	wmote_x *= wmotespeed.value * in_wmotevangscale.value;
	wmote_y *= wmotespeed.value * in_wmotevangscale.value;
	
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
	int nx;
	int ny;

	WPAD_Expansion(WPAD_CHAN_0, &e);
	if(e.type != WPAD_EXP_NUNCHUK)
		return;

	nx = e.nunchuk.js.pos.x - e.nunchuk.js.center.x;
	ny = e.nunchuk.js.pos.y - e.nunchuk.js.center.y;
	if((nx > -in_wmotemovmin.value)&&(nx < in_wmotemovmin.value))
		nx = 0;
	if((ny > -in_wmotemovmin.value)&&(ny < in_wmotemovmin.value))
		ny = 0;
	if((nx == 0)&&(ny == 0))
		return;

	cmd->sidemove += m_side.value * nx * in_wmotemovscale.value;
	cmd->forwardmove += m_forward.value * ny * in_wmotemovscale.value;
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
	int gx;
	int gy;

	PAD_ScanPads();

	gx = PAD_StickX(0);
	gy = PAD_StickY(0);
	if((gx > -in_gcpadmovmin.value)&&(gx < in_gcpadmovmin.value))
		gx = 0;
	if((gy > -in_gcpadmovmin.value)&&(gy < in_gcpadmovmin.value))
		gy = 0;
	if((gx == 0)&&(gy == 0))
		return;

	cmd->sidemove += m_side.value * gx * in_gcpadmovscale.value;
	cmd->forwardmove += m_forward.value * gy * in_gcpadmovscale.value;
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

	if (m_filter.value)
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

	clsct_x *= clsctspeed.value * in_clsctvangscale.value;
	clsct_y *= clsctspeed.value * in_clsctvangscale.value;
	
// add Wii Remote X/Y movement to cmd
	cl.viewangles[YAW] -= m_yaw.value * clsct_x;

	V_StopPitchDrift ();
		
	cl.viewangles[PITCH] += m_yaw.value * clsct_y;
	if (cl.viewangles[PITCH] > 80)
		cl.viewangles[PITCH] = 80;
	if (cl.viewangles[PITCH] < -70)
		cl.viewangles[PITCH] = -70;
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
	if((cx > -in_clsctmovmin.value)&&(cx < in_clsctmovmin.value))
		cx = 0;
	if((cy > -in_clsctmovmin.value)&&(cy < in_clsctmovmin.value))
		cy = 0;
	if((cx == 0)&&(cy == 0))
		return;

	cmd->sidemove += m_side.value * cx * in_clsctmovscale.value;
	cmd->forwardmove += m_forward.value * cy * in_clsctmovscale.value;
}

void IN_Init (void)
{
	// mouse variables
	Cvar_RegisterVariable (&m_filter);

	// on-screen keyboard variables
	Cvar_RegisterVariable(&in_osk);

	// Wii Remote look mode variables
	Cvar_RegisterVariable(&in_wlook);

	// Wii Remote variables
	Cvar_RegisterVariable(&in_wmotemovscale);
	Cvar_RegisterVariable(&in_wmotevangscale);
	Cvar_RegisterVariable(&in_wmotemovmin);

	// Gamecube controller variables
	Cvar_RegisterVariable(&in_gcpadmovscale);
	Cvar_RegisterVariable(&in_gcpadmovmin);

	// Classic controller variables
	Cvar_RegisterVariable(&in_clsctmovscale);
	Cvar_RegisterVariable(&in_clsctvangscale);
	Cvar_RegisterVariable(&in_clsctmovmin);

	IN_StartupMouse ();
	IN_StartupWmote ();
	IN_StartupGCPad ();
	IN_StartupClsCt ();
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

void IN_Shutdown (void)
{
	IN_DeactivateClsCt ();
	IN_DeactivateGCPad ();
	IN_DeactivateWmote ();
	IN_DeactivateMouse ();
}

