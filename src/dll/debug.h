#ifndef DEBUG_H_MODEROM
#define DEBUG_H_MODEROM

#include <common.h>

#define DEBUG_GRAPH_SAMPLES 48
#define DEBUG_LINE_LENGTH   24

typedef enum DebugRow
{
	DEBUG_ROW_FRAME = 0,
	DEBUG_ROW_LOGIC,
	DEBUG_ROW_DRAW,
	DEBUG_ROW_GPU,
	DEBUG_ROW_QUADS,
	DEBUG_ROW_LEAFS,
	DEBUG_ROW_PRIM,
	DEBUG_ROW_TRANS,
	DEBUG_ROW_TEX,
	DEBUG_ROW_WORST,
	DEBUG_ROW_PEAK,
	DEBUG_ROW_TRACK,
	DEBUG_ROW_VERTS,
	DEBUG_ROW_MEM,
	DEBUG_ROW_COUNT
} DebugRow;

typedef struct DebugStats
{
	int haveSample;
	int lastSample;
	int frameUnits;
	int logicUnits;
	int drawUnits;
	int gpuUnits;
	int drawSyncTime;
	int primUsed;
	int primTotal;
	int primCount;
	int primTrans;
	int primTex;
	int quadsVisible;
	int quadsTotal;
	int bspNodes;
	int leafsDrawn;
	int instances;
	int vertices;
	int freeBytes;
	int peakUnits;
	int peakQuads;
	int peakLap;
	int windowPeak;
	int windowPeakQuads;
	int windowPeakLap;
	int windowFrames;
	int fpsBad;
	int refreshCountdown;
	int labelWidth;
	int valueWidth;
	int graphHead;
	unsigned char graph[DEBUG_GRAPH_SAMPLES];
	char fpsLine[DEBUG_LINE_LENGTH];
	char lines[DEBUG_ROW_COUNT][DEBUG_LINE_LENGTH];
} DebugStats;

void Debug_Reset(void);
void Debug_Toggle(void);
int  Debug_IsActive(void);
void Debug_Sample(void);
void Debug_RecordDrawSync(void);
void Debug_RunGameLogic(struct GameTracker* gGT, struct GamepadSystem* gGamepads);
void Debug_RunRenderFrame(struct GameTracker* gGT, struct GamepadSystem* gGamepads);
void Debug_Draw(struct GameTracker* gGT);

#endif
