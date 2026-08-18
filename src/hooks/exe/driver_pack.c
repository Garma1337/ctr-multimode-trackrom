#include "rom/drivers.h"

#include <common.h>

void LOAD_DriverMPK(struct BigHeader* bigfile, int levelLOD, void* callback)
{
	Drivers_QueueModePack(bigfile, callback);
}
