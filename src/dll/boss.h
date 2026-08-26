#ifndef BOSS_H_MODEROM
#define BOSS_H_MODEROM

#include "../config/boss_item_presets.h"
#include "../config/bosses.h"

#include <common.h>

typedef struct BossItemPreset
{
	unsigned short items;
	unsigned char juice;
} BossItemPreset;

extern const BossItemPreset bossItemPresets[BOSS_ITEM_PRESET_COUNT];
extern const char* const bossItemPresetNames[BOSS_ITEM_PRESET_COUNT];

void Boss_InstallStrings();
const char* Boss_GetName(int boss);
const char* Boss_GetItemLabel(int boss);
void Boss_InstallRows();
void Boss_OpenSelect(struct RectMenu* mainMenu);
int  Boss_IsRace();
void Boss_ApplyDrivers();
void Boss_PrepareRace();

#endif
