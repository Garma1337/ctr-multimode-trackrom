#ifndef LEVEL_H_MODEROM
#define LEVEL_H_MODEROM

#include <common.h>

#define RACE_LEVEL_ID    CRASH_COVE
#define CRYSTAL_LEVEL_ID SKULL_ROCK

unsigned Level_GetID();
int Level_IsPlayable(unsigned levelID);
void Level_CommitRequest();

#endif
