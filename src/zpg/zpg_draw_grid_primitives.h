#ifndef ZPG_DRAW_GRID_PRIMITIVES_H
#define ZPG_DRAW_GRID_PRIMITIVES_H

#include "../zpg.h"

/**
 * Draw line algorithm - specific version that makes sure that pixels
 * in the line are always connected horizontally - this is really important!
 */
static void ZPG_DrawLine(ZPGGrid *grid, i32 aX, i32 aY, i32 bX, i32 bY, u8 typeToPaint)
{
    f32 x0 = (f32)aX, y0 = (f32)aY, x1 = (f32)bX, y1 = (f32)bY;
    float dx = x1 - x0;
    if (dx < 0)
    {
        dx = -dx;
    }
    float dy = y1 - y0;
    if (dy < 0)
    {
        dy = -dy;
    }

    i32 plotX = (i32)x0;
    i32 plotY = (i32)y0;

    i32 n = 1;
    i32 x_inc;
    i32 y_inc;
    float error;

    if (dx == 0)
    {
        x_inc = 0;
        error = INFINITY;
    }
    else if (x1 > x0)
    {
        x_inc = 1;
        n += (int)x1 - plotX;
        error = ((int)x0 + 1 - x0) * dy;
    }
    else
    {
        x_inc = -1;
        n += plotX - (int)x1;
        error = (x0 - (int)x0) * dy;
    }

    if (dy == 0)
    {
        y_inc = 0;

        error = -INFINITY;
    }
    else if (y1 > y0)
    {
        y_inc = 1;
        n += (int)y1 - plotY;
        error -= ((int)y0 + 1 - y0) * dx;
    }
    else
    {
        y_inc = -1;
        n += plotY - (int)y1;
        error -= (y0 - (int)y0) * dx;
    }

    for (; n > 0; --n)
    {
        ZPGCell* cell = ZPG_GetCellAt(grid, plotX, plotY);
        if (cell != NULL)
        {
            cell->tile.type = typeToPaint;
        }

        if (error > 0)
        {
            plotY += y_inc;
            error -= dx;
        }
        else
        {
            plotX += x_inc;
            error += dy;
        }
    }
}

static void ZPG_DrawSegmentedLine(ZPGGrid* grid, ZPGPoint* points, i32 numPoints, u8 typeToPaint)
{
    i32 numLines = numPoints - 1;
    if (numLines <= 0) { return; }
    for (i32 i = 0; i < numLines; ++i)
    {
        ZPGPoint* a = &points[i];
        ZPGPoint* b = &points[i + 1];
        ZPG_DrawLine(grid, a->x, a->y, b->x, b->y, typeToPaint);
    }
}

static void ZPG_DrawOuterBorder(ZPGGrid* grid, u8 typeToPaint)
{
    i32 maxX = grid->width - 1;
    i32 maxY = grid->height - 1;
    // top
    ZPG_DrawLine(grid, 0, 0, maxX, 0, typeToPaint);
    // bottom
    ZPG_DrawLine(grid, 0, maxY, maxX, maxY, typeToPaint);
    // left
    ZPG_DrawLine(grid, 0, 0, 0, maxY, typeToPaint);
    // right
    ZPG_DrawLine(grid, maxX, 0, maxX, maxY, typeToPaint);
}

static void ZPG_DrawRect(ZPGGrid* grid, ZPGPoint min, ZPGPoint max, u8 typeToPaint)
{
    // top
    ZPG_DrawLine(grid, min.x, min.y, max.x, min.y, typeToPaint);
    // bottom
    ZPG_DrawLine(grid, min.x, max.y, max.x, max.y, typeToPaint);
    // left
    ZPG_DrawLine(grid, min.x, min.y, min.x, max.y, typeToPaint);
    // right
    ZPG_DrawLine(grid, max.x, min.y, max.x, max.y, typeToPaint);
}

static void ZPG_CapFillBounds(ZPGGrid* grid, ZPGPoint* min, ZPGPoint* max)
{
    if (min->x < 0) { min->x = 0; }
    if (max->x >= grid->width) { max->x = grid->width - 1; }
    if (min->y < 0) { min->y = 0; }
    if (max->y >= grid->height) { max->y = grid->height - 1; }
}

static void ZPG_FillRect(ZPGGrid* grid, ZPGPoint min, ZPGPoint max, u8 typeToPaint)
{
    //printf("Fill rect %d/%d to %d/%d with %d\n", min.x, min.y, max.x, max.y, typeToPaint);
    if (min.x < 0) { min.x = 0; }
    if (max.x >= grid->width) { max.x = grid->width - 1; }
    if (min.y < 0) { min.y = 0; }
    if (max.y >= grid->height) { max.y = grid->height - 1; }
    for (i32 y = min.y; y <= max.y; ++y)
    {
        for (i32 x = min.x; x <= max.x; ++x)
        {
            ZPG_GetCellAt(grid, x, y)->tile.type = typeToPaint;
        }
    }
}

static i32 ZPG_FillRectWithStencil(
    ZPGGrid* grid, ZPGGrid* stencil, ZPGPoint min, ZPGPoint max, u8 typeToPaint)
{
    i32 numCellsPainted = 0;
    //printf("Fill rect %d/%d to %d/%d with %d\n", min.x, min.y, max.x, max.y, typeToPaint);
    if (min.x < 0) { min.x = 0; }
    if (max.x >= grid->width) { max.x = grid->width - 1; }
    if (min.y < 0) { min.y = 0; }
    if (max.y >= grid->height) { max.y = grid->height - 1; }
    for (i32 y = min.y; y <= max.y; ++y)
    {
        for (i32 x = min.x; x <= max.x; ++x)
        {
            if (ZPG_CheckStencilOccupied(stencil, x, y ) == YES) { continue; }
            ZPG_GetCellAt(grid, x, y)->tile.type = typeToPaint;
            numCellsPainted++;
        }
    }
    return numCellsPainted;
}

static void ZPG_BlitGrids(ZPGGrid* target, ZPGGrid* source, ZPGPoint topLeft, ZPGGrid* writeStencil)
{
    if (target == NULL) { return; }
    if (source == NULL) { return; }
    if (topLeft.x >= target->width) { return; }
    if (topLeft.y >= target->height) { return; }

    ZPGPoint sample = {};

    for (i32 sourceY = 0; sourceY < source->height; ++sourceY)
    {
        for(i32 sourceX = 0; sourceX < source->width; ++sourceX)
        {
            ZPGCell* sourceCell = ZPG_GetCellAt(source, sourceX, sourceY);
            ZPGPoint tarPos;
            tarPos.x = topLeft.x + sourceX;
            tarPos.y = topLeft.y + sourceY;
            ZPGCell* targetCell = ZPG_GetCellAt(target, tarPos.x, tarPos.y);
            if (sourceCell == NULL || targetCell == NULL) { continue; }
            *targetCell = *sourceCell;
            if (writeStencil != NULL)
            {
                ZPG_SetCellTypeAt(
                    writeStencil, tarPos.x, tarPos.y, ZPG_STENCIL_TYPE_FULL, NULL);
            }
        }
    }
}

#endif // ZPG_DRAW_GRID_PRIMITIVES_H