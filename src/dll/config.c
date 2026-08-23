#include "boss.h"
#include "config.h"
#include "config_default.h"
#include "game_mode.h"
#include "main_menu.h"
#include "rom.h"
#include "settings.h"

#include <common.h>

_Static_assert(CONFIG_MODE_ARCADE == (1 << MODE_ARCADE), "mode bits drifted");
_Static_assert(CONFIG_MODE_RELIC_RACE == (1 << MODE_RELIC_RACE), "mode bits drifted");
_Static_assert(CONFIG_MODE_TIME_TRIAL == (1 << MODE_TIME_TRIAL), "mode bits drifted");
_Static_assert(CONFIG_MODE_CRYSTAL_CHALLENGE == (1 << MODE_CRYSTAL_CHALLENGE), "mode bits drifted");
_Static_assert(CONFIG_MODE_CTR_TOKEN == (1 << MODE_CTR_TOKEN), "mode bits drifted");
_Static_assert(CONFIG_MODE_BOSS_RACE == (1 << MODE_BOSS_RACE), "mode bits drifted");

_Static_assert(BEHAVIOR_RELIC_SAPPHIRE == (1 << SETTINGS_RELIC_SAPPHIRE), "behavior bits drifted");
_Static_assert(BEHAVIOR_LAPS == (1 << SETTINGS_LAPS), "behavior bits drifted");
_Static_assert(BEHAVIOR_TOKEN_COLOR == (1 << SETTINGS_CTR_TOKEN), "behavior bits drifted");
_Static_assert(BEHAVIOR_MAX_STATS == (1 << SETTINGS_MAX_STATS), "behavior bits drifted");
_Static_assert(BEHAVIOR_GHOST == (1 << SETTINGS_GHOST), "behavior bits drifted");
_Static_assert(BEHAVIOR_FREECAM == (1 << SETTINGS_FREECAM), "behavior bits drifted");
_Static_assert(BEHAVIOR_MODE_ARCADE == (1 << SETTINGS_MODE_ARCADE), "behavior bits drifted");
_Static_assert(BEHAVIOR_BOSS_NITROS_OXIDE == (1 << SETTINGS_BOSS_NITROS_OXIDE), "behavior bits drifted");
_Static_assert(BEHAVIOR_ALL == ((1 << SETTINGS_FIELD_COUNT) - 1), "behavior mask does not cover every field");

_Static_assert(FEATURE_FREECAM == (1 << (SETTINGS_FREECAM - SETTINGS_FEATURE_FIRST)), "feature run drifted");
_Static_assert(FEATURE_HOST_SETTINGS == (1 << (SETTINGS_HOST_SETTINGS - SETTINGS_FEATURE_FIRST)), "feature run drifted");
_Static_assert(FEATURE_MAX_STATS == (1 << (SETTINGS_MAX_STATS - SETTINGS_FEATURE_FIRST)), "feature run drifted");
_Static_assert(CONFIG_MODE_BOSS_RACE == (1 << (SETTINGS_MODE_BOSS_RACE - SETTINGS_MODE_FIRST)), "mode run drifted");
_Static_assert(CONFIG_BOSS_NITROS_OXIDE == (1 << (SETTINGS_BOSS_NITROS_OXIDE - SETTINGS_BOSS_FIRST)), "boss run drifted");

_Static_assert(CONFIG_BOSS_RIPPER_ROO == (1 << BOSS_RIPPER_ROO), "boss bits drifted");
_Static_assert(CONFIG_BOSS_NITROS_OXIDE == (1 << BOSS_NITROS_OXIDE), "boss bits drifted");

static Config config = CONFIG_DEFAULTS;
static ConfigStatus status = CONFIG_OK;

static ConfigStatus Config_Validate(const Config* loaded)
{
	if (loaded->magic != CONFIG_MAGIC)
	{
		return CONFIG_CORRUPT;
	}

	if (loaded->version > CONFIG_VERSION)
	{
		return CONFIG_TOO_NEW;
	}

	if (loaded->size < CONFIG_MIN_SIZE)
	{
		return CONFIG_CORRUPT;
	}

	return CONFIG_OK;
}

static void Config_Normalize()
{
	if (config.laps < CONFIG_LAPS_MIN)
	{
		config.laps = CONFIG_LAPS_MIN;
	}

	if (config.laps > CONFIG_LAPS_MAX)
	{
		config.laps = CONFIG_LAPS_MAX;
	}

	// round an even count up to the next legal one
	config.laps |= 1;

	if (config.ctrToken >= TOKEN_COLOR_COUNT)
	{
		config.ctrToken = TOKEN_YELLOW;
	}

	if (config.modes == 0)
	{
		config.modes = CONFIG_MODE_ALL;
	}

	if (config.bosses == 0)
	{
		config.bosses = CONFIG_BOSS_ALL;
	}
}

static void Config_Refresh()
{
	data.metaDataLEV[CUSTOM_LEVEL_ID].ctrTokenGroupID = config.ctrToken;

	Settings_ApplyCodePatches();
	MainMenu_Build();
	Boss_InstallRows();
}

void Config_Load()
{
	int size = 0;

	Config* loaded = (Config*)LOAD_XnfFile(CONFIG_PATH, CONFIG_ADDR, &size);

	if ((loaded == 0) || (size < CONFIG_MIN_SIZE))
	{
		status = CONFIG_MISSING;
	}
	else
	{
		status = Config_Validate(loaded);

		if (status == CONFIG_OK)
		{
			Config defaults = CONFIG_DEFAULTS;
			unsigned int copy = (loaded->size < sizeof(Config)) ? loaded->size : sizeof(Config);

			config = defaults;
			memcpy(&config, loaded, copy);
		}
	}

	Config_Normalize();
	Config_Refresh();
}

void Config_Set(const Config* next)
{
	config = *next;

	Config_Normalize();
	Config_Refresh();
}

int Config_IsValid(const Config* candidate)
{
	return Config_Validate(candidate) == CONFIG_OK;
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

ConfigStatus Config_GetStatus()
{
	return status;
}

const char* Config_GetStatusText()
{
	switch (status)
	{
	case CONFIG_MISSING: return "CONFIG.BIN NOT FOUND - USING DEFAULTS";
	case CONFIG_CORRUPT: return "CONFIG.BIN UNREADABLE - USING DEFAULTS";
	case CONFIG_TOO_NEW: return "CONFIG.BIN IS NEWER THAN THIS ROM - USING DEFAULTS";
	default: return 0;
	}
}
