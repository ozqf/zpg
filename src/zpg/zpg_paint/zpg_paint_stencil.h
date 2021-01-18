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

static void ZPG_CreateHorizontalBisectStencil(ZPGGrid* target)
{
    ZPG_PARAM_NULL(target, )
    ZPG_Grid_SetAll(target, ZPG_STENCIL_TYPE_EMPTY);
    i32 y = target->height / 2;
    ZPG_DrawLine(target, NULL, 0, y, target->width, y, 1, 0);
}

static void ZPG_CreateVerticalBisectStencil(ZPGGrid* target)
{
    ZPG_PARAM_NULL(target, )
    ZPG_Grid_SetAll(target, ZPG_STENCIL_TYPE_EMPTY);
    i32 x = target->width / 2;
    ZPG_DrawLine(target, NULL, x, 0, x, target->height, 1, 0);
}

#endif // ZPG_PAINT_STENCIL_H