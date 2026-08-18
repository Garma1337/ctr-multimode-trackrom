#ifndef DRIVERS_H
#define DRIVERS_H

#include <common.h>

#define NUM_PRELOADED_DRIVERS 16

void Drivers_Preload(struct BigHeader* bigfile);
struct Model* Drivers_FindModel(char* name);
void Drivers_QueueModePack(struct BigHeader* bigfile, void* callback);

#endif
