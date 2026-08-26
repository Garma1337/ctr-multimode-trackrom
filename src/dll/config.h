#ifndef CONFIG_RUNTIME_H
#define CONFIG_RUNTIME_H

#include "../config/config_schema.h"

#include <common.h>

void Config_Init();
void Config_Normalize(Config* config);
void Config_Set(const Config* next);
const Config* Config_Get();
int Config_IsValid(const Config* candidate);
int Config_IsFeatureEnabled(int feature);
int Config_IsModeEnabled(int mode);
int Config_IsBossEnabled(int boss);

#endif
