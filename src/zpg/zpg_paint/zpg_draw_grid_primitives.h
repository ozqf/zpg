#ifndef ZPG_DRAW_GRID_PRIMITIVES_H
#define ZPG_DRAW_GRID_PRIMITIVES_H

#include "zpg_internal.h"


static f32 ZPG_LerpF32(f32 start, f32 end, f32 lerp)
{
    //return start + lerp * (end - start);
    return start + ((end - start) * lerp);
}

#if 0
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
            ZPG_Grid_GetCellAt(grid, x, y)->tile.type = typeToPaint;
        }
    }
}
#endif

static void ZPG_Grid_DrawPoints(
    ZPGGrid* grid, ZPGPoint* points, i32 numPoints, u8 valToPaint)
{
    if (grid == NULL) { return; }
    if (points == NULL) { return; }
    if (numPoints == NULL) { return; }
    for (i32 i = 0; i < numPoints; ++i)
    {
        ZPG_GRID_SET(grid, points[i].x, points[i].y, valToPaint);
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
            if (ZPG_Grid_CheckStencilOccupied(stencil, x, y ) == YES) { continue; }
            grid->SetValue(x, y, typeToPaint);
            numCellsPainted++;
        }
    }
    return numCellsPainted;
}

static void ZPG_DrawCardinalLine(
    ZPGGrid* target, ZPGGrid* stencil, ZPGPoint origin, ZPGPoint dir, i32 iterations, u8 paintValue)
{
    ZPG_PARAM_NULL(target, )
    ZPG_PARAM_NULL(stencil, )
    for (i32 i = 0; i < iterations; ++i)
    {
        ZPGPoint p = origin;
        p.x += i * dir.x;
        p.x += i * dir.y;
        if (ZPG_Grid_CheckStencilOccupied(stencil, p.x, p.y)) { continue; }
        ZPG_GRID_SET(target, p.x, p.y, paintValue);
    }
}

/**
 * Draw line algorithm - specific version that makes sure that pixels
 * in the line are always connected horizontally - this is really important!
 */
static void ZPG_DrawLine(
    ZPGGrid *grid, ZPGGrid* stencil, i32 aX, i32 aY, i32 bX, i32 bY, u8 typeToPaint, f32 bigRoomChance)
{
    ZPG_PARAM_NULL(grid, )
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
        if (ZPG_IS_POS_SAFE(grid->width, grid->height, plotX, plotY))
        {
            i32 i = ZPG_POS_TO_INDEX(grid->width, plotX, plotY);
            f32 r = ZPG_Randf32(0);
            if (r < bigRoomChance)
            {
                ZPGPoint min = { plotX - 1, plotY - 1 };
                ZPGPoint max = { plotX + 1, plotY + 1 };
                ZPG_FillRectWithStencil(grid, stencil, min, max, typeToPaint);
            }
            else
            {
                grid->cells[i] = typeToPaint;
            }
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

static void ZPG_DrawSegmentedLine(
    ZPGGrid* grid, ZPGGrid* stencil, ZPGPoint* points, i32 numPoints, u8 typeToPaint, f32 bigRoomChance)
{
    ZPG_PARAM_NULL(grid, )
    ZPG_PARAM_NULL(points, )
    i32 numLines = numPoints - 1;
    if (numLines <= 0) { return; }
    for (i32 i = 0; i < numLines; ++i)
    {
        ZPGPoint* a = &points[i];
        ZPGPoint* b = &points[i + 1];
        
        ZPG_DrawLine(grid, stencil, a->x, a->y, b->x, b->y, typeToPaint, bigRoomChance);

    }
}

static void ZPG_DrawOuterBorder(ZPGGrid* grid, ZPGGrid* stencil, u8 typeToPaint)
{
    if (grid == NULL) { return; }
    i32 maxX = grid->width - 1;
    i32 maxY = grid->height - 1;
    // top
    ZPG_DrawLine(grid, stencil, 0, 0, maxX, 0, typeToPaint, 0);
    // bottom
    ZPG_DrawLine(grid, stencil, 0, maxY, maxX, maxY, typeToPaint, 0);
    // left
    ZPG_DrawLine(grid, stencil, 0, 0, 0, maxY, typeToPaint, 0);
    // right
    ZPG_DrawLine(grid, stencil, maxX, 0, maxX, maxY, typeToPaint, 0);
}

static void ZPG_DrawRect(ZPGGrid* grid, ZPGGrid* stencil, ZPGPoint min, ZPGPoint max, u8 typeToPaint)
{
    if (grid == NULL) { return; }
    // top
    ZPG_DrawLine(grid, stencil, min.x, min.y, max.x, min.y, typeToPaint, 0);
    // bottom
    ZPG_DrawLine(grid, stencil, min.x, max.y, max.x, max.y, typeToPaint, 0);
    // left
    ZPG_DrawLine(grid, stencil, min.x, min.y, min.x, max.y, typeToPaint, 0);
    // right
    ZPG_DrawLine(grid, stencil, max.x, min.y, max.x, max.y, typeToPaint, 0);
}

static void ZPG_CapFillBounds(ZPGGrid* grid, ZPGPoint* min, ZPGPoint* max)
{
    if (grid == NULL || min == NULL || max == NULL) { return; }
    if (min->x < 0) { min->x = 0; }
    if (max->x >= grid->width) { max->x = grid->width - 1; }
    if (min->y < 0) { min->y = 0; }
    if (max->y >= grid->height) { max->y = grid->height - 1; }
}

static void ZPG_AddGridsOfSameSize(ZPGGrid* target, ZPGGrid* source)
{
    if (target == NULL || source == NULL) { return; }
    if (target->width != source->width) { return; }
    if (target->height != source->height) { return; }

    i32 numCells = target->width * target->height;
    for (i32 i = 0; i < numCells; ++i)
    {
        target->cells[i] += source->cells[i];
    }
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
            
            ZPGPoint tarPos;
            tarPos.x = topLeft.x + sourceX;
            tarPos.y = topLeft.y + sourceY;
            if (!source->IsSafe(sourceX, sourceY)) { continue; }
            if (!target->IsSafe(tarPos.x, tarPos.y)) { continue; }
            target->SetValue(tarPos.x, tarPos.y, source->GetValue(sourceX, sourceY));

            // ZPGCell* sourceCell = ZPG_Grid_GetCellAt(source, sourceX, sourceY);
            // ZPGCell* targetCell = ZPG_Grid_GetCellAt(target, tarPos.x, tarPos.y);
            // if (sourceCell == NULL || targetCell == NULL) { continue; }
            // *targetCell = *sourceCell;
            if (writeStencil != NULL)
            {
                ZPG_Grid_SetValueWithStencil(
                    writeStencil, tarPos.x, tarPos.y, ZPG_STENCIL_TYPE_FULL, NULL);
            }
        }
    }
}

static i32 ZPG_Grid_FindFirstCellWithType(
    ZPGGrid* grid, u8 type, ZPGPoint* result)
{
    ZPG_BEGIN_GRID_ITERATE(grid)
        if (ZPG_GRID_GET(grid, x, y) == type)
        {
            result->x = x;
            result->y = y;
            return YES;
        }
        /*ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
        if (cell != NULL && cell->tile.type == type)
        {
            *resultX = x;
            *resultY = y;
            return cell;
        }*/
    ZPG_END_GRID_ITERATE
    return NO;
}

static void ZPG_PushPointToStack(
    ZPGPoint* points, i32* numPoints, i32 maxPoints, i32 x, i32 y)
{
    if (*numPoints >= maxPoints) { return; } // TODO: Error code or something?
    points[*numPoints].x = x;
    points[*numPoints].y = y;
    *numPoints += 1;
}

static void ZPG_MatchAndPushFillNode(
    ZPGGrid* grid, ZPGPoint* points, i32* numPoints, i32 maxPoints,
    i32 x, i32 y, u8 queryValue)
{
    if (ZPG_Grid_CheckValueAt(grid, x, y, queryValue, NO) == YES)
    {
        ZPG_PushPointToStack(points, numPoints, maxPoints, x, y);
    }
}

static void ZPG_MatchAndPushFillNode_WithTagCheck(
    ZPGGrid* grid, ZPGGrid* tagGrid, ZPGPoint* points, i32* numPoints, i32 maxPoints,
    i32 x, i32 y, u8 queryValue, u8 tag)
{
    if (!ZPG_GRID_POS_SAFE(grid, x, y)) { return; }
    if (ZPG_GRID_GET(tagGrid, x, y) == 1) { return; }
    // ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    // if (cell == NULL) { return; }
    // already visited?
    // if (cell->tile.tag == tag) { return; }
    i32 bMatch = ZPG_Grid_CheckValueAt(grid, x, y, queryValue, NO);
    if (bMatch == YES)
    {
        //cell->tile.tag = 1;
        ZPG_GRID_SET(tagGrid, x, y, 1);
        ZPG_PushPointToStack(points, numPoints, maxPoints, x, y);
        return;
    }
}

static void ZPG_Grid_FloodFill(
    ZPGGrid* grid, i32 x, i32 y, u8 fillValue)
{
    if (ZPG_Grid_IsPositionSafe(grid, x, y) == NO) { return; }
    //u8 emptyValue = ZPG_Grid_GetCellAt(grid, x, y)->tile.type;
    u8 emptyValue = ZPG_GRID_GET(grid, x, y);
    i32 maxPoints = grid->width * grid->height;
    ZPGPoint* points = (ZPGPoint*)ZPG_Alloc(maxPoints * sizeof(ZPGPoint), ZPG_MEM_TAG_POINTS);
    i32 numPoints = 0;
    ZPG_PushPointToStack(points, &numPoints, maxPoints, x, y);
    i32 exitCounter = 0;
    while (numPoints > 0)
    {
        // Grab and point top of stack
        ZPGPoint* p = &points[numPoints - 1];
        numPoints--;
        ZPG_Grid_SetValueWithStencil(grid, p->x, p->y, fillValue, NULL);
        ZPG_MatchAndPushFillNode(grid, points, &numPoints, maxPoints, p->x - 1, p->y, emptyValue);
        ZPG_MatchAndPushFillNode(grid, points, &numPoints, maxPoints, p->x + 1, p->y, emptyValue);
        ZPG_MatchAndPushFillNode(grid, points, &numPoints, maxPoints, p->x, p->y - 1, emptyValue);
        ZPG_MatchAndPushFillNode(grid, points, &numPoints, maxPoints, p->x, p->y + 1, emptyValue);
        exitCounter++;
        if (exitCounter >= maxPoints)
        {
            printf("ERROR - Fill from %d/%d ran away on %d iterations with %d points\n",
                x, y, exitCounter, numPoints);
            break;
        }
    }
    ZPG_Free(points);
}

/**
 * Tag grid must be an available grid of equal size so that searched cells
 * can be marked
 */
static i32 ZPG_Grid_FloodSearch(
    ZPGGrid* grid, ZPGGrid* tagGrid, i32 posX, i32 posY, ZPGPoint* results, i32 maxResults)
{
    if (ZPG_Grid_IsPositionSafe(grid, posX, posY) == NO) { return 0; }
    u8 emptyValue = ZPG_GRID_GET(grid, posX, posY);
    i32 maxPoints = grid->width * grid->height;
    ZPGPoint* points = (ZPGPoint*)ZPG_Alloc(maxPoints * sizeof(ZPGPoint), ZPG_MEM_TAG_POINTS);
    i32 numPoints = 0;
    ZPG_PushPointToStack(points, &numPoints, maxPoints, posX, posY);
    i32 exitCounter = 0;
    i32 numResults = 0;
    const i32 tagValue = 1;
    while (numPoints > 0)
    {
        // Pop stack and copy result (it may be rewritten immediately)
        ZPGPoint* p = &points[numPoints - 1];
        i32 x = p->x;
        i32 y = p->y;
        numPoints--;
        // add this node to results
        results[numResults].x = x;
        results[numResults].y = y;
        numResults++;
        // set tag so we don't count this cell again!
        ZPG_GRID_SET(tagGrid, x, y, 1);
        // ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
        // cell->tile.tag = 1;
        if (numResults >= maxResults) { printf(" max results\n"); break; }
        // add further nodes
        ZPG_MatchAndPushFillNode_WithTagCheck(
            grid, tagGrid, points, &numPoints, maxPoints, x - 1, y, emptyValue, tagValue);
        ZPG_MatchAndPushFillNode_WithTagCheck(
            grid, tagGrid, points, &numPoints, maxPoints, x + 1, y, emptyValue, tagValue);
        ZPG_MatchAndPushFillNode_WithTagCheck(
            grid, tagGrid, points, &numPoints, maxPoints, x, y - 1, emptyValue, tagValue);
        ZPG_MatchAndPushFillNode_WithTagCheck(
            grid, tagGrid, points, &numPoints, maxPoints, x, y + 1, emptyValue, tagValue);
        exitCounter++;
        if (exitCounter >= maxPoints)
        {
            printf("ERROR - Fill from %d/%d ran away on %d iterations with %d points\n",
                x, y, exitCounter, numPoints);
            break;
        }
    }
    ZPG_Free(points);
    return numResults;
}

#endif // ZPG_DRAW_GRID_PRIMITIVES_H