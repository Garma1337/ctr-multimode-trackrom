#include "rom.h"

#include <common.h>

#define CLIP_SIZE_MENU     16
#define CLIP_SIZE_GARAGE   24000
#define CLIP_SIZE_DEFAULT  3000
#define CLIP_SIZE_CUSTOM   0x4000 // hot-reloaded tracks are in no table

u_int MainDB_GetClipSize(u_int levelID, int numPlyrCurrGame)
{
	if (levelID == CUSTOM_LEVEL_ID)
	{
		return CLIP_SIZE_CUSTOM;
	}

	if (levelID == MAIN_MENU_LEVEL)
	{
		return CLIP_SIZE_MENU;
	}

	if (levelID == ADVENTURE_GARAGE)
	{
		return CLIP_SIZE_GARAGE;
	}

	return CLIP_SIZE_DEFAULT;
}
