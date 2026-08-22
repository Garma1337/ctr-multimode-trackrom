#include "dll/boss.h"
#include "dll/hot_reload.h"
#include "dll/settings.h"
#include "game_mode.h"
#include "rom.h"

#include <common.h>


#define MODE_FLAGS1 \
	(BATTLE_MODE | TIME_TRIAL | ARCADE_MODE | RELIC_RACE | CRYSTAL_CHALLENGE | \
	 ADVENTURE_MODE | ADVENTURE_ARENA | ADVENTURE_CUP | ADVENTURE_BOSS)

#define MODE_FLAGS2 (CUP_ANY_KIND | TOKEN_RACE | SPAWN_AT_BOSS)

void MM_MenuProc_Difficulty(struct RectMenu* menu);

static void OpenDifficultySelect(struct RectMenu* mainMenu)
{
	struct RectMenu* menu = &D230.menuDifficulty;

	menu->stringIndexTitle = LNG_DIFFICULTY;
	menu->rows = &D230.rowsDifficulty[0];
	menu->funcPtr = &MM_MenuProc_Difficulty;

	mainMenu->ptrNextBox_InHierarchy = menu;
	mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
}

void GameMode_InstallLanguageStrings()
{
	// cba to adjust the main menu, so I'm shortening "Crystal Challenge" to "Crystal Race"
	sdata->lngStrings[LNG_CRYSTAL_CHALLENGE] = (char*)"CRYSTAL RACE";
	sdata->lngStrings[LNG_BOSS_RACE] = (char*)"BOSS RACE";
	sdata->lngStrings[LNG_SETTINGS] = (char*)"SETTINGS";
}

void GameMode_Select(struct RectMenu* mainMenu)
{
	struct GameTracker* gGT = sdata->gGT;

	if (mainMenu->rowSelected == MODE_SETTINGS)
	{
		Settings_Open(mainMenu);
		return;
	}

	gGT->gameMode1 &= ~MODE_FLAGS1;
	gGT->gameMode2 &= ~(MODE_FLAGS2 | CHEAT_ALL);

	gGT->numPlyrNextGame = 1;
	gGT->numLaps = 3;
	sdata->gameProgress.unlocks[0] |= UNLOCK_CHARACTERS;

	mainMenu->state |= ONLY_DRAW_TITLE;

	switch (mainMenu->rowSelected)
	{
	case MODE_ARCADE:
		gGT->gameMode1 |= ARCADE_MODE;

		OpenDifficultySelect(mainMenu);
		return;

	case MODE_RELIC_RACE:
		gGT->gameMode1 |= RELIC_RACE;
		break;

	case MODE_TIME_TRIAL:
		gGT->gameMode1 |= TIME_TRIAL;
		break;

	case MODE_CRYSTAL_CHALLENGE:
		gGT->gameMode1 |= CRYSTAL_CHALLENGE;
		break;

	case MODE_CTR_TOKEN:
		gGT->gameMode1 |= ADVENTURE_MODE;
		gGT->gameMode2 |= TOKEN_RACE;
		break;

	case MODE_BOSS_RACE:
		gGT->gameMode1 |= (ADVENTURE_MODE | ADVENTURE_BOSS);

		Boss_OpenSelect(mainMenu);
		return;

	default:
		return;
	}

	GameMode_EnterCharacterSelect();
}

void GameMode_EnterCharacterSelect()
{
	D230.MM_State = 2;
	D230.desiredMenuIndex = 2;
}

void GameMode_CommitLevelRequest()
{
	struct GameTracker* gGT = sdata->gGT;
	unsigned int levelID = sdata->Loading.Lev_ID_To_Load;

	int leavingForHub = (sdata->Loading.OnBegin.AddBitsConfig0 & ADVENTURE_ARENA) != 0;
	int unknownLevel = !GameMode_IsPlayableLevel(levelID) && (levelID != MAIN_MENU_LEVEL);

	if (leavingForHub || unknownLevel)
	{
		sdata->Loading.Lev_ID_To_Load = MAIN_MENU_LEVEL;
		sdata->mainMenuState = MM_STATE_TITLE;
		gGT->gameMode1 &= ~MODE_FLAGS1;
		gGT->gameMode2 &= ~MODE_FLAGS2;
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

void GameMode_ApplyStartingGrid()
{
	int playerLast = (sdata->gGT->gameMode2 & TOKEN_RACE) != 0;

	for (int i = 0; i < NUM_GRID_SLOTS; i++)
	{
		sdata->kartSpawnOrderArray[i] = playerLast ? (char)((i + NUM_GRID_SLOTS - 1) % NUM_GRID_SLOTS) : (char)i;
	}
}

int GameMode_NeedsArcadePack()
{
	return (sdata->gGT->gameMode1 & (TIME_TRIAL | RELIC_RACE | ADVENTURE_BOSS)) == 0;
}

unsigned GameMode_GetLevelID()
{
	if (HotReload_HasStagedTrack())
	{
		return CUSTOM_LEVEL_ID;
	}

	return ((sdata->gGT->gameMode1 & CRYSTAL_CHALLENGE) != 0) ? CRYSTAL_LEVEL_ID : RACE_LEVEL_ID;
}

int GameMode_IsPlayableLevel(unsigned levelID)
{
	return (levelID == CUSTOM_LEVEL_ID) || (levelID == RACE_LEVEL_ID) || (levelID == CRYSTAL_LEVEL_ID);
}
