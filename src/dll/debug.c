#include "debug.h"
#include "math.h"
#include "prim.h"

#include <common.h>

#define RCNT_HBLANK             0xF2000001
#define RCNT_UNITS_PER_SECOND   15720
#define RCNT_UNITS_PER_VSYNC    262
#define RCNT_UNITS_PER_BUDGET   (RCNT_UNITS_PER_VSYNC * 2)

#define DEBUG_PEAK_WINDOW       60
#define DEBUG_REFRESH_FRAMES    15
#define DEBUG_OT_WALK_LIMIT     16384

#define DEBUG_NO_BAR            (-1)
#define DEBUG_OT_DEPTH          0x3FF
#define DEBUG_GRAPH_MAX_PCT     255

#define HUD_MARGIN_X            10
#define HUD_MARGIN_Y            8
#define HUD_ROW_HEIGHT          10
#define HUD_COLUMN_GAP          8
#define HUD_BAR_WIDTH           40
#define HUD_BAR_HEIGHT          6
#define HUD_BAR_NUDGE_Y         1
#define HUD_VALUE_CHARS         9
#define HUD_TOP_Y               (HUD_MARGIN_Y + 18 + 4)
#define HUD_BOTTOM_LEFT_ROWS    3
#define HUD_BOTTOM_RIGHT_ROWS   2
#define HUD_BOTTOM_Y(rows)      (SCREEN_HEIGHT - HUD_MARGIN_Y - HUD_ROW_HEIGHT * (rows))

#define GRAPH_BAR_WIDTH         2
#define GRAPH_HEIGHT            26
#define GRAPH_CEILING           200
#define GRAPH_WIDTH             (DEBUG_GRAPH_SAMPLES * GRAPH_BAR_WIDTH)
#define GRAPH_X                 HUD_MARGIN_X
#define GRAPH_Y                 ((SCREEN_HEIGHT - GRAPH_HEIGHT) / 2)

#define COLOR_FPS_GOOD          TINY_GREEN
#define COLOR_FPS_BAD           RED
#define COLOR_LABEL             WHITE
#define COLOR_VALUE             TROPY_LIGHT_BLUE
#define GRAPH_BUDGET_COLOR      MakeColor(0x80, 0x80, 0x80)
#define FPS_BAD_PERCENT         115 // a locked 30fps frame sits at exactly 100%, so the cutoff needs slack
#define BAR_WARN_PERCENT        75
#define BAR_BG                  MakeColor(0x18, 0x18, 0x18)
#define BAR_GOOD                MakeColor(0x40, 0xD0, 0x50)
#define BAR_WARN                MakeColor(0xE0, 0xC0, 0x30)
#define BAR_BAD                 MakeColor(0xE0, 0x40, 0x30)
#define BAR_NEUTRAL             MakeColor(0x50, 0x90, 0xE0)
#define BAR_CPU                 MakeColor(0x30, 0xB0, 0xC0)
#define BAR_GPU                 MakeColor(0xC0, 0x70, 0xD0)

#define MIPS_J(dest)            ((((unsigned long)(dest) & 0x3FFFFFF) >> 2) | 0x08000000)
#define MIPS_NOP                0

static const char* const rowLabels[DEBUG_ROW_COUNT] = {
	"FRAME", "LOGIC", "DRAW", "GPU", "QUADS", "LEAFS", "PRIM", "TRANS", "TEX", "WORST", "PEAK", "TRACK", "VERTS", "MEM"
};

static const DebugRow blockTopLeft[] = { DEBUG_ROW_FRAME, DEBUG_ROW_LOGIC, DEBUG_ROW_DRAW, DEBUG_ROW_GPU };
static const DebugRow blockTopRight[] = { DEBUG_ROW_QUADS, DEBUG_ROW_LEAFS, DEBUG_ROW_PRIM, DEBUG_ROW_TRANS, DEBUG_ROW_TEX };
static const DebugRow blockBottomLeft[] = { DEBUG_ROW_TRACK, DEBUG_ROW_VERTS, DEBUG_ROW_MEM };
static const DebugRow blockBottomRight[] = { DEBUG_ROW_WORST, DEBUG_ROW_PEAK };

static DebugStats stats;
static int active = 0;
static int hookInstalled = 0;

static void Debug_InstallDrawSyncHook(void)
{
	if (hookInstalled)
	{
		return;
	}

	unsigned long* target = (unsigned long*)MainDrawCb_DrawSync;
	target[0] = MIPS_J(Debug_RecordDrawSync);
	target[1] = MIPS_NOP;

	FlushCache();
	hookInstalled = 1;
}

static int Debug_GetPreciseTime(void)
{
	int total;
	int rcnt;

	do
	{
		total = sdata->rcntTotalUnits;
		rcnt = GetRCnt(RCNT_HBLANK);
	} while (total != sdata->rcntTotalUnits);

	return total + rcnt;
}

static int Debug_UnitsToTenthMs(int units)
{
	return (units * 10000) / RCNT_UNITS_PER_SECOND;
}

static int Debug_UnitsToTenthFps(int units)
{
	if (units <= 0)
	{
		return 0;
	}

	return (RCNT_UNITS_PER_SECOND * 10) / units;
}

static int Debug_CountSetBits(const int* words, int count)
{
	int total = 0;

	for (int i = 0; i < count; i++)
	{
		unsigned int bits = (unsigned int)words[i];

		while (bits)
		{
			bits &= bits - 1;
			total++;
		}
	}

	return total;
}

static u_long* Debug_GetOT(struct GameTracker* gGT)
{
	return &gGT->pushBuffer_UI.ptrOT[0];
}

static void Debug_SampleLevel(struct GameTracker* gGT)
{
	stats.quadsVisible = 0;
	stats.quadsTotal = 0;
	stats.bspNodes = 0;
	stats.instances = 0;
	stats.vertices = 0;

	stats.leafsDrawn = gGT->bspLeafsDrawn;

	if (gGT->level1 == 0 || gGT->level1->ptr_mesh_info == 0)
	{
		return;
	}

	const struct mesh_info* mesh = gGT->level1->ptr_mesh_info;

	stats.quadsTotal = mesh->numQuadBlock;
	stats.bspNodes = mesh->numBspNodes;
	stats.vertices = mesh->numVertex;
	stats.instances = (int)gGT->level1->numInstances;

	if (gGT->visMem1 == 0 || gGT->visMem1->visFaceList[0] == 0)
	{
		return;
	}

	stats.quadsVisible = Debug_CountSetBits(gGT->visMem1->visFaceList[0], (stats.quadsTotal + 31) / 32);
}

static void Debug_WalkOrderTable(struct GameTracker* gGT)
{
	stats.primCount = 0;
	stats.primTrans = 0;
	stats.primTex = 0;

	int players = gGT->numPlyrCurrGame;

	if (players <= 0 || gGT->pushBuffer[0].ptrOT == 0 || gGT->pushBuffer[players - 1].ptrOT == 0)
	{
		return;
	}

	u_int* header = &((u_int*)gGT->pushBuffer[0].ptrOT)[DEBUG_OT_DEPTH];
	u_int endOT = (u_int)gGT->pushBuffer[players - 1].ptrOT - 4;

	for (int guard = 0; guard < DEBUG_OT_WALK_LIMIT; guard++)
	{
		header = (u_int*)((*header & 0xFFFFFF) | 0x80000000);

		if ((u_int)header == endOT)
		{
			return;
		}

		if ((*header & 0xFF000000) == 0)
		{
			continue;
		}

		PrimCode code;
		code.code = ((P_TAG*)header)->code;

		stats.primCount++;

		if (code.poly.semiTransparency)
		{
			stats.primTrans++;
		}

		if (code.poly.textured)
		{
			stats.primTex++;
		}
	}
}

static int Debug_GetLapPercent(struct GameTracker* gGT)
{
	if (gGT == 0 || gGT->drivers[0] == 0 || gGT->level1 == 0 || gGT->level1->ptr_restart_points == 0)
	{
		return 0;
	}

	int maxDist = ((int)gGT->level1->ptr_restart_points->distToFinish) << 3;

	if (maxDist <= 0)
	{
		return 0;
	}

	return Math_Percent(maxDist - (int)gGT->drivers[0]->distanceToFinish_curr, maxDist);
}

static Color Debug_BarColor(int percent)
{
	if (percent >= 100)
	{
		return BAR_BAD;
	}

	if (percent >= BAR_WARN_PERCENT)
	{
		return BAR_WARN;
	}

	return BAR_GOOD;
}

static void Debug_MeasureColumns(void)
{
	int widest = 0;

	for (int i = 0; i < DEBUG_ROW_COUNT; i++)
	{
		int width = DecalFont_GetLineWidth((char*)rowLabels[i], FONT_SMALL);

		if (width > widest)
		{
			widest = width;
		}
	}

	char sample[HUD_VALUE_CHARS + 1];

	for (int i = 0; i < HUD_VALUE_CHARS; i++)
	{
		sample[i] = '0';
	}

	sample[HUD_VALUE_CHARS] = '\0';

	stats.labelWidth = widest;
	stats.valueWidth = DecalFont_GetLineWidth(sample, FONT_SMALL);
}

static int Debug_BlockWidth(void)
{
	return stats.labelWidth + HUD_COLUMN_GAP + HUD_BAR_WIDTH + HUD_COLUMN_GAP + stats.valueWidth;
}

static void Debug_DrawRow(int originX, int originY, int slot, DebugRow row, int percent, Color barColor, u_long* ot, struct PrimMem* primMem)
{
	short rowY = (short)(originY + HUD_ROW_HEIGHT * slot);
	short barX = (short)(originX + stats.labelWidth + HUD_COLUMN_GAP);
	short valueX = (percent == DEBUG_NO_BAR) ? barX : (short)(barX + HUD_BAR_WIDTH + HUD_COLUMN_GAP);

	DecalFont_DrawLineOT((char*)rowLabels[row], originX, rowY, FONT_SMALL, COLOR_LABEL, ot);
	DecalFont_DrawLineOT(stats.lines[row], valueX, rowY, FONT_SMALL, COLOR_VALUE, ot);

	if (percent == DEBUG_NO_BAR)
	{
		return;
	}

	short barY = rowY + HUD_BAR_NUDGE_Y;

	RECT box = {
		.x = barX,
		.y = barY,
		.w = HUD_BAR_WIDTH,
		.h = HUD_BAR_HEIGHT
	};

	int boxParams[2] = { 0, 0 };
	CTR_Box_DrawWireBox(&box, boxParams, ot, primMem);

	short fillWidth = (short)Math_Scale(percent, 100, HUD_BAR_WIDTH);

	Prim_DrawRect(barX, barY, fillWidth, HUD_BAR_HEIGHT, barColor, ot);
	Prim_DrawRect(barX, barY, HUD_BAR_WIDTH, HUD_BAR_HEIGHT, BAR_BG, ot);
}

static int Debug_RowPercent(DebugRow row)
{
	switch (row)
	{
	case DEBUG_ROW_FRAME: return Math_Percent(stats.frameUnits, RCNT_UNITS_PER_BUDGET);
	case DEBUG_ROW_LOGIC: return Math_Percent(stats.logicUnits, RCNT_UNITS_PER_BUDGET);
	case DEBUG_ROW_DRAW:  return Math_Percent(stats.drawUnits, RCNT_UNITS_PER_BUDGET);
	case DEBUG_ROW_GPU:   return Math_Percent(stats.gpuUnits, RCNT_UNITS_PER_BUDGET);
	case DEBUG_ROW_WORST: return Math_Percent(stats.peakUnits, RCNT_UNITS_PER_BUDGET);
	case DEBUG_ROW_QUADS: return Math_Percent(stats.quadsVisible, stats.quadsTotal);
	case DEBUG_ROW_LEAFS: return Math_Percent(stats.leafsDrawn, stats.bspNodes);
	case DEBUG_ROW_PRIM:  return Math_Percent(stats.primUsed, stats.primTotal);
	case DEBUG_ROW_TRANS: return Math_Percent(stats.primTrans, stats.primCount);
	case DEBUG_ROW_TEX:   return Math_Percent(stats.primTex, stats.primCount);
	default:              return DEBUG_NO_BAR;
	}
}

static Color Debug_RowColor(DebugRow row, int percent)
{
	switch (row)
	{
	case DEBUG_ROW_LOGIC:
	case DEBUG_ROW_DRAW:
		return BAR_CPU;
	case DEBUG_ROW_GPU:
	case DEBUG_ROW_TRANS:
		return BAR_GPU;
	case DEBUG_ROW_QUADS:
	case DEBUG_ROW_LEAFS:
	case DEBUG_ROW_TEX:
		return BAR_NEUTRAL;
	default:
		return Debug_BarColor(percent);
	}
}

static void Debug_DrawBlock(const DebugRow* rows, int count, int originX, int originY, u_long* ot, struct PrimMem* primMem)
{
	for (int i = 0; i < count; i++)
	{
		int percent = Debug_RowPercent(rows[i]);
		Debug_DrawRow(originX, originY, i, rows[i], percent, Debug_RowColor(rows[i], percent), ot, primMem);
	}
}

static void Debug_DrawGraph(u_long* ot)
{
	short budgetY = (short)(GRAPH_Y + GRAPH_HEIGHT - ((100 * GRAPH_HEIGHT) / GRAPH_CEILING));

	Prim_DrawRect(GRAPH_X, budgetY, GRAPH_WIDTH, 1, GRAPH_BUDGET_COLOR, ot);

	for (int i = 0; i < DEBUG_GRAPH_SAMPLES; i++)
	{
		int percent = stats.graph[Math_Wrap(stats.graphHead + i, DEBUG_GRAPH_SAMPLES)];

		if (percent <= 0)
		{
			continue;
		}

		// a non-zero sample must still show, so the floor is one pixel
		int height = Math_Scale(percent, GRAPH_CEILING, GRAPH_HEIGHT);

		if (height < 1)
		{
			height = 1;
		}

		short barX = (short)(GRAPH_X + i * GRAPH_BAR_WIDTH);
		short barY = (short)(GRAPH_Y + GRAPH_HEIGHT - height);

		Prim_DrawRect(barX, barY, GRAPH_BAR_WIDTH, (short)height, Debug_BarColor(percent), ot);
	}

	Prim_DrawRect(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT, BAR_BG, ot);
}

static void Debug_RefreshLines(void)
{
	Debug_MeasureColumns();

	int fpsTenths = Debug_UnitsToTenthFps(stats.frameUnits);
	int frameTenthMs = Debug_UnitsToTenthMs(stats.frameUnits);
	int logicTenthMs = Debug_UnitsToTenthMs(stats.logicUnits);
	int drawTenthMs = Debug_UnitsToTenthMs(stats.drawUnits);
	int gpuTenthMs = Debug_UnitsToTenthMs(stats.gpuUnits);
	int peakTenthMs = Debug_UnitsToTenthMs(stats.peakUnits);
	int peakVsyncs = (stats.peakUnits + RCNT_UNITS_PER_VSYNC / 2) / RCNT_UNITS_PER_VSYNC;

	stats.fpsBad = Math_Percent(stats.frameUnits, RCNT_UNITS_PER_BUDGET) >= FPS_BAD_PERCENT;

	sprintf(stats.fpsLine, "%d.%d FPS", fpsTenths / 10, fpsTenths % 10);

	sprintf(stats.lines[DEBUG_ROW_FRAME], "%d.%dMS", frameTenthMs / 10, frameTenthMs % 10);
	sprintf(stats.lines[DEBUG_ROW_LOGIC], "%dMS %d%%", logicTenthMs / 10, Math_Percent(stats.logicUnits, stats.frameUnits));
	sprintf(stats.lines[DEBUG_ROW_DRAW], "%dMS %d%%", drawTenthMs / 10, Math_Percent(stats.drawUnits, stats.frameUnits));
	sprintf(stats.lines[DEBUG_ROW_GPU], "%dMS %d%%", gpuTenthMs / 10, Math_Percent(stats.gpuUnits, stats.frameUnits));
	sprintf(stats.lines[DEBUG_ROW_QUADS], "%d/%d", stats.quadsVisible, stats.quadsTotal);
	sprintf(stats.lines[DEBUG_ROW_LEAFS], "%d/%d", stats.leafsDrawn, stats.bspNodes);
	sprintf(stats.lines[DEBUG_ROW_PRIM], "%d%% %dP", Math_Percent(stats.primUsed, stats.primTotal), stats.primCount);
	sprintf(stats.lines[DEBUG_ROW_TRANS], "%d%% %d", Math_Percent(stats.primTrans, stats.primCount), stats.primTrans);
	sprintf(stats.lines[DEBUG_ROW_TEX], "%d%% %d", Math_Percent(stats.primTex, stats.primCount), stats.primTex);
	sprintf(stats.lines[DEBUG_ROW_WORST], "%d.%dMS %dV", peakTenthMs / 10, peakTenthMs % 10, peakVsyncs);
	sprintf(stats.lines[DEBUG_ROW_PEAK], "%d%% %dQ", stats.peakLap, stats.peakQuads);
	sprintf(stats.lines[DEBUG_ROW_TRACK], "%dN %dI", stats.bspNodes, stats.instances);
	sprintf(stats.lines[DEBUG_ROW_VERTS], "%d", stats.vertices);
	sprintf(stats.lines[DEBUG_ROW_MEM], "%dKB", stats.freeBytes >> 10);
}

void Debug_Reset(void)
{
	Debug_InstallDrawSyncHook();

	stats.haveSample = 0;
	stats.lastSample = 0;
	stats.frameUnits = 0;
	stats.logicUnits = 0;
	stats.drawUnits = 0;
	stats.gpuUnits = 0;
	stats.drawSyncTime = 0;
	stats.primUsed = 0;
	stats.primTotal = 0;
	stats.primCount = 0;
	stats.primTrans = 0;
	stats.primTex = 0;
	stats.quadsVisible = 0;
	stats.quadsTotal = 0;
	stats.bspNodes = 0;
	stats.leafsDrawn = 0;
	stats.instances = 0;
	stats.vertices = 0;
	stats.freeBytes = 0;
	stats.peakUnits = 0;
	stats.peakQuads = 0;
	stats.peakLap = 0;
	stats.windowPeak = 0;
	stats.windowPeakQuads = 0;
	stats.windowPeakLap = 0;
	stats.windowFrames = 0;
	stats.fpsBad = 0;
	stats.refreshCountdown = 0;
	stats.labelWidth = 0;
	stats.valueWidth = 0;
	stats.graphHead = 0;

	for (int i = 0; i < DEBUG_GRAPH_SAMPLES; i++)
	{
		stats.graph[i] = 0;
	}
}

void Debug_Toggle(void)
{
	active = !active;
}

int Debug_IsActive(void)
{
	return active;
}

void Debug_Sample(void)
{
	struct GameTracker* gGT = sdata->gGT;
	int now = Debug_GetPreciseTime();

	if (gGT && active)
	{
		stats.freeBytes = (int)MEMPACK_GetFreeBytes();
	}

	if (stats.haveSample)
	{
		int delta = now - stats.lastSample;

		if (delta > 0 && delta <= RCNT_UNITS_PER_SECOND)
		{
			stats.frameUnits = delta;

			int cpu = now - stats.drawSyncTime;

			if (cpu < 0 || cpu > delta)
			{
				cpu = delta;
			}

			int logic = stats.logicUnits > cpu ? cpu : stats.logicUnits;

			stats.logicUnits = logic;
			stats.drawUnits = cpu - logic;
			stats.gpuUnits = delta - cpu;

			int graphPercent = Math_Percent(delta, RCNT_UNITS_PER_BUDGET);
			stats.graph[stats.graphHead] = (unsigned char)(graphPercent > DEBUG_GRAPH_MAX_PCT ? DEBUG_GRAPH_MAX_PCT : graphPercent);
			stats.graphHead = Math_Wrap(stats.graphHead + 1, DEBUG_GRAPH_SAMPLES);

			if (delta > stats.windowPeak)
			{
				stats.windowPeak = delta;
				stats.windowPeakQuads = stats.quadsVisible;
				stats.windowPeakLap = Debug_GetLapPercent(gGT);
			}
		}
	}

	stats.lastSample = now;
	stats.haveSample = 1;
	stats.windowFrames++;

	if (stats.windowFrames >= DEBUG_PEAK_WINDOW)
	{
		stats.peakUnits = stats.windowPeak;
		stats.peakQuads = stats.windowPeakQuads;
		stats.peakLap = stats.windowPeakLap;
		stats.windowPeak = 0;
		stats.windowPeakQuads = 0;
		stats.windowPeakLap = 0;
		stats.windowFrames = 0;
	}
}

void Debug_RecordDrawSync(void)
{
	struct GameTracker* gGT = sdata->gGT;

	if (gGT == 0 || gGT->bool_DrawOTag_InProgress != 1)
	{
		return;
	}

	gGT->bool_DrawOTag_InProgress = 0;

	if (active)
	{
		stats.drawSyncTime = Debug_GetPreciseTime();
	}
}

void Debug_RunGameLogic(struct GameTracker* gGT, struct GamepadSystem* gGamepads)
{
	if (!active)
	{
		MainFrame_GameLogic(gGT, gGamepads);
		return;
	}

	int start = Debug_GetPreciseTime();
	MainFrame_GameLogic(gGT, gGamepads);
	stats.logicUnits = Debug_GetPreciseTime() - start;
}

void Debug_RunRenderFrame(struct GameTracker* gGT, struct GamepadSystem* gGamepads)
{
	MainFrame_RenderFrame(gGT, gGamepads);

	if (gGT == 0 || !active || (gGT->gameMode1 & (MAIN_MENU | LOADING)) != 0)
	{
		return;
	}

	if (gGT->backBuffer)
	{
		stats.primUsed = (int)((char*)gGT->backBuffer->primMem.curr - (char*)gGT->backBuffer->primMem.start);
		stats.primTotal = (int)((char*)gGT->backBuffer->primMem.end - (char*)gGT->backBuffer->primMem.start);
	}

	Debug_SampleLevel(gGT);

	if ((stats.windowFrames % DEBUG_REFRESH_FRAMES) == 0)
	{
		Debug_WalkOrderTable(gGT);
	}
}

void Debug_Draw(struct GameTracker* gGT)
{
	if (gGT == 0 || gGT->backBuffer == 0)
	{
		return;
	}

	if (stats.refreshCountdown <= 0)
	{
		Debug_RefreshLines();
		stats.refreshCountdown = DEBUG_REFRESH_FRAMES;
	}

	stats.refreshCountdown--;

	u_long* ot = Debug_GetOT(gGT);
	struct PrimMem* primMem = &gGT->backBuffer->primMem;

	int rightX = SCREEN_WIDTH - HUD_MARGIN_X - Debug_BlockWidth();

	DecalFont_DrawLineOT(stats.fpsLine, HUD_MARGIN_X, HUD_MARGIN_Y, FONT_BIG, (stats.fpsBad ? COLOR_FPS_BAD : COLOR_FPS_GOOD), ot);

	Debug_DrawBlock(blockTopLeft, 4, HUD_MARGIN_X, HUD_TOP_Y, ot, primMem);
	Debug_DrawBlock(blockTopRight, 5, rightX, HUD_TOP_Y, ot, primMem);
	Debug_DrawBlock(blockBottomLeft, HUD_BOTTOM_LEFT_ROWS, HUD_MARGIN_X, HUD_BOTTOM_Y(HUD_BOTTOM_LEFT_ROWS), ot, primMem);
	Debug_DrawBlock(blockBottomRight, HUD_BOTTOM_RIGHT_ROWS, rightX, HUD_BOTTOM_Y(HUD_BOTTOM_RIGHT_ROWS), ot, primMem);
	Debug_DrawGraph(ot);
}
