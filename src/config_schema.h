#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_MAGIC 0x4746434D // 'MCFG'
#define CONFIG_VERSION 1
#define CONFIG_FILE_SIZE 256
#define CONFIG_LAPS_MIN 1
#define CONFIG_LAPS_MAX 7
#define CONFIG_MIN_SIZE 8

typedef enum FeatureFlag
{
	FEATURE_FREECAM = 1 << 0,
	FEATURE_DEBUG_HUD = 1 << 1,
	FEATURE_RESERVES = 1 << 2,
	FEATURE_HOT_RELOAD = 1 << 3,
	FEATURE_HOST_SETTINGS = 1 << 4,
	FEATURE_MAX_STATS = 1 << 5,
} FeatureFlag;

typedef enum ModeFlag
{
	CONFIG_MODE_ARCADE = 1 << 0,
	CONFIG_MODE_RELIC_RACE = 1 << 1,
	CONFIG_MODE_TIME_TRIAL = 1 << 2,
	CONFIG_MODE_CRYSTAL_CHALLENGE = 1 << 3,
	CONFIG_MODE_CTR_TOKEN = 1 << 4,
	CONFIG_MODE_BOSS_RACE = 1 << 5,

	CONFIG_MODE_ALL = 0x3F,
} ModeFlag;

typedef enum SettingOptionFlag
{
	OPTION_RELIC_SAPPHIRE = 1 << 0,
	OPTION_RELIC_GOLD = 1 << 1,
	OPTION_RELIC_PLATINUM = 1 << 2,
	OPTION_CRYSTAL_TIME = 1 << 3,
	OPTION_LAPS = 1 << 4,
	OPTION_INTRO_CUTSCENE = 1 << 5,
	OPTION_GHOST = 1 << 6,

	OPTION_TOKEN_COLOR = 1 << 7,

	OPTION_FREECAM = 1 << 8,
	OPTION_DEBUG_HUD = 1 << 9,
	OPTION_RESERVES = 1 << 10,
	OPTION_HOT_RELOAD = 1 << 11,
	OPTION_HOST_SETTINGS = 1 << 12,
	OPTION_MAX_STATS = 1 << 13,

	OPTION_MODE_ARCADE = 1 << 14,
	OPTION_MODE_RELIC_RACE = 1 << 15,
	OPTION_MODE_TIME_TRIAL = 1 << 16,
	OPTION_MODE_CRYSTAL_CHALLENGE = 1 << 17,
	OPTION_MODE_CTR_TOKEN = 1 << 18,
	OPTION_MODE_BOSS_RACE = 1 << 19,

	OPTION_BOSS_RIPPER_ROO = 1 << 20,
	OPTION_BOSS_PAPU_PAPU = 1 << 21,
	OPTION_BOSS_KOMODO_JOE = 1 << 22,
	OPTION_BOSS_PINSTRIPE = 1 << 23,
	OPTION_BOSS_NITROS_OXIDE = 1 << 24,

	OPTION_ALL = 0x1FFFFFF,
} SettingOptionFlag;

typedef enum TokenColor
{
	TOKEN_RED = 0,
	TOKEN_GREEN,
	TOKEN_BLUE,
	TOKEN_YELLOW,
	TOKEN_PURPLE,
	TOKEN_COLOR_COUNT,
} TokenColor;

typedef enum BossFlag
{
	CONFIG_BOSS_RIPPER_ROO = 1 << 0,
	CONFIG_BOSS_PAPU_PAPU = 1 << 1,
	CONFIG_BOSS_KOMODO_JOE = 1 << 2,
	CONFIG_BOSS_PINSTRIPE = 1 << 3,
	CONFIG_BOSS_NITROS_OXIDE = 1 << 4,

	CONFIG_BOSS_ALL = 0x1F,
} BossFlag;

typedef struct Config
{
	unsigned int magic;
	unsigned short version;
	unsigned short size;

	unsigned int features;
	unsigned int modes;
	unsigned int editable;

	int relicSapphire; // ms
	int relicGold;
	int relicPlatinum;
	int crystalTime;

	unsigned char laps;
	unsigned char introCutscene;
	unsigned char ghosts;
	unsigned char bosses;
	unsigned char ctrToken; // TokenColor
} Config;

#if defined(__cplusplus)
static_assert(sizeof(Config) == 44, "CONFIG.BIN layout changed; bump CONFIG_VERSION");
#else
_Static_assert(sizeof(Config) == 44, "CONFIG.BIN layout changed; bump CONFIG_VERSION");
#endif

#endif
