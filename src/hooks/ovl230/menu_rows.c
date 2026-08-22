#include "rom/game_mode.h"

#include <common.h>

struct MenuRow NewRowsMM[MODE_COUNT + 1] =
{
	[MODE_ARCADE] =
	{
		.stringIndex = LNG_ARCADE,
		.rowOnPressUp = MODE_ARCADE,
		.rowOnPressDown = MODE_RELIC_RACE,
		.rowOnPressLeft = MODE_ARCADE,
		.rowOnPressRight = MODE_ARCADE,
	},

	[MODE_RELIC_RACE] =
	{
		.stringIndex = LNG_RELIC_RACE,
		.rowOnPressUp = MODE_ARCADE,
		.rowOnPressDown = MODE_TIME_TRIAL,
		.rowOnPressLeft = MODE_RELIC_RACE,
		.rowOnPressRight = MODE_RELIC_RACE,
	},

	[MODE_TIME_TRIAL] =
	{
		.stringIndex = LNG_TIME_TRIAL,
		.rowOnPressUp = MODE_RELIC_RACE,
		.rowOnPressDown = MODE_CRYSTAL_CHALLENGE,
		.rowOnPressLeft = MODE_TIME_TRIAL,
		.rowOnPressRight = MODE_TIME_TRIAL,
	},

	[MODE_CRYSTAL_CHALLENGE] =
	{
		.stringIndex = LNG_CRYSTAL_CHALLENGE,
		.rowOnPressUp = MODE_TIME_TRIAL,
		.rowOnPressDown = MODE_CTR_TOKEN,
		.rowOnPressLeft = MODE_CRYSTAL_CHALLENGE,
		.rowOnPressRight = MODE_CRYSTAL_CHALLENGE,
	},

	[MODE_CTR_TOKEN] =
	{
		.stringIndex = LNG_CTR_TOKEN,
		.rowOnPressUp = MODE_CRYSTAL_CHALLENGE,
		.rowOnPressDown = MODE_BOSS_RACE,
		.rowOnPressLeft = MODE_CTR_TOKEN,
		.rowOnPressRight = MODE_CTR_TOKEN,
	},

	[MODE_BOSS_RACE] =
	{
		.stringIndex = LNG_BOSS_RACE,
		.rowOnPressUp = MODE_CTR_TOKEN,
		.rowOnPressDown = MODE_SETTINGS,
		.rowOnPressLeft = MODE_BOSS_RACE,
		.rowOnPressRight = MODE_BOSS_RACE,
	},

	[MODE_SETTINGS] =
	{
		.stringIndex = LNG_SETTINGS,
		.rowOnPressUp = MODE_BOSS_RACE,
		.rowOnPressDown = MODE_SETTINGS,
		.rowOnPressLeft = MODE_SETTINGS,
		.rowOnPressRight = MODE_SETTINGS,
	},

	// NULL, end of menu
	[MODE_COUNT] =
	{
		.stringIndex = 0xFFFF,
	}
};
