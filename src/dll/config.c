#include "boss.h"
#include "config.h"
#include "../config/config_default.h"
#include "game_mode.h"
#include "ghost.h"
#include "main_menu.h"
#include "../rom.h"
#include "settings.h"

#include <common.h>

_Static_assert(CONFIG_MODE_ARCADE == (1 << MODE_ARCADE), "mode bits drifted");
_Static_assert(CONFIG_MODE_RELIC_RACE == (1 << MODE_RELIC_RACE), "mode bits drifted");
_Static_assert(CONFIG_MODE_TIME_TRIAL == (1 << MODE_TIME_TRIAL), "mode bits drifted");
_Static_assert(CONFIG_MODE_CRYSTAL_CHALLENGE == (1 << MODE_CRYSTAL_CHALLENGE), "mode bits drifted");
_Static_assert(CONFIG_MODE_CTR_TOKEN == (1 << MODE_CTR_TOKEN), "mode bits drifted");
_Static_assert(CONFIG_MODE_BOSS_RACE == (1 << MODE_BOSS_RACE), "mode bits drifted");

_Static_assert(OPTION_RELIC_SAPPHIRE == (1ULL << SETTINGS_RELIC_SAPPHIRE), "setting option bits drifted");
_Static_assert(OPTION_GHOST_TIME_1 == (1ULL << SETTINGS_GHOST_TIME_1), "setting option bits drifted");
_Static_assert(OPTION_GHOST_CHARACTER_1 == (1ULL << SETTINGS_GHOST_CHARACTER_1), "setting option bits drifted");
_Static_assert(OPTION_GHOST_CHARACTER_2 == (1ULL << SETTINGS_GHOST_CHARACTER_2), "setting option bits drifted");
_Static_assert(OPTION_LAPS == (1ULL << SETTINGS_LAPS), "setting option bits drifted");
_Static_assert(OPTION_TOKEN_COLOR == (1ULL << SETTINGS_CTR_TOKEN), "setting option bits drifted");
_Static_assert(OPTION_MAX_STATS == (1ULL << SETTINGS_MAX_STATS), "setting option bits drifted");
_Static_assert(OPTION_GHOST == (1ULL << SETTINGS_GHOST), "setting option bits drifted");
_Static_assert(OPTION_HIGH_LOD == (1ULL << SETTINGS_HIGH_LOD), "setting option bits drifted");
_Static_assert(OPTION_FREECAM == (1ULL << SETTINGS_FREECAM), "setting option bits drifted");
_Static_assert(OPTION_MODE_ARCADE == (1ULL << SETTINGS_MODE_ARCADE), "setting option bits drifted");
_Static_assert(OPTION_BOSS_1_ENABLED == (1ULL << SETTINGS_BOSS_1_ENABLED), "setting option bits drifted");
_Static_assert(OPTION_BOSS_1_CHARACTER == (1ULL << SETTINGS_BOSS_1_CHARACTER), "setting option bits drifted");
_Static_assert(OPTION_BOSS_1_ITEM_PRESET == (1ULL << SETTINGS_BOSS_1_ITEM_PRESET), "setting option bits drifted");
_Static_assert(OPTION_BOSS_5_ITEM_PRESET == (1ULL << SETTINGS_BOSS_5_ITEM_PRESET), "setting option bits drifted");
_Static_assert(OPTION_ALL == ((1ULL << SETTINGS_FIELD_COUNT) - 1), "option mask does not cover every field");
_Static_assert(CONFIG_BOSS_COUNT == BOSS_COUNT, "boss count drifted");

_Static_assert(FEATURE_FREECAM == (1 << (SETTINGS_FREECAM - SETTINGS_FEATURE_FIRST)), "feature run drifted");
_Static_assert(FEATURE_HOST_SETTINGS == (1 << (SETTINGS_HOST_SETTINGS - SETTINGS_FEATURE_FIRST)), "feature run drifted");
_Static_assert(FEATURE_MAX_STATS == (1 << (SETTINGS_MAX_STATS - SETTINGS_FEATURE_FIRST)), "feature run drifted");
_Static_assert(CONFIG_MODE_BOSS_RACE == (1 << (SETTINGS_MODE_BOSS_RACE - SETTINGS_MODE_FIRST)), "mode run drifted");
_Static_assert(SETTINGS_BOSS_END - SETTINGS_BOSS_FIRST == CONFIG_BOSS_COUNT * SETTINGS_BOSS_ROW_COUNT, "boss run drifted");

_Static_assert(CONFIG_BOSS_1 == (1 << BOSS_1), "boss bits drifted");
_Static_assert(CONFIG_BOSS_5 == (1 << BOSS_5), "boss bits drifted");

static const unsigned char ghostCharacterDefault[GHOST_SLOT_COUNT] = {
	GHOST_CHARACTER_1_DEFAULT,
	GHOST_CHARACTER_2_DEFAULT,
};

static const unsigned char bossCharacterDefault[CONFIG_BOSS_COUNT] = {
	BOSS_CHARACTER_1_DEFAULT,
	BOSS_CHARACTER_2_DEFAULT,
	BOSS_CHARACTER_3_DEFAULT,
	BOSS_CHARACTER_4_DEFAULT,
	BOSS_CHARACTER_5_DEFAULT,
};

static Config config = CONFIG_DEFAULTS;

void Config_Normalize(Config* config)
{
	if (config->laps < CONFIG_LAPS_MIN)
	{
		config->laps = CONFIG_LAPS_MIN;
	}

	if (config->laps > CONFIG_LAPS_MAX)
	{
		config->laps = CONFIG_LAPS_MAX;
	}

	// round an even count up to the next legal one
	config->laps |= 1;

	if (config->ctrToken >= TOKEN_COLOR_COUNT)
	{
		config->ctrToken = TOKEN_YELLOW;
	}

	for (int slot = 0; slot < GHOST_SLOT_COUNT; slot++)
	{
		if (config->ghostCharacter[slot] >= GHOST_CHARACTER_COUNT)
		{
			config->ghostCharacter[slot] = ghostCharacterDefault[slot];
		}
	}

	for (int boss = 0; boss < CONFIG_BOSS_COUNT; boss++)
	{
		if (config->bossCharacter[boss] >= DRIVER_COUNT)
		{
			config->bossCharacter[boss] = bossCharacterDefault[boss];
		}

		if (config->bossItemPreset[boss] >= BOSS_ITEM_PRESET_COUNT)
		{
			config->bossItemPreset[boss] = 0;
		}
	}

	if (config->modes == 0)
	{
		config->modes = CONFIG_MODE_ALL;
	}

	if (config->bosses == 0)
	{
		config->bosses = CONFIG_BOSS_ALL;
	}
}

static void Config_Refresh()
{
	data.metaDataLEV[CUSTOM_LEVEL_ID].ctrTokenGroupID = config.ctrToken;

	Ghost_AdoptDrivers();
	Ghost_ApplyTargets();
	Settings_ApplyCodePatches();
	MainMenu_Build();
	Boss_InstallRows();
}

void Config_Init()
{
	Config_Normalize(&config);
	Config_Refresh();
}

void Config_Set(const Config* next)
{
	config = *next;

	Config_Normalize(&config);
	Config_Refresh();
}

int Config_IsValid(const Config* candidate)
{
	return (candidate->magic == CONFIG_MAGIC) &&
	       (candidate->version <= CONFIG_VERSION) &&
	       (candidate->size >= CONFIG_MIN_SIZE);
}

const Config* Config_Get()
{
	return &config;
}

int Config_IsFeatureEnabled(int feature)
{
	return (config.features & feature) != 0;
}

int Config_IsModeEnabled(int mode)
{
	return (config.modes & (1 << mode)) != 0;
}

int Config_IsBossEnabled(int boss)
{
	return (config.bosses & (1 << boss)) != 0;
}
