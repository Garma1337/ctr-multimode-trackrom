#ifndef CONFIG_DEFAULT_H
#define CONFIG_DEFAULT_H

#include "config_schema.h"

#define CONFIG_DEFAULTS \
{ \
	.magic = CONFIG_MAGIC, \
	.version = CONFIG_VERSION, \
	.size = sizeof(Config), \
	.features = FEATURE_FREECAM | FEATURE_DEBUG_HUD | FEATURE_RESERVES | \
	            FEATURE_HOT_RELOAD | FEATURE_HOST_SETTINGS, \
	.modes = CONFIG_MODE_ALL, \
	.editable = BEHAVIOR_ALL, \
	.relicSapphire = 77000, \
	.relicGold = 65000, \
	.relicPlatinum = 52000, \
	.crystalTime = 120000, \
	.laps = 3, \
	.introCutscene = 1, \
	.ghosts = 1, \
	.bosses = CONFIG_BOSS_ALL, \
	.ctrToken = TOKEN_YELLOW, \
}

#endif
