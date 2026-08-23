#include "boss.h"
#include "rom.h"
#include "main_menu.h"

#include <common.h>

#define MAX_ITEM_SETS 13
#define NUM_HUBS 5
#define MAX_CHECKPOINT 255

static struct MetaDataBOSS itemSets[MAX_ITEM_SETS + 1];
static struct MenuRow bossRows[BOSS_COUNT + 1];
static int selectedBoss = BOSS_RIPPER_ROO;

static const unsigned char bossCharacter[BOSS_COUNT] =
{
	[BOSS_RIPPER_ROO] = RIPPER_ROO,
	[BOSS_PAPU_PAPU] = PAPU_PAPU,
	[BOSS_KOMODO_JOE] = KOMODO_JOE,
	[BOSS_PINSTRIPE] = PINSTRIPE,
	[BOSS_NITROS_OXIDE] = NITROS_OXIDE,
};

static struct MetaDataBOSS* Boss_GetDefaultItemSets(int boss)
{
	struct MetaDataBOSS* tables[BOSS_COUNT] =
	{
		[BOSS_RIPPER_ROO] = &data.BossWeaponRoo[0],
		[BOSS_PAPU_PAPU] = &data.BossWeaponPapu[0],
		[BOSS_KOMODO_JOE] = &data.BossWeaponJoe[0],
		[BOSS_PINSTRIPE] = &data.BossWeaponPinstripe[0],
		[BOSS_NITROS_OXIDE] = &data.BossWeaponOxide[0],
	};

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

static int Boss_CountItemSets(struct MetaDataBOSS* stock)
{
	int count = 0;

	while ((count < MAX_ITEM_SETS) && (stock[count].throwFlag != 0))
	{
		count++;
	}

	return count;
}

static void Boss_BuildItemSets(struct MetaDataBOSS* stock, int count, int numCheckpoints)
{
	for (int i = 0; i < count; i++)
	{
		int checkpoint = (numCheckpoints * i) / count;

		itemSets[i] = stock[i];
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

static void Boss_MenuProc(struct RectMenu* menu)
{
	short row = menu->rowSelected;

	if (row < 0)
	{
		menu->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
		return;
	}

	if (row >= BOSS_COUNT)
	{
		return;
	}

	selectedBoss = row;
	sdata->gGT->bossID = row;

	MainMenu_EnterCharacterSelect();

	menu->state |= ONLY_DRAW_TITLE;
}

void Boss_InstallRows()
{
	for (int i = 0; i < BOSS_COUNT; i++)
	{
		bossRows[i].stringIndex = data.MetaDataCharacters[bossCharacter[i]].name_LNG_long;
		bossRows[i].rowOnPressUp = (char)((i == 0) ? 0 : (i - 1));
		bossRows[i].rowOnPressDown = (char)((i == (BOSS_COUNT - 1)) ? i : (i + 1));
		bossRows[i].rowOnPressLeft = (char)i;
		bossRows[i].rowOnPressRight = (char)i;
	}

	bossRows[BOSS_COUNT].stringIndex = -1;
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

	struct MetaDataBOSS* stock = Boss_GetDefaultItemSets(selectedBoss);
	int count = Boss_CountItemSets(stock);

	if ((count == 0) || (gGT->level1 == 0) || (gGT->level1->cnt_restart_points <= 0))
	{
		Boss_RestoreDefaultItemSets();
		return;
	}

	Boss_BuildItemSets(stock, count, gGT->level1->cnt_restart_points);

	for (int hub = 0; hub < NUM_HUBS; hub++)
	{
		data.bossWeaponMetaPtr[hub] = &itemSets[0];
	}

	sdata->advProgress.timesLostBossRace[gGT->bossID] = 0;
}
