#include "input.h"
#include "math.h"
#include "prim.h"
#include "rom.h"
#include "settings.h"

#include <common.h>

#define STEP_FINE     SECONDS(0.5)
#define STEP_COARSE   SECONDS(5)
#define TIME_MIN      SECONDS(0.5)
#define TIME_MAX      SECONDS(599.5)
#define TIME_MIN_MS   500
#define TIME_MAX_MS   599500

#define DEFAULT_SAPPHIRE  SECONDS(77)
#define DEFAULT_GOLD      SECONDS(65)
#define DEFAULT_PLATINUM  SECONDS(52)
#define DEFAULT_CRYSTAL   SECONDS(120)

#define REPEAT_DELAY_FRAMES  15
#define REPEAT_RATE_FRAMES   3

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
	FIELD_TOGGLE
} FieldKind;

static const char* const fieldLabels[SETTINGS_FIELD_COUNT] = {
	"Relic - Sapphire",
	"Relic - Gold",
	"Relic - Platinum",
	"Crystal Time Limit",
	"Intro Cutscene",
	"Time Trial Ghosts",
};

static const unsigned char fieldKinds[SETTINGS_FIELD_COUNT] = {
	FIELD_TIME,
	FIELD_TIME,
	FIELD_TIME,
	FIELD_TIME,
	FIELD_TOGGLE,
	FIELD_TOGGLE,
};

static int values[SETTINGS_FIELD_COUNT] = {
	DEFAULT_SAPPHIRE,
	DEFAULT_GOLD,
	DEFAULT_PLATINUM,
	DEFAULT_CRYSTAL,
	1,
	1,
};

static int isOpen = 0;
static int cursor = 0;
static int repeatTimer = 0;
static int closing = 0;
static struct RectMenu* savedMenu = 0;
static int draft[SETTINGS_FIELD_COUNT];
static int commitOnClose = 0;
static int hostSequence = 0;
static int hostSeen = 0;

static void Settings_FormatTime(char* out, int ticks)
{
	int tenths = (ticks * 10) / SECOND;

	sprintf(out, "%d:%02d.%d", tenths / 600, (tenths / 10) % 60, tenths % 10);
}

static void Settings_FormatValue(char* out, int field)
{
	if (fieldKinds[field] == FIELD_TOGGLE)
	{
		sprintf(out, "%s", draft[field] ? "On" : "Off");
		return;
	}

	Settings_FormatTime(out, draft[field]);
}

static void Settings_LoadDraft(void)
{
	for (int i = 0; i < SETTINGS_FIELD_COUNT; i++)
	{
		draft[i] = values[i];
	}
}

static void Settings_Commit(void)
{
	for (int i = 0; i < SETTINGS_FIELD_COUNT; i++)
	{
		values[i] = draft[i];
	}

	Settings_ApplyCodePatches();
}

static int Settings_ClampTime(int ticks)
{
	return Math_Clamp(ticks, TIME_MIN, TIME_MAX);
}

static int Settings_TicksFromMs(int ms)
{
	ms = Math_Clamp(ms, TIME_MIN_MS, TIME_MAX_MS);

	return Settings_ClampTime((ms * SECOND) / 1000);
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

	values[SETTINGS_RELIC_SAPPHIRE] = Settings_TicksFromMs(host->relicSapphire);
	values[SETTINGS_RELIC_GOLD] = Settings_TicksFromMs(host->relicGold);
	values[SETTINGS_RELIC_PLATINUM] = Settings_TicksFromMs(host->relicPlatinum);
	values[SETTINGS_CRYSTAL_TIME] = Settings_TicksFromMs(host->crystalTime);
	values[SETTINGS_INTRO_CUTSCENE] = (host->introCutscene != 0);
	values[SETTINGS_GHOST] = (host->ghost != 0);

	Settings_ApplyCodePatches();

	if (isOpen)
	{
		Settings_LoadDraft();
	}
}

void Settings_ApplyCodePatches(void)
{
	*INTRO_CAM_AT = values[SETTINGS_INTRO_CUTSCENE] ? INTRO_CAM_STOCK : INTRO_CAM_SKIP;
	*GHOST_THREAD_AT = values[SETTINGS_GHOST] ? GHOST_STOCK : GHOST_KILL;

	FlushCache();
}

static void Settings_Adjust(int delta)
{
	if (fieldKinds[cursor] == FIELD_TOGGLE)
	{
		draft[cursor] = !draft[cursor];
		return;
	}

	draft[cursor] = Settings_ClampTime(draft[cursor] + delta);
}

static int Settings_Repeated(int buttonMask)
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
	int fired = (fieldKinds[cursor] == FIELD_TOGGLE) ? Input_IsTapped(buttonMask) : Settings_Repeated(buttonMask);

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
		cursor = Math_Wrap(cursor - 1, SETTINGS_FIELD_COUNT);
		OtherFX_Play(SFX_CURSOR, 1);
	}

	if (Input_IsTapped(INPUT_SETTINGS_DOWN))
	{
		cursor = Math_Wrap(cursor + 1, SETTINGS_FIELD_COUNT);
		OtherFX_Play(SFX_CURSOR, 1);
	}

	Settings_ApplyStep(INPUT_SETTINGS_DEC, -STEP_FINE);
	Settings_ApplyStep(INPUT_SETTINGS_INC, STEP_FINE);
	Settings_ApplyStep(INPUT_SETTINGS_DEC_FAST, -STEP_COARSE);
	Settings_ApplyStep(INPUT_SETTINGS_INC_FAST, STEP_COARSE);
}

static int Settings_WidestLabel(void)
{
	int widest = 0;

	for (int i = 0; i < SETTINGS_FIELD_COUNT; i++)
	{
		int width = DecalFont_GetLineWidth((char*)fieldLabels[i], FONT_SMALL);

		if (width > widest)
		{
			widest = width;
		}
	}

	return widest;
}

static void Settings_Draw(struct GameTracker* gGT)
{
	u_long* ot = &gGT->pushBuffer_UI.ptrOT[0];

	int labelWidth = Settings_WidestLabel();
	int valueWidth = DecalFont_GetLineWidth(VALUE_SAMPLE, FONT_SMALL);
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
	int panelH = 2 * PANEL_PAD_Y + TITLE_HEIGHT + SETTINGS_FIELD_COUNT * ROW_SPACING + HINT_LINES * HINT_HEIGHT;

	panelW = Math_Clamp(panelW, 0, SCREEN_WIDTH);

	int panelX = (SCREEN_WIDTH - panelW) / 2;
	int panelY = (SCREEN_HEIGHT - panelH) / 2;

	int labelX = panelX + PANEL_PAD_X + CURSOR_GUTTER;
	int valueX = panelX + panelW - PANEL_PAD_X - CURSOR_GUTTER;
	int rowY = panelY + PANEL_PAD_Y + TITLE_HEIGHT;

	DecalFont_DrawLineOT("SETTINGS", panelX + panelW / 2, panelY + PANEL_PAD_Y, FONT_BIG, COLOR_TITLE | JUSTIFY_CENTER, ot);

	for (int i = 0; i < SETTINGS_FIELD_COUNT; i++)
	{
		int y = rowY + i * ROW_SPACING;
		int active = (i == cursor);
		int color = active ? COLOR_ACTIVE : COLOR_ROW;

		char value[16];
		Settings_FormatValue(value, i);

		DecalFont_DrawLineOT((char*)fieldLabels[i], labelX, y, FONT_SMALL, color, ot);
		DecalFont_DrawLineOT(value, valueX, y, FONT_SMALL, color | JUSTIFY_RIGHT, ot);
	}

	int hintY = panelY + panelH - PANEL_PAD_Y - HINT_LINES * HINT_HEIGHT + 10;

	DecalFont_DrawLineOT(HINT_BACK, labelX, hintY, FONT_SMALL, COLOR_HINT, ot);
	DecalFont_DrawLineOT(HINT_CONFIRM, valueX, hintY, FONT_SMALL, COLOR_HINT | JUSTIFY_RIGHT, ot);
	Prim_DrawShadowedBox(panelX, panelY, panelW, panelH, ot, &gGT->backBuffer->primMem);
}

void Settings_Open(struct RectMenu* mainMenu)
{
	if (isOpen)
	{
		return;
	}

	Settings_LoadDraft();

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
	if (tier < 0 || tier >= SETTINGS_RELIC_TIERS)
	{
		return values[SETTINGS_RELIC_SAPPHIRE];
	}

	return values[SETTINGS_RELIC_SAPPHIRE + tier];
}

int Settings_GetCrystalTime(void)
{
	return values[SETTINGS_CRYSTAL_TIME];
}
