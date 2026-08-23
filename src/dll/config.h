#ifndef CONFIG_RUNTIME_H
#define CONFIG_RUNTIME_H

#include "config_schema.h"

#include <common.h>

typedef enum ConfigStatus
{
	CONFIG_OK = 0,
	CONFIG_MISSING,
	CONFIG_CORRUPT,
	CONFIG_TOO_NEW,
} ConfigStatus;

void Config_Load();
void Config_Set(const Config* next);
const Config* Config_Get();
int Config_IsFeatureEnabled(int feature);
int Config_IsModeEnabled(int mode);
int Config_IsBossEnabled(int boss);
ConfigStatus Config_GetStatus();
const char* Config_GetStatusText();

#endif
