#ifndef ZPG_RANDOM_WALK_H
#define ZPG_RANDOM_WALK_H

#include "../zpg.h"

struct ZPGWalkInfo
{
    ZPGPoint* points;
    i32 numPoints;
    i32 maxPoints;
};

static i32 ZPG_StepPositionAvailable(
    ZPGGrid* grid, ZPGGrid* stencil, i32 x, i32 y)
{
    if (ZPG_CheckStencilOccupied(stencil, x, y) == YES) { return NO; }

}

static void ZPG_FindAvailableDirectionsAt(
    ZPGGrid* grid, ZPGGrid* stencil,
    i32 x, i32 y,
    ZPGPoint* results, i32* numResults)
{
    *numResults = 0;
    i32 len = ZPG_NUM_DIRECTIONS;
    for (i32 i = 0; i < len; ++i)
    {
        ZPGPoint dir = g_directions[i];
        if (ZPG_StepPositionAvailable(grid, stencil, x + dir.x, y + dir.y) == NO)
        {
            continue;
        }
        // Add candidate
        results[*numResults] = dir;
        *numResults += 1;
    }
}

/**
 * Returns final position
 */
static ZPGPoint ZPG_RandomWalkAndFill(
    ZPGGrid* grid, ZPGGrid* stencil, ZPGWalkCfg* cfg, ZPGPoint dir, i32* seed)
{
    ZPGPoint cursor = { cfg->startX, cfg->startY };
    printf("Walk starting at %d/%d\n", cursor.x, cursor.y);
    if (dir.x == 0 && dir.y == 0) { return cursor; }
    // Clear stencil tags - will use to record visits
    if (stencil != NULL)
    {
        ZPG_Grid_ClearAllTags(stencil);
    }

    ZPGCellTypeDef* def = ZPG_GetType(cfg->typeToPaint);
    ZPGWalkInfo info;
    info.numPoints = 0;
    info.maxPoints = 1024;
    info.points = (ZPGPoint*)malloc(sizeof(ZPGPoint) * info.maxPoints);
    i32 tilesPlaced = 0;
    i32 bPainting = YES;
    i32 iterations = 0;
    const i32 max_iterations = 99999;
    while(bPainting == YES)
    {
        // paint current
        ZPG_SetCellTypeGeometry(
            grid, cursor.x, cursor.y, def->value, def->geometryType);
        tilesPlaced++;
        // record visit
        info.points[info.numPoints] = cursor;
        info.numPoints++;
        
        // select next
        // try and walk... if walk fails, start searching backward
        ZPGPoint nextPos;
        f32 r = ZPG_Randf32(*seed);
        *seed += 1;
        if (r < 0.3f)
        {
            dir = ZPG_RandomThreeWayDir(seed, dir);
        }
        nextPos.x = cursor.x + dir.x;
        nextPos.y = cursor.y + dir.y;
        printf("Walk trying %d/%d\n", nextPos.x, nextPos.y);
        i32 bScanForNewTile = NO;
        if (ZPG_CheckStencilOccupied(stencil, nextPos.x, nextPos.y) == YES)
        {
            bScanForNewTile = YES;
        }

        if (bScanForNewTile == YES)
        {
            // Step failed, search back through visited tiles
            // for a few cell to continue from.
            i32 bSelectingTile = YES;
            i32 recordIndex = info.numPoints - 1;
            if (recordIndex <= 0)
            {
                // oh dear
                printf("ABANDONED: Walk and fill has no candidate tiles left\n");
                free(info.points);
                return cursor;
            }
            i32 selectIterations = 0;
            while (bSelectingTile == YES)
            {
                // take the last tile recorded and check if it
                // can be used to continue walking
                ZPGPoint searchPos = info.points[recordIndex];
                nextPos.x = cursor.x + dir.x;
                nextPos.y = cursor.y + dir.y;
                if (ZPG_CheckStencilOccupied(stencil, nextPos.x, nextPos.y) == YES)
                {
                    bSelectingTile = YES;
                }
                selectIterations++;
                if (selectIterations >= max_iterations)
                {
                    printf("  ABORT: select previous tile ran away\n");
                    break;
                }
            }
        }
        nextPos.x = cursor.x + dir.x;
        nextPos.y = cursor.y + dir.y;
        // step
        cursor = nextPos;
        if (tilesPlaced >= cfg->tilesToPlace || iterations >= max_iterations)
        {
            bPainting = NO;
        }
        iterations++;
    }

    free(info.points);
    return cursor;
}

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
                ZPG_FillRect(grid, min, max, cfg->typeToPaint);
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

extern "C" void ZPG_PlotSegmentedPath(
    ZPGGrid* grid, i32* seed, ZPGPoint* points, i32 numPoints, i32 bVertical)
{
    i32 numLines = (numPoints - 1);
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
    
    printf("Drawing Horizontal path from %d/%d to %d/%d\n",
        points[0].x, points[0].y, points[numPoints - 1].x, points[numPoints - 1].y);
    // set mid-line node X positions
    i32 sectionLength;
    if (bVertical == YES) { sectionLength = grid->height / numLines; }
    else { sectionLength = grid->width / numLines; }
    for (i32 i = 1; i < numLines; ++i)
    {
        if (bVertical == YES)
        {
            points[i].x = (i32)ZPG_Randf32InRange(*seed, 0, (f32)(grid->width - 1));
            points[i].y = sectionLength * i;
        }
        else
        {
            points[i].x = sectionLength * i;
            points[i].y = (i32)ZPG_Randf32InRange(*seed, 0, (f32)(grid->height - 1));
        }
        seed++;
    }
}

#endif // ZPG_RANDOM_WALK_H