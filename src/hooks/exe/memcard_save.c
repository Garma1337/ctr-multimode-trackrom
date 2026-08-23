#include "../../rom.h"

#include <common.h>

uint8_t MEMCARD_Save(int slotIdx, char* name, char* unused,
	char* data, int fileSize, unsigned int flags)
{
	volatile int* ghostReady = GHOST_READY;

	*GHOST_SIZE_ADDR = fileSize;
	memcpy(GHOST_ADDR, data, fileSize);
	*ghostReady = 1;

	return 0;
}
