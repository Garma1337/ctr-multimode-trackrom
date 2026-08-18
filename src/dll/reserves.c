#include "math.h"
#include "prim.h"
#include "reserves.h"

#include <common.h>
#include <limits.h>

#define RESERVE_LEVEL_SIZE   SECONDS(5)
#define RESERVE_LEVEL_EMPTY  (-2)
#define RESERVE_COLOR_COUNT  9

#define HUD_FLAG_SPEEDOMETER 8

#define HUD_TABLE_COUNT           4
#define SLIDE_METER_ELEMENT       8
#define SLIDE_METER_WIDTH         49
#define SLIDE_METER_HEIGHT        7
#define SLIDE_METER_HEIGHT_SPLIT  3
#define SLIDE_METER_SPLIT_PLAYERS 2

#define BAR_GAP_Y            0
#define LEVEL_TEXT_GAP_X     18

static const unsigned char barColors[RESERVE_COLOR_COUNT][3] = {
	{ 0x40, 0x40, 0x40 }, // -2, no reserves
	{ 0xFF, 0x00, 0xFF }, // -1, negative
	{ 0xFF, 0x00, 0x00 },
	{ 0xFF, 0xA5, 0x00 },
	{ 0xFF, 0xFF, 0x00 },
	{ 0x00, 0xFF, 0x00 },
	{ 0x00, 0xFF, 0xFF },
	{ 0x00, 0x00, 0xFF },
	{ 0x80, 0x00, 0x80 },
};

static int Reserves_IsSpeedometerVisible(void)
{
	return (sdata->HudAndDebugFlags & HUD_FLAG_SPEEDOMETER) != 0;
}

static Color Reserves_GetColor(int level)
{
	int index = level + 2;

	if (index < 0 || index >= RESERVE_COLOR_COUNT)
	{
		index = 0;
	}

	return MakeColor(barColors[index][0], barColors[index][1], barColors[index][2]);
}

static int Reserves_CalculateLevel(int reserves, int* outProgress)
{
	if (reserves == 0)
	{
		*outProgress = 0;
		return RESERVE_LEVEL_EMPTY;
	}

	if (reserves < 0)
	{
		*outProgress = (RESERVE_LEVEL_SIZE * reserves) / SHRT_MIN;
		return -1;
	}

	*outProgress = ((reserves - 1) % RESERVE_LEVEL_SIZE) + 1;
	return (reserves - 1) / RESERVE_LEVEL_SIZE;
}

void Reserves_Draw(struct GameTracker* gGT)
{
	if (Reserves_IsSpeedometerVisible() || gGT->drivers[0] == 0 || gGT->backBuffer == 0)
	{
		return;
	}

	int players = gGT->numPlyrCurrGame;

	if (players < 1)
	{
		players = 1;
	}

	if (players > HUD_TABLE_COUNT)
	{
		players = HUD_TABLE_COUNT;
	}

	const struct UiElement2D* hud = data.hudStructPtr[players - 1];

	short barWidth = SLIDE_METER_WIDTH;
	short barHeight = (players > SLIDE_METER_SPLIT_PLAYERS) ? SLIDE_METER_HEIGHT_SPLIT : SLIDE_METER_HEIGHT;

	short barX = hud[SLIDE_METER_ELEMENT].x - barWidth;
	short barY = hud[SLIDE_METER_ELEMENT].y + BAR_GAP_Y;

	int levelProgress;
	int level = Reserves_CalculateLevel(gGT->drivers[0]->reserves, &levelProgress);

	u_long* ot = &gGT->pushBuffer_UI.ptrOT[0];

	RECT box = {
		.x = barX,
		.y = barY,
		.w = barWidth,
		.h = barHeight
	};

	int boxParams[2] = { 0, 0 };
	CTR_Box_DrawWireBox(&box, boxParams, ot, &gGT->backBuffer->primMem);

	int fillWidth = Math_Scale(levelProgress, RESERVE_LEVEL_SIZE, barWidth);

	Prim_DrawRect(barX, barY, fillWidth, barHeight, Reserves_GetColor(level), ot);
	Prim_DrawRect(barX, barY, barWidth, barHeight, Reserves_GetColor(RESERVE_LEVEL_EMPTY), ot);

	char levelStr[8];
	sprintf(levelStr, "%d", level >= 0 ? level : 0);
	DecalFont_DrawLineOT(levelStr, barX - LEVEL_TEXT_GAP_X, barY, FONT_SMALL, WHITE, ot);
}
