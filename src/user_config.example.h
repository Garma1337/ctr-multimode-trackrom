#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// Copy this file to src/user_config.h and edit that copy. It is not tracked by
// git, so nothing here conflicts when you pull. Define only what you change;
// everything else follows the defaults in src/config/config_default.h and rom.h.

// === Track ===

// #define BAKED_TRACK     1    // Baked-in track enabled
// #define CUSTOM_LEVEL_ID 12   // POLAR_PASS; also retarget the bake lines in buildList.txt

// === Rules ===

// #define CFG_MODES    (CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE) // Only enables Time Trial and Relic Race
// #define CFG_LAPS     5 // 5 lap races
// #define CFG_CTR_TOKEN TOKEN_RED // Set CTR token color to Red

// #define CFG_RELIC_SAPPHIRE 90000 // 1:30.00 for Sapphire
// #define CFG_RELIC_GOLD     80000 // 1:20.00 for Gold
// #define CFG_RELIC_PLATINUM 70000 // 1:10.00 for Platinum

// === What players may edit in the settings panel ===

// #define CFG_EDITABLE 0 // Nothing
// #define CFG_EDITABLE (OPTION_RELIC_SAPPHIRE | OPTION_RELIC_GOLD | OPTION_RELIC_PLATINUM) // Only the relic target times

// === Which tools ship ===

// #define CFG_FEATURES FEATURE_MAX_STATS // Only "max stats" feature enabled

// === Boss slots ===

// #define CFG_BOSS_CHARACTER_1 TINY_TIGER // Boss 1 (Ripper Roo) uses Tiny as character
// #define CFG_BOSS_CHARACTER_5 N_GIN      // Boss 5 (Oxide) uses N. Gin as character

// This table replaces the boss selection with new boss names.
// #define BOSS_LIST(X) \
// X(1, "Jak",      BossWeaponRoo)       \
// X(2, "Sonic",    BossWeaponPapu)      \
// X(3, "Link",     BossWeaponJoe)       \
// X(4, "Greymon",  BossWeaponPinstripe) \
// X(5, "Kong",     BossWeaponOxide)

// === Boss item presets ===

// Replacing the list replaces it entirely; the first entry is what an unconfigured boss uses, so keep a sane one there.
// #define BOSS_ITEM_PRESET_LIST(X) \
// 	X(VANILLA,  "Vanilla",  BOSS_ITEMS_VANILLA,                       BOSS_JUICE_VANILLA) \
// 	X(NASTY,    "Nasty",    BOSS_ITEM_MISSILE | BOSS_ITEM_WARP_ORB,   BOSS_JUICE_NONE)

// #define CFG_BOSS_ITEM_PRESET_1 BOSS_ITEM_PRESET_NASTY // Use preset "Nasty" for Boss 1 (Ripper Roo)

#endif
