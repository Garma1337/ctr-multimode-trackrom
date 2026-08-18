#ifndef GAME_MODE_H_MODEROM
#define GAME_MODE_H_MODEROM

#include <common.h>

#define LNG_ARCADE 0x4E
#define LNG_RELIC_RACE 0xB8
#define LNG_TIME_TRIAL 0x4D
#define LNG_CRYSTAL_CHALLENGE 0xBE
#define LNG_CTR_TOKEN 0x176
#define LNG_SETTINGS 0x51

#define RACE_LEVEL_ID    CRASH_COVE
#define CRYSTAL_LEVEL_ID SKULL_ROCK

typedef enum GameModeRow
{
	MODE_ARCADE = 0,
	MODE_RELIC_RACE,
	MODE_TIME_TRIAL,
	MODE_CRYSTAL_CHALLENGE,
	MODE_CTR_TOKEN,
	MODE_SETTINGS,
	MODE_COUNT,
} GameModeRow;

#define NUM_GRID_SLOTS 8

void GameMode_InstallLanguageStrings();
void GameMode_Select(struct RectMenu* mainMenu);
void GameMode_CommitLevelRequest();
void GameMode_ApplyStartingGrid();
int GameMode_NeedsArcadePack();
unsigned GameMode_GetLevelID();
int GameMode_IsPlayableLevel(unsigned levelID);

#endif
