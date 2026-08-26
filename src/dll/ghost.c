#include "../rom.h"
#include "config.h"
#include "ghost.h"
#include "settings.h"

#include <common.h>

_Static_assert(GHOST_CHARACTER_1_DEFAULT == N_TROPY, "ghost character defaults drifted from enum Characters");
_Static_assert(GHOST_CHARACTER_2_DEFAULT == NITROS_OXIDE, "ghost character defaults drifted from enum Characters");

#define TIME_NEVER 0x7FFFFFFF

#define GHOST_FLAG_OPEN_1 1
#define GHOST_FLAG_OPEN_2 2
#define GHOST_DRIVER_SLOT 2

#define BEST_RACE_ENTRY 1

static int hasTape[GHOST_SLOT_COUNT];

static void* Ghost_FindTape(struct Level* lev, int slot)
{
	if ((lev == 0) || (lev->ptrSpawnType1 == 0) || (lev->ptrSpawnType1->count <= ST1_NOXIDE))
	{
		return 0;
	}

	void** spawns = ST1_GETPOINTERS(lev->ptrSpawnType1);

	return spawns[ST1_NTROPY + slot];
}

static void Ghost_ReadTapes(struct GameTracker* gGT)
{
	int changed = 0;

	for (int slot = 0; slot < GHOST_SLOT_COUNT; slot++)
	{
		int present = (Ghost_FindTape(gGT->level1, slot) != 0);

		changed |= (present != hasTape[slot]);
		hasTape[slot] = present;
	}

	if (changed)
	{
		Ghost_ApplyTargets();
	}
}

static unsigned int Ghost_ResolveFlags(unsigned int flags)
{
	if (!hasTape[0])
	{
		return 0;
	}

	if (!hasTape[1])
	{
		return flags & GHOST_FLAG_OPEN_1;
	}

	unsigned int best = sdata->gameProgress.highScoreTracks[CUSTOM_LEVEL_ID].scoreEntry[BEST_RACE_ENTRY].time;

	if ((best != 0) && ((int)best <= Settings_GetGhostTime(1)))
	{
		flags |= GHOST_FLAG_OPEN_1 | GHOST_FLAG_OPEN_2;
	}

	return flags;
}

void Ghost_AdoptDrivers(void)
{
	const Config* config = Config_Get();

	for (int slot = 0; slot < GHOST_SLOT_COUNT; slot++)
	{
		data.characterIDs[GHOST_DRIVER_SLOT + slot] = config->ghostCharacter[slot];
	}
}

void Ghost_ApplyTargets(void)
{
	data.metaDataLEV[CUSTOM_LEVEL_ID].timeTrial = hasTape[0] ? Settings_GetGhostTime(0) : TIME_NEVER;
}

void Ghost_Update(struct GameTracker* gGT)
{
	if (gGT->levelID != CUSTOM_LEVEL_ID)
	{
		return;
	}

	Ghost_ReadTapes(gGT);

	unsigned int* flags = &sdata->gameProgress.highScoreTracks[CUSTOM_LEVEL_ID].timeTrialFlags;

	*flags = Ghost_ResolveFlags(*flags);
}

void Ghost_Forget(void)
{
	for (int slot = 0; slot < GHOST_SLOT_COUNT; slot++)
	{
		hasTape[slot] = 0;
	}

	Ghost_ApplyTargets();
}
