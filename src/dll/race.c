#include "debug.h"
#include "freecam.h"
#include "input.h"
#include "race.h"
#include "reserves.h"

#include <common.h>

#define HUD_FLAG_RACING 1

static int hudHidden = 0;
static int savedHudFlags = 0;

static int Race_InRace(struct GameTracker* gGT)
{
	if (gGT->level1 == 0 || gGT->boolDemoMode != 0)
	{
		return 0;
	}

	if ((gGT->gameMode1 & (LOADING | MAIN_MENU | END_OF_RACE | START_OF_RACE)) != 0)
	{
		return 0;
	}

	return gGT->levelID != MAIN_MENU_LEVEL;
}

static void Race_HideVanillaHud(struct GameTracker* gGT)
{
	if (!hudHidden)
	{
		savedHudFlags = gGT->hudFlags;
		hudHidden = 1;
	}

	gGT->hudFlags &= ~HUD_FLAG_RACING;
}

static void Race_RestoreVanillaHud(struct GameTracker* gGT)
{
	if (!hudHidden)
	{
		return;
	}

	gGT->hudFlags = (gGT->hudFlags & ~HUD_FLAG_RACING) | (savedHudFlags & HUD_FLAG_RACING);
	hudHidden = 0;
}

void Race_Reset(void)
{
	hudHidden = 0;
	Freecam_Reset();
	Debug_Reset();
}

void Race_CaptureInput(void)
{
	Input_Capture();
}

void Race_RunGameLogic(struct GameTracker* gGT, struct GamepadSystem* gGS)
{
	Debug_RunGameLogic(gGT, gGS);
}

void Race_Update(struct GameTracker* gGT)
{
	if (gGT == 0)
	{
		return;
	}

	if (!Race_InRace(gGT))
	{
		if (Freecam_IsActive())
		{
			Freecam_Deactivate();
		}

		Race_RestoreVanillaHud(gGT);
		return;
	}

	Debug_Sample();

	if (Input_IsTapped(INPUT_DEBUG_HUD_TOGGLE))
	{
		Debug_Toggle();
	}

	Freecam_CheckToggle();

	if (Freecam_IsActive())
	{
		Freecam_Update();
	}

	if (Freecam_IsActive() || Debug_IsActive())
	{
		Race_HideVanillaHud(gGT);
	}
	else
	{
		Race_RestoreVanillaHud(gGT);
	}

	if (Debug_IsActive())
	{
		Debug_Draw(gGT);
		return;
	}

	if (Freecam_IsActive())
	{
		Freecam_DrawHelp();
		return;
	}

	Reserves_Draw(gGT);
}

void Race_RunRenderFrame(struct GameTracker* gGT, struct GamepadSystem* gGS)
{
	Debug_RunRenderFrame(gGT, gGS);
}
