#ifndef SETTINGS_H
#define SETTINGS_H

#include <common.h>

typedef enum SettingsField
{
	SETTINGS_RELIC_SAPPHIRE = 0,
	SETTINGS_RELIC_GOLD,
	SETTINGS_RELIC_PLATINUM,
	SETTINGS_CRYSTAL_TIME,
	SETTINGS_INTRO_CUTSCENE,
	SETTINGS_GHOST,
	SETTINGS_FIELD_COUNT
} SettingsField;

#define SETTINGS_RELIC_TIERS 3

void Settings_ApplyCodePatches(void);
void Settings_PollHost(void);
void Settings_Open(struct RectMenu* mainMenu);
int  Settings_IsOpen(void);
void Settings_Update(void);
int  Settings_GetRelicTime(int tier);
int  Settings_GetCrystalTime(void);

#endif
