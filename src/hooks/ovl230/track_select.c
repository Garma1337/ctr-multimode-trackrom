#include "rom.h"
#include "rom/game_mode.h"

#include <common.h>

void MM_TrackSelect_MenuProc(struct RectMenu* menu)
{
	sdata->gameTracker.currLEV = GameMode_GetLevelID();
	D230.trackSel_transitionState = EXITING_MENU;

	sdata->ptrGhostTapePlaying = MEMPACK_AllocHighMem(GHOST_FILESIZE);
	memset(sdata->ptrGhostTapePlaying, 0, 0x28);
	sdata->boolReplayHumanGhost = 0;

	sdata->ptrDesiredMenu = &data.menuQueueLoadTrack;
	sdata->errorMessagePosIndex = 0;
}
