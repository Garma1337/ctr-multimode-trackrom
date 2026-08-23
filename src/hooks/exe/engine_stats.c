#include "dll/config.h"

#include <common.h>

#define METAPHYS_COUNT 65
#define MAX_STAT_FIRST 9
#define MAX_STAT_END   13

static const int maxStats[MAX_STAT_END - MAX_STAT_FIRST] = { 544, 1152, 13900, 15400 };

void VehBirth_SetConsts(struct Driver* driver)
{
	u_char* d = (u_char*)driver;

	int engineID = data.MetaDataCharacters[data.characterIDs[driver->driverID]].engineID;
	int pinStats = Config_IsFeatureEnabled(FEATURE_MAX_STATS);

	for (u_int i = 0; i < METAPHYS_COUNT; i++)
	{
		struct MetaPhys* metaPhys = &data.metaPhys[i];

		int value = metaPhys->value[engineID];

		if (pinStats && (i >= MAX_STAT_FIRST) && (i < MAX_STAT_END))
		{
			value = maxStats[i - MAX_STAT_FIRST];
		}

		void* dst = &d[metaPhys->offset];

		if (metaPhys->size == 1)
		{
			*(char*)dst = (char)value;
			continue;
		}

		if (metaPhys->size == 2)
		{
			*(short*)dst = (short)value;
			continue;
		}

		*(int*)dst = value;
	}
}
