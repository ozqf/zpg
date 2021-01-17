#ifndef ZPG_PAINT_STENCIL_H
#define ZPG_PAINT_STENCIL_H

#include "../zpg_internal.h"


static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height)
{
    ZPGGrid* stencil = ZPG_CreateGrid(width, height);
    ZPG_Grid_SetAll(stencil, ZPG_STENCIL_TYPE_EMPTY);
    ZPG_DrawOuterBorder(stencil, NULL, ZPG_STENCIL_TYPE_FULL);
    return stencil;
}

static ZPGGrid* ZPG_CreateHorizontalBisectStencil(i32 width, i32 height)
{
    ZPGGrid* stencil = ZPG_CreateGrid(width, height);
    ZPG_Grid_SetAll(stencil, ZPG_STENCIL_TYPE_EMPTY);
    i32 y = height / 2;
    ZPG_DrawLine(stencil, NULL, 0, y, width, y, 1, 0);
    return stencil;
}

#endif // ZPG_PAINT_STENCIL_H