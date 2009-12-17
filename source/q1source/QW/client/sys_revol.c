// sys_revol.c -- system driver for the Nintendo Wii using devkitPPC / libogc
// (based on sys_null.c)

#include <gccore.h>
#include <fat.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#include <network.h>
#include <wiikeyboard/keyboard.h>
#include <ogc/usbmouse.h>
#include <sys/stat.h>

#include "quakedef.h"
#include "errno.h"

#include "osk_revol.h"
#include "Keys_dat.h"

qboolean isDedicated;

void* sys_framebuffer[2] = {NULL, NULL};

GXRModeObj* sys_rmode;

int sys_current_weapon;

extern qboolean	scr_drawdialog;

extern qboolean m_state_is_quit;

extern cvar_t in_osk;

u32 sys_previous_keys;

u32 sys_previous_pad_keys;

double sys_previous_time;

int sys_netinit_error;

char sys_ipaddress_text[16];

char sys_basedir[MAX_QPATH + 1];

float sys_previous_char_time;

qboolean sys_current_char_count;

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New data structures for big stack handling:
#define BIGSTACK_SIZE 2 * 1024 * 1024

byte sys_bigstack[BIGSTACK_SIZE];

int sys_bigstack_cursize;
// <<< FIX

void Sys_GetBaseDir(int argc, char** argv)
{
	int i;
	char* p;
	qboolean FirstFound;
	qboolean AllFound;

	if(argc == 0)
	{
		strcpy(sys_basedir, "/");
	} else 
	{
		p = argv[0];
		i = 0;
		FirstFound = false;
		AllFound = false;
		while(i < MAX_QPATH)
		{
			if(p[i] == 0)
			{
				break;
			} else if(p[i] == ':')
			{
				if(FirstFound)
				{
					break;
				} else
				{
					FirstFound = true;
				};
			} else if(p[i] == '/')
			{
				if(FirstFound)
				{
					AllFound = true;
				};
				break;
			};
			i++;
		};
		if(AllFound)
		{
			strncpy(sys_basedir, p, i + 1);
			sys_basedir[i + 1] = 0;
		} else
		{
			strcpy(sys_basedir, "/");
		};
	};
}

/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             10
FILE    *sys_handles[MAX_HANDLES];

int             findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

void Sys_FileSeekCur (int handle, int offset)
{
	fseek (sys_handles[handle], offset, SEEK_CUR);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (char *path)
{
	FILE    *f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
	mkdir(path, 0777);
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_DebugLog(char *file, char *fmt, ...)
{
    //va_list argptr; 
    //static char data[1024];
    //int fd;
    //
    //va_start(argptr, fmt);
    //vsprintf(data, fmt, argptr);
    //va_end(argptr);
    //fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
    //write(fd, data, strlen(data));
    //close(fd);
}

void Sys_Error (char *error, ...)
{
	va_list         argptr;

	printf ("Sys_Error: ");   
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	VIDEO_SetNextFramebuffer(sys_framebuffer[1]);
	VIDEO_WaitVSync();

    Host_Shutdown();

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{
    Host_Shutdown();

	exit (0);
}

double Sys_DoubleTime (void)
{
	static int		base;
	static qboolean	initialized = false;
	u64 t;
	int ms;

	t = gettime();
	ms = ticks_to_millisecs(t);
	if (!initialized)
	{
		base = ms;
		initialized = true;
	}
	return ((double)(ms - base)) / 1000.0;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
}

void Sys_KeyPress(char c)
{
	// Do nothing; keys are being polled straight from the keyboard
}

char Sys_FindKey(u16 symbol)
{
	if((symbol >= 32)&&(symbol <= 127))
	{
		return (char)symbol;
	} else if(symbol == KS_Tab)
	{
		return K_TAB;
	} else if((symbol == KS_Return)||(symbol == KS_KP_Enter))
	{
		return K_ENTER;
	} else if(symbol == KS_Escape)
	{
		return K_ESCAPE;
	} else if(symbol == KS_BackSpace)
	{
		return K_BACKSPACE;
	} else if((symbol == KS_Up)||(symbol == KS_KP_Up))
	{
		return K_UPARROW;
	} else if((symbol == KS_Down)||(symbol == KS_KP_Down))
	{
		return K_DOWNARROW;
	} else if((symbol == KS_Left)||(symbol == KS_KP_Left))
	{
		return K_LEFTARROW;
	} else if((symbol == KS_Right)||(symbol == KS_KP_Right))
	{
		return K_RIGHTARROW;
	} else if((symbol == KS_Alt_L)||(symbol == KS_Alt_R))
	{
		return K_ALT;
	} else if((symbol == KS_Control_L)||(symbol == KS_Control_R))
	{
		return K_CTRL;
	} else if((symbol == KS_Shift_L)||(symbol == KS_Shift_R))
	{
		return K_SHIFT;
	} else if((symbol == KS_f1)||(symbol == KS_F1))
	{
		return K_F1;
	} else if((symbol == KS_f2)||(symbol == KS_F2))
	{
		return K_F2;
	} else if((symbol == KS_f3)||(symbol == KS_F3))
	{
		return K_F3;
	} else if((symbol == KS_f4)||(symbol == KS_F4))
	{
		return K_F4;
	} else if((symbol == KS_f5)||(symbol == KS_F5))
	{
		return K_F5;
	} else if((symbol == KS_f6)||(symbol == KS_F6))
	{
		return K_F6;
	} else if((symbol == KS_f7)||(symbol == KS_F7))
	{
		return K_F7;
	} else if((symbol == KS_f8)||(symbol == KS_F8))
	{
		return K_F8;
	} else if((symbol == KS_f9)||(symbol == KS_F9))
	{
		return K_F9;
	} else if((symbol == KS_f10)||(symbol == KS_F10))
	{
		return K_F10;
	} else if((symbol == KS_f11)||(symbol == KS_F11))
	{
		return K_F11;
	} else if((symbol == KS_f12)||(symbol == KS_F12))
	{
		return K_F12;
	} else if((symbol == KS_Insert)||(symbol == KS_KP_Insert))
	{
		return K_INS;
	} else if((symbol == KS_Delete)||(symbol == KS_KP_Delete))
	{
		return K_DEL;
	} else if((symbol == KS_Next)||(symbol == KS_KP_Next))
	{
		return K_PGDN;
	} else if((symbol == KS_Prior)||(symbol == KS_KP_Prior))
	{
		return K_PGUP;
	} else if((symbol == KS_Home)||(symbol == KS_KP_Home))
	{
		return K_HOME;
	} else if((symbol == KS_End)||(symbol == KS_KP_End))
	{
		return K_END;
	} else if(symbol == KS_KP_Divide)
	{
		return '/';
	} else if(symbol == KS_KP_Subtract)
	{
		return '-';
	} else if(symbol == KS_KP_Add)
	{
		return '+';
	} else if((symbol >= KS_KP_0)&&(symbol <= KS_KP_9))
	{
		return '0' + (symbol - KS_KP_0);
	} else if(symbol == KS_Pause)
	{
		return K_PAUSE;
	} else
	{
		return 0;
	};
}

void Sys_SendKeyEvents (void)
{
	u32 k;
	u32 g;
	keyboard_event e;

	WPAD_ScanPads();
	k = WPAD_ButtonsHeld(WPAD_CHAN_0);
	if((sys_previous_keys & WPAD_BUTTON_1) != (k & WPAD_BUTTON_1))
	{
		Key_Event(K_ENTER, ((k & WPAD_BUTTON_1) == WPAD_BUTTON_1));
	};
	if((sys_previous_keys & WPAD_BUTTON_2) != (k & WPAD_BUTTON_2))
	{
		Key_Event(K_ESCAPE, ((k & WPAD_BUTTON_2) == WPAD_BUTTON_2));
	};
	if((sys_previous_keys & WPAD_BUTTON_PLUS) != (k & WPAD_BUTTON_PLUS))
	{
		if(((k & WPAD_BUTTON_PLUS) == WPAD_BUTTON_PLUS)&&(sys_current_weapon < 8))
		{
			sys_current_weapon++;
		};
		Key_Event('0' + sys_current_weapon, ((k & WPAD_BUTTON_PLUS) == WPAD_BUTTON_PLUS));
	};
	if((sys_previous_keys & WPAD_BUTTON_MINUS) != (k & WPAD_BUTTON_MINUS))
	{
		if(((k & WPAD_BUTTON_MINUS) == WPAD_BUTTON_MINUS)&&(sys_current_weapon > 1))
		{
			sys_current_weapon--;
		};
		Key_Event('0' + sys_current_weapon, ((k & WPAD_BUTTON_MINUS) == WPAD_BUTTON_MINUS));
	};
	if((sys_previous_keys & WPAD_BUTTON_B) != (k & WPAD_BUTTON_B))
	{
		Key_Event(K_MOUSE1, ((k & WPAD_BUTTON_B) == WPAD_BUTTON_B));
	};
	if((sys_previous_keys & WPAD_NUNCHUK_BUTTON_Z) != (k & WPAD_NUNCHUK_BUTTON_Z))
	{
		Key_Event(' ', ((k & WPAD_NUNCHUK_BUTTON_Z) == WPAD_NUNCHUK_BUTTON_Z));
	};
	if((sys_previous_keys & WPAD_BUTTON_UP) != (k & WPAD_BUTTON_UP))
	{
		Key_Event(K_UPARROW, ((k & WPAD_BUTTON_UP) == WPAD_BUTTON_UP));
	};
	if((sys_previous_keys & WPAD_BUTTON_DOWN) != (k & WPAD_BUTTON_DOWN))
	{
		Key_Event(K_DOWNARROW, ((k & WPAD_BUTTON_DOWN) == WPAD_BUTTON_DOWN));
	};
	if((sys_previous_keys & WPAD_BUTTON_LEFT) != (k & WPAD_BUTTON_LEFT))
	{
		Key_Event(K_LEFTARROW, ((k & WPAD_BUTTON_LEFT) == WPAD_BUTTON_LEFT));
	};
	if((sys_previous_keys & WPAD_BUTTON_RIGHT) != (k & WPAD_BUTTON_RIGHT))
	{
		Key_Event(K_RIGHTARROW, ((k & WPAD_BUTTON_RIGHT) == WPAD_BUTTON_RIGHT));
	};
	if(scr_drawdialog)
	{
		if((sys_previous_keys & WPAD_BUTTON_HOME) != (k & WPAD_BUTTON_HOME))
		{
			Key_Event('y', ((k & WPAD_BUTTON_HOME) == WPAD_BUTTON_HOME));
		};
	} else if(m_state_is_quit)
	{
		if((sys_previous_keys & WPAD_BUTTON_HOME) != (k & WPAD_BUTTON_HOME))
		{
			Key_Event('y', ((k & WPAD_BUTTON_HOME) == WPAD_BUTTON_HOME));
		};
	} else 
	{
		if((sys_previous_keys & WPAD_BUTTON_HOME) != (k & WPAD_BUTTON_HOME))
		{
			Key_Event(K_PAUSE, ((k & WPAD_BUTTON_HOME) == WPAD_BUTTON_HOME));
		};
	};
	if((sys_previous_keys & WPAD_NUNCHUK_BUTTON_C) != (k & WPAD_NUNCHUK_BUTTON_C))
	{
		if((k & WPAD_NUNCHUK_BUTTON_C) == WPAD_NUNCHUK_BUTTON_C)
		{
			if(in_osk.value == 0)
			{
				Cvar_SetValue("in_osk", 1);
			} else
			{
				Cvar_SetValue("in_osk", 0);
			};
		};
	};
	if((sys_previous_keys & WPAD_BUTTON_A) != (k & WPAD_BUTTON_A))
	{
		if(in_osk.value)
		{
			if(osk_selected != 0)
			{
				if(osk_selected->key == 22)
				{
					if((k & WPAD_BUTTON_A) == WPAD_BUTTON_A)
					{
						if(osk_layout == okl_normal)
						{
							osk_layout = okl_caps;
							osk_capspressed = osk_selected;
						} else if(osk_layout == okl_shift)
						{
							osk_layout = okl_shiftcaps;
							osk_capspressed = osk_selected;
						} else if(osk_layout == okl_shiftcaps)
						{
							osk_layout = okl_shift;
							osk_capspressed = 0;
						} else if(osk_layout == okl_caps)
						{
							osk_layout = okl_normal;
							osk_capspressed = 0;
						};
					};
				} else if(osk_selected->key == 23)
				{
					if((k & WPAD_BUTTON_A) == WPAD_BUTTON_A)
					{
						if(osk_layout == okl_normal)
						{
							osk_layout = okl_shift;
							osk_shiftpressed = osk_selected;
						} else if(osk_layout == okl_shift)
						{
							osk_layout = okl_normal;
							osk_shiftpressed = 0;
						} else if(osk_layout == okl_shiftcaps)
						{
							osk_layout = okl_caps;
							osk_shiftpressed = 0;
						} else if(osk_layout == okl_caps)
						{
							osk_layout = okl_shiftcaps;
							osk_shiftpressed = osk_selected;
						};
					};
				} else
				{
					Key_Event(osk_selected->key, ((k & WPAD_BUTTON_A) == WPAD_BUTTON_A));
					if((k & WPAD_BUTTON_A) != WPAD_BUTTON_A)
					{
						if(osk_layout == okl_shift)
						{
							osk_layout = okl_normal;
							osk_shiftpressed = 0;
						} else if(osk_layout == okl_shiftcaps)
						{
							osk_layout = okl_caps;
							osk_shiftpressed = 0;
						};
					};
				};
			};
		};
	};
	sys_previous_keys = k;
	PAD_ScanPads();
	g = PAD_ButtonsHeld(0);
	if((sys_previous_pad_keys & PAD_BUTTON_A) != (g & PAD_BUTTON_A))
	{
		Key_Event(K_ENTER, ((g & PAD_BUTTON_A) == PAD_BUTTON_A));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_B) != (g & PAD_BUTTON_B))
	{
		Key_Event(K_ESCAPE, ((g & PAD_BUTTON_B) == PAD_BUTTON_B));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_X) != (g & PAD_BUTTON_X))
	{
		if(((g & PAD_BUTTON_X) == PAD_BUTTON_X)&&(sys_current_weapon < 8))
		{
			sys_current_weapon++;
		};
		Key_Event('0' + sys_current_weapon, ((g & PAD_BUTTON_X) == PAD_BUTTON_X));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_Y) != (g & PAD_BUTTON_Y))
	{
		if(((g & PAD_BUTTON_Y) == PAD_BUTTON_Y)&&(sys_current_weapon > 0))
		{
			sys_current_weapon--;
		};
		Key_Event('0' + sys_current_weapon, ((g & PAD_BUTTON_Y) == PAD_BUTTON_Y));
	};
	if((sys_previous_pad_keys & PAD_TRIGGER_L) != (g & PAD_TRIGGER_L))
	{
		Key_Event(K_MOUSE1, ((g & PAD_TRIGGER_L) == PAD_TRIGGER_L));
	};
	if((sys_previous_pad_keys & PAD_TRIGGER_R) != (g & PAD_TRIGGER_R))
	{
		Key_Event(' ', ((g & PAD_TRIGGER_R) == PAD_TRIGGER_R));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_UP) != (g & PAD_BUTTON_UP))
	{
		Key_Event(K_UPARROW, ((g & PAD_BUTTON_UP) == PAD_BUTTON_UP));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_DOWN) != (g & PAD_BUTTON_DOWN))
	{
		Key_Event(K_DOWNARROW, ((g & PAD_BUTTON_DOWN) == PAD_BUTTON_DOWN));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_LEFT) != (g & PAD_BUTTON_LEFT))
	{
		Key_Event(K_LEFTARROW, ((g & PAD_BUTTON_LEFT) == PAD_BUTTON_LEFT));
	};
	if((sys_previous_pad_keys & PAD_BUTTON_RIGHT) != (g & PAD_BUTTON_RIGHT))
	{
		Key_Event(K_RIGHTARROW, ((g & PAD_BUTTON_RIGHT) == PAD_BUTTON_RIGHT));
	};
	if(scr_drawdialog)
	{
		if((sys_previous_pad_keys & PAD_BUTTON_START) != (g & PAD_BUTTON_START))
		{
			Key_Event('y', ((g & PAD_BUTTON_START) == PAD_BUTTON_START));
		};
	} else if(m_state_is_quit)
	{
		if((sys_previous_pad_keys & PAD_BUTTON_START) != (g & PAD_BUTTON_START))
		{
			Key_Event('y', ((g & PAD_BUTTON_START) == PAD_BUTTON_START));
		};
	} else 
	{
		if((sys_previous_pad_keys & PAD_BUTTON_START) != (g & PAD_BUTTON_START))
		{
			Key_Event(K_PAUSE, ((g & PAD_BUTTON_START) == PAD_BUTTON_START));
		};
	};
	sys_previous_pad_keys = g;
	while(KEYBOARD_GetEvent(&e))
    {
        if(e.type == KEYBOARD_PRESSED)
        {
			Key_Event(Sys_FindKey(e.symbol), true);
        } else if(e.type == KEYBOARD_RELEASED)
        {
			Key_Event(Sys_FindKey(e.symbol), false);
        };
    };
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New function to help local -> heap allocation issues
void* Sys_Malloc(int size, char* purpose)
{
	void* m;
	
	m = malloc(size);
	if(m == 0)
	{
		Sys_Error ("Sys_Malloc: %s - failed on %i bytes", purpose, size);
	};
	return m;
}
// <<< FIX

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New functions for big stack handling:
void Sys_BigStackRewind(void)
{
	sys_bigstack_cursize = 0;
}

void* Sys_BigStackAlloc(int size, char* purpose)
{
	void* p;

	p = 0;
	if(sys_bigstack_cursize + size < BIGSTACK_SIZE)
	{
		p = sys_bigstack + sys_bigstack_cursize;
		sys_bigstack_cursize = sys_bigstack_cursize + size;
	} else
	{
		Sys_Error ("Sys_BigStackAlloc: %s - failed on %i bytes", purpose, size);
	};
	return p;
}

void Sys_BigStackFree(int size, char* purpose)
{
	if(sys_bigstack_cursize - size >= 0)
	{
		sys_bigstack_cursize = sys_bigstack_cursize - size;
	} else
	{
		Sys_Error ("Sys_BigStackFree: %s - underflow on %i bytes", purpose, sys_bigstack_cursize - size);
	};
}
// <<< FIX

//=============================================================================

void Sys_PowerOff(s32 chan)
{
	if(chan == WPAD_CHAN_0)
	{
		SYS_ResetSystem(SYS_POWEROFF, 0, 0);
	};
}

int main (int argc, char* argv[])
{
	isDedicated = false;

	sys_current_weapon = 1;

	VIDEO_Init();
	WPAD_Init();
	PAD_Init();

	sys_rmode = VIDEO_GetPreferredMode(NULL);
	VIDEO_Configure(sys_rmode);

	sys_framebuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));
	sys_framebuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(sys_rmode));

	VIDEO_ClearFrameBuffer(sys_rmode, sys_framebuffer[0], COLOR_BLACK);

	CON_Init(sys_framebuffer[1], 20, 20, sys_rmode->fbWidth, sys_rmode->xfbHeight, sys_rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_SetNextFramebuffer(sys_framebuffer[1]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(sys_rmode->viTVMode & VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	};

	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_0, sys_rmode->fbWidth, sys_rmode->xfbHeight);

	if(!fatInitDefault())
	{
		Sys_Error("Filesystem not enabled");
	};

	Sys_GetBaseDir(argc, argv);

	sys_previous_time = Sys_DoubleTime();
	do 
	{
		sys_netinit_error = if_config(sys_ipaddress_text, NULL, NULL, true);
	} while((sys_netinit_error == -EAGAIN)&&((Sys_DoubleTime() - sys_previous_time) < 3));
	if(sys_netinit_error < 0)
	{
		Sys_Error("Network not enabled\n");
	};

	if (KEYBOARD_Init(Sys_KeyPress) != 0)
	{
		Sys_Printf("Keyboard not found\n");
	};

	OSK_LoadKeys(Keys_dat, Keys_dat_size);

	static quakeparms_t    parms;

	parms.memsize = 8*1024*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = sys_basedir;

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	printf ("Host_Init\n");
	Host_Init (&parms);

	WPAD_SetPowerButtonCallback(Sys_PowerOff);

	VIDEO_SetNextFramebuffer(sys_framebuffer[0]);

	while (1)
	{
		sys_previous_time = Sys_DoubleTime();
		Host_Frame (1.0/30.0);
		KEYBOARD_FlushEvents();
		MOUSE_FlushEvents();
		VIDEO_Flush();
		VIDEO_WaitVSync();
		while((Sys_DoubleTime() - sys_previous_time) < (1.0/30.0))
		{
			LWP_YieldThread();
		};
	};	

	return 0;
}
