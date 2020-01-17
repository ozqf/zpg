#ifndef ZPG_UTILS_H
#define ZPG_UTILS_H

#include "zpg_internal.h"

static f32 ZPG_Distance(ZPGPoint a, ZPGPoint b)
{
    return sqrtf(((f32)b.x * (f32)a.x) + ((f32)b.y * (f32)a.y));
}

static i32 ZPG_RandomDir(i32* seed)
{
    f32 r = ZPG_Randf32(*seed);
    *seed += 1;
    if (r > 0.5f) { return -1; }
    else { return 1; }
}

static ZPGPoint ZPG_RandomGridCellOutsideStencil(ZPGGrid* stencil, i32* seed)
{
    i32 bBadPos = YES;
    ZPGPoint p = {};
    while (bBadPos)
    {
        p.x = (i32)ZPG_Randf32InRange(*seed, 0, (f32)stencil->width - 1);
        *seed++;
        p.y = (i32)ZPG_Randf32InRange(*seed, 0, (f32)stencil->height - 1);
        *seed++;
        if (ZPG_Grid_CheckStencilOccupied(stencil, p.x, p.y) == NO)
        {
            bBadPos = NO;
        }
    }
    
    return p;
}

static ZPGPoint ZPG_RandomGridCell(ZPGGrid* grid, i32* seed)
{
    ZPGPoint p;
    p.x = (i32)ZPG_Randf32InRange(*seed, 0, (f32)grid->width - 1);
    *seed++;
    p.y = (i32)ZPG_Randf32InRange(*seed, 0, (f32)grid->height - 1);
    *seed++;
    return p;
}

static ZPGPoint ZPG_RandomFourWayDir(i32* seed)
{
    f32 r = ZPG_Randf32(*seed);
    *seed += 1;
    ZPGPoint p = {};
    // left
    if (r < 0.25f) { p.x = -1; }
    // right
    else if (r < 0.5f) { p.x = 1; }
    // up
    else if (r < 0.75f) { p.y = -1; }
    // down
    else { p.y = 1; }
    return p;
}

static ZPGPoint ZPG_RandomThreeWayDir(i32* seed, ZPGPoint curDir)
{
    f32 r = ZPG_Randf32(*seed);
    *seed += 1;
    ZPGPoint p = curDir;
    f32 radians = atan2f((f32)p.y, (f32)p.x);
    f32 degrees = radians * RAD2DEG;
    if (r < 0.33333333f)
    {
        // turn left
        degrees += 90;
        radians = degrees * DEG2RAD;
        p.x = (i32)cosf(radians);
        p.y = (i32)sinf(radians);
    }
    else if (r < 0.6666666f)
    {
        // turn right
        degrees -= 90;
        radians = degrees * DEG2RAD;
        p.x = (i32)cosf(radians);
        p.y = (i32)sinf(radians);
    }
    // (else do nothing)
    return p;
}

//static 

static void ZPG_StepGridWithBorder(
    ZPGPoint *cursor, ZPGPoint *dir, ZPGPoint topLeft, ZPGPoint bottomRight)
{
    // Step
    cursor->x += dir->x;
    cursor->y += dir->y;
    // if out of grid, move back and change dir on that axis
    // Horizontal:
    if (dir->x != 0)
    {
        // left side
        if (cursor->x < topLeft.x)
        {
            cursor->x = topLeft.x;
            dir->x = 1;
        }
        // right side
        else if (cursor->x >= bottomRight.x)
        {
            cursor->x = bottomRight.x - 1;
            dir->x = -1;
        }
    }
    // Vertical
    if (dir->y != 0)
    {
        // top
        if (cursor->y < topLeft.y)
        {
            cursor->y = topLeft.y;
            dir->y = 1;
        }
        // bottom
        else if (cursor->y >= bottomRight.y)
        {
            cursor->y = bottomRight.y - 1;
            dir->y = -1;
        }
    }
}


/**
 * If currently under the stencil, move in given dir until not
 * Is allowed to paint tiles as it goes, just not change direction
 * returns 0 if this fails
 */
static i32 ZPG_MarchOutOfStencil(
    ZPGGrid* grid,
    ZPGGrid* stencil,
    ZPGPoint* cursor,
    ZPGPoint* dir,
    i32 bPaintPath,
    u8 typeToPaint)
{
    if (stencil == NULL) { return YES; }
    ZPGCellTypeDef* def = ZPG_GetType(typeToPaint);
    i32 bMoving = YES;
    i32 bSuccessful = NO;
    ZPGPoint nextPos = {};
    while (bMoving)
    {
        i32 bOverStencil = ZPG_Grid_CheckStencilOccupied(stencil, cursor->x, cursor->y);
        if (bOverStencil == YES)
        {
            nextPos.x = cursor->x + dir->x;
            nextPos.y = cursor->y + dir->y;
            if (ZPG_Grid_IsPositionSafe(grid, nextPos.x, nextPos.y) == NO) { return NO; }
            *cursor = nextPos;
            if (bPaintPath == YES)
            {
                ZPG_Grid_SetCellTypeGeometry(
                    grid, cursor->x, cursor->y, typeToPaint, def->geometryType);
                //ZPG_Grid_SetCellTypeAt(grid, cursor->x, cursor->y, typeToPaint);
            }
        }
        else
        {
            bMoving = false;
            bSuccessful = YES;
            // mark exit as a start
            //printf("Exiting stencil at %d/%d\n", cursor->x, cursor->y);
        }
    }
    return bSuccessful;
}

/**
 * Returns YES if the ray hit something
 */
static i32 ZPG_RaycastForHitOrGridEdge(
    ZPGGrid *grid, i32 aX, i32 aY, i32 bX, i32 bY, ZPGPoint* result)
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
    // If we hit something or step out of the grid, we want to
    // return the LAST safe position
    i32 lastPlotX = plotX;
    i32 lastPlotY = plotY;

    for (; n > 0; --n)
    {
        i32 bHit = NO;
        ZPGCell* cell = ZPG_Grid_GetCellAt(grid, plotX, plotY);
        if (cell != NULL)
        {
            //ZPG_Grid_SetCellTypeAt(grid, plotX, plotY, ZPG2_CELL_TYPE_VOID, NULL);
            if (cell->tile.type != 0)
            {
                bHit = YES;
                printf("Ray hit grid at %d/%d - result %d/%d\n",
                    plotX, plotY, lastPlotX, lastPlotY);
            }
        }
        else
        {
            printf("Ray off grid at %d/%d - result %d/%d\n",
                plotX, plotY, lastPlotX, lastPlotY);
            bHit = YES;
        }
    
        if (bHit == YES)
        {
            if (result != NULL)
            {
                result->x = lastPlotX;
                result->y = lastPlotY;
            }
            return YES;
        }
        
        lastPlotX = plotX;
        lastPlotY = plotY;

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
    return NO;
}


#endif // ZPG_UTILS_H