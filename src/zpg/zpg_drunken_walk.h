#ifndef ZPG_RANDOM_WALK_H
#define ZPG_RANDOM_WALK_H

#include "../zpg.h"

static void ZPG_RandomStepWithinRect(
    ZPGGrid* grid, ZPGRect rect, ZPGPoint* cursor, ZPGPoint* dir, i32* seed)
{
    // if already over a stencil cell, keep moving forward.
    /*i32 bOverStencil = ZPG_CheckStencilOccupied(stencil, cursor->x, cursor->y);
    if (bOverStencil == YES)
    {
        cursor->x += dir->x;
        cursor->y += dir->y;
        return;
    }*/

    // form 2 - select a tile to move to, and pick another if not acceptable
    #if 0

    for(;;)
    {
        i32 searchFails = 0;
        ZPGPoint candidate;
        *dir = ZPG_RandomFourWayDir(seed);
        candidate.x = cursor->x + dir->x;
        candidate.y = cursor->y + dir->y;
        i32 bOverStencil = ZPG_CheckStencilOccupied(stencil, candidate->x, candidate->y);
        if (bOverStencil == NO)
        {
            break;
        }
        searchFails++;
        if (searchFails > 99)
        {
            printf("Stuck!\n");
            return;
        }
    }
    #endif

    #if 1
    // form 1 - move around within a rectangular border, with random
    // direction changes.
    ZPG_StepGridWithBorder(cursor, dir, rect.min, rect.max);
    const f32 turnChance = 0.3f;
    f32 changeDirChance = ZPG_Randf32(*seed);
    *seed += 1;
    if (changeDirChance > turnChance)
    {
        *dir = ZPG_RandomThreeWayDir(seed, *dir);
        *seed += 1;
    }
    #endif
}

/**
 * Returns final position
 */
extern "C" ZPGPoint ZPG_GridRandomWalk(
    ZPGGrid* grid, ZPGGrid* stencil, ZPGRect* borderRect, ZPGWalkCfg* cfg, ZPGPoint dir)
{
    ZPGPoint cursor = { cfg->startX, cfg->startY };
    ZPGPoint lastPos = cursor;
    i32 bMarkStencilExitsAsStart = YES;
    ZPG_SetCellTypeAt(grid, cursor.x, cursor.y, cfg->typeToPaint, NULL);
    // move start out of stencil if required.
    if (ZPG_MarchOutOfStencil(grid, stencil, &cursor, &dir, YES, cfg->typeToPaint) == NO)
    {
        printf("ABORT: Failed to move walk out of stencil\n");
        return cursor;
    }
    if (cfg->bPlaceObjectives)
    {
        ZPG_SetCellTypeAt(grid, cursor.x, cursor.y, ZPG2_CELL_TYPE_START, NULL);
    }

    ZPGRect border;
    // setup border
    if (borderRect == NULL)
    {
        // Default, leave a one tile border around the map
        border.min.x = 1;
        border.min.y = 1;
        border.max.x = grid->width - 1;
        border.max.y = grid->height - 1;
    }
    else
    {
        border = *borderRect;
    }
    

    const i32 escapeCounter = 99999;
    i32 iterations = 0;
    i32 tilesPlaced = 0;
    while (tilesPlaced < cfg->tilesToPlace)
    {
        ZPGCell* cell = ZPG_GetCellAt(grid, cursor.x, cursor.y);
        if (cell != NULL && cell->tile.type != cfg->typeToPaint)
        {
            f32 r = ZPG_Randf32(cfg->seed++);
            if (r < cfg->bigRoomChance)
            {
                ZPGPoint min = { cursor.x - 1, cursor.y - 1 };
                ZPGPoint max = { cursor.x + 1, cursor.y + 1 };
                ZPG_FillRectWithStencil(grid, stencil, min, max, cfg->typeToPaint);
            }
            else
            {
                ZPG_SetCellTypeAt(grid, cursor.x, cursor.y, cfg->typeToPaint, NULL);
            }
            lastPos = cursor;
            tilesPlaced++;
        }
        ZPG_RandomStepWithinRect(grid, border, &cursor, &dir, &cfg->seed);
        iterations++;
        if (iterations >= escapeCounter)
        {
            printf("ABORT! Walk ran away\n");
            break;
        }
    }
    //ZPG_SetCellTagAt(grid, lastPos.x, lastPos.y, ZPG_CELL_TAG_RANDOM_WALK_END);
    if (cfg->bPlaceObjectives)
    {
        ZPG_SetCellTypeAt(grid, cursor.x, cursor.y, ZPG2_CELL_TYPE_END, NULL);
    }
    //printf("Drunken walk placed %d tiles in %d iterations\n",
    //    tilesPlaced, iterations);
    return cursor;
}

static void ZPG_PrintPointsArray(ZPGPoint* points, i32 numPoints)
{
    for (i32 i = 0; i < numPoints; ++i)
    {
        printf("Point %d: %d/%d\n", i, points[i].x, points[i].y);
    }
}

extern "C" void ZPG_PlotSegmentedPath_Old(
    ZPGGrid* grid, i32* seed, ZPGPoint* points, i32 numPoints, i32 bVertical, i32 bEndpointsSet)
{
    i32 lastIndex = numPoints - 1;
    i32 numLines = (numPoints - 1);
    if (bEndpointsSet == NO)
    {
        if (bVertical == YES)
        {
            // set start
            points[0].x = grid->width / 2;
            points[0].y = 0;
            // set end
            points[numPoints - 1].x = grid->width / 2;
            points[numPoints - 1].y = grid->height - 1;
        }
        else
        {
            // set start
            points[0].x = 0;
            points[0].y = grid->height / 2;
            // set end
            points[numPoints - 1].x = grid->width - 1;
            points[numPoints - 1].y = grid->height / 2;
        }
    }
    i32 lineWidth = points[lastIndex].x - points[0].x;
    i32 lineHeight = points[lastIndex].y - points[0].y;
    
    printf("Drawing Horizontal path from %d/%d to %d/%d\n",
        points[0].x, points[0].y, points[lastIndex].x, points[lastIndex].y);
    // set mid-line node X positions
    i32 sectionLength;
    if (bVertical == YES) { sectionLength = lineHeight / numLines; }
    else { sectionLength = lineWidth / numLines; }
    for (i32 i = 1; i < numLines; ++i)
    {
        if (bVertical == YES)
        {
            points[i].x = (i32)ZPG_Randf32InRange(*seed, 0, (f32)(grid->width - 1)) + points[0].x;
            points[i].y = sectionLength * i;
        }
        else
        {
            points[i].x = sectionLength * i +  + points[0].x;
            points[i].y = (i32)ZPG_Randf32InRange(*seed, 0, (f32)(grid->height - 1));
        }
        seed++;
    }
}

/**
 * Assumes 0 and numPoints - 1 are preset as the start/end positions
 */
extern "C" void ZPG_PlotSegmentedPath(
    ZPGGrid* grid,
    ZPGGrid* stencil,
    i32* seed,
    ZPGPoint* points,
    i32 numPoints,
    f32 pointOffsetMax)
{
    i32 lastIndex = numPoints - 1;
    i32 numLines = (numPoints - 1);
    ZPGPoint* firstPoint = &points[0];
    ZPGPoint* lastPoint = &points[lastIndex];
    f32 dx = (f32)lastPoint->x - (f32)firstPoint->x;
    if (dx < 0) { dx = -dx; }
    f32 dy = (f32)lastPoint->y - (f32)firstPoint->y;
    if (dy < 0) { dy = -dy; }
    f32 length = sqrtf((dx * dx) + (dy * dy));
    printf("Line length %.3f\n", length);
    dx /= length;
    dy /= length;
    f32 leftNormalX = dy;// -dy;
    f32 leftNormalY = -dx;//dx;
    f32 rightNormalX = -dy;//dy;
    f32 rightNormalY = dx;//-dx;
    printf("Left normal %.3f/%.3f\n", leftNormalX, leftNormalY);
    printf("Right normal %.3f/%.3f\n", rightNormalX, rightNormalY);
    f32 step = 1.f / (f32)numLines;
    f32 lerp = step; // start one step in as first is already set
    for (i32 i = 1; i < numLines; ++i)
    {
        // calc x/y on line
        ZPGPoint* p = &points[i];
        p->x = (i32)(ZPG_LerpF32((f32)firstPoint->x, (f32)lastPoint->x, lerp));
        p->y = (i32)(ZPG_LerpF32((f32)firstPoint->y, (f32)lastPoint->y, lerp));
        lerp += step;
        // cast rays from node along line normals
        // to find the maximum possible offset positions
        ZPGPoint leftRayEnd;
        leftRayEnd.x = (i32)((f32)p->x + (leftNormalX * pointOffsetMax));
        leftRayEnd.y = (i32)((f32)p->y + (leftNormalY * pointOffsetMax));
        i32 bHit = ZPG_RaycastForHitOrGridEdge(
            stencil, p->x, p->y, leftRayEnd.x, leftRayEnd.y, &leftRayEnd);
        ZPG_SetCellTypeAt(
            grid, leftRayEnd.x, leftRayEnd.y, ZPG2_CELL_TYPE_PATH, NULL);
        ZPGPoint rightRayEnd;
        rightRayEnd.x = (i32)((f32)p->x + (rightNormalX * pointOffsetMax));
        rightRayEnd.y = (i32)((f32)p->y + (rightNormalY * pointOffsetMax));
        bHit = ZPG_RaycastForHitOrGridEdge(
            stencil, p->x, p->y, rightRayEnd.x, rightRayEnd.y, &rightRayEnd);
        ZPG_SetCellTypeAt(
            grid, rightRayEnd.x, rightRayEnd.y, ZPG2_CELL_TYPE_VOID, NULL);
        f32 r1 = ZPG_Randf32(*seed);
        *seed += 1;
        f32 r2 = ZPG_Randf32(*seed);
        *seed += 1;
        i32 offsetDX = (i32)((f32)(rightRayEnd.x - leftRayEnd.x) * r1);
        i32 offsetDY = (i32)((f32)(rightRayEnd.y - leftRayEnd.y) * r1);
        printf("Offsets %d/%d r1 %.3f\n", offsetDX, offsetDY, r1);
        p->x = leftRayEnd.x + offsetDX;
        p->y = leftRayEnd.y + offsetDY;
    }
}

#endif // ZPG_RANDOM_WALK_H