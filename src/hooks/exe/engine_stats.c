#include "../../dll/config.h"

#include <common.h>

#define METAPHYS_COUNT 65

#define MAX_STAT_FIRST 9
#define MAX_STAT_END   13

typedef struct PhysEntry
{
	int unused;
	int offset;
	int size;
	int value[4];
} PhysEntry;

_Static_assert(sizeof(PhysEntry) == 0x1C, "metaPhys stride must stay 28 bytes");

static const int maxStats[MAX_STAT_END - MAX_STAT_FIRST] = { 544, 1152, 13900, 15400 };

void VehBirth_SetConsts(struct Driver* driver)
{
	u_char* d = (u_char*)driver;
	PhysEntry* entries = (PhysEntry*)&data.metaPhys[0];

	int engineID = data.MetaDataCharacters[data.characterIDs[driver->driverID]].engineID;
	int pinStats = Config_IsFeatureEnabled(FEATURE_MAX_STATS);

	for (int i = 0; i < METAPHYS_COUNT; i++)
	{
		PhysEntry* entry = &entries[i];

		int value = entry->value[engineID];

		if (pinStats && (i >= MAX_STAT_FIRST) && (i < MAX_STAT_END))
		{
			value = maxStats[i - MAX_STAT_FIRST];
		}

		void* dst = &d[entry->offset];

		if (entry->size == 1)
		{
			*(char*)dst = (char)value;
			continue;
		}

		if (entry->size == 2)
		{
			*(short*)dst = (short)value;
			continue;
		}

		if (entry->size == 4)
		{
			*(int*)dst = value;
		}
	}
}
