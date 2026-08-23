#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_MAGIC 0x4746434D // 'MCFG'
#define CONFIG_VERSION 1
#define CONFIG_FILE_SIZE 256
#define CONFIG_LAPS_MIN 1
#define CONFIG_LAPS_MAX 7

typedef enum FeatureFlag
{
	FEATURE_FREECAM = 1 << 0,
	FEATURE_DEBUG_HUD = 1 << 1,
	FEATURE_RESERVES = 1 << 2,
	FEATURE_HOT_RELOAD = 1 << 3,
	FEATURE_HOST_SETTINGS = 1 << 4,
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

typedef enum BehaviorFlag
{
	BEHAVIOR_RELIC_SAPPHIRE = 1 << 0,
	BEHAVIOR_RELIC_GOLD = 1 << 1,
	BEHAVIOR_RELIC_PLATINUM = 1 << 2,
	BEHAVIOR_CRYSTAL_TIME = 1 << 3,
	BEHAVIOR_LAPS = 1 << 4,
	BEHAVIOR_INTRO_CUTSCENE = 1 << 5,
	BEHAVIOR_GHOST = 1 << 6,

	BEHAVIOR_FREECAM = 1 << 7,
	BEHAVIOR_DEBUG_HUD = 1 << 8,
	BEHAVIOR_RESERVES = 1 << 9,
	BEHAVIOR_HOT_RELOAD = 1 << 10,
	BEHAVIOR_HOST_SETTINGS = 1 << 11,

	BEHAVIOR_MODE_ARCADE = 1 << 12,
	BEHAVIOR_MODE_RELIC_RACE = 1 << 13,
	BEHAVIOR_MODE_TIME_TRIAL = 1 << 14,
	BEHAVIOR_MODE_CRYSTAL_CHALLENGE = 1 << 15,
	BEHAVIOR_MODE_CTR_TOKEN = 1 << 16,
	BEHAVIOR_MODE_BOSS_RACE = 1 << 17,

	BEHAVIOR_BOSS_RIPPER_ROO = 1 << 18,
	BEHAVIOR_BOSS_PAPU_PAPU = 1 << 19,
	BEHAVIOR_BOSS_KOMODO_JOE = 1 << 20,
	BEHAVIOR_BOSS_PINSTRIPE = 1 << 21,
	BEHAVIOR_BOSS_NITROS_OXIDE = 1 << 22,

	BEHAVIOR_ALL = 0x7FFFFF,
} BehaviorFlag;

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
} Config;

#if defined(__cplusplus)
static_assert(sizeof(Config) == 40, "CONFIG.BIN layout changed; bump CONFIG_VERSION");
#else
_Static_assert(sizeof(Config) == 40, "CONFIG.BIN layout changed; bump CONFIG_VERSION");
#endif

#endif
