#ifndef BOSSES_H
#define BOSSES_H

#include "config_schema.h"
#include "overrides.h"

#include <common.h>

#ifndef BOSS_LIST
#define BOSS_LIST(X) \
	X(1, "Ripper Roo", RIPPER_ROO,   BossWeaponRoo)       \
	X(2, "Papu Papu",  PAPU_PAPU,    BossWeaponPapu)      \
	X(3, "Komodo Joe", KOMODO_JOE,   BossWeaponJoe)       \
	X(4, "Pinstripe",  PINSTRIPE,    BossWeaponPinstripe) \
	X(5, "N. Oxide",   NITROS_OXIDE, BossWeaponOxide)
#endif

#define X(slot, name, character, vanillaItems) BOSS_##slot,
typedef enum BossSlot
{
	BOSS_LIST(X)
	BOSS_COUNT,
} BossSlot;
#undef X

_Static_assert(BOSS_COUNT == CONFIG_BOSS_COUNT, "BOSS_LIST must define exactly CONFIG_BOSS_COUNT bosses");

#endif
