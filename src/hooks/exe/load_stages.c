#include "dll/hot_reload.h"
#include "dll/level.h"
#include "rom.h"

#include <common.h>

#define OVR_END_CRYSTAL    0
#define OVR_END_ARCADE     1
#define OVR_END_RELIC      2
#define OVR_END_TIME_TRIAL 3
#define OVR_END_VERSUS     4

#define OVR_THREADS_MAIN_MENU 0
#define OVR_THREADS_RACING    1
#define OVR_THREADS_ADV_HUB   2
#define OVR_THREADS_CUTSCENE  3

#define AUDIO_NONE          (-1)
#define AUDIO_STOP_CSEQ     1
#define AUDIO_CREDITS       2
#define AUDIO_INTRO         3
#define AUDIO_ND_CRATE      4
#define AUDIO_ADV_HUB       5
#define AUDIO_ADV_PODIUM    6
#define AUDIO_MAIN_MENU     7

#define PRIM_MEM_BASE 0x80600000
#define PRIM_MEM_SIZE (0x100000 - 0x1000)

#define LEVEL_LOD_SOLO 8

#define NUM_PODIUM_MODELS 8

static void ApplyStagedLevelIfRequested();
static int PickEndRaceOverlay(struct GameTracker* gGT);
static int PickThreadOverlay(struct GameTracker* gGT, int levelID);
static int PickLoadedAudioState(struct GameTracker* gGT);
static void ConfigureGameModeForLevel(struct GameTracker* gGT, int levelID);
static void ReservePrimAndOtMemory(struct GameTracker* gGT);
static void AdoptGhostDriversFromLevel();
static void SetupPodiumModels(struct GameTracker* gGT);
static void OnStagedLevelLoaded(struct LoadQueueSlot* lqs);

int LOAD_TenStages(struct GameTracker* gGT, int loadingStage, struct BigHeader* bigfile)
{
	if (sdata->load_inProgress != 0)
	{
		return loadingStage;
	}

	int levelID = gGT->levelID;

	ApplyStagedLevelIfRequested();

	switch (loadingStage)
	{
	case 0:
	{
		if ((levelID != ADVENTURE_GARAGE) && (levelID != NAUGHTY_DOG_CRATE))
		{
			Cutscene_VolumeBackup();
		}

		CDSYS_XAPauseRequest();

		MEMPACK_SwapPacks(0);
		MEMPACK_PopToState(sdata->bookmarkID);
		sdata->bookmarkID = MEMPACK_PushState();

		gGT->hudFlags &= ~(1 | 8);
		gGT->renderFlags &= 0x1000;

		gGT->level1 = 0;
		gGT->level2 = 0;

		DrawSync(0);

		gGT->overlayTransition = 0;
		gGT->gameMode1 &= ~(GAME_CUTSCENE | END_OF_RACE | ADVENTURE_ARENA | MAIN_MENU);
		gGT->gameMode2 &= ~(LEV_SWAP | CREDITS | NO_LEV_INSTANCE);

		gGT->visMem1 = 0;
		gGT->visMem2 = 0;

		ConfigureGameModeForLevel(gGT, levelID);

		gGT->hudFlags |= 2;
		gGT->Debug_ToggleNormalSpawn = 1;

		sdata->levelLOD = gGT->numPlyrCurrGame;

		if ((gGT->gameMode1 & MAIN_MENU) != 0)
		{
			sdata->levelLOD = 1;
		}

		if ((gGT->gameMode1 & (TIME_TRIAL | RELIC_RACE)) != 0)
		{
			sdata->levelLOD = LEVEL_LOD_SOLO;
		}

		ReservePrimAndOtMemory(gGT);

		if (((gGT->gameMode1 & (GAME_CUTSCENE | ADVENTURE_ARENA)) != 0) ||
			((gGT->gameMode2 & CREDITS) != 0))
		{
			MainInit_JitPoolsNew(gGT);
		}

		break;
	}

	case 1:
	{
		if (sdata->XA_State == 4)
		{
			return loadingStage;
		}

		if ((gGT->gameMode2 & (LEV_SWAP | CREDITS)) != 0)
		{
			break;
		}

		if ((gGT->gameMode1 & (GAME_CUTSCENE | MAIN_MENU)) != 0)
		{
			break;
		}

		LOAD_OvrEndRace((u_int*)PickEndRaceOverlay(gGT));
		break;
	}

	case 2:
	{
		LOAD_OvrLOD(gGT->numPlyrCurrGame);
		break;
	}

	case 3:
	{
		LOAD_OvrThreads((u_int)PickThreadOverlay(gGT, levelID));
		break;
	}

	case 4:
	{
		if ((levelID != ADVENTURE_GARAGE) && (levelID != NAUGHTY_DOG_CRATE))
		{
			Music_Restart();
		}

		if ((gGT->gameMode1 & MAIN_MENU) != 0)
		{
			switch (sdata->mainMenuState)
			{
			case MM_STATE_TITLE:        MM_JumpTo_Title_FirstTime(); break;
			case MM_STATE_CHARACTERS:   MM_JumpTo_Characters();      break;
			case MM_STATE_TRACKS:       MM_JumpTo_TrackSelect();     break;
			case MM_STATE_BATTLE_SETUP: MM_JumpTo_BattleSetup();     break;
			case MM_STATE_ADV_GARAGE:   CS_Garage_Init();            break;
			case MM_STATE_SCRAPBOOK:    MM_JumpTo_Scrapbook();       break;
			}
		}

		sdata->ptrMPK = 0;
		sdata->load_inProgress = 1;

		if (levelID == CUSTOM_LEVEL_ID)
		{
			AdoptGhostDriversFromLevel();
		}

		LOAD_DriverMPK(bigfile, sdata->levelLOD, &LOAD_Callback_DriverModels);
		break;
	}

	case 5:
	{
		sdata->PLYROBJECTLIST = (sdata->ptrMPK == 0) ? 0 : (unsigned int)sdata->ptrMPK + 4;

		LibraryOfModels_Clear(gGT);
		LOAD_GlobalModelPtrs_MPK();
		DecalGlobal_Clear(gGT);

		if ((sdata->ptrMPK == 0) || (*(int*)sdata->ptrMPK == 0))
		{
			gGT->mpkIcons = 0;
		}
		else
		{
			DecalGlobal_Store(gGT, (struct Icon*)*(int*)sdata->ptrMPK);
			gGT->mpkIcons = *(int*)sdata->ptrMPK;
		}

		if ((levelID != ADVENTURE_GARAGE) && (levelID != NAUGHTY_DOG_CRATE))
		{
			Music_Stop();
			CseqMusic_StopAll();
			Music_LoadBanks();
		}
		break;
	}

	case 6:
	{
		if ((levelID != ADVENTURE_GARAGE) && (levelID != NAUGHTY_DOG_CRATE))
		{
			if (Music_AsyncParseBanks() == 0)
			{
				return loadingStage;
			}

			Cutscene_VolumeRestore();
		}

		sdata->load_inProgress = 1;

		int levelLOD = sdata->levelLOD;

		if (levelID == CUSTOM_LEVEL_ID)
		{
			if (HotReload_HasStagedTrack())
			{
				sdata->load_inProgress = 0;
				HotReload_UploadVram();
				break;
			}

			levelLOD = 1;
		}

		u_int vramIndex = LOAD_GetBigfileIndex(levelID, levelLOD, LVI_VRAM);
		LOAD_AppendQueue((int)bigfile, LT_VRAM, vramIndex, 0, 0);

		u_int levIndex = LOAD_GetBigfileIndex(levelID, levelLOD, LVI_LEV);

		if (Level_IsPlayable(levelID))
		{
			LOAD_AppendQueue((int)bigfile, LT_RAW, levIndex, CUSTOM_MAP_PTR_ADDR, &OnStagedLevelLoaded);
		}
		else
		{
			LOAD_AppendQueue((int)bigfile, LT_DRAM, levIndex, 0, &LOAD_Callback_LEV);
		}

		if ((gGT->gameMode2 & LEV_SWAP) != 0)
		{
			u_int ptrIndex = LOAD_GetBigfileIndex(levelID, levelLOD, LVI_PTR);
			LOAD_AppendQueue((int)bigfile, LT_RAW, ptrIndex, sdata->PatchMem_Ptr, LOAD_Callback_PatchMem);
		}
		break;
	}

	case 7:
	{
		struct Level* lev = (levelID == CUSTOM_LEVEL_ID) ? (struct Level*)CUSTOM_LEV_ADDR : sdata->ptrLevelFile;

		gGT->level1 = lev;

		if (lev != 0)
		{
			gGT->visMem1 = lev->visMem;
			DecalGlobal_Store(gGT, (struct Icon*)lev->levTexLookup);
		}

		DebugFont_Init(gGT);

		if (lev != 0)
		{
			LibraryOfModels_Store(gGT, lev->numModels, lev->ptrModelsPtrArray);

			gGT->ptrCircle = (u_int)DecalGlobal_FindInLEV(lev, rdata.s_circle);
			gGT->ptrClod = (u_int)DecalGlobal_FindInLEV(lev, rdata.s_clod);
			gGT->ptrDustpuff = (u_int)DecalGlobal_FindInLEV(lev, rdata.s_dustpuff);
			gGT->ptrSmoking = (u_int)DecalGlobal_FindInLEV(lev, rdata.s_smokering);
			gGT->ptrSparkle = (u_int)DecalGlobal_FindInLEV(lev, rdata.s_sparkle);
		}

		if (gGT->mpkIcons != 0)
		{
			u_int* icons = (u_int*)*(u_int*)((u_int)gGT->mpkIcons + 4);

			gGT->trafficLightIcon[0] = (struct Icon*)DecalGlobal_FindInMPK(icons, rdata.s_lightredoff);
			gGT->trafficLightIcon[1] = (struct Icon*)DecalGlobal_FindInMPK(icons, rdata.s_lightredon);
			gGT->trafficLightIcon[2] = (struct Icon*)DecalGlobal_FindInMPK(icons, rdata.s_lightgreenoff);
			gGT->trafficLightIcon[3] = (struct Icon*)DecalGlobal_FindInMPK(icons, rdata.s_lightgreenon);
		}

		gGT->gameMode1_prevFrame = 1;

		if (((gGT->gameMode1 & (GAME_CUTSCENE | ADVENTURE_ARENA)) == 0) && ((gGT->gameMode2 & CREDITS) == 0))
		{
			MainInit_JitPoolsNew(gGT);
		}
		break;
	}

	case 8:
	{
		SetupPodiumModels(gGT);

		int audioState = PickLoadedAudioState(gGT);
		if (audioState != AUDIO_NONE)
		{
			Audio_SetState_Safe(audioState);
		}

		break;
	}

	case 9:
	{
		if (sdata->XA_State == 2)
		{
			return loadingStage;
		}

		int inMenu = ((gGT->gameMode1 & MAIN_MENU) != 0) && (levelID != ADVENTURE_GARAGE);

		if (!inMenu)
		{
			gGT->renderFlags = ((gGT->gameMode2 & CREDITS) == 0) ? (gGT->renderFlags | 0xFFFFEFFF) : ((gGT->renderFlags & 0x1000) | 0x20);
		}
		else
		{
			gGT->renderFlags = (gGT->renderFlags & 0x1000) | 0x20;

			if (RaceFlag_IsFullyOffScreen() == 1)
			{
				RaceFlag_BeginTransition(1);
			}
		}

		gGT->hudFlags |= 8;
		gGT->framesInThisLEV = 0;
		gGT->msInThisLEV = 0;

		ElimBG_Deactivate(gGT);

		return LOAD_STAGE_DONE;
	}

	default:
		return loadingStage;
	}

	return loadingStage + 1;
}

static void ApplyStagedLevelIfRequested()
{
	volatile int* trigger = TRIGGER_HOT_RELOAD;

	if (*trigger != HOT_RELOAD_EXEC)
	{
		return;
	}

	HotReload_ApplyStagedLevel();
	*trigger = HOT_RELOAD_DONE;
}

static void ConfigureGameModeForLevel(struct GameTracker* gGT, int levelID)
{
	int defaultTo1P = 1;

	if (levelID >= SCRAPBOOK)
	{
		gGT->gameMode1 |= MAIN_MENU;
	}
	else if (levelID >= CREDITS_CRASH)
	{
		gGT->gameMode1 |= GAME_CUTSCENE;
		gGT->gameMode2 |= (LEV_SWAP | CREDITS);
	}
	else if (levelID >= NAUGHTY_DOG_CRATE)
	{
		gGT->gameMode1 |= GAME_CUTSCENE;
	}
	else if (levelID >= MAIN_MENU_LEVEL)
	{
		gGT->gameMode1 |= MAIN_MENU;

		if (levelID == ADVENTURE_GARAGE)
		{
			sdata->mainMenuState = MM_STATE_ADV_GARAGE;
		}
		else
		{
			gGT->numPlyrNextGame = gGT->numPlyrCurrGame;
			gGT->numPlyrCurrGame = 4;
			defaultTo1P = 0;
		}
	}
	else if (levelID >= INTRO_RACE_TODAY)
	{
		gGT->gameMode1 |= GAME_CUTSCENE;
		gGT->gameMode2 |= LEV_SWAP;
	}
	else if (levelID >= GEM_STONE_VALLEY)
	{
		gGT->gameMode1 |= ADVENTURE_ARENA;
		gGT->gameMode2 |= LEV_SWAP;
	}
	else
	{
		gGT->numPlyrCurrGame = gGT->numPlyrNextGame;
		defaultTo1P = 0;
	}

	if (defaultTo1P)
	{
		gGT->numPlyrCurrGame = 1;
		gGT->numPlyrNextGame = 1;
	}
}

static void ReservePrimAndOtMemory(struct GameTracker* gGT)
{
	MainInit_PrimMem((u_int*)gGT);
	MainInit_OTMem((u_int*)gGT);

	for (int i = 0; i < 2; i++)
	{
		struct PrimMem* primMem = &gGT->db[i].primMem;
		void* start = (void*)(PRIM_MEM_BASE + i * 0x100000);

		primMem->size = PRIM_MEM_SIZE;
		primMem->unk2 = (int)start;
		primMem->curr = start;
		primMem->start = start;

		void* end = (void*)((int)start + PRIM_MEM_SIZE);
		primMem->end = end;
		primMem->endMin100 = (void*)((int)end - 0x100);
	}
}

static void OnStagedLevelLoaded(struct LoadQueueSlot* lqs)
{
	// LOAD_DramFileCallback only runs the pointer map when the offset is
	// non-negative -- adventure hub LEVs store a negative one. Match that, or a
	// negative offset sends LOAD_RunPtrMap relocating whatever is behind us.
	if (*CUSTOM_MAP_PTR_ADDR >= 0)
	{
		HotReload_ApplyStagedLevel();
	}

	sdata->ptrLevelFile = (struct Level*)CUSTOM_LEV_ADDR;
	sdata->load_inProgress = 0;
}

static void AdoptGhostDriversFromLevel()
{
	struct Level* lev = (struct Level*)CUSTOM_LEV_ADDR;

	if (lev->ptrSpawnType1->count <= 0)
	{
		return;
	}

	void** spawns = ST1_GETPOINTERS(lev->ptrSpawnType1);

	if (spawns[ST1_NTROPY])
	{
		data.characterIDs[2] = ((struct GhostHeader*)spawns[ST1_NTROPY])->characterID;
	}

	if (spawns[ST1_NOXIDE])
	{
		data.characterIDs[3] = ((struct GhostHeader*)spawns[ST1_NOXIDE])->characterID;
	}
}

static int PickEndRaceOverlay(struct GameTracker* gGT)
{
	if ((gGT->gameMode1 & CRYSTAL_CHALLENGE) != 0)
	{
		return OVR_END_CRYSTAL;
	}

	if ((gGT->gameMode1 & RELIC_RACE) != 0)
	{
		return OVR_END_RELIC;
	}

	if ((gGT->gameMode1 & TIME_TRIAL) != 0)
	{
		return OVR_END_TIME_TRIAL;
	}

	if ((gGT->gameMode1 & (ARCADE_MODE | ADVENTURE_MODE)) != 0)
	{
		return OVR_END_ARCADE;
	}

	return OVR_END_VERSUS;
}

static int PickThreadOverlay(struct GameTracker* gGT, int levelID)
{
	if ((levelID != ADVENTURE_GARAGE) && ((gGT->gameMode1 & MAIN_MENU) != 0))
	{
		return OVR_THREADS_MAIN_MENU;
	}

	if (levelID <= LAB_BASEMENT)
	{
		return OVR_THREADS_RACING;
	}

	if ((levelID <= CITADEL_CITY) && (gGT->podiumRewardID == 0))
	{
		return OVR_THREADS_ADV_HUB;
	}

	return OVR_THREADS_CUTSCENE;
}

static void SetupPodiumModels(struct GameTracker* gGT)
{
	if (((gGT->gameMode1 & ADVENTURE_ARENA) == 0) || (gGT->podiumRewardID == 0))
	{
		return;
	}

	int* podiumModel = &data.podiumModel_firstPlace;

	for (int i = 0; i < NUM_PODIUM_MODELS; i++, podiumModel++)
	{
		if (*podiumModel == 0)
		{
			continue;
		}

		if (i < NUM_PODIUM_MODELS - 1)
		{
			*podiumModel += 4;
		}

		struct Model* model = (struct Model*)*podiumModel;

		if (model->id != -1)
		{
			gGT->modelPtr[model->id] = model;
		}
	}

	MEMPACK_SwapPacks((int)gGT->activeMempackIndex);
}

static int PickLoadedAudioState(struct GameTracker* gGT)
{
	int levelID = gGT->levelID;

	if (levelID == MAIN_MENU_LEVEL)
	{
		return AUDIO_MAIN_MENU;
	}

	if ((levelID >= GEM_STONE_VALLEY) && (levelID <= CITADEL_CITY))
	{
		return (gGT->podiumRewardID == 0) ? AUDIO_ADV_HUB : AUDIO_ADV_PODIUM;
	}

	if (levelID == INTRO_RACE_TODAY)
	{
		return AUDIO_INTRO;
	}

	if (levelID == CREDITS_CRASH)
	{
		return AUDIO_CREDITS;
	}

	if (levelID == NAUGHTY_DOG_CRATE)
	{
		return AUDIO_ND_CRATE;
	}

	if ((levelID == OXIDE_ENDING) || (levelID == OXIDE_TRUE_ENDING))
	{
		return AUDIO_STOP_CSEQ;
	}

	return AUDIO_NONE;
}
