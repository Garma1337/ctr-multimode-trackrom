#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <common.h>

#define MAIN_MENU_MAX_ENTRIES 12
#define MAIN_MENU_NO_ENTRY (-1)

#define LNG_ARCADE 0x4E
#define LNG_RELIC_RACE 0xB8
#define LNG_TIME_TRIAL 0x4D
#define LNG_CRYSTAL_CHALLENGE 0xBE
#define LNG_CTR_TOKEN 0x176
#define LNG_BOSS_RACE 0x4C
#define LNG_SETTINGS 0x51
#define LNG_DIFFICULTY 0x159

void MainMenu_Reset();
void MainMenu_AddEntry(int id, short stringIndex);
void MainMenu_Attach(struct RectMenu* mainMenu);
int MainMenu_GetEntryId(int row);

void MainMenu_InstallStrings();
void MainMenu_Build();
void MainMenu_Select(struct RectMenu* mainMenu);
void MainMenu_EnterCharacterSelect();

#endif
