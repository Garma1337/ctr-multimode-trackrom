#include "prim.h"

#include <common.h>

#define BOX_FILL_PADDING_X 3
#define BOX_FILL_PADDING_Y 2
#define BOX_BORDER_TYPE    8

void GetPrimitiveMem(void** ppPrim, size_t primSize)
{
	struct DB* backBuffer = sdata->gGT->backBuffer;

	if (backBuffer->primMem.curr > backBuffer->primMem.endMin100)
	{
		*ppPrim = nullptr;
		return;
	}

	*ppPrim = backBuffer->primMem.curr;
	backBuffer->primMem.curr = (void*)((size_t)backBuffer->primMem.curr + primSize);
	((Tag*)*ppPrim)->size = (primSize - sizeof(Tag)) / sizeof(u32);
}

void Prim_Add(void* prim, void* ot)
{
	((Tag*)prim)->addr = ((Tag*)ot)->addr;
	((Tag*)ot)->addr = (u32)prim;
}

void Prim_DrawRect(short x, short y, short w, short h, Color color, u_long* ot)
{
	PolyF4* p;
	GetPrimMem(p);

	if (p == nullptr)
	{
		return;
	}

	const PrimCode code = {
		.poly = {
			.renderCode = RenderCode_Polygon,
			.quad = 1,
		}
	};

	color.code = code;
	p->colorCode = color;
	p->v[0].pos.x = x;      p->v[0].pos.y = y;
	p->v[1].pos.x = x + w;  p->v[1].pos.y = y;
	p->v[2].pos.x = x;      p->v[2].pos.y = y + h;
	p->v[3].pos.x = x + w;  p->v[3].pos.y = y + h;

	Prim_Add(p, ot);
}

void Prim_DrawShadowedBox(int x, int y, int w, int h, u_long* ot, struct PrimMem* primMem)
{
	RECT outer = { .x = x, .y = y, .w = w, .h = h };

	RECTMENU_DrawInnerRect(&outer, BOX_BORDER_TYPE, ot);

	RECT fill = {
		.x = x + BOX_FILL_PADDING_X,
		.y = y + BOX_FILL_PADDING_Y,
		.w = w - 2 * BOX_FILL_PADDING_X,
		.h = h - 2 * BOX_FILL_PADDING_Y,
	};

	CTR_Box_DrawClearBox(&fill, (Color*)&sdata->DrawSolidBoxData[0], 0, ot, primMem);
}
