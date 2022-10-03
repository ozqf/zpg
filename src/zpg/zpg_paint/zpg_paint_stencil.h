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

static void ZPG_Draw_BorderStencil(ZPGGrid* target)
{
    ZPG_PARAM_NULL(target, )
    ZPG_Grid_SetAll(target, ZPG_STENCIL_TYPE_EMPTY);
    ZPG_DrawOuterBorder(target, NULL, ZPG_STENCIL_TYPE_FULL);
}

static void ZPG_Draw_HorizontalBisectStencil(ZPGGrid* target)
{
    ZPG_PARAM_NULL(target, )
    i32 y = target->height / 2;
    ZPG_DrawLine(target, NULL, 0, y, target->width, y, 1, 0);
}

static void ZPG_Draw_VerticalBisectStencil(ZPGGrid* target)
{
    ZPG_PARAM_NULL(target, )
    i32 x = target->width / 2;
    ZPG_DrawLine(target, NULL, x, 0, x, target->height, 1, 0);
}

static void ZPG_Draw_RandomVerticalBisectStencil(ZPGGrid* target, f32 offsetScale, i32* seed)
{
    ZPG_PARAM_NULL(target, )
    if (offsetScale > 1)
    {
        offsetScale = 0;
    }
    else if (offsetScale < 0)
    {
        offsetScale = 0;
    }
    f32 maxOffset = ((f32)(target->width / 2) * offsetScale);
    i32 x0 = target->width / 2;
    x0 += (i32)ZPG_Randf32InRange(ZPG_INC_SEED_PTR(seed), -maxOffset, maxOffset);
    i32 x1 = target->width / 2;
    x1 += (i32)ZPG_Randf32InRange(ZPG_INC_SEED_PTR(seed), -maxOffset, maxOffset);
    ZPG_DrawLine(target, NULL, x0, 0, x1, target->height, 1, 0);
}

static void ZPG_Draw_RandomHorizontalBisectStencil(ZPGGrid* target, f32 offsetScale, i32* seed)
{
    ZPG_PARAM_NULL(target, )
    if (offsetScale > 1)
    {
        offsetScale = 0;
    }
    else if (offsetScale < 0)
    {
        offsetScale = 0;
    }
    f32 maxOffset = ((f32)(target->height / 2) * offsetScale);
    i32 y0 = target->height / 2;
    y0 += (i32)ZPG_Randf32InRange(ZPG_INC_SEED_PTR(seed), -maxOffset, maxOffset);
    i32 y1 = target->height / 2;
    y1 += (i32)ZPG_Randf32InRange(ZPG_INC_SEED_PTR(seed), -maxOffset, maxOffset);
    ZPG_DrawLine(target, NULL, 0, y0, target->width, y1, 1, 0);
}

#endif // ZPG_PAINT_STENCIL_H