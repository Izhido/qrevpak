#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

typedef enum 
{
	Start,
	Wait,
	Finished
} AppState;

typedef struct
{
	int Foreground;
	int Background;
	int Bold;
	char Char;
} CharInScreen;

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
	CON_GetMetrics(&w, &h);
	ScreenCache = (CharInScreen*)malloc(w * h * sizeof(CharInScreen));
	wmPosX = -1000;
	wmPosY = -1000;
	wmPrevPosX = -1000;
	wmPrevPosY = -1000;
	State = Start;
	while(State != Finished) 
	{
		switch(State)
		{
			case Start:
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
				msg = "Q U A K E   R E V   P A K";
				i = (int)(h / 2);
				j = (int)(w / 2) - 12;
				printf("\x1b[46m\x1b[%d;%dH%s", i, j, msg);
				l = strlen(msg);
				for(p = 0; p < l; p++)
				{
					ScreenCache[i * w + j + p].Foreground = 30;
					ScreenCache[i * w + j + p].Background = 46;
					ScreenCache[i * w + j + p].Char = msg[p];
				};
				msg = "[ (A) Enter]";
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
				msg = "[ [B] Exit ]";
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
				State = Wait;
				break;
			};
		}
		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);
		if((pressed & WPAD_BUTTON_HOME) != 0)
		{
			State = Finished;
		};
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
		VIDEO_WaitVSync();
	};
	free(ScreenCache);
	return 0;
}
