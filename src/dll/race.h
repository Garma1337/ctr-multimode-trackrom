#ifndef RACE_H_MODEROM
#define RACE_H_MODEROM

#include <common.h>

void Race_ApplyStartingGrid(void);
void Race_Reset(void);
void Race_CaptureInput(void);
void Race_RunGameLogic(struct GameTracker* gGT, struct GamepadSystem* gGS);
void Race_Update(struct GameTracker* gGT);
void Race_RunRenderFrame(struct GameTracker* gGT, struct GamepadSystem* gGS);

#endif
