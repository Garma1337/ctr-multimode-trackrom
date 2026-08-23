#include "dll/main_menu.h"

#include <common.h>

void MM_MenuProc_Main(struct RectMenu* mainMenu)
{
	mainMenu->rows = MainMenu_GetRows();

	DecalFont_DrawLine("Multi-Gamemode Test ROM", 5, 197, FONT_SMALL, BLUE);
	DecalFont_DrawLine(__DATE__, 5, 207, FONT_SMALL, LIME_GREEN);
	DecalFont_DrawLine(__TIME__, 170, 207, FONT_SMALL, LIME_GREEN);

	if (mainMenu->unk1e == 1)
	{
		MM_Title_MenuUpdate();
	}

	MM_Title_Init();

	if (((mainMenu->state & EXECUTE_FUNCPTR) == 0) || (mainMenu->unk1e != 0) || ((mainMenu->rowSelected) < 0))
	{
		return;
	}

	MainMenu_Select(mainMenu);
}
