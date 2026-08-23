#include "../../dll/boss.h"
#include "../../dll/config.h"
#include "../../dll/hot_reload.h"
#include "../../dll/level.h"
#include "../../dll/oxide.h"
#include "../../dll/race.h"
#include "../../dll/settings.h"
#include "../../rom.h"
#include "../../rom/boot.h"

#include <common.h>

#define FRAME_TIME_MS FPS_HALF(32)

#define LNG_DEMO_MODE_HINT 0x230

#define AUDIO_INTRO_CUTSCENE 9
#define AUDIO_TRAFFIC_LIGHTS 10
#define AUDIO_RESET_STAGE 1

static void OnLoadingEnd();
static void OnResetStage();
static void OnGameplay();
static int AdvanceLoading(struct GameTracker* gGT);
static void RunFrame(struct GameTracker* gGT, struct GamepadSystem* gGS);
static void RunDemoMode(struct GameTracker* gGT, struct GamepadSystem* gGS);

u_int CTR_Main()
{
	__main();

	do
	{
		LOAD_NextQueuedFile();
		CDSYS_XAPauseAtEnd();

		switch (sdata->mainGameState)
		{
		case STATE_INIT:        Boot_Run();     break;
		case STATE_LOADING_END: OnLoadingEnd(); break;
		case STATE_RESET_STAGE: OnResetStage(); break;
		case STATE_GAMEPLAY:    OnGameplay();   break;
		}

		if (Config_IsFeatureEnabled(FEATURE_HOT_RELOAD))
		{
			HotReload_Poll();
		}
	} while (true);
}

static void OnLoadingEnd()
{
	struct GameTracker* gGT = sdata->gGT;
	struct GamepadSystem* gGS = sdata->gGamepads;

	ElimBG_Deactivate(gGT);
	MainStats_RestartRaceCountLoss();
	Voiceline_ClearTimeStamp();

	gGT->gameMode1 &= ~END_OF_RACE;

	if (gGT->levelID == MAIN_MENU_LEVEL)
	{
		if (RaceFlag_IsFullyOffScreen() != 0) { RaceFlag_SetFullyOnScreen(); }
	}
	else
	{
		if (RaceFlag_IsFullyOnScreen() != 0) { RaceFlag_BeginTransition(2); }
	}

	DropRain_Reset(gGT);
	GAMEPROG_GetPtrHighScoreTrack();
	Boss_PrepareRace();
	MainInit_FinalizeInit(gGT);
	HotReload_MarkLevelUnpacked();
	GAMEPAD_GetNumConnected(gGS);
	sdata->boolSoundPaused = 0;
	VehBirth_EngineAudio_AllPlayers();

	if (gGT->levelID < NITRO_COURT)
	{
		Audio_SetState_Safe(AUDIO_INTRO_CUTSCENE);
	}
	else if (gGT->levelID < GEM_STONE_VALLEY)
	{
		Audio_SetState_Safe(AUDIO_TRAFFIC_LIGHTS);
	}

	sdata->mainGameState = STATE_GAMEPLAY;
	gGT->clockEffectEnabled &= ~1;

	Race_Reset();

	if (Level_IsPlayable(gGT->levelID)) { sdata->ptrActiveMenu = 0; }
}

static void OnResetStage()
{
	Audio_SetState_Safe(AUDIO_RESET_STAGE);
	MEMPACK_PopState();
	LevInstDef_RePack(sdata->gGT->level1->ptr_mesh_info, 0);
	sdata->mainGameState = STATE_LOADING_END;
}

static void OnGameplay()
{
	struct GameTracker* gGT = sdata->gGT;
	struct GamepadSystem* gGS = sdata->gGamepads;

	if (sdata->Loading.stage != LOAD_STAGE_IDLE)
	{
		if (!AdvanceLoading(gGT))
		{
			return;
		}
	}

	RunFrame(gGT, gGS);
}

static int AdvanceLoading(struct GameTracker* gGT)
{
	int finishedLoading = 0;

	if ((RaceFlag_IsFullyOnScreen() == 1) || (gGT->levelID == NAUGHTY_DOG_CRATE) || (sdata->pause_state != 0))
	{
		gGT->gameMode1 |= LOADING;
	}

	gGT->elapsedTimeMS = FRAME_TIME_MS;

	int stage = sdata->Loading.stage;

	if (stage == LOAD_STAGE_AWAIT_VLC)
	{
		if (sdata->bool_IsLoaded_VlcTable != 1)
		{
			return 0;
		}

		finishedLoading = 1;
	}
	else if (stage == LOAD_STAGE_RESTART)
	{
		if (RaceFlag_IsFullyOnScreen() == 1)
		{
			sdata->mainGameState = STATE_RESET_STAGE;
			sdata->Loading.stage = LOAD_STAGE_IDLE;
			gGT->gameMode1 &= ~LOADING;
			return 0;
		}
	}
	else if (stage == LOAD_STAGE_AWAIT_FLAG)
	{
		if (RaceFlag_IsFullyOnScreen() == 1)
		{
			Level_CommitRequest();

			gGT->hudFlags &= ~8;

			u_int addConfig0 = sdata->Loading.OnBegin.AddBitsConfig0;
			u_int remConfig0 = sdata->Loading.OnBegin.RemBitsConfig0;
			u_int addConfig8 = sdata->Loading.OnBegin.AddBitsConfig8;
			u_int remConfig8 = sdata->Loading.OnBegin.RemBitsConfig8;

			sdata->Loading.OnBegin.AddBitsConfig0 = 0;
			sdata->Loading.OnBegin.RemBitsConfig0 = 0;
			sdata->Loading.OnBegin.AddBitsConfig8 = 0;
			sdata->Loading.OnBegin.RemBitsConfig8 = 0;

			gGT->gameMode1 = (gGT->gameMode1 | addConfig0) & ~remConfig0;
			gGT->gameMode2 = (gGT->gameMode2 | addConfig8) & ~remConfig8;

			HotReload_RepackLevel();
			MainRaceTrack_StartLoad(sdata->Loading.Lev_ID_To_Load);
		}
		else if (RaceFlag_IsFullyOffScreen() == 1)
		{
			RaceFlag_BeginTransition(1);
		}
	}
	else
	{
		sdata->Loading.stage = LOAD_TenStages(gGT, stage, sdata->ptrBigfile1);

		if (sdata->Loading.stage == LOAD_STAGE_DONE)
		{
			if ((gGT->levelID == MAIN_MENU_LEVEL) || (gGT->levelID == SCRAPBOOK))
			{
				MainLoadVLC();
				sdata->Loading.stage = LOAD_STAGE_AWAIT_VLC;
				return 0;
			}

			finishedLoading = 1;
		}
	}

	if (finishedLoading)
	{
		sdata->Loading.stage = LOAD_STAGE_IDLE;
		sdata->mainGameState = STATE_LOADING_END;
		gGT->gameMode1 &= ~LOADING;
		return 0;
	}

	return 1;
}

static void RunFrame(struct GameTracker* gGT, struct GamepadSystem* gGS)
{
	if ((gGT->trafficLightsTimer > SECONDS(-1)) && ((gGT->gameMode1 & (START_OF_RACE | PAUSE_ALL)) == 0))
	{
		gGT->trafficLightsTimer -= gGT->elapsedTimeMS;

		if (gGT->trafficLightsTimer < SECONDS(-1))
		{
			gGT->trafficLightsTimer = SECONDS(-1);
		}
	}

	sdata->frameCounter++;
	GAMEPAD_ProcessAnyoneVars(gGS);
	Race_CaptureInput();
	MainFrame_ResetDB(gGT);

	if (gGT->boolDemoMode != 0)
	{
		RunDemoMode(gGT, gGS);
	}

	if ((gGT->gameMode1 & LOADING) == 0)
	{
		Race_RunGameLogic(gGT, gGS);
		Oxide_HideMenuWheels();
	}

	if (gGT->boolDemoMode != 0)
	{
		gGT->hudFlags &= ~1;
	}

	if (Config_IsFeatureEnabled(FEATURE_HOST_SETTINGS))
	{
		Settings_PollHost();
	}

	Race_Update(gGT);
	Settings_Update();

	gGT->vSync_between_drawSync = 0;
	Race_RunRenderFrame(gGT, gGS);

	if (sdata->boolDraw3D_AdvMask != 0)
	{
		AH_MaskHint_Update();
	}
}

static void RunDemoMode(struct GameTracker* gGT, struct GamepadSystem* gGS)
{
	gGT->hudFlags &= ~1;

	if (sdata->Loading.stage != LOAD_STAGE_IDLE)
	{
		return;
	}

	gGT->demoCountdownTimer--;

	if (gGT->demoCountdownTimer < 1)
	{
		gGT->boolDemoMode = 0;
		gGT->numPlyrNextGame = 1;
		sdata->mainMenuState = MM_STATE_TITLE;
		MainRaceTrack_RequestLoad(MAIN_MENU_LEVEL);
	}
	else if (gGS->anyoneHeldCurr != 0)
	{
		gGT->boolDemoMode = 0;
		MainRaceTrack_RequestLoad(MAIN_MENU_LEVEL);
	}

	int posY = (gGT->numPlyrCurrGame == 1) ? 35 : 100;
	DecalFont_DrawMultiLine(sdata->lngStrings[LNG_DEMO_MODE_HINT], 0x100, posY, 0x200, 2, 0xFFFF8000);
}
