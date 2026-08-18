#include "oxide.h"
#include "rom.h"

#include <common.h>

extern unsigned char oxideModel[];

#define SCRAP_PTRS_OFFSET      0xBF4
#define SCRAP_EXTRA_ID_OFFSET  0xC2A
#define COUNT_RESTORE_OFFSET   0xC0
#define COUNT_MENUPROC1_OFFSET 0xC50
#define COUNT_MENUPROC2_OFFSET 0x107C

#define OXIDE_ROSTER_ENTRIES 16
#define KART_WHEEL_SIZE      0xCCC

#define OXIDE_SCALE_MENU 5
#define OXIDE_SCALE_RACE 7
#define OXIDE_SCALE_DEN  8

void MM_JumpTo_Scrapbook(void);
void MM_Characters_RestoreIDs(void);
void MM_Characters_MenuProc(void);

static const struct CharacterSelectMeta oxide_csm_1p2p[] = {
	{.posX = 0x080, .posY = 0x60, .indexNext = { 0x00, 0x04, 0x08, 0x01 }, .characterID = 0x00, .unlockFlags = 0xFFFF }, // crash
	{.posX = 0x0C0, .posY = 0x60, .indexNext = { 0x01, 0x05, 0x00, 0x02 }, .characterID = 0x01, .unlockFlags = 0xFFFF }, // cortex
	{.posX = 0x100, .posY = 0x60, .indexNext = { 0x02, 0x06, 0x01, 0x03 }, .characterID = 0x02, .unlockFlags = 0xFFFF }, // tiny
	{.posX = 0x140, .posY = 0x60, .indexNext = { 0x03, 0x07, 0x02, 0x09 }, .characterID = 0x03, .unlockFlags = 0xFFFF }, // coco
	{.posX = 0x080, .posY = 0x87, .indexNext = { 0x00, 0x0C, 0x0A, 0x05 }, .characterID = 0x04, .unlockFlags = 0xFFFF }, // n.gin
	{.posX = 0x0C0, .posY = 0x87, .indexNext = { 0x01, 0x0D, 0x04, 0x06 }, .characterID = 0x05, .unlockFlags = 0xFFFF }, // dingo
	{.posX = 0x100, .posY = 0x87, .indexNext = { 0x02, 0x0E, 0x05, 0x07 }, .characterID = 0x06, .unlockFlags = 0xFFFF }, // polar
	{.posX = 0x140, .posY = 0x87, .indexNext = { 0x03, 0x0F, 0x06, 0x0B }, .characterID = 0x07, .unlockFlags = 0xFFFF }, // pura
	{.posX = 0x040, .posY = 0x60, .indexNext = { 0x08, 0x0A, 0x08, 0x00 }, .characterID = 0x0C, .unlockFlags = 0x0005 }, // ntropy
	{.posX = 0x180, .posY = 0x60, .indexNext = { 0x09, 0x0B, 0x03, 0x09 }, .characterID = 0x08, .unlockFlags = 0x000A }, // pinstripe
	{.posX = 0x040, .posY = 0x87, .indexNext = { 0x08, 0x0A, 0x0A, 0x04 }, .characterID = 0x0A, .unlockFlags = 0x0007 }, // roo
	{.posX = 0x180, .posY = 0x87, .indexNext = { 0x09, 0x0F, 0x07, 0x0B }, .characterID = 0x09, .unlockFlags = 0x0008 }, // papu
	{.posX = 0x080, .posY = 0xAE, .indexNext = { 0x04, 0x0C, 0x0C, 0x0D }, .characterID = 0x0B, .unlockFlags = 0x0009 }, // kom joe
	{.posX = 0x0C0, .posY = 0xAE, .indexNext = { 0x05, 0x0D, 0x0C, 0x0E }, .characterID = 0x0D, .unlockFlags = 0x0006 }, // penta
	{.posX = 0x100, .posY = 0xAE, .indexNext = { 0x06, 0x0E, 0x0D, 0x0F }, .characterID = 0x0E, .unlockFlags = 0x000B }, // fake crash
	{.posX = 0x140, .posY = 0xAE, .indexNext = { 0x07, 0x0F, 0x0E, 0x0B }, .characterID = 0x0F, .unlockFlags = 0xFFFF }, // oxide
};

static const struct CharacterSelectMeta oxide_csm_3p[] = {
	{.posX = 0x020, .posY = 0x47, .indexNext = { 0x0C, 0x04, 0x00, 0x01 }, .characterID = 0x00, .unlockFlags = 0xFFFF }, // crash
	{.posX = 0x060, .posY = 0x47, .indexNext = { 0x0D, 0x05, 0x00, 0x02 }, .characterID = 0x01, .unlockFlags = 0xFFFF }, // cortex
	{.posX = 0x0A0, .posY = 0x47, .indexNext = { 0x0E, 0x06, 0x01, 0x03 }, .characterID = 0x02, .unlockFlags = 0xFFFF }, // tiny
	{.posX = 0x0E0, .posY = 0x47, .indexNext = { 0x0F, 0x07, 0x02, 0x03 }, .characterID = 0x03, .unlockFlags = 0xFFFF }, // coco
	{.posX = 0x020, .posY = 0x6E, .indexNext = { 0x00, 0x08, 0x04, 0x05 }, .characterID = 0x04, .unlockFlags = 0xFFFF }, // n.gin
	{.posX = 0x060, .posY = 0x6E, .indexNext = { 0x01, 0x09, 0x04, 0x06 }, .characterID = 0x05, .unlockFlags = 0xFFFF }, // dingo
	{.posX = 0x0A0, .posY = 0x6E, .indexNext = { 0x02, 0x0A, 0x05, 0x07 }, .characterID = 0x06, .unlockFlags = 0xFFFF }, // polar
	{.posX = 0x0E0, .posY = 0x6E, .indexNext = { 0x03, 0x0B, 0x06, 0x07 }, .characterID = 0x07, .unlockFlags = 0xFFFF }, // pura
	{.posX = 0x020, .posY = 0x95, .indexNext = { 0x04, 0x08, 0x08, 0x09 }, .characterID = 0x0C, .unlockFlags = 0x0005 }, // ntropy
	{.posX = 0x060, .posY = 0x95, .indexNext = { 0x05, 0x09, 0x08, 0x0A }, .characterID = 0x08, .unlockFlags = 0x000A }, // pinstripe
	{.posX = 0x0A0, .posY = 0x95, .indexNext = { 0x06, 0x0A, 0x09, 0x0B }, .characterID = 0x0A, .unlockFlags = 0x0007 }, // roo
	{.posX = 0x0E0, .posY = 0x95, .indexNext = { 0x07, 0x0B, 0x0A, 0x0B }, .characterID = 0x09, .unlockFlags = 0x0008 }, // papu
	{.posX = 0x020, .posY = 0x20, .indexNext = { 0x0C, 0x00, 0x0C, 0x0D }, .characterID = 0x0B, .unlockFlags = 0x0009 }, // kom joe
	{.posX = 0x060, .posY = 0x20, .indexNext = { 0x0D, 0x01, 0x0C, 0x0E }, .characterID = 0x0D, .unlockFlags = 0x0006 }, // penta
	{.posX = 0x0A0, .posY = 0x20, .indexNext = { 0x0E, 0x02, 0x0D, 0x0F }, .characterID = 0x0E, .unlockFlags = 0x000B }, // fake crash
	{.posX = 0x0E0, .posY = 0x20, .indexNext = { 0x0F, 0x03, 0x0E, 0x0F }, .characterID = 0x0F, .unlockFlags = 0xFFFF }, // oxide
};

static const struct CharacterSelectMeta oxide_csm_4p[] = {
	{.posX = 0x080, .posY = 0x47, .indexNext = { 0x00, 0x04, 0x0A, 0x01 }, .characterID = 0x00, .unlockFlags = 0xFFFF }, // crash
	{.posX = 0x0C0, .posY = 0x47, .indexNext = { 0x0E, 0x05, 0x00, 0x02 }, .characterID = 0x01, .unlockFlags = 0xFFFF }, // cortex
	{.posX = 0x100, .posY = 0x47, .indexNext = { 0x0F, 0x06, 0x01, 0x03 }, .characterID = 0x02, .unlockFlags = 0xFFFF }, // tiny
	{.posX = 0x140, .posY = 0x47, .indexNext = { 0x03, 0x07, 0x02, 0x0B }, .characterID = 0x03, .unlockFlags = 0xFFFF }, // coco
	{.posX = 0x080, .posY = 0x6E, .indexNext = { 0x00, 0x04, 0x0C, 0x05 }, .characterID = 0x04, .unlockFlags = 0xFFFF }, // n.gin
	{.posX = 0x0C0, .posY = 0x6E, .indexNext = { 0x01, 0x08, 0x04, 0x06 }, .characterID = 0x05, .unlockFlags = 0xFFFF }, // dingo
	{.posX = 0x100, .posY = 0x6E, .indexNext = { 0x02, 0x09, 0x05, 0x07 }, .characterID = 0x06, .unlockFlags = 0xFFFF }, // polar
	{.posX = 0x140, .posY = 0x6E, .indexNext = { 0x03, 0x07, 0x06, 0x0D }, .characterID = 0x07, .unlockFlags = 0xFFFF }, // pura
	{.posX = 0x0C0, .posY = 0x95, .indexNext = { 0x05, 0x08, 0x08, 0x09 }, .characterID = 0x0C, .unlockFlags = 0x0005 }, // ntropy
	{.posX = 0x100, .posY = 0x95, .indexNext = { 0x06, 0x09, 0x08, 0x09 }, .characterID = 0x08, .unlockFlags = 0x000B }, // pinstripe
	{.posX = 0x040, .posY = 0x47, .indexNext = { 0x0A, 0x0C, 0x0A, 0x00 }, .characterID = 0x0A, .unlockFlags = 0x0007 }, // roo
	{.posX = 0x180, .posY = 0x47, .indexNext = { 0x0B, 0x0D, 0x03, 0x0B }, .characterID = 0x09, .unlockFlags = 0x0008 }, // papu
	{.posX = 0x040, .posY = 0x6E, .indexNext = { 0x0A, 0x0C, 0x0C, 0x04 }, .characterID = 0x0B, .unlockFlags = 0x0009 }, // kom joe
	{.posX = 0x180, .posY = 0x6E, .indexNext = { 0x0B, 0x0D, 0x07, 0x0D }, .characterID = 0x0D, .unlockFlags = 0x000A }, // penta
	{.posX = 0x0C0, .posY = 0x20, .indexNext = { 0x0E, 0x01, 0x0E, 0x0F }, .characterID = 0x0E, .unlockFlags = 0x0006 }, // fake crash
	{.posX = 0x100, .posY = 0x20, .indexNext = { 0x0F, 0x02, 0x0E, 0x0F }, .characterID = 0x0F, .unlockFlags = 0xFFFF }, // oxide
};

static void Oxide_ApplyScale(struct Model* model, int num)
{
	for (int i = 0; i < 3; i++)
	{
		model->headers[0].scale[i] = (model->headers[0].scale[i] * num) / OXIDE_SCALE_DEN;
	}
}

void Oxide_ScaleRaceModel(void)
{
	struct Model** charModels = CHAR_MODEL_PTRS;
	struct Model* model = charModels[NITROS_OXIDE];

	if (model == 0)
	{
		return;
	}

	Oxide_ApplyScale(model, OXIDE_SCALE_RACE);
}

void Oxide_InstallMenuRoster(void)
{
	const struct CharacterSelectMeta** ptrs = (const struct CharacterSelectMeta**)((char*)MM_JumpTo_Scrapbook + SCRAP_PTRS_OFFSET);
	ptrs[0] = oxide_csm_1p2p;
	ptrs[1] = oxide_csm_1p2p;
	ptrs[2] = oxide_csm_3p;
	ptrs[3] = oxide_csm_4p;

	*((char*)MM_JumpTo_Scrapbook + SCRAP_EXTRA_ID_OFFSET) = (char)NITROS_OXIDE;

	*((char*)MM_Characters_RestoreIDs + COUNT_RESTORE_OFFSET) = (char)OXIDE_ROSTER_ENTRIES;
	*((char*)MM_Characters_MenuProc + COUNT_MENUPROC1_OFFSET) = (char)OXIDE_ROSTER_ENTRIES;
	*((char*)MM_Characters_MenuProc + COUNT_MENUPROC2_OFFSET) = (char)OXIDE_ROSTER_ENTRIES;

	struct Model* model = (struct Model*)&oxideModel[4];
	char* patchPtr = (char*)model + *(unsigned int*)&oxideModel[0];

	LOAD_RunPtrMap((char*)model, (int*)&patchPtr[4], *(unsigned int*)&patchPtr[0] >> 2);
	Oxide_ApplyScale(model, OXIDE_SCALE_MENU);
}

void Oxide_HideMenuWheels(void)
{
	if (sdata->ptrActiveMenu != &D230.menuCharacterSelect)
	{
		return;
	}

	struct GameTracker* gGT = sdata->gGT;

	for (int i = 0; i < gGT->numPlyrNextGame; i++)
	{
		struct Driver* driver = gGT->drivers[i];

		if (driver == 0)
		{
			continue;
		}

		driver->wheelSize = (data.characterIDs[i] == NITROS_OXIDE) ? 0 : KART_WHEEL_SIZE;
	}
}
