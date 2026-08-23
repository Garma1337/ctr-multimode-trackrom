#include "config_default.h"

const struct
{
	Config config;
	unsigned char padding[CONFIG_FILE_SIZE - sizeof(Config)];
} configFile = { .config = CONFIG_DEFAULTS };
