#ifndef BOSS_ITEM_PRESETS_H
#define BOSS_ITEM_PRESETS_H

#include "config_schema.h"
#include "overrides.h"

#ifndef BOSS_ITEM_PRESET_LIST
#define BOSS_ITEM_PRESET_LIST(X) \
	X(VANILLA,    "Vanilla",          BOSS_ITEMS_VANILLA,                                                                BOSS_JUICE_VANILLA) \
	X(TNT,        "TNT",              BOSS_ITEM_TNT,                                                                     BOSS_JUICE_VANILLA) \
	X(BOMBS,      "Bombs",            BOSS_ITEM_BOMB,                                                                    BOSS_JUICE_VANILLA) \
	X(BEAKERS,    "Beakers",          BOSS_ITEM_BEAKER,                                                                  BOSS_JUICE_VANILLA) \
	X(MISSILES,   "Missiles",         BOSS_ITEM_MISSILE,                                                                 BOSS_JUICE_NONE) \
	X(CLASSIC,    "Classic",          BOSS_ITEM_TNT | BOSS_ITEM_BEAKER | BOSS_ITEM_BOMB,                                 BOSS_JUICE_RANDOM) \
	X(RUTHLESS,   "Ruthless",         BOSS_ITEM_MISSILE | BOSS_ITEM_WARP_ORB | BOSS_ITEM_CLOCK | BOSS_ITEM_INVISIBILITY, BOSS_JUICE_ALWAYS) \
    X(DEFENSIVE,  "Defensive",        BOSS_ITEM_TNT | BOSS_ITEM_BEAKER | BOSS_ITEM_MASK | BOSS_ITEM_SHIELD,              BOSS_JUICE_ALWAYS) \
	X(SPEEDY,     "Speedy",           BOSS_ITEM_TURBO | BOSS_ITEM_SUPER_ENGINE | BOSS_ITEM_MASK,                         BOSS_JUICE_ALWAYS)
#endif

#define X(id, name, items, juice) BOSS_ITEM_PRESET_##id,
typedef enum BossItemPresetID
{
	BOSS_ITEM_PRESET_LIST(X)
	BOSS_ITEM_PRESET_COUNT,
} BossItemPresetID;
#undef X

#endif
