#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include "ezxml.h"
#include "QRevPAK_xml.h"
#include "QRevPAK_ini.h"
#include "run_dol.h"
#include <unistd.h>

typedef enum 
{
	Start,
	StartWait,
	StartAPressed,
	StartAPressedWait,
	StartAReleased,
	StartBPressed,
	StartBPressedWait,
	StartBReleased,
	Load,
	LoadError,
	LoadErrorWait,
	LoadErrorAPressed,
	LoadErrorAPressedWait,
	LoadErrorAReleased,
	List,
	ListWait,
	ListAPressed,
	ListAReleased,
	ListBPressed,
	ListBReleased,
	ListUpPressed,
	ListUpReleased,
	ListDownPressed,
	ListDownReleased,
	ListScrollUpPressed,
	ListScrollUpReleased,
	ListScrollDownPressed,
	ListScrollDownReleased,
	ListUseGXPressed,
	ListUseGXReleased,
	Launch,
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

typedef enum
{
	AtBeginning,
	AtWhitespace,
	AtText
} TextParseState;

typedef struct
{
	int Start;
	int Length;
} LineData;

typedef struct
{
	int CRCValue;
	bool FoundInDefault;
	char* Name;
	LineData* NameLines;
	int NameLineCount;
	int DescriptionCount;
	char** Description;
	char* Engine;
	char* Parameters;
	int RequiredFilesCount;
	char** RequiredFiles;
} GameEntry;

typedef struct
{
	bool InScreen;
	bool Complete;
	int Start;
	int Length;
	int EntryIndex;
} GameEntryLocation;

typedef struct
{
	char* Engine;
	char* GX;
} GXEngineEntry;

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
	char basedir[MAXPATHLEN + 1];
	char xmlfname[MAXPATHLEN + 1];
	u32 pressed;
	FILE* f;
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
	GameEntryLocation* EntryLocations;
	GXEngineEntry* GXEngines;
	int GXEnginesCount;
	ezxml_t datKey;
	char* msgKey;
	ezxml_t datValue;
	char* msgValue;
	int TopEntryIndex;
	int SelectedEntryIndex;
	int ScrollPosition;
	int ei;
	char* eng;
	char* engFullPath;
	int newArgc;
	char** newArgv;
	char* reqFullPath;
	TextParseState t;
	int fg;
	int bg;
	int bl;
	bool CursorHasMoved;
	bool ScrollLatched;
	int TickCount;
	AppState DefaultListState;
	bool ScrollButtonInverted;
	bool UseGXEngines;

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
	ScreenCache = (CharInScreen*)malloc((w + 1) * (h + 1) * sizeof(CharInScreen));
	wmPosX = -1000;
	wmPosY = -1000;
	wmPrevPosX = -1000;
	wmPrevPosY = -1000;
	DefaultListState = ListWait;
	TickCount = 0;
	ScrollLatched = false;
	ScrollPosition = 0;
	SelectedEntryIndex = 0;
	TopEntryIndex = 0;
	ScrollButtonInverted = false;
	ErrorLinesCount = 0;
	fatInitDefault();
	if(getcwd(basedir, MAXPATHLEN) == 0)
	{
		strcpy(basedir, "/");
	};
	ErrorLines = NULL;
	xmlFromFile = NULL;
	xmlDefault = NULL;
	Entries = NULL;
	EntryIndices = NULL;
	EntryIndicesCount = 0;
	EntryLocations = NULL;
	GXEngines = NULL;
	GXEnginesCount = 0;
	UseGXEngines = false;
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
			msg = "Quake Rev PAK Release 4 (C) Heriberto Delgado.";
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
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 32;
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
			State = StartAPressedWait;
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
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 46;
			ScreenCache[i * w + j].Char = 32;
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
			State = StartBPressedWait;
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
		} else if(State == Load)
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
		} else if(State == LoadError)
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
			State = LoadErrorWait;
		} else if(State == LoadErrorAPressed)
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
			State = LoadErrorAPressedWait;
		} else if(State == LoadErrorAReleased)
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
		} else if(State == List)
		{
			printf("\x1b[46m\x1b[37;1m");
			memset(EntryLocations, 0, EntryIndicesCount * sizeof(GameEntryLocation));
			for(i = 4; i < (h - 7); i++)
			{
				printf("\x1b[%d;5H", i);
				for(j = 5; j < (w - 5); j++)
				{
					printf(" ");
					ScreenCache[i * w + j].Background = 46;
					ScreenCache[i * w + j].Foreground = 37;
					ScreenCache[i * w + j].Bold = 1;
					ScreenCache[i * w + j].Char = 32;
				};
			};
			i = h - 7;
			j = 6;
			printf("\x1b[44m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (w - 4); j++)
			{
				printf("%c", 223);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 223;
			};
			j--;
			i = 4;
			printf("\x1b[%d;%dH%c", i, j, 220);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = 44;
			ScreenCache[i * w + j].Char = 220;
			for(i++; i < (h - 7); i++)
			{
				printf("\x1b[%d;%dH%c", i, j, 219);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Background = 44;
				ScreenCache[i * w + j].Char = 219;
			};
			printf("\x1b[46m");
			i = 4;
			ei = TopEntryIndex;
			while(ei <= EntryIndicesCount)
			{
				if((ei == SelectedEntryIndex) || ((ei == SelectedEntryIndex + 1) && (ei != TopEntryIndex)))
				{
					printf("\x1b[37;1m");
					fg = 37;
					bl = 1;
				} else
				{
					printf("\x1b[30;0m");
					fg = 30;
					bl = 0;
				};
				j = 5;
				if(ei == TopEntryIndex)
				{
					if(ei == SelectedEntryIndex)
					{
						c = 201;
					} else
					{
						c = 218;
					};
				} else if(ei == EntryIndicesCount)
				{
					if(ei == SelectedEntryIndex + 1)
					{
						c = 200;
					} else
					{
						c = 192;
					};
				} else if(ei == SelectedEntryIndex)
				{
					c = 201;
				} else if((ei == SelectedEntryIndex + 1) && (ei != TopEntryIndex))
				{
					c = 200;
				} else
				{
					c = 195;
				};
				printf("\x1b[%d;%dH%c\x1b[%d;%dH", i, j, c, i, j + 1);
				ScreenCache[i * w + j].Foreground = fg;
				ScreenCache[i * w + j].Bold = bl;
				ScreenCache[i * w + j].Char = c;
				if((ei == SelectedEntryIndex) || ((ei == SelectedEntryIndex + 1) && (ei != TopEntryIndex)))
				{
					c = 205;
				} else
				{
					c = 196;
				};
				for(j = j + 1; j < 21; j++)
				{
					printf("%c", c);
					ScreenCache[i * w + j].Foreground = fg;
					ScreenCache[i * w + j].Bold = bl;
					ScreenCache[i * w + j].Char = c;
				};
				if(ei == TopEntryIndex)
				{
					if(ei == SelectedEntryIndex)
					{
						c = 209;
					} else
					{
						c = 194;
					};
				} else if(ei == EntryIndicesCount)
				{
					if(ei == SelectedEntryIndex + 1)
					{
						c = 207;
					} else
					{
						c = 193;
					};
				} else if(ei == SelectedEntryIndex)
				{
					c = 209;
				} else if((ei == SelectedEntryIndex + 1) && (ei != TopEntryIndex))
				{
					c = 207;
				} else
				{
					c = 197;
				};
				printf("\x1b[%d;%dH%c\x1b[%d;%dH", i, j, c, i, j + 1);
				ScreenCache[i * w + j].Foreground = fg;
				ScreenCache[i * w + j].Bold = bl;
				ScreenCache[i * w + j].Char = c;
				if((ei == SelectedEntryIndex) || ((ei == SelectedEntryIndex + 1) && (ei != TopEntryIndex)))
				{
					c = 205;
				} else
				{
					c = 196;
				};
				for(j = j + 1; j < (w - 6); j++)
				{
					printf("%c", c);
					ScreenCache[i * w + j].Foreground = fg;
					ScreenCache[i * w + j].Bold = bl;
					ScreenCache[i * w + j].Char = c;
				};
				if((i == 4) || (i == (h - 8)))
				{
					if(ei == TopEntryIndex)
					{
						if(ei == SelectedEntryIndex)
						{
							c = 187;
						} else
						{
							c = 191;
						};
					} else if(ei == EntryIndicesCount)
					{
						if(ei == SelectedEntryIndex + 1)
						{
							c = 188;
						} else
						{
							c = 217;
						};
					} else if(ei == SelectedEntryIndex)
					{
						c = 187;
					} else if((ei == SelectedEntryIndex + 1) && (ei != TopEntryIndex))
					{
						c = 188;
					} else
					{
						c = 180;
					};
					printf("\x1b[%d;%dH%c", i, j, c); 
					ScreenCache[i * w + j].Foreground = fg;
					ScreenCache[i * w + j].Bold = bl;
					ScreenCache[i * w + j].Char = c;
				};
				i++;
				if(ei == SelectedEntryIndex)
				{
					printf("\x1b[37;1m");
					fg = 37;
					bl = 1;
				} else
				{
					printf("\x1b[30;0m");
					fg = 30;
					bl = 0;
				};
				if(ei < EntryIndicesCount)
				{
					EntryLocations[ei].InScreen = true;
					EntryLocations[ei].Start = i;
					m = Entries[EntryIndices[ei]].NameLineCount;
					if(m < Entries[EntryIndices[ei]].DescriptionCount)
					{
						m = Entries[EntryIndices[ei]].DescriptionCount; 
					};
					m = m + 2;
					p = 0;
					while(m > 0)
					{
						if(i < (h - 8))
						{
							if(ei == SelectedEntryIndex)
							{
								c = 186;
							} else
							{
								c = 179;
							};
							printf("\x1b[%d;5H%c\x1b[%d;21H%c", i, c, i, 179);
							ScreenCache[i * w + 5].Foreground = fg;
							ScreenCache[i * w + 5].Bold = bl;
							ScreenCache[i * w + 5].Char = c;
							ScreenCache[i * w + 21].Foreground = fg;
							ScreenCache[i * w + 21].Bold = bl;
							ScreenCache[i * w + 21].Char = 179;
							if((i == 4) || (i == (h - 8)))
							{
								printf("\x1b[%d;%dH%c", i, w - 6, c);
								ScreenCache[i * w + w - 6].Foreground = fg;
								ScreenCache[i * w + w - 6].Bold = bl;
								ScreenCache[i * w + w - 6].Char = c;
							};
							if(p > 0)
							{
								if(p <= Entries[EntryIndices[ei]].NameLineCount)
								{
									msg = Entries[EntryIndices[ei]].Name;
									printf("\x1b[%d;7H", i);
									for(j = 0; j < Entries[EntryIndices[ei]].NameLines[p - 1].Length; j++)
									{
										c = msg[Entries[EntryIndices[ei]].NameLines[p - 1].Start + j];
										printf("%c", c);
										ScreenCache[i * w + j + 7].Foreground = fg;
										ScreenCache[i * w + j + 7].Bold = bl;
										ScreenCache[i * w + j + 7].Char = c;
									};
								};
								if(p <= Entries[EntryIndices[ei]].DescriptionCount)
								{
									msg = Entries[EntryIndices[ei]].Description[p - 1];
									printf("\x1b[%d;23H", i);
									j = 0;
									while(msg[j] != 0)
									{
										c = msg[j];
										printf("%c", c);
										ScreenCache[i * w + j + 23].Foreground = fg;
										ScreenCache[i * w + j + 23].Bold = bl;
										ScreenCache[i * w + j + 23].Char = c;
										if(j < (w - 7))
										{
											j++;
										} else
										{
											break;
										};
									};
								};
							};
							i++;
							p++;
							m--;
						} else
						{
							break;
						};
					};
					EntryLocations[ei].Length = i - EntryLocations[ei].Start;
					EntryLocations[ei].EntryIndex = ei;
					if(m == 0)
					{
						EntryLocations[ei].Complete = true;
					};
				};
				if(i < (h - 7))
				{
					ei++;
				} else
				{
					break;
				};
			};
			if((ScrollButtonInverted) && (DefaultListState == ListScrollUpPressed))
			{
				printf("\x1b[40m\x1b[36;0m");
				bg = 40;
			} else
			{
				printf("\x1b[30;0m");
				bg = 46;
			};
			i = 5;
			j = w - 6;
			printf("\x1b[%d;%dH%c", i, j, 30);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = bg;
			ScreenCache[i * w + j].Bold = 0;
			ScreenCache[i * w + j].Char = 30;
			if((ScrollButtonInverted) && (DefaultListState == ListScrollUpPressed))
			{
				printf("\x1b[46m\x1b[30;0m");
			};
			for(i++; i < (h - 9); i++)
			{
				printf("\x1b[%d;%dH%c", i, j, 176);
				ScreenCache[i * w + j].Foreground = 30;
				ScreenCache[i * w + j].Bold = 0;
				ScreenCache[i * w + j].Char = 176;
			};
			if((ScrollButtonInverted) && (DefaultListState == ListScrollDownPressed))
			{
				printf("\x1b[40m\x1b[36;0m");
				bg = 40;
			} else
			{
				bg = 46;
			};
			printf("\x1b[%d;%dH%c", i, j, 31);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Background = bg;
			ScreenCache[i * w + j].Bold = 0;
			ScreenCache[i * w + j].Char = 31;
			if((ScrollButtonInverted) && (DefaultListState == ListScrollDownPressed))
			{
				printf("\x1b[46m\x1b[30;0m");
			};
			i = ScrollPosition + 6;
			printf("\x1b[%d;%dH%c", i, j, 219);
			ScreenCache[i * w + j].Foreground = 30;
			ScreenCache[i * w + j].Bold = 0;
			ScreenCache[i * w + j].Char = 219;
			i = h - 5;
			j = 4;
			printf("\x1b[44m\x1b[37;1m\x1b[%d;%dH[", i, j);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 0;
			ScreenCache[i * w + j].Char = '[';
			j++;
			if(UseGXEngines)
			{
				c = 254;
			} else
			{
				c = 32;
			};
			printf("\x1b[%d;%dH%c", i, j, c);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 0;
			ScreenCache[i * w + j].Char = c;
			j++;
			msg = "] Use GX-accelerated engines if available";
			printf("\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Bold = 0;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			msg = "Wiimote=Select A=Launch";
			i = h - 3;
			j = w - 26;
			printf("\x1b[40m\x1b[37;1m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Background = 40;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			State = DefaultListState;
		} else if(State == Launch)
		{
			m = strlen(Entries[EntryIndices[SelectedEntryIndex]].Name) + 14;
			printf("\x1b[30;0m\x1b[44m");
			for(i = (int)(h / 2) - 1; i < (h / 2) + 2; i++)
			{
				j = (int)(w / 2) - ((m / 2) + 1);
				printf("\x1b[%d;%dH", i, j);
				for(; j < (int)(w / 2) + ((m / 2) + 1); j++)
				{
					printf(" ");
					ScreenCache[i * w + j].Foreground = 30;
					ScreenCache[i * w + j].Background = 44;
					ScreenCache[i * w + j].Bold = 0;
					ScreenCache[i * w + j].Char = 32;
				};
			};
			i = (int)(h / 2) - 1;
			j = (int)(w / 2) - ((m / 2) + 1);
			printf("\x1b[37;1m\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 201, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 201;
			for(j = j + 1; j < (int)(w / 2) - 5; j++)
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
			msg = "Launching";
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
			for(j = j + 1; j < (int)(w / 2) + (m / 2); j++)
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
			j = (int)(w / 2) - ((m / 2) + 1);
			printf("\x1b[%d;%dH%c\x1b[%d;%dH", i, j, 200, i, j + 1);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 200;
			for(j = j + 1; j < (int)(w / 2) + (m / 2); j++)
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
			j = (int)(w / 2) - ((m / 2) + 1);
			printf("\x1b[%d;%dH%c", i, j, 186);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 186;
			j = j + ((m / 2) + 1) + ((m / 2) + 1) - 1;
			printf("\x1b[%d;%dH%c", i, j, 186);
			ScreenCache[i * w + j].Foreground = 37;
			ScreenCache[i * w + j].Bold = 1;
			ScreenCache[i * w + j].Char = 186;
			msg = "Launching ";
			j = (int)(w / 2) - (m / 2);
			printf("\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			j += p;
			msg = Entries[EntryIndices[SelectedEntryIndex]].Name;
			printf("\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			j += p;
			msg = "...";
			printf("\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 37;
				ScreenCache[i * w + j + p].Bold = 1;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
			i = (int)(h / 2) + 2;
			j = (int)(w / 2) - (m / 2);
			printf("\x1b[46m\x1b[30;0m\x1b[%d;%dH", i, j);
			for(; j < (int)(w / 2) + ((m / 2) + 2); j++)
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
			msg = "                       ";
			i = h - 3;
			j = w - 26;
			printf("\x1b[40m\x1b[30;0m\x1b[%d;%dH%s", i, j, msg);
			l = strlen(msg);
			for(p = 0; p < l; p++)
			{
				ScreenCache[i * w + j + p].Foreground = 30;
				ScreenCache[i * w + j + p].Background = 40;
				ScreenCache[i * w + j + p].Bold = 0;
				ScreenCache[i * w + j + p].Char = msg[p];
			};
		} else if(State == Finishing)
		{
			printf("\x1b[30;0m\x1b[40m\x1b[2J");
			State = Finished;
		};
		if(State == StartAReleased)
		{
			State = Load;
		} else if(State == StartBReleased)
		{
			State = Finishing;
		} else if(State == Load)
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
							dat = ezxml_child(ent, "required");
							if(dat != NULL)
							{
								lin = ezxml_child(dat, "file");
								while(lin != NULL)
								{
									Entries[j].RequiredFilesCount++;
									lin = lin->next;
								};
								if(Entries[j].RequiredFilesCount > 0)
								{
									Entries[j].RequiredFiles = (char**)malloc(Entries[j].RequiredFilesCount * sizeof(char*));
									pl = 0;
									lin = ezxml_child(dat, "file");
									while(lin != NULL)
									{
										Entries[j].RequiredFiles[pl] = (char*)malloc(strlen(lin->txt) + 1);
										strcpy(Entries[j].RequiredFiles[pl], lin->txt);
										pl++;
										lin = lin->next;
									};
								};
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
									if(Entries[i].RequiredFiles != NULL)
									{
										for(pl = Entries[i].RequiredFilesCount - 1; pl >= 0; pl--)
										{
											free(Entries[i].RequiredFiles[pl]); 
										};
										free(Entries[i].RequiredFiles);
										Entries[i].RequiredFiles = NULL;
										Entries[i].RequiredFilesCount = 0;
									};
									dat = ezxml_child(ent, "required");
									if(dat != NULL)
									{
										lin = ezxml_child(dat, "file");
										while(lin != NULL)
										{
											Entries[i].RequiredFilesCount++;
											lin = lin->next;
										};
										if(Entries[i].RequiredFilesCount > 0)
										{
											Entries[i].RequiredFiles = (char**)malloc(Entries[i].RequiredFilesCount * sizeof(char*));
											pl = 0;
											lin = ezxml_child(dat, "file");
											while(lin != NULL)
											{
												Entries[i].RequiredFiles[pl] = (char*)malloc(strlen(lin->txt) + 1);
												strcpy(Entries[i].RequiredFiles[pl], lin->txt);
												pl++;
												lin = lin->next;
											};
										};
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
								dat = ezxml_child(ent, "required");
								if(dat != NULL)
								{
									lin = ezxml_child(dat, "file");
									while(lin != NULL)
									{
										Entries[j].RequiredFilesCount++;
										lin = lin->next;
									};
									if(Entries[j].RequiredFilesCount > 0)
									{
										Entries[j].RequiredFiles = (char**)malloc(Entries[j].RequiredFilesCount * sizeof(char*));
										pl = 0;
										lin = ezxml_child(dat, "file");
										while(lin != NULL)
										{
											Entries[j].RequiredFiles[pl] = (char*)malloc(strlen(lin->txt) + 1);
											strcpy(Entries[j].RequiredFiles[pl], lin->txt);
											pl++;
											lin = lin->next;
										};
									};
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
						for(i = 0; i < EntriesCount; i++)
						{
							if(Entries[i].Name != NULL)
							{
								msg = Entries[i].Name;
								t = AtBeginning;
								m = 1;
								p = 0;
								pl = 0;
								j = 0;
								while(msg[j] != 0)
								{
									if(msg[j] <= 32)
									{
										if(t != AtWhitespace)
										{
											t = AtWhitespace;
										};
									} else if(t != AtText)
									{
										pl = j;
										t = AtText;
									};
									j++;
									if((j - p) > 15)
									{
										if(p < pl)
										{
											p = pl;
										} else
										{
											p = j;
										};
										m++;
									};
								};
								Entries[i].NameLines = (LineData*)malloc(m * sizeof(LineData));
								m = 0;
								p = 0;
								j = 0;
								while(msg[j] != 0)
								{
									if(msg[j] <= 32)
									{
										if(t != AtWhitespace)
										{
											t = AtWhitespace;
										};
									} else if(t != AtText)
									{
										pl = j;
										t = AtText;
									};
									j++;
									if((j - p) > 14)
									{
										Entries[i].NameLines[m].Start = p;
										if(p < pl)
										{
											p = pl;
										} else
										{
											p = j;
										};
										Entries[i].NameLines[m].Length = p - Entries[i].NameLines[m].Start;
										m++;
									};
								};
								Entries[i].NameLines[m].Start = p;
								Entries[i].NameLines[m].Length = j - Entries[i].NameLines[m].Start;
								m++;
								Entries[i].NameLineCount = m;
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
												if(Entries[i].RequiredFiles != NULL)
												{
													msg = "    <required>\n";
													fwrite(msg, 1, strlen(msg), f);
													for(pl = 0; pl < Entries[i].RequiredFilesCount; pl++)
													{
														msg = "      <file>";
														fwrite(msg, 1, strlen(msg), f);
														msg = Entries[i].RequiredFiles[pl];
														fwrite(msg, 1, strlen(msg), f);
														msg = "</file>\n";
														fwrite(msg, 1, strlen(msg), f);
													};
													msg = "    </required>\n";
													fwrite(msg, 1, strlen(msg), f);
												};
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
						free(xmlDefault);
						xmlDefault = NULL;
						free(xmlFromFile);
						xmlFromFile = NULL;
						EntryLocations = (GameEntryLocation*)malloc(EntryIndicesCount * sizeof(GameEntryLocation));
						ScrollLatched = false;
						TickCount = 0;
						TopEntryIndex = -1;
						SelectedEntryIndex = -1;
						strcpy(xmlfname, basedir);
						strcat(xmlfname, "QRevPAK.ini");
						f = fopen(xmlfname, "rb");
						if(f == NULL)
						{
							f = fopen(xmlfname, "wb");
							if(f != NULL)
							{
								fwrite(QRevPAK_ini, 1, QRevPAK_ini_size, f);
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
									xmlDefault = (char*)malloc(QRevPAK_ini_size + 1);
									memcpy(xmlDefault, QRevPAK_ini, QRevPAK_ini_size);
									xmlDefault[QRevPAK_ini_size] = 0;
									docDefault = ezxml_parse_str(xmlDefault, QRevPAK_ini_size);
									GXEnginesCount = 0;
									ent = ezxml_child(docFromFile, "GXEngine");
									while(ent != NULL)
									{
										GXEnginesCount++;
										ent = ent->next;
									};
									ent = ezxml_child(docDefault, "GXEngine");
									while(ent != NULL)
									{
										GXEnginesCount++;
										ent = ent->next;
									};
									GXEngines = (GXEngineEntry*)malloc(GXEnginesCount * sizeof(GXEngineEntry));
									memset(GXEngines, 0, GXEnginesCount * sizeof(GXEngineEntry));
									msg = NULL;
									dat = ezxml_child(docFromFile, "SelectedEntryIndex");
									if(dat != NULL)
									{
										i = atoi(dat->txt);
									};
									dat = ezxml_child(docFromFile, "SelectedEntry");
									if(dat != NULL)
									{
										msg = dat->txt;
									};
									if((msg != NULL) && (i >= 0) && (i < EntryIndicesCount))
									{
										if(Entries[EntryIndices[i]].Name != NULL)
										{
											if(strcmp(msg, Entries[EntryIndices[i]].Name) == 0)
											{
												TopEntryIndex = i;
												SelectedEntryIndex = i;
											};
										};
									};
									if((TopEntryIndex < 0) && (msg != NULL))
									{
										i = 0;
										while(i < EntryIndicesCount)
										{
											if(Entries[EntryIndices[i]].Name != NULL)
											{
												if(strcmp(msg, Entries[EntryIndices[i]].Name) == 0)
												{
													TopEntryIndex = i;
													SelectedEntryIndex = i;
													break;
												};
											};
											i++;
										};
									};
									dat = ezxml_child(docFromFile, "UseGXEngines");
									if(dat != NULL)
									{
										i = atoi(dat->txt);
										UseGXEngines = (i != 0);
									} else
									{
										UseGXEngines = false;
									};
									m = 0;
									ent = ezxml_child(docFromFile, "GXEngine");
									while(ent != NULL)
									{
										datKey = ezxml_child(ent, "engine");
										if(datKey != NULL)
										{
											msgKey = datKey->txt;
											if(msgKey != NULL)
											{
												if(strlen(msgKey) > 0)
												{
													i = 0;
													while(i < m)
													{
														if(strcmp(GXEngines[i].Engine, msgKey) == 0)
														{
															break;
														} else
														{
															i++;
														};
													};
													if(i == m)
													{
														datValue = ezxml_child(ent, "gx");
														if(datValue != NULL)
														{
															msgValue = datValue->txt;
															if(msgValue != NULL)
															{
																if(strlen(msgValue) > 0)
																{
																	GXEngines[i].Engine = (char*)malloc(strlen(datKey->txt) + 1);
																	strcpy(GXEngines[i].Engine, datKey->txt);
																	GXEngines[i].GX = (char*)malloc(strlen(datValue->txt) + 1);
																	strcpy(GXEngines[i].GX, datValue->txt);
																	m++;
																};
															};
														};
													};
												};
											};
										};
										ent = ent->next;
									};
									ent = ezxml_child(docDefault, "GXEngine");
									while(ent != NULL)
									{
										datKey = ezxml_child(ent, "engine");
										msgKey = datKey->txt;
										i = 0;
										while(i < m)
										{
											if(strcmp(GXEngines[i].Engine, msgKey) == 0)
											{
												break;
											} else
											{
												i++;
											};
										};
										if(i == m)
										{
											datValue = ezxml_child(ent, "gx");
											msgValue = datValue->txt;
											GXEngines[i].Engine = (char*)malloc(strlen(datKey->txt) + 1);
											strcpy(GXEngines[i].Engine, datKey->txt);
											GXEngines[i].GX = (char*)malloc(strlen(datValue->txt) + 1);
											strcpy(GXEngines[i].GX, datValue->txt);
											m++;
										};
										ent = ent->next;
									};
									ezxml_free(docDefault);
									free(xmlDefault);
									xmlDefault = NULL;
								};
								ezxml_free(docFromFile);
							};
							free(xmlFromFile);
							xmlFromFile = NULL;
						};
						if(TopEntryIndex < 0)
						{
							TopEntryIndex = 0;
							SelectedEntryIndex = 0;
						};
						ScrollPosition = (h - 13) * TopEntryIndex / (EntryIndicesCount - 1);
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
				State = LoadError;
			} else
			{
				DefaultListState = ListWait;
				State = List;
			};
		} else if((State == LoadErrorAReleased) || (State == ListBReleased))
		{
			free(EntryLocations);
			EntryLocations = NULL;
			free(EntryIndices);
			EntryIndices = NULL;
			free(xmlDefault);
			xmlDefault = NULL;
			free(xmlFromFile);
			xmlFromFile = NULL;
			if(ErrorLines != NULL)
			{
				for(i = ErrorLinesCount - 1; i >= 0; i--)
				{
					free(ErrorLines[i]); 
				};
				free(ErrorLines);
				ErrorLines = NULL;
			};
			if(GXEngines != NULL)
			{
				for(i = GXEnginesCount - 1; i >= 0; i--)
				{
					free(GXEngines[i].GX);
					free(GXEngines[i].Engine);
				};
				free(GXEngines);
				GXEngines = NULL;
			};
			if(Entries != NULL)
			{
				for(i = EntriesCount - 1; i >= 0; i--)
				{
					for(j = Entries[i].RequiredFilesCount  - 1; j >= 0; j--)
					{
						free(Entries[i].RequiredFiles[j]); 
					};
					free(Entries[i].RequiredFiles); 
					free(Entries[i].Parameters); 
					free(Entries[i].Engine); 
					for(j = Entries[i].DescriptionCount  - 1; j >= 0; j--)
					{
						free(Entries[i].Description[j]); 
					};
					free(Entries[i].Description);
					free(Entries[i].NameLines);
					free(Entries[i].Name);
				};
				free(Entries);
				Entries = NULL;
			};
			State = Start;
		} else if(State == ListAReleased)
		{
			if(ScrollLatched)
			{
				ScrollLatched = false;
				DefaultListState = ListWait;
				State = DefaultListState;
			} else
			{
				State = Launch;
			};
		} else if(State == ListUpPressed)
		{
			if((TickCount == 0) || (TickCount == 30) || ((TickCount > 30) && ((TickCount % 6) == 0)))
			{
				if((EntryLocations[SelectedEntryIndex].InScreen) && (SelectedEntryIndex > 0))
				{
					SelectedEntryIndex--;
					State = List;
				};
				if(!(EntryLocations[SelectedEntryIndex].InScreen))
				{
					TopEntryIndex = SelectedEntryIndex;
					ScrollPosition = (h - 16) * TopEntryIndex / (EntryIndicesCount - 1);
					State = List;
				};
			};
			TickCount++;
		} else if(State == ListDownPressed)
		{
			if((TickCount == 0) || (TickCount == 30) || ((TickCount > 30) && ((TickCount % 6) == 0)))
			{
				if((EntryLocations[SelectedEntryIndex].InScreen) && (SelectedEntryIndex < (EntryIndicesCount - 1)))
				{
					SelectedEntryIndex++;
					State = List;
				} else if(SelectedEntryIndex < (EntryIndicesCount - 1))
				{
					TopEntryIndex = SelectedEntryIndex;
					ScrollPosition = (h - 16) * TopEntryIndex / (EntryIndicesCount - 1);
					State = List;
				};
			};
			TickCount++;
			if((!(EntryLocations[SelectedEntryIndex].Complete)) && (SelectedEntryIndex > TopEntryIndex))
			{
				TopEntryIndex++;
				ScrollPosition = (h - 16) * TopEntryIndex / (EntryIndicesCount - 1);
			};
		} else if((State == ListUpReleased) || (State == ListDownReleased))
		{
			TickCount = 0;
			DefaultListState = ListWait;
			State = DefaultListState;
		} else if(State == ListScrollUpPressed)
		{
			if(((TickCount == 0) || (TickCount == 30) || ((TickCount > 30) && ((TickCount % 6) == 0))) && (ScrollPosition > 0) && (wmPosX >= (w - 8)) && (wmPosX <= (w - 4)) && (wmPosY >= 4) && (wmPosY < 7))
			{
				ScrollPosition--;
				TopEntryIndex = (EntryIndicesCount - 1) * ScrollPosition / (h - 16);
				ScrollButtonInverted = !ScrollButtonInverted;
				State = List;
			};
			TickCount++;
		} else if(State == ListScrollDownPressed)
		{
			if(((TickCount == 0) || (TickCount == 30) || ((TickCount > 30) && ((TickCount % 6) == 0))) && (ScrollPosition < (h - 16)) && (wmPosX >= (w - 8)) && (wmPosX <= (w - 4)) && (wmPosY >= (h - 10)) && (wmPosY < (h - 7)))
			{
				ScrollPosition++;
				TopEntryIndex = (EntryIndicesCount - 1) * ScrollPosition / (h - 16);
				ScrollButtonInverted = !ScrollButtonInverted;
				State = List;
			};
			TickCount++;
		} else if((State == ListScrollUpReleased) || (State == ListScrollDownReleased))
		{
			TickCount = 0;
			DefaultListState = ListWait;
			if(ScrollButtonInverted)
			{
				ScrollButtonInverted = !ScrollButtonInverted;
				State = List;
			} else
			{
				State = DefaultListState;
			};
		} else if(State == ListUseGXReleased)
		{
			UseGXEngines = !UseGXEngines;
			DefaultListState = ListWait;
			State = List;
		} else if(State == Launch)
		{
			strcpy(xmlfname, basedir);
			strcat(xmlfname, "QRevPAK.ini");
			f = fopen(xmlfname, "wb");
			if(f != NULL)
			{
				msg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<Configuration>\n  <SelectedEntryIndex>";
				fwrite(msg, 1, strlen(msg), f);
				sprintf(num, "%d", SelectedEntryIndex);
				fwrite(num, 1, strlen(num), f);
				msg = "</SelectedEntryIndex>\n  <SelectedEntry>";
				fwrite(msg, 1, strlen(msg), f);
				msg = Entries[EntryIndices[SelectedEntryIndex]].Name;
				if(msg != NULL)
				{
					fwrite(msg, 1, strlen(msg), f);
				};
				msg = "</SelectedEntry>\n  <UseGXEngines>";
				fwrite(msg, 1, strlen(msg), f);
				if(UseGXEngines)
				{
					msg = "1";
				} else
				{
					msg = "0";
				};
				fwrite(msg, 1, strlen(msg), f);
				msg = "</UseGXEngines>\n";
				fwrite(msg, 1, strlen(msg), f);
				for(i = 0; i < GXEnginesCount; i++)
				{
					if(GXEngines[i].Engine == NULL)
					{
						break;
					} else
					{
						msg = "  <GXEngine>\n    <engine>";
						fwrite(msg, 1, strlen(msg), f);
						msg = GXEngines[i].Engine;
						fwrite(msg, 1, strlen(msg), f);
						msg = "</engine>\n    <gx>";
						fwrite(msg, 1, strlen(msg), f);
						msg = GXEngines[i].GX;
						fwrite(msg, 1, strlen(msg), f);
						msg = "</gx>\n  </GXEngine>\n";
						fwrite(msg, 1, strlen(msg), f);
					};
				};
				msg = "</Configuration>\n";
				fwrite(msg, 1, strlen(msg), f);
				fclose(f);
			};
			eng = (char*)malloc(MAXPATHLEN);
			if(UseGXEngines)
			{
				i = 0;
				while(i < GXEnginesCount)
				{
					if(GXEngines[i].Engine == NULL)
					{
						i = GXEnginesCount;
						break;
					} else if(strcmp(GXEngines[i].Engine, Entries[EntryIndices[SelectedEntryIndex]].Engine) == 0)
					{
						break;
					} else
					{
						i++;
					};
				};
				if(i < GXEnginesCount)
				{
					strcpy(eng, GXEngines[i].GX);
				} else
				{
					strcpy(eng, Entries[EntryIndices[SelectedEntryIndex]].Engine);
				};
			} else
			{
				strcpy(eng, Entries[EntryIndices[SelectedEntryIndex]].Engine);
			};
			strcat(eng, ".dol");
			newArgc = 0;
			newArgv = NULL;
			msg = Entries[EntryIndices[SelectedEntryIndex]].Parameters;
			if(msg != NULL)
			{
				m = strlen(basedir) + strlen(eng);
				engFullPath = (char*)malloc(m + 1);
				strcpy(engFullPath, basedir);
				strcat(engFullPath, eng);
				m = strlen(msg);
				msg = (char*)malloc(m + 1);
				strcpy(msg, Entries[EntryIndices[SelectedEntryIndex]].Parameters);
				t = AtBeginning;
				newArgc++;
				j = 0;
				while(msg[j] != 0)
				{
					if(msg[j] <= 32)
					{
						if(t != AtWhitespace)
						{
							t = AtWhitespace;
						};
					} else if(t != AtText)
					{
						newArgc++;
						t = AtText;
					};
					j++;
				};
				newArgv = (char**)malloc(newArgc * sizeof(char*));
				i = 0;
				newArgv[i] = engFullPath;
				i++;
				t = AtBeginning;
				j = 0;
				while(msg[j] != 0)
				{
					if(msg[j] <= 32)
					{
						if(t != AtWhitespace)
						{
							t = AtWhitespace;
							msg[j] = 0;
						};
					} else if(t != AtText)
					{
						newArgv[i] = msg + j;
						i++;
						t = AtText;
					};
					j++;
				};
			};
			reqFullPath = NULL;
			if(Entries[EntryIndices[SelectedEntryIndex]].RequiredFilesCount > 0)
			{
				i = 0;
				do
				{
					m = strlen(basedir) + strlen(Entries[EntryIndices[SelectedEntryIndex]].RequiredFiles[i]);
					reqFullPath = (char*)malloc(m + 1);
					strcpy(reqFullPath, basedir);
					strcat(reqFullPath, Entries[EntryIndices[SelectedEntryIndex]].RequiredFiles[i]);
					f = fopen(reqFullPath, "rb");
					if(f == NULL)
					{
						ErrorLinesCount = 6;
						ErrorLines = (char**)malloc(ErrorLinesCount * sizeof(char*));
						ErrorLines[0] = (char*)malloc(40);
						strcpy(ErrorLines[0], "The following file couldn't be found:");
						ErrorLines[1] = (char*)malloc(1);
						ErrorLines[1][0] = 0;
						ErrorLines[2] = reqFullPath;
						ErrorLines[3] = (char*)malloc(1);
						ErrorLines[3][0] = 0;
						ErrorLines[4] = (char*)malloc(50);
						strcpy(ErrorLines[4], "Be sure that the file exists at that location");
						ErrorLines[5] = (char*)malloc(30);
						strcpy(ErrorLines[5], "in order to launch the game.");
						State = LoadError;
					} else
					{
						fclose(f);
						free(reqFullPath);
						i++;
					};
				} while((State == Launch)&&(i < Entries[EntryIndices[SelectedEntryIndex]].RequiredFilesCount));
			};
			if(State == Launch)
			{
				runDOL(eng, newArgc, (const char**)newArgv);
				if(newArgv != NULL)
				{
					free(newArgv);
					free(msg);
					free(engFullPath);
				};
				ErrorLinesCount = 2;
				ErrorLines = (char**)malloc(ErrorLinesCount * sizeof(char*));
				ErrorLines[0] = (char*)malloc(20 + strlen(eng));
				strcpy(ErrorLines[0], "Can't launch \"");
				strcat(ErrorLines[0], eng);
				strcat(ErrorLines[0], "\".");
				ErrorLines[1] = (char*)malloc(32);
				strcpy(ErrorLines[1], "It is not possible to continue.");
				State = LoadError;
				free(eng);
			};
		};
		if((State != Finishing) && (State != Finished))
		{
			CursorHasMoved = false;
			WPAD_IR(WPAD_CHAN_0, &wm);
			if(wm.valid)
			{
				if((wmPosX == -1000)||(wmPosY == -1000))
				{
					wmPosX = (int)(wm.x * w / rmode->fbWidth);
					wmPosY = (int)(wm.y * h / rmode->xfbHeight);
					wmPrevPosX = wmPosX;
					wmPrevPosY = wmPosY;
					if(!((wmPrevPosX >= (w - 1))&&(wmPrevPosY >= (h-1))))
					{
						printf("\x1b[40m\x1b[37;0m\x1b[%d;%dH%c", wmPosY, wmPosX, 178);
					};
					CursorHasMoved = true;
				} else
				{
					wmPosX = (int)(wm.x * w / rmode->fbWidth);
					wmPosY = (int)(wm.y * h / rmode->xfbHeight);
					if((wmPosX != wmPrevPosX)||(wmPosY != wmPrevPosY))
					{
						if((wmPrevPosX >= 0)&&(wmPrevPosY >= 0)&&(wmPrevPosX < w)&&(wmPrevPosY < h)&&(!((wmPrevPosX >= (w - 1))&&(wmPrevPosY >= (h-1)))))
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
						if(!((wmPrevPosX >= (w - 1))&&(wmPrevPosY >= (h-1))))
						{
							printf("\x1b[40m\x1b[37;0m\x1b[%d;%dH%c", wmPosY, wmPosX, 178);
						};
						CursorHasMoved = true;
					};
				};
			} else
			{
				if((wmPosX != -1000)&&(wmPosY != -1000))
				{
					if((wmPrevPosX >= 0)&&(wmPrevPosY >= 0)&&(wmPrevPosX < w)&&(wmPrevPosY < h)&&(!((wmPrevPosX >= (w - 1))&&(wmPrevPosY >= (h-1)))))
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
			if((State == ListWait)&&(!ScrollLatched)&&(wmPosX >= 5)&&(wmPosX < (w - 8))&&(wmPosY >= 4)&&(wmPosY < (h - 7)))
			{
				i = 0;
				while(i < EntryIndicesCount)
				{
					if(EntryLocations[i].InScreen)
					{
						if((wmPosY >= EntryLocations[i].Start)&&(wmPosY < EntryLocations[i].Start + EntryLocations[i].Length))
						{
							if(SelectedEntryIndex != EntryLocations[i].EntryIndex)
							{
								SelectedEntryIndex = EntryLocations[i].EntryIndex;
								State = List;
							};
							if((CursorHasMoved)&&(!(EntryLocations[i].Complete))&&(TopEntryIndex < EntryIndicesCount - 1))
							{
								TopEntryIndex++;
								ScrollPosition = (h - 16) * TopEntryIndex / (EntryIndicesCount - 1);
								State = List;
							};
							break;
						};
					};
					i++;
				}
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
				} else if(State == LoadErrorWait)
				{
					State = LoadErrorAPressed;
				} else if(State == ListWait)
				{
					if((wmPosX >= (w - 8))&&(wmPosX <= (w - 4)))
					{
						if((wmPosY >= 6)&&(wmPosY < (h - 9)))
						{
							ScrollLatched = true;
							DefaultListState = ListAPressed;
							State = DefaultListState;
						} else if((wmPosY >= 3)&&(wmPosY < 6))
						{
							DefaultListState = ListScrollUpPressed;
							State = DefaultListState;
						} else if((wmPosY >= (h - 9))&&(wmPosY < (h - 6)))
						{
							DefaultListState = ListScrollDownPressed;
							State = DefaultListState;
						}
					} else if((wmPosX >= 3)&&(wmPosX <= 7)&&(wmPosY >= (h - 6))&&(wmPosY < (h - 3)))
					{
						DefaultListState = ListUseGXPressed;
						State = DefaultListState;
					} else if(EntryLocations[SelectedEntryIndex].InScreen) 
					{
						DefaultListState = ListAPressed;
						State = DefaultListState;
					};
				};
			} else if((pressed & WPAD_BUTTON_B) != 0)
			{
				if(State == StartWait)
				{
					State = StartBPressed;
				} else if((State == ListWait)&&(!ScrollLatched))
				{
					State = ListBPressed;
				};
			} else if((pressed & WPAD_BUTTON_UP) != 0)
			{
				if((State == ListWait)&&(!ScrollLatched))
				{
					DefaultListState = ListUpPressed;
					State = DefaultListState;
				};
			} else if((pressed & WPAD_BUTTON_DOWN) != 0)
			{
				if((State == ListWait)&&(!ScrollLatched))
				{
					DefaultListState = ListDownPressed;
					State = DefaultListState;
				};
			};
			if((pressed & WPAD_BUTTON_A) == 0)
			{
				if(State == StartAPressedWait)
				{
					State = StartAReleased;
				} else if((State == StartBPressedWait) && ((pressed & WPAD_BUTTON_B) == 0))
				{
					State = StartBReleased;
				} else if(State == LoadErrorAPressedWait)
				{
					State = LoadErrorAReleased;
				} else if(State == ListAPressed)
				{
					State = ListAReleased;
				} else if(State == ListScrollUpPressed)
				{
					State = ListScrollUpReleased;
				} else if(State == ListScrollDownPressed)
				{
					State = ListScrollDownReleased;
				} else if(State == ListUseGXPressed)
				{
					State = ListUseGXReleased;
				};
			};
			if((pressed & WPAD_BUTTON_B) == 0)
			{
				if((State == StartBPressedWait) && ((pressed & WPAD_BUTTON_A) == 0))
				{
					State = StartBReleased;
				} else if(State == ListBPressed)
				{
					State = ListBReleased;
				};
			};
			if((pressed & WPAD_BUTTON_UP) == 0)
			{
				if(State == ListUpPressed)
				{
					State = ListUpReleased;
				};
			};
			if((pressed & WPAD_BUTTON_DOWN) == 0)
			{
				if(State == ListDownPressed)
				{
					State = ListDownReleased;
				};
			};
			if((State == ListAPressed)&&(ScrollLatched)&&(wmPosX >= (w - 8))&&(wmPosX <= (w - 4))&&(wmPosY >= 4)&&(wmPosY < (h - 7)))
			{
				i = wmPosY - 6;
				if(i < 0)
				{
					i = 0;
				};
				if(i > (h - 16))
				{
					i = h - 16;
				};
				if(ScrollPosition != i)
				{
					ScrollPosition = i;
					TopEntryIndex = (EntryIndicesCount - 1) * ScrollPosition / (h - 16);
					State = List;
				};
			};
		};
		VIDEO_WaitVSync();
	};
	free(EntryLocations);
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
	if(GXEngines != NULL)
	{
		for(i = GXEnginesCount - 1; i >= 0; i--)
		{
			free(GXEngines[i].GX);
			free(GXEngines[i].Engine);
		};
		free(GXEngines);
	};
	if(Entries != NULL)
	{
		for(i = EntriesCount - 1; i >= 0; i--)
		{
			for(j = Entries[i].RequiredFilesCount  - 1; j >= 0; j--)
			{
				free(Entries[i].RequiredFiles[j]); 
			};
			free(Entries[i].RequiredFiles);
			free(Entries[i].Parameters); 
			free(Entries[i].Engine); 
			for(j = Entries[i].DescriptionCount  - 1; j >= 0; j--)
			{
				free(Entries[i].Description[j]); 
			};
			free(Entries[i].Description);
			free(Entries[i].NameLines);
			free(Entries[i].Name);
		};
		free(Entries);
	};
	free(ScreenCache);
	return 0;
}
