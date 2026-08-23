#include "boss.h"
#include "config.h"
#include "game_mode.h"
#include "main_menu.h"
#include "settings.h"

#include <common.h>

#define ROW_TERMINATOR (-1)

static struct MenuRow rows[MAIN_MENU_MAX_ENTRIES + 1];
static unsigned char entryId[MAIN_MENU_MAX_ENTRIES];
static int entryCount;

void MM_MenuProc_Difficulty(struct RectMenu* menu);

static void MainMenu_OpenDifficultySelect(struct RectMenu* mainMenu)
{
	struct RectMenu* menu = &D230.menuDifficulty;

	menu->stringIndexTitle = LNG_DIFFICULTY;
	menu->rows = &D230.rowsDifficulty[0];
	menu->funcPtr = &MM_MenuProc_Difficulty;

	mainMenu->ptrNextBox_InHierarchy = menu;
	mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
}

void MainMenu_Reset()
{
	entryCount = 0;
	rows[0].stringIndex = ROW_TERMINATOR;
}

void MainMenu_AddEntry(int id, short stringIndex)
{
	if (entryCount >= MAIN_MENU_MAX_ENTRIES)
	{
		return;
	}

	int row = entryCount++;

	entryId[row] = (unsigned char)id;

	rows[row].stringIndex = stringIndex;
	rows[row].rowOnPressUp = (char)((row == 0) ? 0 : (row - 1));
	rows[row].rowOnPressLeft = (char)row;
	rows[row].rowOnPressRight = (char)row;
	rows[row].rowOnPressDown = (char)row;

	if (row > 0)
	{
		rows[row - 1].rowOnPressDown = (char)row;
	}

	rows[entryCount].stringIndex = ROW_TERMINATOR;
}

void MainMenu_Attach(struct RectMenu* mainMenu)
{
	mainMenu->rows = &rows[0];
	if (mainMenu->rowSelected >= entryCount)
	{
		mainMenu->rowSelected = (short)(entryCount - 1);
	}
}

int MainMenu_GetEntryId(int row)
{
	if ((row < 0) || (row >= entryCount))
	{
		return MAIN_MENU_NO_ENTRY;
	}

	return entryId[row];
}

void MainMenu_InstallStrings()
{
	// cba to adjust the main menu, so I'm shortening "Crystal Challenge" to "Crystal Race"
	sdata->lngStrings[LNG_CRYSTAL_CHALLENGE] = (char*)"CRYSTAL RACE";
	sdata->lngStrings[LNG_BOSS_RACE] = (char*)"BOSS RACE";
	sdata->lngStrings[LNG_SETTINGS] = (char*)"SETTINGS";
}

static void MainMenu_AddMode(int mode, short stringIndex)
{
	if (Config_IsModeEnabled(mode))
	{
		MainMenu_AddEntry(mode, stringIndex);
	}
}

void MainMenu_Build()
{
	MainMenu_Reset();

	MainMenu_AddMode(MODE_ARCADE, LNG_ARCADE);
	MainMenu_AddMode(MODE_RELIC_RACE, LNG_RELIC_RACE);
	MainMenu_AddMode(MODE_TIME_TRIAL, LNG_TIME_TRIAL);
	MainMenu_AddMode(MODE_CRYSTAL_CHALLENGE, LNG_CRYSTAL_CHALLENGE);
	MainMenu_AddMode(MODE_CTR_TOKEN, LNG_CTR_TOKEN);
	MainMenu_AddMode(MODE_BOSS_RACE, LNG_BOSS_RACE);

	if (Settings_IsAvailable())
	{
		MainMenu_AddEntry(MODE_SETTINGS, LNG_SETTINGS);
	}
}

void MainMenu_Select(struct RectMenu* mainMenu)
{
	struct GameTracker* gGT = sdata->gGT;

	int mode = MainMenu_GetEntryId(mainMenu->rowSelected);
	if (mode == MAIN_MENU_NO_ENTRY)
	{
		return;
	}

	if (mode == MODE_SETTINGS)
	{
		Settings_Open(mainMenu);
		return;
	}

	GameMode_Clear();
	gGT->gameMode2 &= ~CHEAT_ALL;

	gGT->numPlyrNextGame = 1;
	gGT->numLaps = Config_Get()->laps;
	sdata->gameProgress.unlocks[0] |= UNLOCK_CHARACTERS;

	mainMenu->state |= ONLY_DRAW_TITLE;

	GameMode_Apply(mode);

	if (mode == MODE_ARCADE)
	{
		MainMenu_OpenDifficultySelect(mainMenu);
		return;
	}

	if (mode == MODE_BOSS_RACE)
	{
		Boss_OpenSelect(mainMenu);
		return;
	}

	MainMenu_EnterCharacterSelect();
}

void MainMenu_EnterCharacterSelect()
{
	D230.MM_State = 2;
	D230.desiredMenuIndex = 2;
}
