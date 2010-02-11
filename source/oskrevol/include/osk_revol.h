#include <gccore.h>
#define BOOL_IMPLEMENTED 1

#define OSK_WIDTH 606

#define OSK_HEIGHT 300

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

extern oskkey_t* osk_selected;

void OSK_LoadKeys(const u8* keys, int len);

oskkey_t* OSK_KeyAt(int x, int y);

int OSK_HandleKeys(bool KeyPressed);

void OSK_Draw(GXRModeObj* rmode, void* dest);
