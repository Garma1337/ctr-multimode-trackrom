#include <common.h>

extern unsigned char oxideModel[];

int* MM_Characters_GetModelByName(int* name)
{
	if (sdata->gGT->level1 == 0)
	{
		return 0;
	}

	int** models = (int**)sdata->gGT->level1->ptrModelsPtrArray;

	for (int* model = *models; model != 0; model = *(++models))
	{
		if (*model == *name)
		{
			return model;
		}
	}

	struct Model* oxideMenu = (struct Model*)&oxideModel[4];

	return (*(int*)oxideMenu == *name) ? (int*)oxideMenu : 0;
}
