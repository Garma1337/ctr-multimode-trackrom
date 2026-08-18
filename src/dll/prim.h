#ifndef PRIM_H_MODEROM
#define PRIM_H_MODEROM

#include <common.h>

void Prim_Add(void* prim, void* ot);
void Prim_DrawRect(short x, short y, short w, short h, Color color, u_long* ot);
void Prim_DrawShadowedBox(int x, int y, int w, int h, u_long* ot, struct PrimMem* primMem);

#endif
