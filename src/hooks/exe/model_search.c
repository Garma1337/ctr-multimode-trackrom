#include "../../rom/drivers.h"

#include <common.h>

struct Model* VehBirth_GetModelByName(char* searchName)
{
	return Drivers_FindModel(searchName);
}
