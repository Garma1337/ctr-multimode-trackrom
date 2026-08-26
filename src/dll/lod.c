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

#define LOD_MASK_STOCK 0x312901FCUL // andi $t1, $t1, 0x1FC
#define LOD_MASK_HIGH  0x3129FFFCUL // andi $t1, $t1, 0xFFFC

#define LOD_TABLE_AT    ((unsigned long*)0x800AB460)
#define LOD_TABLE_STOCK 0x800A0EF4UL
#define LOD_TABLE_HIGH  0x800A6F40UL

void Lod_Apply(void)
{
	struct GameTracker* gGT = sdata->gGT;

	if ((gGT == 0) || !Level_IsPlayable(gGT->levelID))
	{
		return;
	}

	int high = (Config_Get()->highLod != 0);

	for (int i = 0; i < LOD_MASK_SITES; i++)
	{
		*lodMaskAt[i] = high ? LOD_MASK_HIGH : LOD_MASK_STOCK;
	}

	*LOD_TABLE_AT = high ? LOD_TABLE_HIGH : LOD_TABLE_STOCK;

	FlushCache();
}
