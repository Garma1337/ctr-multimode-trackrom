#ifndef BOSSES_H
#define BOSSES_H

#include "config_schema.h"
#include "overrides.h"

#include <common.h>

#ifndef BOSS_LIST
#define BOSS_LIST(X) \
	X(1, "Ripper Roo", BossWeaponRoo)       \
	X(2, "Papu Papu",  BossWeaponPapu)      \
	X(3, "Komodo Joe", BossWeaponJoe)       \
	X(4, "Pinstripe",  BossWeaponPinstripe) \
	X(5, "N. Oxide",   BossWeaponOxide)
#endif

#define X(slot, name, vanillaItems) BOSS_##slot,
typedef enum BossSlot
{
	BOSS_LIST(X)
	BOSS_COUNT,
} BossSlot;
#undef X

_Static_assert(BOSS_COUNT == CONFIG_BOSS_COUNT, "BOSS_LIST must define exactly CONFIG_BOSS_COUNT bosses");

#endif
