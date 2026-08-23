#include "game_mode.h"

#include <common.h>

void GameMode_Clear()
{
	struct GameTracker* gGT = sdata->gGT;

	gGT->gameMode1 &= ~MODE_FLAGS1;
	gGT->gameMode2 &= ~MODE_FLAGS2;
}

void GameMode_Apply(int mode)
{
	struct GameTracker* gGT = sdata->gGT;

	switch (mode)
	{
	case MODE_ARCADE:
		gGT->gameMode1 |= ARCADE_MODE;
		break;

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
		break;
	}
}

int GameMode_NeedsArcadePack()
{
	return (sdata->gGT->gameMode1 & (TIME_TRIAL | RELIC_RACE | ADVENTURE_BOSS)) == 0;
}
