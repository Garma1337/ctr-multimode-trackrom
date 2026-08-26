#ifndef GHOST_H
#define GHOST_H

#include <common.h>

void Ghost_AdoptDrivers(void);
void Ghost_ApplyTargets(void);
void Ghost_Update(struct GameTracker* gGT);
void Ghost_Forget(void);

#endif
