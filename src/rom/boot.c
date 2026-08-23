#include "boot.h"
#include "dll/boss.h"
#include "dll/main_menu.h"
#include "dll/oxide.h"
#include "dll/settings.h"
#include "drivers.h"
#include "rom.h"

#include <common.h>

#define MEMPACK_SIZE 0x200000 // 2 MB

#define LANG_ENGLISH    1
#define BI_SHARED_VRM   0x102

#define FRAME_WIDTH  0x200
#define FRAME_HEIGHT 0xD8

// The two frame buffers sit one above the other in VRAM
#define BUFFER_1_TOP 0x128

#define SCREEN_OFFSET_Y 0xC
#define SCREEN_WIDTH    0x100

#define GEOM_SCREEN_DISTANCE 0x140

#define ALL_BATTLE_WEAPONS 0x34DE
#define TEAM_PER_PLAYER    0x3020100

#define OVERLAY_NONE 0xFF

static void Boot_SetupDoubleBuffers(struct GameTracker* gGT)
{
	SetDefDrawEnv(&gGT->db[0].drawEnv, 0, 0, FRAME_WIDTH, FRAME_HEIGHT);
	SetDefDrawEnv(&gGT->db[1].drawEnv, 0, BUFFER_1_TOP, FRAME_WIDTH, FRAME_HEIGHT);
	SetDefDispEnv(&gGT->db[0].dispEnv, 0, BUFFER_1_TOP, FRAME_WIDTH, FRAME_HEIGHT);
	SetDefDispEnv(&gGT->db[1].dispEnv, 0, 0, FRAME_WIDTH, FRAME_HEIGHT);

	for (int i = 0; i < 2; i++)
	{
		gGT->db[i].dispEnv.screen.x = 0;
		gGT->db[i].dispEnv.screen.y = SCREEN_OFFSET_Y;
		gGT->db[i].dispEnv.screen.w = SCREEN_WIDTH;
		gGT->db[i].dispEnv.screen.h = FRAME_HEIGHT;

		gGT->db[i].drawEnv.isbg = 1;
		gGT->db[i].drawEnv.r0 = 0;
		gGT->db[i].drawEnv.g0 = 0;
		gGT->db[i].drawEnv.b0 = 0;
	}
}

static void Boot_SetupRaceDefaults(struct GameTracker* gGT)
{
	gGT->battleLifeLimit = 5;
	gGT->constVal_9000 = 9000;
	gGT->numLaps = 3;
	gGT->battleSetup.enabledWeapons |= ALL_BATTLE_WEAPONS;
	gGT->numPlyrCurrGame = 1;
	gGT->numPlyrNextGame = 1;
	*(u_int*)&gGT->battleSetup.teamOfEachPlayer = TEAM_PER_PLAYER;
	gGT->trafficLightsTimer = SECONDS(-1);
}

static void Boot_SetupGeometry(struct GameTracker* gGT)
{
	InitGeom();

	SetGeomOffset(0x100, 0x78);
	SetGeomScreen(GEOM_SCREEN_DISTANCE);
	RenderBucket_InitDepthGTE();
	Vector_BakeMatrixTable();

	gGT->swapchainIndex = 0;
	gGT->backBuffer = &gGT->db[0];

	gGT->overlayIndex_EndOfRace = OVERLAY_NONE;
	gGT->overlayIndex_LOD = OVERLAY_NONE;
	gGT->overlayIndex_Threads = OVERLAY_NONE;
}

void Boot_Run()
{
	struct GameTracker* gGT = sdata->gGT;
	struct GamepadSystem* gGS = sdata->gGamepads;

	SetVideoMode(0);
	ResetCallback();

	MEMPACK_Init(MEMPACK_SIZE);
	LOAD_InitCD();
	RaceFlag_SetFullyOffScreen();

	ResetGraph(0);
	SetGraphDebug(0);
	MainInit_VRAMClear();
	SetDispMask(1);
	Boot_SetupDoubleBuffers(gGT);
	Boot_SetupRaceDefaults(gGT);

	Timer_Init();
	EnterCriticalSection();
	DrawSyncCallback(&MainDrawCb_DrawSync);
	ExitCriticalSection();

	MEMCARD_InitCard();
	VSync(0);
	GAMEPAD_Init(gGS);
	VSync(0);
	GAMEPAD_GetNumConnected(gGS);

	sdata->ptrBigfile1 = LOAD_ReadDirectory(rdata.s_PathTo_Bigfile);
	LOAD_LangFile((int)sdata->ptrBigfile1, LANG_ENGLISH);
	GAMEPROG_NewGame_OnBoot();
	gGT->overlayIndex_null_notUsed = 0;
	gGT->levelID = MAIN_MENU_LEVEL;

	Boot_SetupGeometry(gGT);

	PutDispEnv(&gGT->db[1].dispEnv);
	PutDrawEnv(&gGT->db[1].drawEnv);
	DrawSync(0);
	howl_InitGlobals(data.kartHwlPath);
	VSyncCallback(MainDrawCb_Vsync);
	DecalGlobal_Clear(gGT);

	int vramSize;
	LOAD_VramFile(sdata->ptrBigfile1, BI_SHARED_VRM, 0, &vramSize, 0xFFFFFFFF);

	sdata->mainGameState = STATE_GAMEPLAY;
	sdata->Loading.stage = LOAD_STAGE_FIRST;
	gGT->gameMode1 |= LOADING;
	gGT->clockEffectEnabled &= ~1;

	HOST_SETTINGS->magic = 0;

	Drivers_Preload(sdata->ptrBigfile1);

	int dllSize;
	LOAD_XnfFile(DLL_PATH, DLL_ADDR, &dllSize);

	MainMenu_InstallStrings();
	Boss_InstallRows();
	MainMenu_Build();
	Oxide_ScaleRaceModel();
	Settings_ApplyCodePatches();
}
