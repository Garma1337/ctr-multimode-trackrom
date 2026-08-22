#ifndef BOSS_H_MODEROM
#define BOSS_H_MODEROM

#include <common.h>

typedef enum BossRow
{
	BOSS_RIPPER_ROO = 0,
	BOSS_PAPU_PAPU,
	BOSS_KOMODO_JOE,
	BOSS_PINSTRIPE,
	BOSS_NITROS_OXIDE,
	BOSS_COUNT,
} BossRow;

void Boss_InstallRows();
void Boss_OpenSelect(struct RectMenu* mainMenu);
int  Boss_IsRace();
void Boss_ApplyDrivers();
void Boss_PrepareRace();

#endif
