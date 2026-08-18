#ifndef FREECAM_H
#define FREECAM_H

#include <common.h>

#define DRIVER_FUNC_PTR_COUNT 0xD
#define FREECAM_MAX_DRIVERS   8

typedef struct Freecam
{
	int            isActive;
	int            prevToggleState;
	int            showHelp;
	int            speedQuarters;
	short          pos[3];
	short          rot[3];
	struct Driver* frozenDrivers[FREECAM_MAX_DRIVERS];
	void* savedFuncPtrs[FREECAM_MAX_DRIVERS][DRIVER_FUNC_PTR_COUNT];
} Freecam;

void Freecam_Reset(void);
int  Freecam_IsActive(void);
void Freecam_CheckToggle(void);
void Freecam_Update(void);
void Freecam_Deactivate(void);
void Freecam_DrawHelp(void);

#endif
