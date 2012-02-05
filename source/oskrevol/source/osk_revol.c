#include "osk_revol.h"
#include <malloc.h>
#include <string.h>
#include "Keyboard_img.h"
#include "KeyboardInverted_img.h"

oskkey_t** osk_allkeys = 0;

osklayout_t osk_layout = okl_normal;

oskkey_t* osk_selected = 0;

oskkey_t* osk_shiftpressed = 0;

oskkey_t* osk_capspressed = 0;

void OSK_LoadKeys(const u8* keys, int len)
{
	int i;
	int j;
	oskkey_t* k;
	int Stage;
	bool NumberSet;
	bool Exit;
	unsigned char c;

	osk_allkeys = malloc(512 * sizeof(oskkey_t*));
	memset(osk_allkeys, 0, 512 * sizeof(oskkey_t*));
	i = 0;
	j = 0;
	while(i < len)
	{
		if(keys[i] > 32)
		{
			k = malloc(sizeof(oskkey_t));
			k->layout = okl_normal;
			k->left = 0;
			k->top = 0;
			k->right = 0;
			k->bottom = 0;
			k->key = 0;
			Stage = 0;
			NumberSet = false;
			Exit = false;
			while((i < len)&&(!Exit))
			{
				c = keys[i];
				switch(Stage)
				{
					case 0:
					{
						if((c >= '0')&&(c <= '9'))
						{
							k->layout = k->layout * 10 + (c - '0');
							NumberSet = true;
						} else if((c == ',')&&(NumberSet))
						{
							NumberSet = false;
							Stage = 1;
						} else if((c > 32)||((c <= 32)&&(NumberSet)))
						{
							Exit = true;
						};
						break;
					};
					case 1:
					{
						if((c >= '0')&&(c <= '9'))
						{
							k->left = k->left * 10 + (c - '0');
							NumberSet = true;
						} else if((c == ',')&&(NumberSet))
						{
							NumberSet = false;
							Stage = 2;
						} else if((c > 32)||((c <= 32)&&(NumberSet)))
						{
							Exit = true;
						};
						break;
					};
					case 2:
					{
						if((c >= '0')&&(c <= '9'))
						{
							k->top = k->top * 10 + (c - '0');
							NumberSet = true;
						} else if((c == ',')&&(NumberSet))
						{
							NumberSet = false;
							Stage = 3;
						} else if((c > 32)||((c <= 32)&&(NumberSet)))
						{
							Exit = true;
						};
						break;
					};
					case 3:
					{
						if((c >= '0')&&(c <= '9'))
						{
							k->right = k->right * 10 + (c - '0');
							NumberSet = true;
						} else if((c == ',')&&(NumberSet))
						{
							NumberSet = false;
							Stage = 4;
						} else if((c > 32)||((c <= 32)&&(NumberSet)))
						{
							Exit = true;
						};
						break;
					};
					case 4:
					{
						if((c >= '0')&&(c <= '9'))
						{
							k->bottom = k->bottom * 10 + (c - '0');
							NumberSet = true;
						} else if((c == ':')&&(NumberSet))
						{
							NumberSet = false;
							Stage = 5;
						} else if((c > 32)||((c <= 32)&&(NumberSet)))
						{
							Exit = true;
						};
						break;
					};
					case 5:
					{
						if(c == '{')
						{
							Stage = 7;
						} else if(c == ';')
						{
							Exit = true;
						} else if(c > 32)
						{
							k->key = c;
							Stage = 6;
						};
						break;
					};
					case 6:
					{
						if(c == ';')
						{
							Stage = 10;
							Exit = true;
						} else if(c > 32)
						{
							Exit = true;
						};
						break;
					};
					case 7:
					{
						if((c >= '0')&&(c <= '9'))
						{
							k->key = k->key * 10 + (c - '0');
							NumberSet = true;
						} else if((c == '{')&&(!NumberSet))
						{
							k->key = c;
							Stage = 8;
						} else if(c == '}')
						{
							if(NumberSet)
							{
								NumberSet = false;
								Stage = 9;
							} else
							{
								k->key = c;
								Stage = 8;
							};
						} else if((c == ';')&&(!NumberSet))
						{
							k->key = c;
							Stage = 8;
						} else if((c > 32)||((c <= 32)&&(NumberSet)))
						{
							Exit = true;
						};
						break;
					};
					case 8:
					{
						if(c == '}')
						{
							Stage = 9;
						} else if(c > 32)
						{
							Exit = true;
						};
						break;
					};
					case 9:
					{
						if(c == ';')
						{
							Stage = 10;
							Exit = true;
						} else if(c > 32)
						{
							Exit = true;
						};
						break;
					};
				};
				i++;
			};
			osk_allkeys[j] = k;
			j++;
		} else
		{
			i++;
		};
	};
}

oskkey_t* OSK_KeyAt(int x, int y)
{
	oskkey_t* k;
	int i;

	i = 0;
	do
	{
		k = osk_allkeys[i];
		if(k)
		{
			if((osk_layout == k->layout)&&(x >= k->left)&&(x <= k->right)&&(y >= k->top)&&(y <= k->bottom))
			{
				break;
			};
		};
		i++;
	} while(k);
	return k;
}

int OSK_HandleKeys(bool KeyPressed)
{
	int k;

	k = -1;
	if(osk_selected != 0)
	{
		if(osk_selected->key == 22)
		{
			if(KeyPressed)
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
			if(KeyPressed)
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
			k = osk_selected->key;
			if(!KeyPressed)
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
	return k;
}

void OSK_DrawKeyboard(GXRModeObj* rmode, void* dest)
{
	u32* v;
	u32 vinc;
	const u8* img;
	int j;
	int i;
	u8 a;
	u32 c;

	v = ((u32*)(dest)) + ((rmode->viHeight - OSK_HEIGHT) / 4 * rmode->viWidth) + ((rmode->viWidth - OSK_WIDTH) / 4);
	vinc = (rmode->viWidth - OSK_WIDTH) / 2;
	img = Keyboard_img;
	for(j = 0; j < OSK_HEIGHT; j++)
	{
		for(i = 0; i < OSK_WIDTH; i += 2)
		{
			a = (*img);
			if(a == 0)
			{
				img += 5;
			} else
			{
				img++;
				c = *((u32*)img);
				img += 4;
				*v = c;
			};
			v++;
		};
		v += vinc;
	};
}

void OSK_DrawKey(GXRModeObj* rmode, void* dest, oskkey_t* key)
{
	int i;
	int j;
	int m;
	int n;
	u32* v;
	u32 vinc;
	const u8* img;
	int imginc;
	u8 a;
	u32 c;

	i = key->left & ~1;
	j = key->top & ~1;
	m = (key->right & ~1) - i;
	n = (key->bottom & ~1) - j;
	v = ((u32*)(dest)) + ((rmode->viHeight - OSK_HEIGHT + j + j) / 4 * rmode->viWidth) + ((rmode->viWidth - OSK_WIDTH + i + i) / 4);
	vinc = (rmode->viWidth - m) / 2;
	img = KeyboardInverted_img + (j * OSK_WIDTH / 2 * 5) + (i / 2 * 5);
	imginc = (OSK_WIDTH - m) / 2 * 5;
	for(j = 0; j < n; j++)
	{
		for(i = 0; i < m; i += 2)
		{
			a = (*img);
			if(a == 0)
			{
				img += 5;
			} else
			{
				img++;
				c = *((u32*)img);
				img += 4;
				*v = c;
			};
			v++;
		};
		v += vinc;
		img += imginc;
	};
}

void OSK_Draw(GXRModeObj* rmode, void* dest)
{
	OSK_DrawKeyboard(rmode, dest);
	if(osk_selected != 0)
	{
		OSK_DrawKey(rmode, dest, osk_selected);
	};
	if(osk_shiftpressed != 0)
	{
		OSK_DrawKey(rmode, dest, osk_shiftpressed);
	};
	if(osk_capspressed != 0)
	{
		OSK_DrawKey(rmode, dest, osk_capspressed);
	};
}
