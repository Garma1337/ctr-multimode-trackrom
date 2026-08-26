#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// Copy this file to src/user_config.h and edit that copy. It is not tracked by
// git, so nothing here conflicts when you pull. Define only what you change;
// everything else follows the defaults in src/config/config_default.h and rom.h.

// === Track ===

// #define BAKED_TRACK     1
// #define CUSTOM_LEVEL_ID 12   // POLAR_PASS; also retarget the bake lines in buildList.txt

// === Rules ===

// #define CFG_MODES    (CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE)
// #define CFG_LAPS     5
// #define CFG_CTR_TOKEN TOKEN_RED

// #define CFG_RELIC_SAPPHIRE 90000
// #define CFG_RELIC_GOLD     80000
// #define CFG_RELIC_PLATINUM 70000

// === What players may edit in the settings panel ===

// #define CFG_EDITABLE 0
// #define CFG_EDITABLE (OPTION_RELIC_SAPPHIRE | OPTION_RELIC_GOLD | OPTION_RELIC_PLATINUM)

// === Which tools ship ===

// #define CFG_FEATURES FEATURE_MAX_STATS

// === Boss item presets ===

// Replacing the list replaces it entirely; the first entry is what an
// unconfigured boss uses, so keep a sane one there.
// #define BOSS_ITEM_PRESET_LIST(X) \
// 	X(VANILLA,  "Vanilla",  BOSS_ITEMS_VANILLA,                       BOSS_JUICE_VANILLA) \
// 	X(NASTY,  "Nasty",  BOSS_ITEM_MISSILE | BOSS_ITEM_WARP_ORB, BOSS_JUICE_NONE)

// #define CFG_BOSS_ITEM_PRESET_1 BOSS_ITEM_PRESET_NASTY

#endif
