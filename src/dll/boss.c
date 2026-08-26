#include "../rom.h"
#include "boss.h"
#include "config.h"
#include "main_menu.h"

#include <common.h>

#define MAX_ITEM_SETS 13
#define NUM_HUBS 5
#define MAX_CHECKPOINT 255

#define ITEM_TURBO         0x00
#define ITEM_BOMB          0x65
#define ITEM_MISSILE       0x02
#define ITEM_TNT           0x64
#define ITEM_BEAKER        0x66
#define ITEM_SHIELD        0x06
#define ITEM_MASK          0x07
#define ITEM_CLOCK         0x08
#define ITEM_WARP_ORB      0x09
#define ITEM_INVISIBILITY  0x0C
#define ITEM_SUPER_ENGINE  0x0D
#define ITEM_NONE 0x0F

#define THROW_FLAG_THROW 2

static struct MetaDataBOSS itemSets[MAX_ITEM_SETS + 1];
static struct MenuRow bossRows[BOSS_COUNT + 1];
static unsigned char bossRowId[BOSS_COUNT];
static int bossRowCount;
static int selectedBoss = BOSS_1;

static const unsigned char itemCode[BOSS_ITEM_COUNT] =
{
	ITEM_TURBO,
	ITEM_BOMB,
	ITEM_MISSILE,
	ITEM_TNT,
	ITEM_BEAKER,
	ITEM_SHIELD,
	ITEM_MASK,
	ITEM_CLOCK,
	ITEM_WARP_ORB,
	ITEM_INVISIBILITY,
	ITEM_SUPER_ENGINE,
};

#define X(id, name, items, juice) { items, juice },
const BossItemPreset bossItemPresets[BOSS_ITEM_PRESET_COUNT] = { BOSS_ITEM_PRESET_LIST(X) };
#undef X

#define X(id, name, items, juice) name,
const char* const bossItemPresetNames[BOSS_ITEM_PRESET_COUNT] = { BOSS_ITEM_PRESET_LIST(X) };
#undef X

#define X(slot, name, character, vanillaItems) character,
static const unsigned char bossCharacter[BOSS_COUNT] = { BOSS_LIST(X) };
#undef X

#define X(slot, name, character, vanillaItems) name,
static const char* const bossName[BOSS_COUNT] = { BOSS_LIST(X) };
#undef X

#define X(slot, name, character, vanillaItems) "Items - " name,
static const char* const bossItemLabel[BOSS_COUNT] = { BOSS_LIST(X) };
#undef X

static const unsigned char vanillaBossCharacter[BOSS_COUNT] =
{
	RIPPER_ROO,
	PAPU_PAPU,
	KOMODO_JOE,
	PINSTRIPE,
	NITROS_OXIDE,
};

static short Boss_GetStringIndex(int boss)
{
	return data.MetaDataCharacters[vanillaBossCharacter[boss]].name_LNG_long;
}

static struct MetaDataBOSS* Boss_GetVanillaItemSets(int boss)
{
#define X(slot, name, character, vanillaItems) &data.vanillaItems[0],
	struct MetaDataBOSS* tables[BOSS_COUNT] = { BOSS_LIST(X) };
#undef X

	return tables[boss];
}

static void Boss_RestoreDefaultItemSets()
{
	data.bossWeaponMetaPtr[0] = &data.BossWeaponOxide[0];
	data.bossWeaponMetaPtr[1] = &data.BossWeaponRoo[0];
	data.bossWeaponMetaPtr[2] = &data.BossWeaponPapu[0];
	data.bossWeaponMetaPtr[3] = &data.BossWeaponJoe[0];
	data.bossWeaponMetaPtr[4] = &data.BossWeaponPinstripe[0];
}

static int Boss_CountItemSets(struct MetaDataBOSS* vanilla)
{
	int count = 0;

	while ((count < MAX_ITEM_SETS) && (vanilla[count].throwFlag != 0))
	{
		count++;
	}

	return count;
}

static void Boss_BuildItemSets(struct MetaDataBOSS* vanilla, int count, int numCheckpoints)
{
	for (int i = 0; i < count; i++)
	{
		int checkpoint = (numCheckpoints * i) / count;

		itemSets[i] = vanilla[i];
		itemSets[i].trackCheckpoint = (unsigned char)((checkpoint > MAX_CHECKPOINT) ? MAX_CHECKPOINT : checkpoint);
		itemSets[i].unk1 = 1;
	}

	itemSets[count].trackCheckpoint = 0;
	itemSets[count].throwFlag = 0;
	itemSets[count].weaponType = 0;
	itemSets[count].unk1 = 0;
	itemSets[count].weaponCooldown = 0;
	itemSets[count].juiceFlag = 0;
}

static void Boss_ApplyConfiguredItems(int boss, int count)
{
	const BossItemPreset* preset = &bossItemPresets[Config_Get()->bossItemPreset[boss]];

	int mask = preset->items;
	int juice = preset->juice;

	unsigned char allowed[BOSS_ITEM_COUNT];
	int allowedCount = 0;

	for (int i = 0; i < BOSS_ITEM_COUNT; i++)
	{
		if ((mask & (1 << i)) != 0)
		{
			allowed[allowedCount++] = itemCode[i];
		}
	}

	int next = 0;

	for (int i = 0; i < count; i++)
	{
		if (juice != BOSS_JUICE_VANILLA)
		{
			itemSets[i].juiceFlag = (unsigned short)(juice - BOSS_JUICE_NONE);
		}

		if ((allowedCount == 0) || (itemSets[i].weaponType == ITEM_NONE))
		{
			continue;
		}

		itemSets[i].weaponType = allowed[next];
		itemSets[i].throwFlag = THROW_FLAG_THROW;

		next = (next + 1) % allowedCount;
	}
}

static void Boss_MenuProc(struct RectMenu* menu)
{
	short row = menu->rowSelected;

	if (row < 0)
	{
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		return;
	}

	if (row >= bossRowCount)
	{
		return;
	}

	selectedBoss = bossRowId[row];
	sdata->gGT->bossID = selectedBoss;

	MainMenu_EnterCharacterSelect();

	menu->state |= ONLY_DRAW_TITLE;
}

const char* Boss_GetName(int boss)
{
	return bossName[boss];
}

const char* Boss_GetItemLabel(int boss)
{
	return bossItemLabel[boss];
}

void Boss_InstallStrings()
{
	for (int boss = 0; boss < BOSS_COUNT; boss++)
	{
		sdata->lngStrings[Boss_GetStringIndex(boss)] = (char*)bossName[boss];
	}
}

void Boss_InstallRows()
{
	bossRowCount = 0;

	for (int boss = 0; boss < BOSS_COUNT; boss++)
	{
		if (!Config_IsBossEnabled(boss))
		{
			continue;
		}

		int row = bossRowCount++;

		bossRowId[row] = (unsigned char)boss;

		bossRows[row].stringIndex = Boss_GetStringIndex(boss);
		bossRows[row].rowOnPressUp = (char)((row == 0) ? 0 : (row - 1));
		bossRows[row].rowOnPressDown = (char)row;
		bossRows[row].rowOnPressLeft = (char)row;
		bossRows[row].rowOnPressRight = (char)row;

		if (row > 0)
		{
			bossRows[row - 1].rowOnPressDown = (char)row;
		}
	}

	selectedBoss = bossRowId[0];
	bossRows[bossRowCount].stringIndex = -1;
}

void Boss_OpenSelect(struct RectMenu* mainMenu)
{
	struct RectMenu* menu = &D230.menuDifficulty;

	menu->stringIndexTitle = LNG_BOSS_RACE;
	menu->rows = &bossRows[0];
	menu->funcPtr = &Boss_MenuProc;

	mainMenu->ptrNextBox_InHierarchy = menu;
	mainMenu->state |= DRAW_NEXT_MENU_IN_HIERARCHY;
}

int Boss_IsRace()
{
	return (sdata->gGT->gameMode1 & ADVENTURE_BOSS) != 0;
}

void Boss_ApplyDrivers()
{
	data.characterIDs[1] = (char)bossCharacter[selectedBoss];
}

void Boss_PrepareRace()
{
	struct GameTracker* gGT = sdata->gGT;

	if (!Boss_IsRace())
	{
		Boss_RestoreDefaultItemSets();
		return;
	}

	struct MetaDataBOSS* vanilla = Boss_GetVanillaItemSets(selectedBoss);
	int count = Boss_CountItemSets(vanilla);

	if ((count == 0) || (gGT->level1 == 0) || (gGT->level1->cnt_restart_points <= 0))
	{
		Boss_RestoreDefaultItemSets();
		return;
	}

	Boss_BuildItemSets(vanilla, count, gGT->level1->cnt_restart_points);
	Boss_ApplyConfiguredItems(selectedBoss, count);

	for (int hub = 0; hub < NUM_HUBS; hub++)
	{
		data.bossWeaponMetaPtr[hub] = &itemSets[0];
	}

	sdata->advProgress.timesLostBossRace[gGT->bossID] = 0;
}
