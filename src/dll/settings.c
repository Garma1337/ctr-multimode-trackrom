#include "../rom.h"
#include "boss.h"
#include "config.h"
#include "input.h"
#include "math.h"
#include "prim.h"
#include "settings.h"

#include <common.h>

#define STEP_FINE_MS   500
#define STEP_COARSE_MS 5000
#define TIME_MIN_MS    500
#define TIME_MAX_MS    599500

#define LAPS_STEP 2

#define REPEAT_DELAY_FRAMES  15
#define REPEAT_RATE_FRAMES   3

#define VISIBLE_ROWS   12

#define PANEL_PAD_X    10
#define PANEL_PAD_Y    7
#define TITLE_HEIGHT   24
#define ROW_SPACING    10
#define HINT_HEIGHT    16
#define COLUMN_GAP     30
#define CURSOR_GUTTER  14

#define GLYPH_CROSS    "*"
#define GLYPH_TRIANGLE "^"
#define HINT_CONFIRM   GLYPH_CROSS " Confirm"
#define HINT_BACK      GLYPH_TRIANGLE " Back"
#define HINT_LINES     1
#define VALUE_SAMPLE   "00:00.0"

#define INTRO_CAM_AT     ((unsigned long*)0x8001AF3C)
#define INTRO_CAM_STOCK  0x28420004UL // slti $v0, $v0, 4
#define INTRO_CAM_SKIP   0x28420010UL // slti $v0, $v0, 16

#define GHOST_THREAD_AT  ((unsigned long*)0x800277F0)
#define GHOST_STOCK      0x34421000UL // ori $v0, $v0, 0x1000
#define GHOST_KILL       0x34420800UL // ori $v0, $v0, 0x800

#define COLOR_TITLE   BLUE
#define COLOR_ROW     ORANGE
#define COLOR_ACTIVE  LIME_GREEN
#define COLOR_HINT    GRAY

#define SFX_CURSOR    0
#define SFX_BACK      2
#define SFX_CONFIRM   1

typedef enum FieldKind
{
	FIELD_TIME = 0,
	FIELD_LAPS,
	FIELD_ENUM,
	FIELD_TOGGLE
} FieldKind;

static const char* const fieldLabels[SETTINGS_BOSS_FIRST] = {
	"Relic Time - Sapphire",
	"Relic Time - Gold",
	"Relic Time - Platinum",

	"Crystal Time Limit",
	"Ghost Time 1",
	"Ghost Time 2",
	"Ghost Character 1",
	"Ghost Character 2",
	"Lap Count",
	"Intro Cutscene",
	"Time Trial Ghosts",
	"High Detail Tracks",
	"CTR Token Color",

	"Freecam",
	"Debug HUD",
	"Reserves Display",
	"Hot Reload",
	"Host Settings",
	"Updated Engine Stats",

	"Mode - Arcade",
	"Mode - Relic Race",
	"Mode - Time Trial",
	"Mode - Crystal Race",
	"Mode - CTR Token",
	"Mode - Boss Race",

};

static const unsigned char fieldKinds[SETTINGS_FEATURE_FIRST] = {
	FIELD_TIME,
	FIELD_TIME,
	FIELD_TIME,
	FIELD_TIME,
	FIELD_TIME,
	FIELD_TIME,
	FIELD_ENUM,
	FIELD_ENUM,
	FIELD_LAPS,
	FIELD_TOGGLE,
	FIELD_TOGGLE,
	FIELD_TOGGLE,
	FIELD_ENUM,
};

static const char* const tokenNames[TOKEN_COLOR_COUNT] = {
	"Red",
	"Green",
	"Blue",
	"Yellow",
	"Purple",
};

static const char* const characterNames[GHOST_CHARACTER_COUNT] = {
	"Crash",
	"Cortex",
	"Tiny",
	"Coco",
	"N. Gin",
	"Dingodile",
	"Polar",
	"Pura",
	"Pinstripe",
	"Papu Papu",
	"Ripper Roo",
	"Komodo Joe",
	"N. Tropy",
	"Penta",
	"Fake Crash",
	"N. Oxide",
};

static const char* const* fieldOptions[SETTINGS_FIELD_COUNT] = {
	[SETTINGS_CTR_TOKEN] = tokenNames,
	[SETTINGS_GHOST_CHARACTER_1] = characterNames,
	[SETTINGS_GHOST_CHARACTER_2] = characterNames,
};

static const unsigned char fieldOptionCount[SETTINGS_FIELD_COUNT] = {
	[SETTINGS_CTR_TOKEN] = TOKEN_COLOR_COUNT,
	[SETTINGS_GHOST_CHARACTER_1] = GHOST_CHARACTER_COUNT,
	[SETTINGS_GHOST_CHARACTER_2] = GHOST_CHARACTER_COUNT,

	[SETTINGS_BOSS_ITEM_PRESET_1] = BOSS_ITEM_PRESET_COUNT,
	[SETTINGS_BOSS_ITEM_PRESET_2] = BOSS_ITEM_PRESET_COUNT,
	[SETTINGS_BOSS_ITEM_PRESET_3] = BOSS_ITEM_PRESET_COUNT,
	[SETTINGS_BOSS_ITEM_PRESET_4] = BOSS_ITEM_PRESET_COUNT,
	[SETTINGS_BOSS_ITEM_PRESET_5] = BOSS_ITEM_PRESET_COUNT,
};

static int isOpen = 0;
static int cursor = 0;
static int scroll = 0;
static int repeatTimer = 0;
static int closing = 0;
static struct RectMenu* savedMenu = 0;
static int draft[SETTINGS_FIELD_COUNT];
static unsigned char visibleField[SETTINGS_FIELD_COUNT];
static int visibleCount = 0;
static int commitOnClose = 0;
static int columnLabelWidth = 0;
static int columnValueWidth = 0;
static int hostSequence = 0;
static int hostSeen = 0;

static const char* Settings_GetLabel(int field)
{
	if (field < SETTINGS_BOSS_FIRST)
	{
		return fieldLabels[field];
	}

	if (field < SETTINGS_BOSS_END)
	{
		return Boss_GetName(field - SETTINGS_BOSS_FIRST);
	}

	return Boss_GetItemLabel(field - SETTINGS_BOSS_ITEM_PRESET_FIRST);
}

static const char* Settings_GetOptionName(int field, int option)
{
	if (field >= SETTINGS_BOSS_ITEM_PRESET_FIRST)
	{
		return bossItemPresetRegistry[option].name;
	}

	return fieldOptions[field][option];
}

static int Settings_GetKind(int field)
{
	if (field < SETTINGS_FEATURE_FIRST)
	{
		return fieldKinds[field];
	}

	return (field >= SETTINGS_BOSS_ITEM_PRESET_FIRST) ? FIELD_ENUM : FIELD_TOGGLE;
}

static int Settings_GetCursorField(void)
{
	return visibleField[cursor];
}

static int Settings_CalculateTicksFromMs(int ms)
{
	return (Math_Clamp(ms, TIME_MIN_MS, TIME_MAX_MS) * SECOND) / 1000;
}

static void Settings_FormatTime(char* out, int ms)
{
	int tenths = ms / 100;

	sprintf(out, "%d:%02d.%d", tenths / 600, (tenths / 10) % 60, tenths % 10);
}

static void Settings_FormatValue(char* out, int field)
{
	switch (Settings_GetKind(field))
	{
	case FIELD_TIME:
		Settings_FormatTime(out, draft[field]);
		return;

	case FIELD_LAPS:
		sprintf(out, "%d", draft[field]);
		return;

	case FIELD_ENUM:
		sprintf(out, "%s", Settings_GetOptionName(field, draft[field]));
		return;

	case FIELD_TOGGLE:
		sprintf(out, "%s", draft[field] ? "On" : "Off");
		return;
	}
}

static void Settings_UnpackBits(unsigned int bits, int first, int end)
{
	for (int i = first; i < end; i++)
	{
		draft[i] = (bits & (1 << (i - first))) != 0;
	}
}

static unsigned int Settings_PackBits(int first, int end)
{
	unsigned int bits = 0;

	for (int i = first; i < end; i++)
	{
		if (draft[i])
		{
			bits |= 1 << (i - first);
		}
	}

	return bits;
}

static void Settings_LoadDraft(void)
{
	const Config* config = Config_Get();

	draft[SETTINGS_RELIC_SAPPHIRE] = config->relicSapphire;
	draft[SETTINGS_RELIC_GOLD] = config->relicGold;
	draft[SETTINGS_RELIC_PLATINUM] = config->relicPlatinum;
	draft[SETTINGS_CRYSTAL_TIME] = config->crystalTime;
	draft[SETTINGS_GHOST_TIME_1] = config->ghostTime[0];
	draft[SETTINGS_GHOST_TIME_2] = config->ghostTime[1];
	draft[SETTINGS_GHOST_CHARACTER_1] = config->ghostCharacter[0];
	draft[SETTINGS_GHOST_CHARACTER_2] = config->ghostCharacter[1];
	draft[SETTINGS_LAPS] = config->laps;
	draft[SETTINGS_INTRO_CUTSCENE] = (config->introCutscene != 0);
	draft[SETTINGS_GHOST] = (config->ghosts != 0);
	draft[SETTINGS_HIGH_LOD] = (config->highLod != 0);
	draft[SETTINGS_CTR_TOKEN] = config->ctrToken;

	for (int boss = 0; boss < CONFIG_BOSS_COUNT; boss++)
	{
		draft[SETTINGS_BOSS_ITEM_PRESET_FIRST + boss] = config->bossItemPreset[boss];
	}

	Settings_UnpackBits(config->features, SETTINGS_FEATURE_FIRST, SETTINGS_FEATURE_END);
	Settings_UnpackBits(config->modes, SETTINGS_MODE_FIRST, SETTINGS_MODE_END);
	Settings_UnpackBits(config->bosses, SETTINGS_BOSS_FIRST, SETTINGS_BOSS_END);
}

static void Settings_Commit(void)
{
	Config next = *Config_Get();

	next.relicSapphire = draft[SETTINGS_RELIC_SAPPHIRE];
	next.relicGold = draft[SETTINGS_RELIC_GOLD];
	next.relicPlatinum = draft[SETTINGS_RELIC_PLATINUM];
	next.crystalTime = draft[SETTINGS_CRYSTAL_TIME];
	next.ghostTime[0] = draft[SETTINGS_GHOST_TIME_1];
	next.ghostTime[1] = draft[SETTINGS_GHOST_TIME_2];
	next.ghostCharacter[0] = (unsigned char)draft[SETTINGS_GHOST_CHARACTER_1];
	next.ghostCharacter[1] = (unsigned char)draft[SETTINGS_GHOST_CHARACTER_2];
	next.laps = (unsigned char)draft[SETTINGS_LAPS];
	next.introCutscene = (unsigned char)draft[SETTINGS_INTRO_CUTSCENE];
	next.ghosts = (unsigned char)draft[SETTINGS_GHOST];
	next.highLod = (unsigned char)draft[SETTINGS_HIGH_LOD];
	next.ctrToken = (unsigned char)draft[SETTINGS_CTR_TOKEN];

	for (int boss = 0; boss < CONFIG_BOSS_COUNT; boss++)
	{
		next.bossItemPreset[boss] = (unsigned char)draft[SETTINGS_BOSS_ITEM_PRESET_FIRST + boss];
	}

	next.features = Settings_PackBits(SETTINGS_FEATURE_FIRST, SETTINGS_FEATURE_END);
	next.modes = Settings_PackBits(SETTINGS_MODE_FIRST, SETTINGS_MODE_END);
	next.bosses = (unsigned char)Settings_PackBits(SETTINGS_BOSS_FIRST, SETTINGS_BOSS_END);

	Config_Set(&next);
}

static int Settings_IsFieldEditable(int field)
{
	unsigned long long editable = Config_Get()->editable;
	unsigned int word = (field < 32) ? (unsigned int)editable : (unsigned int)(editable >> 32);

	return (word & (1u << (field & 31))) != 0;
}

static void Settings_BuildVisibleFields(void)
{
	visibleCount = 0;

	for (int field = 0; field < SETTINGS_FIELD_COUNT; field++)
	{
		if (Settings_IsFieldEditable(field))
		{
			visibleField[visibleCount++] = (unsigned char)field;
		}
	}
}

static void Settings_ScrollToCursor(void)
{
	if (cursor < scroll)
	{
		scroll = cursor;
	}

	if (cursor >= (scroll + VISIBLE_ROWS))
	{
		scroll = cursor - VISIBLE_ROWS + 1;
	}
}

static void Settings_Adjust(int delta)
{
	int field = Settings_GetCursorField();

	switch (Settings_GetKind(field))
	{
	case FIELD_TIME:
		draft[field] = Math_Clamp(draft[field] + delta, TIME_MIN_MS, TIME_MAX_MS);
		return;

	case FIELD_LAPS:
		draft[field] = Math_Clamp(draft[field] + ((delta > 0) ? LAPS_STEP : -LAPS_STEP),
			CONFIG_LAPS_MIN, CONFIG_LAPS_MAX) | 1;
		return;

	case FIELD_ENUM:
		draft[field] = Math_Wrap(draft[field] + ((delta > 0) ? 1 : -1), fieldOptionCount[field]);
		return;

	case FIELD_TOGGLE:
		draft[field] = !draft[field];
		return;
	}
}

static int Settings_IsRepeatedButtonPress(int buttonMask)
{
	if (Input_IsTapped(buttonMask))
	{
		return 1;
	}

	if (!Input_IsHeld(buttonMask))
	{
		return 0;
	}

	return (repeatTimer > REPEAT_DELAY_FRAMES) && ((repeatTimer % REPEAT_RATE_FRAMES) == 0);
}

static void Settings_ApplyStep(int buttonMask, int delta)
{
	int fired = (Settings_GetKind(Settings_GetCursorField()) == FIELD_TIME) ? Settings_IsRepeatedButtonPress(buttonMask) : Input_IsTapped(buttonMask);

	if (!fired)
	{
		return;
	}

	Settings_Adjust(delta);

	if (Input_IsTapped(buttonMask))
	{
		OtherFX_Play(SFX_CONFIRM, 1);
	}
}

static void Settings_HandleInput(void)
{
	if (Input_IsHeld(INPUT_SETTINGS_DEC) || Input_IsHeld(INPUT_SETTINGS_INC) ||
		Input_IsHeld(INPUT_SETTINGS_DEC_FAST) || Input_IsHeld(INPUT_SETTINGS_INC_FAST))
	{
		repeatTimer++;
	}
	else
	{
		repeatTimer = 0;
	}

	if (Input_IsTapped(INPUT_SETTINGS_UP))
	{
		cursor = Math_Wrap(cursor - 1, visibleCount);
		Settings_ScrollToCursor();
		OtherFX_Play(SFX_CURSOR, 1);
	}

	if (Input_IsTapped(INPUT_SETTINGS_DOWN))
	{
		cursor = Math_Wrap(cursor + 1, visibleCount);
		Settings_ScrollToCursor();
		OtherFX_Play(SFX_CURSOR, 1);
	}

	Settings_ApplyStep(INPUT_SETTINGS_DEC, -STEP_FINE_MS);
	Settings_ApplyStep(INPUT_SETTINGS_INC, STEP_FINE_MS);
	Settings_ApplyStep(INPUT_SETTINGS_DEC_FAST, -STEP_COARSE_MS);
	Settings_ApplyStep(INPUT_SETTINGS_INC_FAST, STEP_COARSE_MS);
}

static int Settings_CalculateWidestLabel(void)
{
	int widest = 0;

	for (int i = 0; i < visibleCount; i++)
	{
		int width = DecalFont_GetLineWidth((char*)Settings_GetLabel(visibleField[i]), FONT_SMALL);

		if (width > widest)
		{
			widest = width;
		}
	}

	return widest;
}

static int Settings_CalculateWidestValue(void)
{
	int widest = DecalFont_GetLineWidth(VALUE_SAMPLE, FONT_SMALL);

	for (int i = 0; i < visibleCount; i++)
	{
		int field = visibleField[i];

		if (Settings_GetKind(field) != FIELD_ENUM)
		{
			continue;
		}

		for (int option = 0; option < fieldOptionCount[field]; option++)
		{
			int width = DecalFont_GetLineWidth((char*)Settings_GetOptionName(field, option), FONT_SMALL);

			if (width > widest)
			{
				widest = width;
			}
		}
	}

	return widest;
}

static void Settings_Draw(struct GameTracker* gGT)
{
	u_long* ot = &gGT->pushBuffer_UI.ptrOT[0];

	int rowsShown = (visibleCount < VISIBLE_ROWS) ? visibleCount : VISIBLE_ROWS;

	int labelWidth = columnLabelWidth;
	int valueWidth = columnValueWidth;
	int confirmWidth = DecalFont_GetLineWidth(HINT_CONFIRM, FONT_SMALL);
	int backWidth = DecalFont_GetLineWidth(HINT_BACK, FONT_SMALL);
	int hintWidth = backWidth + COLUMN_GAP + confirmWidth;
	int titleWidth = DecalFont_GetLineWidth("SETTINGS", FONT_BIG);

	int content = labelWidth + COLUMN_GAP + valueWidth;

	if (hintWidth > content)
	{
		content = hintWidth;
	}

	if (titleWidth > content)
	{
		content = titleWidth;
	}

	int panelW = content + 2 * (PANEL_PAD_X + CURSOR_GUTTER);
	int panelH = 2 * PANEL_PAD_Y + TITLE_HEIGHT + rowsShown * ROW_SPACING + HINT_LINES * HINT_HEIGHT;

	panelW = Math_Clamp(panelW, 0, SCREEN_WIDTH);

	int panelX = (SCREEN_WIDTH - panelW) / 2;
	int panelY = (SCREEN_HEIGHT - panelH) / 2;

	int labelX = panelX + PANEL_PAD_X + CURSOR_GUTTER;
	int valueX = panelX + panelW - PANEL_PAD_X - CURSOR_GUTTER;
	int rowY = panelY + PANEL_PAD_Y + TITLE_HEIGHT;

	DecalFont_DrawLineOT("SETTINGS", panelX + panelW / 2, panelY + PANEL_PAD_Y, FONT_BIG, COLOR_TITLE | JUSTIFY_CENTER, ot);

	for (int i = 0; i < rowsShown; i++)
	{
		int slot = scroll + i;
		int field = visibleField[slot];
		int y = rowY + i * ROW_SPACING;
		int color = (slot == cursor) ? COLOR_ACTIVE : COLOR_ROW;

		char value[16];
		Settings_FormatValue(value, field);

		DecalFont_DrawLineOT((char*)Settings_GetLabel(field), labelX, y, FONT_SMALL, color, ot);
		DecalFont_DrawLineOT(value, valueX, y, FONT_SMALL, color | JUSTIFY_RIGHT, ot);
	}

	int hintY = panelY + panelH - PANEL_PAD_Y - HINT_LINES * HINT_HEIGHT + 10;

	DecalFont_DrawLineOT(HINT_BACK, labelX, hintY, FONT_SMALL, COLOR_HINT, ot);
	DecalFont_DrawLineOT(HINT_CONFIRM, valueX, hintY, FONT_SMALL, COLOR_HINT | JUSTIFY_RIGHT, ot);
	Prim_DrawShadowedBox(panelX, panelY, panelW, panelH, ot, &gGT->backBuffer->primMem);
}

void Settings_ApplyCodePatches(void)
{
	const Config* config = Config_Get();

	*INTRO_CAM_AT = config->introCutscene ? INTRO_CAM_STOCK : INTRO_CAM_SKIP;
	*GHOST_THREAD_AT = config->ghosts ? GHOST_STOCK : GHOST_KILL;

	FlushCache();
}

void Settings_PollHost(void)
{
	volatile HostSettings* host = HOST_SETTINGS;

	if (host->magic != HOST_SETTINGS_MAGIC)
	{
		return;
	}

	if (hostSeen && host->sequence == hostSequence)
	{
		return;
	}

	hostSequence = host->sequence;
	hostSeen = 1;

	Config next;
	memcpy(&next, (const void*)&host->config, sizeof(next));

	if (!Config_IsValid(&next))
	{
		return;
	}

	Config_Set(&next);

	if (isOpen)
	{
		Settings_LoadDraft();
	}
}

void Settings_Open(struct RectMenu* mainMenu)
{
	if (isOpen)
	{
		return;
	}

	Settings_BuildVisibleFields();

	if (visibleCount == 0)
	{
		return;
	}

	Settings_LoadDraft();

	columnLabelWidth = Settings_CalculateWidestLabel();
	columnValueWidth = Settings_CalculateWidestValue();

	cursor = 0;
	scroll = 0;
	savedMenu = mainMenu;
	isOpen = 1;
	closing = 0;
	commitOnClose = 0;
	repeatTimer = 0;

	RECTMENU_Hide(mainMenu);
}

int Settings_IsOpen(void)
{
	return isOpen;
}

void Settings_Update(void)
{
	struct GameTracker* gGT = sdata->gGT;

	if (!isOpen || gGT == 0 || gGT->backBuffer == 0)
	{
		return;
	}

	if (gGT->levelID != MAIN_MENU_LEVEL)
	{
		isOpen = 0;
		closing = 0;
		commitOnClose = 0;
		savedMenu = 0;
		return;
	}

	if (closing)
	{
		if (Input_IsHeld(closing))
		{
			Settings_Draw(gGT);
			return;
		}

		if (commitOnClose)
		{
			Settings_Commit();
		}

		if (savedMenu)
		{
			RECTMENU_Show(savedMenu);
			savedMenu = 0;
		}

		isOpen = 0;
		closing = 0;
		commitOnClose = 0;
		return;
	}

	if (Input_IsTapped(INPUT_SETTINGS_CONFIRM))
	{
		OtherFX_Play(SFX_CONFIRM, 1);
		closing = INPUT_SETTINGS_CONFIRM;
		commitOnClose = 1;
		Settings_Draw(gGT);
		return;
	}

	if (Input_IsTapped(INPUT_SETTINGS_BACK))
	{
		OtherFX_Play(SFX_BACK, 1);
		closing = INPUT_SETTINGS_BACK;
		commitOnClose = 0;
		Settings_Draw(gGT);
		return;
	}

	Settings_HandleInput();
	Settings_Draw(gGT);
}

int Settings_GetRelicTime(int tier)
{
	const Config* config = Config_Get();

	if (tier == 1)
	{
		return Settings_CalculateTicksFromMs(config->relicGold);
	}

	if (tier == 2)
	{
		return Settings_CalculateTicksFromMs(config->relicPlatinum);
	}

	return Settings_CalculateTicksFromMs(config->relicSapphire);
}

int Settings_GetCrystalTime(void)
{
	return Settings_CalculateTicksFromMs(Config_Get()->crystalTime);
}

int Settings_GetGhostTime(int slot)
{
	return Settings_CalculateTicksFromMs(Config_Get()->ghostTime[slot]);
}

int Settings_IsAvailable(void)
{
	return Config_Get()->editable != 0;
}
