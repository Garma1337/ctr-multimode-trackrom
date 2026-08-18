#ifndef HOT_RELOAD_H
#define HOT_RELOAD_H

#include <common.h>

void HotReload_Poll();
int HotReload_HasStagedTrack();
void HotReload_UploadVram();
void HotReload_ApplyStagedLevel();

#endif
