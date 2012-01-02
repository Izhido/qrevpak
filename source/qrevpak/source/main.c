#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include "ezxml.h"
#include "QRevPAK_xml.h"

typedef enum 
{
	Start,
	StartWait,
	StartAPressed,
	StartAReleased,
	StartBPressed,
	StartBReleased,
	Loading,
	LoadingError,
	LoadingErrorWait,
	LoadingErrorAPressed,
	LoadingErrorAReleased,
	List,
	Finishing,
	Finished
} AppState;

typedef struct
{
	int Foreground;
	int Background;
	int Bold;
	char Char;
} CharInScreen;

typedef struct
{
	int CRCValue;
	bool FoundInDefault;
	char* Name;
	int DescriptionCount;
	char** Description;
	char* Engine;
	char* Parameters;
} GameEntry;

void ShutDown(s32 chan)
{	
	SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

int main(int argc, char **argv) 
{
	void* xfb;
	GXRModeObj* rmode;
	int w;
	int h;
	AppState State;
	int i;
	int j;
	CharInScreen* ScreenCache;
	char* msg;
	int p;
	int l;
	ir_t wm;
	int wmPosX;
	int wmPosY;
	int wmPrevPosX;
	int wmPrevPosY;
	char basedir[16];
	char xmlfname[32];
	u32 pressed;
	FILE* f;
	char* bd;
	bool FirstFound;
	bool AllFound;
	GameEntry* Entries;
	int EntriesCount;
	char** ErrorLines;
	int ErrorLinesCount;
	char* xmlFromFile;
	char* xmlDefault;
	ezxml_t docFromFile;
	ezxml_t docDefault;
	int m;
	int rw;
	int rh;
	int pl;
	ezxml_t ent;
	const char* crcFromDoc;
	int crcValueFromDoc;
	int crcValue;
	char c;
	ezxml_t dat;
	ezxml_t lin;
	bool modified;
	char num[16];
	int EntryIndicesCount;
	int* EntryIndices;

	VIDEO_Init();
	WPAD_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	CON_Init(xfb, 0, 0, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE)
	{
		VIDEO_WaitVSync();
	};
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_0, rmode->fbWidth, rmode->xfbHeight);
	WPAD_SetPowerButtonCallback(&ShutDown);
	CON_GetMetrics(&w, &h);
	ScreenCache = (CharInScreen*)malloc(w * h * sizeof(CharInScreen));
	wmPosX = -1000;
	wmPosY = -1000;
	wmPrevPosX = -1000;
	wmPrevPosY = -1000;
	fatInitDefault();
	if(argc == 0)
	{
		basedir[0] = '/';
		basedir[1] = 0;
	} else 
	{
		bd = argv[0];
		i = 0;
		FirstFound = false;
		AllFound = false;
		while(i < 16)
		{
			if(bd[i] == 0)
			{
				break;
			} else if(bd[i] == ':')
			{
				if(FirstFound)
				{
					break;
				} else
				{
					FirstFound = true;
				};
			} else if(bd[i] == '/')
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
			strncpy(basedir, bd, i + 1);
			basedir[i + 1] = 0;
		} else
		{
			basedir[0] = '/';
			basedir[1] = 0;
		};
	};
	Entries = NULL;
	EntryIndices = NULL;
	ErrorLines = NULL;
	xmlFromFile = NULL;
	xmlDefault = NULL;
	State = Start;
	while(State != Finished) 
	{
		if(State == Start)
		{
			printf("\x1b[30;0m\x1b[40m\x1b[2J\x1b[44m");
			for(i = 0; i < w * h; i++)
			{
				ScreenCache[i].Foreground = 30;
				ScreenCache[i].Background = 40;
				ScreenCache[i].Bold = 0;
				ScreenCache[i].Char = ' ';
			};
			for(i = 3; i < (h - 3); i++)
			{
				printf("\x1b[%d;3H", i);
				for(j = 3; j < (w - 3); j++)
				{
					printf(" ");
					ScreenCache[i * w + j].Background = 44;
				};
			};
			printf("\x1b[46m");
			for(i = (int)(h / 2) - 6; i < (int)(h / 2) + 7; i++)
			{
				printf("\x1b[%d;%dH", i, (int)(w / 2) - 25);
				for(j = (int)(w / 2) - 25; j < (int)(w / 2) + 26; j++)
				{
					printf(" ");
					ScreenCache[i * w + j].Background = 46;
				};
			};
			i = (int)(h / 2) - 5;
			j = (int)(w / 2) - 24;
			printf("\x1b[37;1m\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 218, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 218;
			for(j = j + 1; j < (int)(w / 2) + 24; j++)
			{
				printf("%c", 196);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 196;
			};
			printf("\x1b[%d;%dH%c", i, j, 191); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 191;
			i = (int)(h / 2) + 5;
			j = (int)(w / 2) - 24;
			printf("\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 192, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 192;
			for(j = j + 1; j < (int)(w / 2) + 24; j++)
			{
				printf("%c", 196);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 196;
			};
			printf("\x1b[%d;%dH%c", i, j, 217); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 217;
			j = (int)(w / 2) - 24;
			for(i = (int)(h / 2) - 4; i < (int)(h / 2) + 5; i++)
			{
				printf("\x1b[%d;%dH%c\x1b[%d;%dH%c", i, j, 179, i, j + 48, 179);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 179;
				ScreenCache[i * w + j + 48].Foreground = 37;
				ScreenCache[i * w + j + 48].Bold = 1;
				ScreenCache[i * w + j + 48].Char = 179;
			};
			msg = "Q U A K E   R E V   P A K";
			i = (int)(h / 2);
			j = (int)(w / 2) - 12;
			printf("\x1b[30;0m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 46;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			msg = "  (A) Enter ";
			i = (int)(h / 2) + 3;
			j = (int)(w / 2) - 16;
			printf("\x1b[47m\x1b[30m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			j++;
			printf("\x1b[46m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) - 3; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 223;
			};
			i--;
			j--;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 220;
			msg = "  [B] Exit  ";
			j = (int)(w / 2) + 4;
			printf("\x1b[47m\x1b[30m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			j++;
			printf("\x1b[46m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 16; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 223;
			};
			i--;
			j--;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 220;
			i = (int)(h / 2) + 7;
			j = (int)(w / 2) - 24;
			printf("\x1b[44m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 27; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 223;
			};
			j--;
			i = (int)(h / 2) - 6;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 44;
			ScreenCache[i * w + j].Char = 220;
			for(i++; i < (int)(h / 2) + 7; i++)
			{
				printf("\x1b[%d;%dH%c", i, j, 219);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 219;
			};
			msg = "Quake Rev Pak Release 1 (C) Heriberto Delgado.";
			i = h - 3;
			j = 3;
			printf("\x1b[40m\x1b[37m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Background = 40;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			msg = "A=Enter B=Exit";
			i = h - 3;
			j = w - 17;
			printf("\x1b[37;1m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Background = 40;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			State = StartWait;
		} else if(State == StartAPressed)
		{
			msg = "  (A) Enter ";
			i = (int)(h / 2) + 3;
			j = (int)(w / 2) - 16;
			printf("\x1b[30m\x1b[46m\x1b[%d;%dH ", i, j);
			ScreenCache[i * w + j + p].Foreground = 30;
			ScreenCache[i * w + j + p].Background = 46;
			ScreenCache[i * w + j + p].Char = 32;
			j++;
			printf("\x1b[47m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			printf("\x1b[46m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) - 2; j++)
			{
				printf(" ");
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 32;
			};
		} else if(State == StartAReleased)
		{
			msg = "  (A) Enter ";
			i = (int)(h / 2) + 3;
			j = (int)(w / 2) - 16;
			printf("\x1b[47m\x1b[30m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			j++;
			printf("\x1b[46m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) - 3; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 223;
			};
			i--;
			j--;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 220;
		} else if(State == StartBPressed)
		{
			msg = "  [B] Exit  ";
			i = (int)(h / 2) + 3;
			j = (int)(w / 2) + 4;
			printf("\x1b[30m\x1b[46m\x1b[%d;%dH ", i, j);
			ScreenCache[i * w + j + p].Foreground = 30;
			ScreenCache[i * w + j + p].Background = 46;
			ScreenCache[i * w + j + p].Char = 32;
			j++;
			printf("\x1b[47m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			printf("\x1b[46m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 16; j++)
			{
				printf(" ");
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 32;
			};
		} else if(State == StartBReleased)
		{
			msg = "  [B] Exit  ";
			i = (int)(h / 2) + 3;
			j = (int)(w / 2) + 4;
			printf("\x1b[47m\x1b[30m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			j++;
			printf("\x1b[46m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 16; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 223;
			};
			i--;
			j--;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 220;
			j++;
			printf("\x1b[46m\x1b[%d;%dH ", i, j);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 32;
		} else if(State == Loading)
		{
			printf("\x1b[30;0m\x1b[44m");
			for(i = (int)(h / 2) - 1; i < (h / 2) + 2; i++)
			{
				j = (int)(w / 2) - 15;
				printf("\x1b[%d;%dH", i, j);
				for(; j < (int)(w / 2) + 15; j++)
				{
					printf(" ");
					ScreenCache[i * w + j].Foreground = 30;
					ScreenCache[i * w + j].Background = 44;
					ScreenCache[i * w + j].Bold = 0;
					ScreenCache[i * w + j].Char = 32;
				};
			};
			i = (int)(h / 2) - 1;
			j = (int)(w / 2) - 15;
			printf("\x1b[37;1m\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 201, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 201;
			for(j = j + 1; j < (int)(w / 2) - 4; j++)
			{
				printf("%c", 205);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 205;
			};
			printf("\x1b[%d;%dH%c", i, j, 181); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 181;
			j++;
			msg = "Loading";
			printf("\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			j = j + strlen(msg);
			printf("\x1b[%d;%dH%c", i, j, 198); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 198;
			for(j = j + 1; j < (int)(w / 2) + 14; j++)
			{
				printf("%c", 205);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 205;
			};
			printf("\x1b[%d;%dH%c", i, j, 187); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 187;
			i = (int)(h / 2) + 1;
			j = (int)(w / 2) - 15;
			printf("\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 200, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 200;
			for(j = j + 1; j < (int)(w / 2) + 14; j++)
			{
				printf("%c", 205);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 205;
			};
			printf("\x1b[%d;%dH%c", i, j, 188); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 188;
			i = (int)(h / 2);
			j = (int)(w / 2) - 15;
			printf("\x1b[%d;%dH%c", i, j, 186);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 186;
			j = j + 29;
			printf("\x1b[%d;%dH%c", i, j, 186);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 186;
			msg = "Please wait...";
			j = (int)(w / 2) - 14;
			printf("\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i = (int)(h / 2) + 2;
			j = (int)(w / 2) - 14;
			printf("\x1b[46m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 16; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 223;
			};
			j--;
			i = (int)(h / 2) - 1;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 220;
			for(i++; i < (int)(h / 2) + 2; i++)
			{
				printf("\x1b[%d;%dH%c", i, j, 219);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 219;
			};
			msg = "              ";
			i = h - 3;
			j = w - 17;
			printf("\x1b[40m\x1b[30;0m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 40;
				ScreenCache[i * w + j + p].Bold = 0;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
		} else if(State == LoadingError)
		{
			m = 0;
			for(i = 0; i < ErrorLinesCount; i++)
			{
				j = strlen(ErrorLines[i]);
				if(m < j)
				{
					m = j;
				};
			};
			if(m < 8)
			{
				m = 8;
			};
			if(m > w - 4)
			{
				m = w - 4;
			};
			if((m & 1) != 0)
			{
				rw = 1;
			} else
			{
				rw = 0;
			};
			if((ErrorLinesCount & 1) != 0)
			{
				rh = 1;
			} else
			{
				rh = 0;
			};
			printf("\x1b[30;0m\x1b[44m");
			for(i = (int)(h / 2) - (int)(ErrorLinesCount / 2) - 2; i < (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh + 3; i++)
			{
				j = (int)(w / 2) - (int)(m / 2) - 1;
				printf("\x1b[%d;%dH", i, j);
				for(; j < (int)(w / 2) + (int)(m / 2) + rw + 2; j++)
				{
					printf(" ");
					ScreenCache[i * w + j].Foreground = 30;
					ScreenCache[i * w + j].Background = 44;
					ScreenCache[i * w + j].Bold = 0;
					ScreenCache[i * w + j].Char = 32;
				};
			};
			i = (int)(h / 2) - (int)(ErrorLinesCount / 2) - 2;
			j = (int)(w / 2) - (int)(m / 2) - 1;
			printf("\x1b[37;1m\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 201, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 201;
			for(j = j + 1; j < (int)(w / 2) - 2; j++)
			{
				printf("%c", 205);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 205;
			};
			printf("\x1b[%d;%dH%c", i, j, 181); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 181;
			j++;
			msg = "ERROR";
			printf("\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			j = j + strlen(msg);
			printf("\x1b[%d;%dH%c", i, j, 198); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 198;
			for(j = j + 1; j < (int)(w / 2) + (int)(m / 2) + rw + 1; j++)
			{
				printf("%c", 205);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 205;
			};
			printf("\x1b[%d;%dH%c", i, j, 187); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 187;
			i = (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh + 2;
			j = (int)(w / 2) - (int)(m / 2) - 1;
			printf("\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 200, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 200;
			for(j = j + 1; j < (int)(w / 2) + (int)(m / 2) + rw + 1; j++)
			{
				printf("%c", 205);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 205;
			};
			printf("\x1b[%d;%dH%c", i, j, 188); 
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 188;
			i = (int)(h / 2) - (int)(ErrorLinesCount / 2) - 1;
			j = (int)(w / 2) - (int)(m / 2) - 1;
			for(i = (int)(h / 2) - (int)(ErrorLinesCount / 2) - 1; i < (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh + 2; i++)
			{
				printf("\x1b[%d;%dH%c\x1b[%d;%dH%c", i, j, 186, i, j + m + rw + 1, 186);
				ScreenCache[i * w + j].Foreground = 37;
				ScreenCache[i * w + j].Bold = 1;
				ScreenCache[i * w + j].Char = 186;
				ScreenCache[i * w + j + m + rw + 1].Foreground = 37;
				ScreenCache[i * w + j + m + rw + 1].Bold = 1;
				ScreenCache[i * w + j + m + rw + 1].Char = 186;
			};
			i = (int)(h / 2) - (int)(ErrorLinesCount / 2) - 1;
			j = (int)(w / 2) - (int)(m / 2);
			for(pl = 0; pl < ErrorLinesCount; pl++)
			{
				msg = ErrorLines[pl];
				l = strlen(msg);
				if(l > m)
				{
					l = m;
				};
				printf("\x1b[%d;%dH", i, j);
				for(p = 0; p < l; p++)
				{
					printf("%c", msg[p]);
					ScreenCache[i * w + j + p].Foreground = 37;
					ScreenCache[i * w + j + p].Bold = 1;
					ScreenCache[i * w + j + p].Char = msg[p];
				};
				i++;
			};
			i = (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh + 3;
			j = (int)(w / 2) - (int)(m / 2);
			printf("\x1b[46m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + (int)(m / 2) + rw + 3; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 223;
			};
			j--;
			i = (int)(h / 2) - (int)(ErrorLinesCount / 2) - 2;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 220;
			for(i++; i < (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh + 3; i++)
			{
				printf("\x1b[%d;%dH%c", i, j, 219);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 46;
				ScreenCache[i * w + j].Char = 219;
			};
			msg = " (A) Back ";
			i = (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh;
			j = (int)(w / 2) - 4;
			printf("\x1b[47m\x1b[30m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			j++;
			printf("\x1b[44m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 7; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 223;
			};
			i--;
			j--;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 44;
			ScreenCache[i * w + j].Char = 220;
			msg = "        A=Back";
			i = h - 3;
			j = w - 17;
			printf("\x1b[40m\x1b[37;1m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Background = 40;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
		} else if(State == LoadingErrorAPressed)
		{
			msg = " (A) Back ";
			i = (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh;
			j = (int)(w / 2) - 4;
			printf("\x1b[30m\x1b[44m\x1b[%d;%dH ", i, j);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 44;
			ScreenCache[i * w + j].Char = 32;
			j++;
			printf("\x1b[47m\x1b[30m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			printf("\x1b[44m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 7; j++)
			{
				printf(" ");
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 32;
			};
		} else if(State == LoadingErrorAReleased)
		{
			msg = " (A) Back ";
			i = (int)(h / 2) + (int)(ErrorLinesCount / 2) + rh;
			j = (int)(w / 2) - 4;
			printf("\x1b[47m\x1b[30m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 47;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i++;
			j++;
			printf("\x1b[44m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + 7; j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 223;
			};
			i--;
			j--;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 44;
			ScreenCache[i * w + j].Char = 220;
		} else if(State == Finishing)
		{
			printf("\x1b[30;0m\x1b[40m\x1b[2J");
		};
		if (State == Finishing)
		{
			State = Finished;
		} else if(State == StartBReleased)
		{
			State = Finishing;
		} else if(State == StartAReleased)
		{
			State = Loading;
		} else if(State == Loading)
		{
			strcpy(xmlfname, basedir);
			strcat(xmlfname, "QRevPAK.xml");
			f = fopen(xmlfname, "rb");
			if(f == NULL)
			{
				f = fopen(xmlfname, "wb");
				if(f != NULL)
				{
					fwrite(QRevPAK_xml, 1, QRevPAK_xml_size, f);
					fclose(f);
					f = fopen(xmlfname, "rb");
				};
			};
			if(f != NULL)
			{
				fseek(f, 0, SEEK_END);
				m = ftell(f);
				fseek(f, 0, SEEK_SET);
				if((m > 8)&&(m < (1024*1024)))
				{
					xmlFromFile = (char*)malloc(m + 1);
					i = fread(xmlFromFile, 1, m, f);
					if(i == m)
					{
						xmlFromFile[m] = 0;
					} else
					{
						free(xmlFromFile);
						xmlFromFile = NULL;
					};
				};
				fclose(f);
				if(xmlFromFile != NULL)
				{
					docFromFile = ezxml_parse_str(xmlFromFile, m);
					if(docFromFile != NULL)
					{
						xmlDefault = (char*)malloc(QRevPAK_xml_size + 1);
						memcpy(xmlDefault, QRevPAK_xml, QRevPAK_xml_size);
						xmlDefault[QRevPAK_xml_size] = 0;
						docDefault = ezxml_parse_str(xmlDefault, QRevPAK_xml_size);
						EntriesCount = 0;
						ent = ezxml_child(docFromFile, "GameEntry");
						while(ent != NULL)
						{
							EntriesCount++;
							ent = ent->next;
						};
						ent = ezxml_child(docDefault, "GameEntry");
						while(ent != NULL)
						{
							EntriesCount++;
							ent = ent->next;
						};
						Entries = (GameEntry*)malloc(EntriesCount * sizeof(GameEntry));
						memset(Entries, 0, EntriesCount * sizeof(GameEntry));
						for(i = 0; i < EntriesCount; i++)
						{
							Entries[i].CRCValue = -1;
						};
						j = 0;
						ent = ezxml_child(docFromFile, "GameEntry");
						while(ent != NULL)
						{
							crcValue = -1;
							crcFromDoc = ezxml_attr(ent, "crc");
							if(crcFromDoc != NULL)
							{
								crcValueFromDoc = atoi(crcFromDoc);
								msg = ezxml_toxml(ent);
								m = strlen(msg);
								i = m - 1;
								while(i >= 0)
								{
									if(msg[i] == '<')
									{
										msg[i] = 0;
										break;
									} else
									{
										i--;
									};
								};
								if(i > 0)
								{
									i = 0;
									while(i < m)
									{
										if(msg[i] == 0)
										{
											i = m;
										} else if(msg[i] == '>')
										{
											i++;
											break;
										} else
										{
											i++;
										};
									};
									if(i > 0)
									{
										m = strlen(msg);
										crcValue = 0;
										for(;i < m; i++)
										{
											c = msg[i];
											if((c > 32) && (c <= 127))
											{
												crcValue += c;
												while(crcValue > 65535)
												{
													crcValue = crcValue - 65536;
												};
											};
										};
										if(crcValue != crcValueFromDoc)
										{
											crcValue = -1;
										};
									};
								};
								free(msg);
							};
							if(crcValue >= 0)
							{
								Entries[j].CRCValue = crcValue;
							};
							dat = ezxml_child(ent, "name");
							if(dat != NULL)
							{
								Entries[j].Name = (char*)malloc(strlen(dat->txt) + 1);
								strcpy(Entries[j].Name, dat->txt);
							};
							dat = ezxml_child(ent, "description");
							if(dat != NULL)
							{
								lin = ezxml_child(dat, "line");
								while(lin != NULL)
								{
									Entries[j].DescriptionCount++;
									lin = lin->next;
								};
								if(Entries[j].DescriptionCount > 0)
								{
									Entries[j].Description = (char**)malloc(Entries[j].DescriptionCount * sizeof(char*));
									pl = 0;
									lin = ezxml_child(dat, "line");
									while(lin != NULL)
									{
										Entries[j].Description[pl] = (char*)malloc(strlen(lin->txt) + 1);
										strcpy(Entries[j].Description[pl], lin->txt);
										pl++;
										lin = lin->next;
									};
								};
							};
							dat = ezxml_child(ent, "engine");
							if(dat != NULL)
							{
								Entries[j].Engine = (char*)malloc(strlen(dat->txt) + 1);
								strcpy(Entries[j].Engine, dat->txt);
							};
							dat = ezxml_child(ent, "parameters");
							if(dat != NULL)
							{
								Entries[j].Parameters = (char*)malloc(strlen(dat->txt) + 1);
								strcpy(Entries[j].Parameters, dat->txt);
							};
							j++;
							ent = ent->next;
						};
						modified = false;
						ent = ezxml_child(docDefault, "GameEntry");
						while(ent != NULL)
						{
							crcFromDoc = ezxml_attr(ent, "crc");
							crcValueFromDoc = atoi(crcFromDoc);
							dat = ezxml_child(ent, "name");
							i = 0;
							while(i < EntriesCount)
							{
								if(Entries[i].CRCValue >= 0)
								{
									if(Entries[i].Name != NULL)
									{
										if(strcmp(Entries[i].Name, dat->txt) == 0)
										{
											break;
										};
									};
								};
								i++;
							};
							if(i < EntriesCount)
							{
								Entries[i].FoundInDefault = true;
								if(Entries[i].CRCValue != crcValueFromDoc)
								{
									modified = true;
									Entries[i].CRCValue = crcValueFromDoc;
									if(Entries[i].Description != NULL)
									{
										for(pl = Entries[i].DescriptionCount - 1; pl >= 0; pl--)
										{
											free(Entries[i].Description[pl]); 
										};
										free(Entries[i].Description);
										Entries[i].Description = NULL;
										Entries[i].DescriptionCount = 0;
									};
									dat = ezxml_child(ent, "description");
									if(dat != NULL)
									{
										lin = ezxml_child(dat, "line");
										while(lin != NULL)
										{
											Entries[i].DescriptionCount++;
											lin = lin->next;
										};
										if(Entries[i].DescriptionCount > 0)
										{
											Entries[i].Description = (char**)malloc(Entries[i].DescriptionCount * sizeof(char*));
											pl = 0;
											lin = ezxml_child(dat, "line");
											while(lin != NULL)
											{
												Entries[i].Description[pl] = (char*)malloc(strlen(lin->txt) + 1);
												strcpy(Entries[i].Description[pl], lin->txt);
												pl++;
												lin = lin->next;
											};
										};
									};
									free(Entries[i].Engine);
									dat = ezxml_child(ent, "engine");
									if(dat != NULL)
									{
										Entries[i].Engine = (char*)malloc(strlen(dat->txt) + 1);
										strcpy(Entries[i].Engine, dat->txt);
									};
									free(Entries[i].Parameters);
									Entries[i].Parameters = NULL;
									dat = ezxml_child(ent, "parameters");
									if(dat != NULL)
									{
										Entries[i].Parameters = (char*)malloc(strlen(dat->txt) + 1);
										strcpy(Entries[i].Parameters, dat->txt);
									};
								};
							} else
							{
								modified = true;
								Entries[j].FoundInDefault = true;
								Entries[j].CRCValue = crcValueFromDoc;
								dat = ezxml_child(ent, "name");
								Entries[j].Name = (char*)malloc(strlen(dat->txt) + 1);
								strcpy(Entries[j].Name, dat->txt);
								dat = ezxml_child(ent, "description");
								if(dat != NULL)
								{
									lin = ezxml_child(dat, "line");
									while(lin != NULL)
									{
										Entries[j].DescriptionCount++;
										lin = lin->next;
									};
									if(Entries[j].DescriptionCount > 0)
									{
										Entries[j].Description = (char**)malloc(Entries[j].DescriptionCount * sizeof(char*));
										pl = 0;
										lin = ezxml_child(dat, "line");
										while(lin != NULL)
										{
											Entries[j].Description[pl] = (char*)malloc(strlen(lin->txt) + 1);
											strcpy(Entries[j].Description[pl], lin->txt);
											pl++;
											lin = lin->next;
										};
									};
								};
								dat = ezxml_child(ent, "engine");
								Entries[j].Engine = (char*)malloc(strlen(dat->txt) + 1);
								strcpy(Entries[j].Engine, dat->txt);
								dat = ezxml_child(ent, "parameters");
								if(dat != NULL)
								{
									Entries[j].Parameters = (char*)malloc(strlen(dat->txt) + 1);
									strcpy(Entries[j].Parameters, dat->txt);
								};
								j++;
							};
							ent = ent->next;
						};
						EntryIndicesCount = 0;
						for(i = 0; i < EntriesCount; i++)
						{
							if((Entries[i].Name != NULL) && (Entries[i].Engine != NULL))
							{
								if((strlen(Entries[i].Name) > 0) && (strlen(Entries[i].Engine) > 0))
								{
									if((Entries[i].CRCValue < 0) || ((Entries[i].CRCValue >= 0) && (Entries[i].FoundInDefault)))
									{
										EntryIndicesCount++;
									};
								}
							};
							if((Entries[i].CRCValue >= 0) && (!Entries[i].FoundInDefault))
							{
								modified = true;
							};
						};
						EntryIndices = (int*)malloc(EntryIndicesCount * sizeof(int));
						j = 0;
						for(i = 0; i < EntriesCount; i++)
						{
							if((Entries[i].Name != NULL) && (Entries[i].Engine != NULL))
							{
								if((strlen(Entries[i].Name) > 0) && (strlen(Entries[i].Engine) > 0))
								{
									if((Entries[i].CRCValue < 0) || ((Entries[i].CRCValue >= 0) && (Entries[i].FoundInDefault)))
									{
										EntryIndices[j] = i;
										j++;
									};
								}
							};
						};
						if(modified)
						{
							f = fopen(xmlfname, "wb");
							if(f != NULL)
							{
								msg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<GameEntries>\n";
								fwrite(msg, 1, strlen(msg), f);
								for(i = 0; i < EntriesCount; i++)
								{
									if((Entries[i].Name != NULL) && (Entries[i].Engine != NULL))
									{
										if((strlen(Entries[i].Name) > 0) && (strlen(Entries[i].Engine) > 0))
										{
											if((Entries[i].CRCValue < 0) || ((Entries[i].CRCValue >= 0) && (Entries[i].FoundInDefault)))
											{
												msg = "  <GameEntry";
												fwrite(msg, 1, strlen(msg), f);
												if(Entries[i].CRCValue >= 0)
												{
													msg = " crc=\"";
													fwrite(msg, 1, strlen(msg), f);
													num[0] = 0;
													sprintf(num, "%d", Entries[i].CRCValue);
													fwrite(num, 1, strlen(num), f);
													msg = "\"";
													fwrite(msg, 1, strlen(msg), f);
												};
												msg = ">\n    <name>";
												fwrite(msg, 1, strlen(msg), f);
												msg = Entries[i].Name;
												fwrite(msg, 1, strlen(msg), f);
												msg = "</name>\n";
												fwrite(msg, 1, strlen(msg), f);
												if(Entries[i].Description != NULL)
												{
													msg = "    <description>\n";
													fwrite(msg, 1, strlen(msg), f);
													for(pl = 0; pl < Entries[i].DescriptionCount; pl++)
													{
														msg = "      <line>";
														fwrite(msg, 1, strlen(msg), f);
														msg = Entries[i].Description[pl];
														fwrite(msg, 1, strlen(msg), f);
														msg = "</line>\n";
														fwrite(msg, 1, strlen(msg), f);
													};
													msg = "    </description>\n";
													fwrite(msg, 1, strlen(msg), f);
												};
												msg = "    <engine>";
												fwrite(msg, 1, strlen(msg), f);
												msg = Entries[i].Engine;
												fwrite(msg, 1, strlen(msg), f);
												msg = "</engine>\n";
												fwrite(msg, 1, strlen(msg), f);
												msg = "    <parameters>";
												fwrite(msg, 1, strlen(msg), f);
												if(Entries[i].Parameters != NULL)
												{
													msg = Entries[i].Parameters;
													fwrite(msg, 1, strlen(msg), f);
												};
												msg = "</parameters>\n";
												fwrite(msg, 1, strlen(msg), f);
												msg = "  </GameEntry>\n";
												fwrite(msg, 1, strlen(msg), f);
											};
										};
									};
								};
								msg = "</GameEntries>\n";
								fwrite(msg, 1, strlen(msg), f);
								fclose(f);
							};
						};
						ezxml_free(docDefault);
						ezxml_free(docFromFile);
						free(xmlFromFile);
					} else
					{
						free(xmlFromFile);
						xmlFromFile = NULL;
						ErrorLinesCount = 2;
						ErrorLines = (char**)malloc(ErrorLinesCount * sizeof(char*));
						ErrorLines[0] = (char*)malloc(18 + strlen(xmlfname));
						strcpy(ErrorLines[0], "Can't parse \"");
						strcat(ErrorLines[0], xmlfname);
						strcat(ErrorLines[0], "\".");
						ErrorLines[1] = (char*)malloc(32);
						strcpy(ErrorLines[1], "It's not possible to continue.");
					};
				} else
				{
					ErrorLinesCount = 2;
					ErrorLines = (char**)malloc(ErrorLinesCount * sizeof(char*));
					ErrorLines[0] = (char*)malloc(20 + strlen(xmlfname));
					strcpy(ErrorLines[0], "Can't read from \"");
					strcat(ErrorLines[0], xmlfname);
					strcat(ErrorLines[0], "\".");
					ErrorLines[1] = (char*)malloc(32);
					strcpy(ErrorLines[1], "It is not possible to continue.");
					ErrorLinesCount = 2;
				};
			} else
			{
				ErrorLinesCount = 2;
				ErrorLines = (char**)malloc(ErrorLinesCount * sizeof(char*));
				ErrorLines[0] = (char*)malloc(16 + strlen(xmlfname));
				strcpy(ErrorLines[0], "Can't open \"");
				strcat(ErrorLines[0], xmlfname);
				strcat(ErrorLines[0], "\".");
				ErrorLines[1] = (char*)malloc(32);
				strcpy(ErrorLines[1], "It is not possible to continue.");
			};
			if(ErrorLines != NULL)
			{
				State = LoadingError;
			} else
			{
				State = List;
			};
		} else if(State == LoadingError)
		{
			for(i = ErrorLinesCount - 1; i >= 0; i--)
			{
				free(ErrorLines[i]); 
			};
			free(ErrorLines);
			ErrorLines = NULL;
			State = LoadingErrorWait;
		} else if(State == LoadingErrorAReleased)
		{
			State = Start;
		};
		if (State != Finishing)
		{
			WPAD_IR(WPAD_CHAN_0, &wm);
			if(wm.valid)
			{
				if((wmPosX == -1000)||(wmPosY == -1000))
				{
					wmPosX = (int)(wm.x * w / rmode->fbWidth);
					wmPosY = (int)(wm.y * h / rmode->xfbHeight);
					wmPrevPosX = wmPosX;
					wmPrevPosY = wmPosY;
					printf("\x1b[40m\x1b[37;0m\x1b[%d;%dH%c", wmPosY, wmPosX, 178);
				} else
				{
					wmPosX = (int)(wm.x * w / rmode->fbWidth);
					wmPosY = (int)(wm.y * h / rmode->xfbHeight);
					if((wmPosX != wmPrevPosX)||(wmPosY != wmPrevPosY))
					{
						if((wmPrevPosX >= 0)&&(wmPrevPosY >= 0)&&(wmPrevPosX < w)&&(wmPrevPosY < h))
						{
							printf("\x1b[%d;%dm\x1b[%dm\x1b[%d;%dH%c", ScreenCache[wmPrevPosY * w + wmPrevPosX].Foreground, 
																	   ScreenCache[wmPrevPosY * w + wmPrevPosX].Bold,
																	   ScreenCache[wmPrevPosY * w + wmPrevPosX].Background, 
																	   wmPrevPosY,
																	   wmPrevPosX,
																	   ScreenCache[wmPrevPosY * w + wmPrevPosX].Char);
						};
						wmPrevPosX = wmPosX;
						wmPrevPosY = wmPosY;
						printf("\x1b[40m\x1b[37;0m\x1b[%d;%dH%c", wmPosY, wmPosX, 178);
					};
				};
			} else
			{
				if((wmPosX != -1000)&&(wmPosY != -1000))
				{
					if((wmPosX >= 0)&&(wmPosY >= 0)&&(wmPosX < w)&&(wmPosY < h))
					{
						printf("\x1b[%d;%dm\x1b[%dm\x1b[%d;%dH%c", ScreenCache[wmPrevPosY * w + wmPrevPosX].Foreground, 
																   ScreenCache[wmPrevPosY * w + wmPrevPosX].Bold,
																   ScreenCache[wmPrevPosY * w + wmPrevPosX].Background, 
																   wmPosY,
																   wmPosX,
																   ScreenCache[wmPrevPosY * w + wmPrevPosX].Char);
					};
					wmPosX = -1000;
					wmPosY = -1000;
				};
			};
			WPAD_ScanPads();
			pressed = WPAD_ButtonsHeld(0);
			if((pressed & WPAD_BUTTON_HOME) != 0)
			{
				State = Finishing;
			} else if((pressed & WPAD_BUTTON_A) != 0)
			{
				if(State == StartWait)
				{
					if((wmPosX >= (w / 2) + 4) && (wmPosX <= (w / 2) + 16) && (wmPosY == (h / 2) + 3))
					{
						State = StartBPressed;
					} else
					{
						State = StartAPressed;
					};
				} else if(State == LoadingErrorWait)
				{
					State = LoadingErrorAPressed;
				};
			} else if((pressed & WPAD_BUTTON_B) != 0)
			{
				if(State == StartWait)
				{
					State = StartBPressed;
				};
			};
			if((pressed & WPAD_BUTTON_A) == 0)
			{
				if(State == StartAPressed)
				{
					State = StartAReleased;
				} else if((State == StartBPressed) && ((pressed & WPAD_BUTTON_B) == 0))
				{
					State = StartBReleased;
				} else if(State == LoadingErrorAPressed)
				{
					State = LoadingErrorAReleased;
				};
			};
			if((pressed & WPAD_BUTTON_B) == 0)
			{
				if((State == StartBPressed) && ((pressed & WPAD_BUTTON_A) == 0))
				{
					State = StartBReleased;
				};				
			};
		};
		VIDEO_WaitVSync();
	};
	free(EntryIndices);
	free(xmlDefault);
	free(xmlFromFile);
	if(ErrorLines != NULL)
	{
		for(i = ErrorLinesCount - 1; i >= 0; i--)
		{
			free(ErrorLines[i]); 
		};
		free(ErrorLines);
	};
	if(Entries != NULL)
	{
		for(i = EntriesCount - 1; i >= 0; i--)
		{
			free(Entries[i].Parameters); 
			free(Entries[i].Engine); 
			for(j = Entries[i].DescriptionCount  - 1; j >= 0; j--)
			{
				free(Entries[i].Description[j]); 
			};
			free(Entries[i].Description);
			free(Entries[i].Name);
		};
		free(Entries);
	};
	free(ScreenCache);
	return 0;
}
