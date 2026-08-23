#include "game_mode.h"
#include "hot_reload.h"
#include "level.h"
#include "../rom.h"
#include "settings.h"

#include <common.h>

unsigned Level_GetID()
{
	if (BAKED_TRACK || HotReload_HasStagedTrack())
	{
		return CUSTOM_LEVEL_ID;
	}

	return ((sdata->gGT->gameMode1 & CRYSTAL_CHALLENGE) != 0) ? CRYSTAL_LEVEL_ID : RACE_LEVEL_ID;
}

int Level_IsPlayable(unsigned levelID)
{
	return (levelID == CUSTOM_LEVEL_ID) || (levelID == RACE_LEVEL_ID) || (levelID == CRYSTAL_LEVEL_ID);
}

void Level_CommitRequest()
{
	struct GameTracker* gGT = sdata->gGT;
	unsigned int levelID = sdata->Loading.Lev_ID_To_Load;

	int leavingForHub = (sdata->Loading.OnBegin.AddBitsConfig0 & ADVENTURE_ARENA) != 0;
	int unknownLevel = !Level_IsPlayable(levelID) && (levelID != MAIN_MENU_LEVEL);

	if (leavingForHub || unknownLevel)
	{
		sdata->Loading.Lev_ID_To_Load = MAIN_MENU_LEVEL;
		sdata->mainMenuState = MM_STATE_TITLE;
		GameMode_Clear();

		sdata->Loading.OnBegin.AddBitsConfig8 &= ~SPAWN_AT_BOSS;
		return;
	}

	if ((gGT->gameMode1 & CRYSTAL_CHALLENGE) != 0)
	{
		gGT->originalEventTime = Settings_GetCrystalTime();
	}

	if (levelID < NITRO_COURT)
	{
		for (int tier = 0; tier < SETTINGS_RELIC_TIERS; tier++)
		{
			data.RelicTime[levelID * SETTINGS_RELIC_TIERS + tier] = Settings_GetRelicTime(tier);
		}
	}
}
