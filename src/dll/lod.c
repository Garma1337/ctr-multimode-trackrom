#include "config.h"
#include "level.h"
#include "lod.h"

#include <common.h>

#define LOD_MASK_SITES 6

static unsigned long* const lodMaskAt[LOD_MASK_SITES] =
{
	(unsigned long*)0x800A0F18,
	(unsigned long*)0x800A1E80,
	(unsigned long*)0x800A36D8,
	(unsigned long*)0x800A4FD0,
	(unsigned long*)0x800A6F70,
	(unsigned long*)0x800A8B90,
};

#define LOD_MASK_VANILLA 0x312901FCUL // andi $t1, $t1, 0x1FC
#define LOD_MASK_HIGH    0x3129FFFCUL // andi $t1, $t1, 0xFFFC

#define LOD_TABLE_AT      ((unsigned long*)0x800AB460)
#define LOD_TABLE_VANILLA 0x800A0EF4UL
#define LOD_TABLE_HIGH    0x800A6F40UL

#define DRIVER_LOD_HEADERS 4
#define DRIVER_LOD_NEAR    0
#define DRIVER_LOD_CUTOFF  (DRIVER_LOD_HEADERS - 1)

#define DRIVER_LOD_NEAR_HIGH 0x1000

#define MODEL_ID_DYNAMIC (-1)
#define MODEL_NAME_BIG1 0x31676962

static void Lod_ApplyTracks(int high)
{
	for (int i = 0; i < LOD_MASK_SITES; i++)
	{
		*lodMaskAt[i] = high ? LOD_MASK_HIGH : LOD_MASK_VANILLA;
	}

	*LOD_TABLE_AT = high ? LOD_TABLE_HIGH : LOD_TABLE_VANILLA;
}

static int Lod_IsMultiDetailDriver(const struct Model* model)
{
	if ((model == 0) || (model->id != MODEL_ID_DYNAMIC) || (model->numHeaders != DRIVER_LOD_HEADERS))
	{
		return 0;
	}

	return *(const unsigned int*)&model->headers[0].name[0] != MODEL_NAME_BIG1;
}

static void Lod_ApplyCharacters(void)
{
	struct Model** models = (struct Model**)sdata->PLYROBJECTLIST;

	if (models == 0)
	{
		return;
	}

	for (int i = 0; models[i] != 0; i++)
	{
		struct Model* model = models[i];

		if (!Lod_IsMultiDetailDriver(model))
		{
			continue;
		}

		struct ModelHeader* headers = model->headers;

		headers[DRIVER_LOD_NEAR].maxDistanceLOD = DRIVER_LOD_NEAR_HIGH;

		for (int header = DRIVER_LOD_NEAR + 1; header < DRIVER_LOD_CUTOFF; header++)
		{
			headers[header].maxDistanceLOD = 0;
		}
	}
}

void Lod_Apply(void)
{
	struct GameTracker* gGT = sdata->gGT;

	if ((gGT == 0) || !Level_IsPlayable(gGT->levelID))
	{
		return;
	}

	const Config* config = Config_Get();

	Lod_ApplyTracks(config->highLodTracks != 0);

	if (config->highLodCharacters != 0)
	{
		Lod_ApplyCharacters();
	}

	FlushCache();
}
