#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_MAGIC 0x4746434D // 'MCFG'
#define CONFIG_VERSION 3
#define CONFIG_LAPS_MIN 1
#define CONFIG_LAPS_MAX 7
#define CONFIG_MIN_SIZE 8

#define GHOST_SLOT_COUNT 2
#define DRIVER_COUNT 16
#define GHOST_CHARACTER_COUNT DRIVER_COUNT

#define GHOST_CHARACTER_1_DEFAULT 12 // N. Tropy
#define GHOST_CHARACTER_2_DEFAULT 15 // Oxide

#define BOSS_CHARACTER_1_DEFAULT 10 // Ripper Roo
#define BOSS_CHARACTER_2_DEFAULT  9 // Papu Papu
#define BOSS_CHARACTER_3_DEFAULT 11 // Komodo Joe
#define BOSS_CHARACTER_4_DEFAULT  8 // Pinstripe
#define BOSS_CHARACTER_5_DEFAULT 15 // Oxide

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

#define OPTION_MODE_ARCADE            (1ULL << 0)
#define OPTION_MODE_RELIC_RACE        (1ULL << 1)
#define OPTION_MODE_TIME_TRIAL        (1ULL << 2)
#define OPTION_MODE_CRYSTAL_CHALLENGE (1ULL << 3)
#define OPTION_MODE_CTR_TOKEN         (1ULL << 4)
#define OPTION_MODE_BOSS_RACE         (1ULL << 5)

#define OPTION_LAPS                   (1ULL << 6)

#define OPTION_RELIC_SAPPHIRE         (1ULL << 7)
#define OPTION_RELIC_GOLD             (1ULL << 8)
#define OPTION_RELIC_PLATINUM         (1ULL << 9)

#define OPTION_CRYSTAL_TIME           (1ULL << 10)

#define OPTION_TOKEN_COLOR            (1ULL << 11)

#define OPTION_GHOST                  (1ULL << 12)
#define OPTION_GHOST_TIME_1           (1ULL << 13)
#define OPTION_GHOST_CHARACTER_1      (1ULL << 14)
#define OPTION_GHOST_TIME_2           (1ULL << 15)
#define OPTION_GHOST_CHARACTER_2      (1ULL << 16)

#define OPTION_BOSS_1_ENABLED         (1ULL << 17)
#define OPTION_BOSS_1_CHARACTER       (1ULL << 18)
#define OPTION_BOSS_1_ITEM_PRESET     (1ULL << 19)

#define OPTION_BOSS_2_ENABLED         (1ULL << 20)
#define OPTION_BOSS_2_CHARACTER       (1ULL << 21)
#define OPTION_BOSS_2_ITEM_PRESET     (1ULL << 22)

#define OPTION_BOSS_3_ENABLED         (1ULL << 23)
#define OPTION_BOSS_3_CHARACTER       (1ULL << 24)
#define OPTION_BOSS_3_ITEM_PRESET     (1ULL << 25)

#define OPTION_BOSS_4_ENABLED         (1ULL << 26)
#define OPTION_BOSS_4_CHARACTER       (1ULL << 27)
#define OPTION_BOSS_4_ITEM_PRESET     (1ULL << 28)

#define OPTION_BOSS_5_ENABLED         (1ULL << 29)
#define OPTION_BOSS_5_CHARACTER       (1ULL << 30)
#define OPTION_BOSS_5_ITEM_PRESET     (1ULL << 31)

#define OPTION_INTRO_CUTSCENE         (1ULL << 32)
#define OPTION_HIGH_LOD               (1ULL << 33)

#define OPTION_FREECAM                (1ULL << 34)
#define OPTION_DEBUG_HUD              (1ULL << 35)
#define OPTION_RESERVES               (1ULL << 36)
#define OPTION_HOT_RELOAD             (1ULL << 37)
#define OPTION_HOST_SETTINGS          (1ULL << 38)
#define OPTION_MAX_STATS              (1ULL << 39)
#define OPTION_ALL 0xFFFFFFFFFFULL

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
	CONFIG_BOSS_1 = 1 << 0,
	CONFIG_BOSS_2 = 1 << 1,
	CONFIG_BOSS_3 = 1 << 2,
	CONFIG_BOSS_4 = 1 << 3,
	CONFIG_BOSS_5 = 1 << 4,

	CONFIG_BOSS_ALL = 0x1F,
} BossFlag;

#define CONFIG_BOSS_COUNT 5

#define BOSS_ITEM_TURBO         (1 << 0)
#define BOSS_ITEM_BOMB          (1 << 1)
#define BOSS_ITEM_MISSILE       (1 << 2)
#define BOSS_ITEM_TNT           (1 << 3)
#define BOSS_ITEM_BEAKER        (1 << 4)
#define BOSS_ITEM_SHIELD        (1 << 5)
#define BOSS_ITEM_MASK          (1 << 6)
#define BOSS_ITEM_CLOCK         (1 << 7)
#define BOSS_ITEM_WARP_ORB      (1 << 8)
#define BOSS_ITEM_INVISIBILITY  (1 << 9)
#define BOSS_ITEM_SUPER_ENGINE  (1 << 10)
#define BOSS_ITEM_COUNT 11
#define BOSS_ITEMS_VANILLA 0

typedef enum BossJuice
{
	BOSS_JUICE_VANILLA = 0,
	BOSS_JUICE_NONE,
	BOSS_JUICE_ALWAYS,
	BOSS_JUICE_RANDOM,
	BOSS_JUICE_COUNT,
} BossJuice;

typedef struct Config
{
	unsigned int magic;
	unsigned short version;
	unsigned short size;

	unsigned long long editable;

	unsigned int features;
	unsigned int modes;

	int relicSapphire; // ms
	int relicGold;
	int relicPlatinum;
	int crystalTime;
	int ghostTime[GHOST_SLOT_COUNT];

	unsigned char laps;
	unsigned char introCutscene;
	unsigned char ghosts;
	unsigned char highLod;
	unsigned char bosses;
	unsigned char ctrToken; // TokenColor
	unsigned char ghostCharacter[GHOST_SLOT_COUNT];
	unsigned char bossCharacter[CONFIG_BOSS_COUNT];
	unsigned char bossItemPreset[CONFIG_BOSS_COUNT]; // index into BOSS_ITEM_PRESET_LIST
} Config;

#if defined(__cplusplus)
static_assert(sizeof(Config) == 72, "Config layout changed; bump CONFIG_VERSION and rebuild the editor");
#else
_Static_assert(sizeof(Config) == 72, "Config layout changed; bump CONFIG_VERSION and rebuild the editor");
#endif

#endif
