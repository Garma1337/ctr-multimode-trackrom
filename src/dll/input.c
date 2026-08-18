#include "input.h"

#include <common.h>

static int snapshotHeld = 0;
static int snapshotTapped = 0;

void Input_Capture(void)
{
	struct GamepadBuffer* pad = &sdata->gGamepads->gamepad[0];
	snapshotHeld = pad->buttonsHeldCurrFrame;
	snapshotTapped = pad->buttonsTapped;
}

int Input_IsHeld(int buttonMask)
{
	return (snapshotHeld & buttonMask) == buttonMask;
}

int Input_IsTapped(int buttonMask)
{
	return (snapshotTapped & buttonMask) == buttonMask;
}
