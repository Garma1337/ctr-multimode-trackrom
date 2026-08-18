#include "drivers.h"
#include "game_mode.h"
#include "rom.h"

#include <common.h>

#define OXIDE_TIMETRIAL_PACK (BI_TIMETRIALPACK + 15)
#define MODEL_NAME_LENGTH 16

void Drivers_Preload(struct BigHeader* bigfile)
{
	struct Model** preloaded = CHAR_MODEL_PTRS;
	char* dest = DRIVER_ADDR;

	for (int i = 0; i < NUM_PRELOADED_DRIVERS; i++)
	{
		u_int fileSize;
		LOAD_ReadFile(bigfile, LT_DRAM, BI_RACERMODELHI + i, (u_int*)dest, &fileSize, 0);

		char* model = dest + sizeof(int);
		int* pointerMap = (int*)(model + *(int*)dest);
		LOAD_RunPtrMap(model, pointerMap + 1, *pointerMap >> 2);

		preloaded[i] = (struct Model*)model;
		dest += fileSize;
	}

	preloaded[NUM_PRELOADED_DRIVERS] = NULL;
}

struct Model* Drivers_FindModel(char* name)
{
	struct Model** tables[] =
	{
		(struct Model**)sdata->PLYROBJECTLIST,
		CHAR_MODEL_PTRS,
	};

	const int tableCount = sizeof(tables) / sizeof(tables[0]);

	for (int table = 0; table < tableCount; table++)
	{
		struct Model** models = tables[table];

		if (models == NULL)
		{
			continue;
		}

		for (int i = 0; models[i] != NULL; i++)
		{
			if (strncmp(models[i]->name, name, MODEL_NAME_LENGTH) == 0)
			{
				return models[i];
			}
		}
	}

	return NULL;
}

void Drivers_QueueModePack(struct BigHeader* bigfile, void* callback)
{
	if (sdata->gGT->gameMode1 & MAIN_MENU)
	{
		LOAD_AppendQueue((int)bigfile, LT_DRAM, BI_ADVENTUREPACK, NULL, callback);
		return;
	}

	if (!GameMode_NeedsArcadePack())
	{
		LOAD_AppendQueue((int)bigfile, LT_DRAM, OXIDE_TIMETRIAL_PACK, NULL, callback);
		return;
	}

	LOAD_Robots1P(data.characterIDs[0]);
	GameMode_ApplyStartingGrid();

	LOAD_AppendQueue((int)bigfile, LT_DRAM, BI_1PARCADEPACK + data.characterIDs[0], NULL, callback);
}
