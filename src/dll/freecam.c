#include "freecam.h"
#include "input.h"

#include <common.h>

#define FREECAM_SPEED_MIN     1
#define FREECAM_SPEED_MAX     16
#define FREECAM_SPEED_DEFAULT 6

#define FREECAM_CAMERA_FLAG   0x8000
#define QUAD_FLAG_INVISIBLE   0x8000

#define HELP_COLOR            GRAY
#define HELP_LINE_3_Y         (SCREEN_HEIGHT - 35)
#define HELP_LINE_2_Y         (SCREEN_HEIGHT - 25)
#define HELP_LINE_1_Y         (SCREEN_HEIGHT - 15)
#define HELP_MARGIN_X         8

static Freecam freecam = { 0 };

static int visDummy;

static void Freecam_ForceAllVisible(void)
{
	struct GameTracker* gGT = sdata->gGT;
	struct Level* lev = gGT->level1;

	if (!lev)
	{
		return;
	}

	struct mesh_info* mi = lev->ptr_mesh_info;
	struct VisMem* vm = gGT->visMem1;

	if (!mi || !vm)
	{
		return;
	}

	int leafBytes = ((mi->numBspNodes + 31) >> 5) << 2;
	int faceBytes = ((mi->numQuadBlock + 31) >> 5) << 2;

	memset(vm->visLeafList[0], 0xFF, leafBytes);
	memset(vm->visFaceList[0], 0xFF, faceBytes);

	struct QuadBlock* qb = mi->ptrQuadBlockArray;
	for (int i = 0; i < mi->numQuadBlock; i++)
	{
		if (qb[i].quadFlags & QUAD_FLAG_INVISIBLE)
		{
			((int*)vm->visFaceList[0])[i >> 5] &= ~(1 << (i & 0x1F));
		}
	}

	gGT->cameraDC[0].visLeafSrc = &visDummy;
	gGT->cameraDC[0].visFaceSrc = &visDummy;
	vm->visLeafSrc[0] = &visDummy;
	vm->visFaceSrc[0] = &visDummy;
}

static void Freecam_Activate(void)
{
	struct GameTracker* gGT = sdata->gGT;

	freecam.pos[0] = gGT->pushBuffer[0].pos[0];
	freecam.pos[1] = gGT->pushBuffer[0].pos[1];
	freecam.pos[2] = gGT->pushBuffer[0].pos[2];
	freecam.rot[0] = gGT->pushBuffer[0].rot[0];
	freecam.rot[1] = gGT->pushBuffer[0].rot[1];
	freecam.rot[2] = 0;

	gGT->cameraDC[0].flags |= FREECAM_CAMERA_FLAG;

	for (int d = 0; d < FREECAM_MAX_DRIVERS; d++)
	{
		struct Driver* driver = gGT->drivers[d];

		freecam.frozenDrivers[d] = driver;

		if (driver == 0)
		{
			continue;
		}

		for (int i = 0; i < DRIVER_FUNC_PTR_COUNT; i++)
		{
			freecam.savedFuncPtrs[d][i] = driver->funcPtrs[i];
			driver->funcPtrs[i] = 0;
		}
	}

	Freecam_ForceAllVisible();
	freecam.isActive = 1;
}

static void Freecam_MoveAlong(int dirX, int dirZ, int speed, int sign)
{
	int len = MATH_FastSqrt(dirX * dirX + dirZ * dirZ, 0x18) >> FRACTIONAL_BITS;

	if (len < 1)
	{
		return;
	}

	int unitX = (dirX << FRACTIONAL_BITS) / len;
	int unitZ = (dirZ << FRACTIONAL_BITS) / len;

	freecam.pos[0] += sign * (speed * unitX >> 11);
	freecam.pos[2] += sign * (speed * unitZ >> 11);
}

void Freecam_Reset(void)
{
	struct GameTracker* gGT = sdata->gGT;

	gGT->cameraDC[0].visLeafSrc = 0;
	gGT->cameraDC[0].visFaceSrc = 0;

	freecam.isActive = 0;
	freecam.prevToggleState = 0;
	freecam.showHelp = 1;
	freecam.speedQuarters = FREECAM_SPEED_DEFAULT;

	for (int d = 0; d < FREECAM_MAX_DRIVERS; d++)
	{
		freecam.frozenDrivers[d] = 0;
	}
}

int Freecam_IsActive(void)
{
	return freecam.isActive;
}

void Freecam_Deactivate(void)
{
	struct GameTracker* gGT = sdata->gGT;

	gGT->cameraDC[0].flags &= ~FREECAM_CAMERA_FLAG;
	gGT->cameraDC[0].visLeafSrc = 0;
	gGT->cameraDC[0].visFaceSrc = 0;

	for (int d = 0; d < FREECAM_MAX_DRIVERS; d++)
	{
		struct Driver* driver = freecam.frozenDrivers[d];

		if (driver == 0)
		{
			continue;
		}

		for (int i = 0; i < DRIVER_FUNC_PTR_COUNT; i++)
		{
			driver->funcPtrs[i] = freecam.savedFuncPtrs[d][i];
		}

		freecam.frozenDrivers[d] = 0;
	}

	freecam.isActive = 0;
}

void Freecam_CheckToggle(void)
{
	int isPressed = Input_IsHeld(INPUT_FREECAM_TOGGLE);

	if (isPressed && !freecam.prevToggleState)
	{
		if (freecam.isActive)
		{
			Freecam_Deactivate();
		}
		else
		{
			Freecam_Activate();
		}
	}

	freecam.prevToggleState = isPressed;
}

void Freecam_Update(void)
{
	struct GameTracker* gGT = sdata->gGT;
	MATRIX matrix;

	if (Input_IsTapped(INPUT_FREECAM_TOGGLE_HELP))
	{
		freecam.showHelp = !freecam.showHelp;
	}

	if (Input_IsTapped(INPUT_FREECAM_CYCLE_SPEED))
	{
		freecam.speedQuarters++;

		if (freecam.speedQuarters > FREECAM_SPEED_MAX)
		{
			freecam.speedQuarters = FREECAM_SPEED_MIN;
		}
	}

	int speed = gGT->elapsedTimeMS;
	speed = speed * freecam.speedQuarters / 4;

	if (Input_IsHeld(INPUT_FREECAM_PITCH_UP))   freecam.rot[0] += speed;
	if (Input_IsHeld(INPUT_FREECAM_PITCH_DOWN)) freecam.rot[0] -= speed;
	if (Input_IsHeld(INPUT_FREECAM_YAW_LEFT))   freecam.rot[1] += speed;
	if (Input_IsHeld(INPUT_FREECAM_YAW_RIGHT))  freecam.rot[1] -= speed;

	ConvertRotToMatrix(&matrix, freecam.rot);

	if (Input_IsHeld(INPUT_FREECAM_STRAFE_LEFT))       Freecam_MoveAlong(matrix.m[0][0], matrix.m[0][2], speed, -1);
	else if (Input_IsHeld(INPUT_FREECAM_STRAFE_RIGHT)) Freecam_MoveAlong(matrix.m[0][0], matrix.m[0][2], speed, 1);

	if (Input_IsHeld(INPUT_FREECAM_FORWARD))           Freecam_MoveAlong(matrix.m[2][0], matrix.m[2][2], speed, 1);
	else if (Input_IsHeld(INPUT_FREECAM_BACKWARD))     Freecam_MoveAlong(matrix.m[2][0], matrix.m[2][2], speed, -1);

	if (Input_IsHeld(INPUT_FREECAM_DOWN)) freecam.pos[1] -= speed;
	if (Input_IsHeld(INPUT_FREECAM_UP))   freecam.pos[1] += speed;

	gGT->pushBuffer[0].pos[0] = freecam.pos[0];
	gGT->pushBuffer[0].pos[1] = freecam.pos[1];
	gGT->pushBuffer[0].pos[2] = freecam.pos[2];
	gGT->pushBuffer[0].rot[0] = freecam.rot[0];
	gGT->pushBuffer[0].rot[1] = freecam.rot[1];
	gGT->pushBuffer[0].rot[2] = freecam.rot[2];
}

void Freecam_DrawHelp(void)
{
	char speedLine[48];

	if (!freecam.showHelp)
	{
		return;
	}

	int whole = freecam.speedQuarters / 4;
	int frac = (freecam.speedQuarters % 4) * 25;

	if (frac > 0)
	{
		sprintf(speedLine, "R2 Toggle Help    L2 Speed %d.%02d", whole, frac);
	}
	else
	{
		sprintf(speedLine, "R2 Toggle Help    L2 Speed %d", whole);
	}

	DecalFont_DrawLine("^* Pitch          D-pad Move", HELP_MARGIN_X, HELP_LINE_3_Y, FONT_SMALL, HELP_COLOR);
	DecalFont_DrawLine("[@ Yaw            L1/R1 Up/Down", HELP_MARGIN_X, HELP_LINE_2_Y, FONT_SMALL, HELP_COLOR);
	DecalFont_DrawLine(speedLine, HELP_MARGIN_X + 1, HELP_LINE_1_Y, FONT_SMALL, HELP_COLOR);
}
