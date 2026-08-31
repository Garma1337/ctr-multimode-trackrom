#ifndef ROM_H
#define ROM_H

#include "config/config_schema.h"
#include "config/overrides.h"

#include <common.h>

#ifndef CUSTOM_LEVEL_ID
#define CUSTOM_LEVEL_ID 0
#endif

#ifndef BAKED_TRACK
#define BAKED_TRACK 0
#endif

#define TRIGGER_HOT_RELOAD (volatile int*) 0x8000C000
#define TRIGGER_VRM_RELOAD (volatile int*) 0x8000C004
#define GHOST_READY        (volatile int*) 0x8000C008
#define CHAR_MODEL_PTRS    (struct Model**) 0x8000C010

#define HOST_SETTINGS_LOCATION 0x8000C080
#define HOST_SETTINGS_MAGIC    0x53544553 // 'SETS'
#define HOST_SETTINGS          ((volatile HostSettings*) HOST_SETTINGS_LOCATION)

typedef struct HostSettings
{
	int magic;      // HOST_SETTINGS_MAGIC once the editor has written here
	int sequence;   // changed by the editor on every push
	Config config;
} HostSettings;

_Static_assert(sizeof(HostSettings) == (8 + sizeof(Config)), "HostSettings layout changed; update the editor side too");

typedef enum HotReloadStep
{
	HOT_RELOAD_DONE = 0,
	HOT_RELOAD_START,
	HOT_RELOAD_LOAD,
	HOT_RELOAD_READY,
	HOT_RELOAD_EXEC,
} HotReloadStep;

#define VRM_FILESIZE    0x70038
#define GHOST_FILESIZE  0x3E00
#define DRIVER_FILESIZE 0x54BC4

#define VRM_LOCATION        0x80200000
#define GHOST_SIZE_LOCATION (VRM_LOCATION + VRM_FILESIZE)
#define GHOST_LOCATION      (GHOST_SIZE_LOCATION + sizeof(int))
#define DRIVER_LOCATION     (GHOST_LOCATION + GHOST_FILESIZE)
#define DLL_LOCATION        (DRIVER_LOCATION + DRIVER_FILESIZE)

#define CD_SECTOR_SIZE      0x800

#define PRIM_MEM_BASE           0x80600000
#define CUSTOM_LEV_MAP_LOCATION 0x802F0800
#define CUSTOM_LEV_LOCATION     (CUSTOM_LEV_MAP_LOCATION + sizeof(int))

#define CUSTOM_LEV_MAX_SIZE     (PRIM_MEM_BASE - CUSTOM_LEV_MAP_LOCATION)

#define CUSTOM_VRAM_ADDR    (char*) VRM_LOCATION
#define GHOST_SIZE_ADDR     (int*) GHOST_SIZE_LOCATION
#define GHOST_ADDR          (char*) GHOST_LOCATION
#define DRIVER_ADDR         (char*) DRIVER_LOCATION
#define CUSTOM_MAP_PTR_ADDR (int*) CUSTOM_LEV_MAP_LOCATION
#define CUSTOM_LEV_ADDR     (char*) CUSTOM_LEV_LOCATION
#define DLL_ADDR            (char*) DLL_LOCATION
#define DLL_PATH            "\\DLL.BIN;1"

_Static_assert(DLL_LOCATION == 0x802C8A00, "DLL_LOCATION must match the DLL line in buildList.txt");
_Static_assert(CUSTOM_LEV_MAP_LOCATION > DLL_LOCATION, "the custom LEV must sit above the DLL");

// sdata->mainGameState
typedef enum MainState
{
	STATE_INIT = 0,
	STATE_LOADING_END = 1,
	STATE_RESET_STAGE = 2,
	STATE_GAMEPLAY = 3,
} MainState;

// sdata->mainMenuState
typedef enum MainMenuState
{
	MM_STATE_TITLE = 0,
	MM_STATE_CHARACTERS = 1,
	MM_STATE_TRACKS = 2,
	MM_STATE_BATTLE_SETUP = 3,
	MM_STATE_ADV_GARAGE = 4,
	MM_STATE_SCRAPBOOK = 5,
} MainMenuState;

// sdata->Loading.stage
typedef enum LoadStage
{
	LOAD_STAGE_AWAIT_VLC = -6,
	LOAD_STAGE_RESTART = -5,
	LOAD_STAGE_AWAIT_FLAG = -4,
	LOAD_STAGE_DONE = -2,
	LOAD_STAGE_IDLE = -1,
	LOAD_STAGE_FIRST = 0,
} LoadStage;

#endif
