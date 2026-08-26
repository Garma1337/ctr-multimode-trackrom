#include "ghost.h"
#include "hot_reload.h"
#include "../rom.h"

#include <common.h>

#define VRAM_CHAIN_MARKER 0x20

static int stagedTrack = 0;
static int levelUnpacked = 0;

static void HotReload_UploadVramChain(int* buffer)
{
	int size = buffer[1];
	struct VramHeader* header = (struct VramHeader*)&buffer[2];

	while (size != 0)
	{
		LoadImage(&header->rect, VRAMHEADER_GETPIXLES(header));

		buffer = (int*)header;
		buffer = &buffer[size >> 2];

		size = buffer[0];
		header = (struct VramHeader*)&buffer[1];
	}
}

void HotReload_UploadVram()
{
	int* buffer = (int*)CUSTOM_VRAM_ADDR;

	if (buffer[0] == VRAM_CHAIN_MARKER)
	{
		HotReload_UploadVramChain(buffer);
		return;
	}

	LoadImage(&((struct VramHeader*)buffer)->rect, VRAMHEADER_GETPIXLES((struct VramHeader*)buffer));
}

void HotReload_ApplyStagedLevel()
{
	const char* level = CUSTOM_LEV_ADDR;
	const int* mapOffset = CUSTOM_MAP_PTR_ADDR;
	int* pointerMap = (int*)(level + *mapOffset);

	LOAD_RunPtrMap(level, pointerMap + 1, *pointerMap >> 2);

	levelUnpacked = 0;
}

void HotReload_Poll()
{
	volatile int* vramTrigger = TRIGGER_VRM_RELOAD;
	if (*vramTrigger)
	{
		HotReload_UploadVram();
		*vramTrigger = 0;
		return;
	}

	struct GameTracker* gGT = sdata->gGT;
	volatile int* trigger = TRIGGER_HOT_RELOAD;

	if (*trigger == HOT_RELOAD_LOAD && ((gGT->gameMode1 & LOADING) || gGT->levelID == MAIN_MENU_LEVEL))
	{
		*trigger = HOT_RELOAD_READY;
		while (*trigger != HOT_RELOAD_EXEC) {}
		return;
	}

	if (*trigger != HOT_RELOAD_START || (gGT->gameMode1 & LOADING))
	{
		return;
	}

	*trigger = HOT_RELOAD_LOAD;
	stagedTrack = 1;

	if (gGT->levelID == MAIN_MENU_LEVEL)
	{
		return;
	}

	GhostTape_Destroy();
	sdata->mainMenuState = MM_STATE_TITLE;
	gGT->gameMode1 |= MAIN_MENU;
	Ghost_Forget();
	sdata->gameProgress.highScoreTracks[CUSTOM_LEVEL_ID].timeTrialFlags = 0;
	MainRaceTrack_RequestLoad(MAIN_MENU_LEVEL);
}

int HotReload_HasStagedTrack()
{
	return stagedTrack;
}

void HotReload_MarkLevelUnpacked()
{
	if ((sdata->gGT->levelID == CUSTOM_LEVEL_ID) && stagedTrack)
	{
		levelUnpacked = 1;
	}
}

void HotReload_RepackLevel()
{
	volatile int* trigger = TRIGGER_HOT_RELOAD;

	if (levelUnpacked && (*trigger == HOT_RELOAD_DONE))
	{
		LevInstDef_RePack(sdata->gGT->level1->ptr_mesh_info, 0);
	}

	levelUnpacked = 0;
}
