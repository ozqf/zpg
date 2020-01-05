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

/**
 * Returns number of directions written
 * Given array must have ZPG_NUM_DIRECTIONS of capacity
 */
static i32 ZPG_FindAvailableDirectionsAt(
    ZPGGrid* grid, ZPGGrid* stencil,
    i32 x, i32 y,
    ZPGPoint* cellsVisited,
    i32 numVisits,
    ZPGPoint* results)
{
    ZPGPoint query;
    i32 numResults = 0;
    i32 len = ZPG_NUM_DIRECTIONS;
    for (i32 i = 0; i < len; ++i)
    {
        ZPGPoint dir = g_directions[i];
        query.x = x + dir.x;
        query.y = y + dir.y;
        if (ZPG_CheckStencilOccupied(stencil, query.x, query.y) == YES)
        {
            continue;
        }
        i32 alreadyVisited = NO;
        for (i32 j = numVisits - 1; j >= 0; --j)
        {
			ZPGPoint visit = cellsVisited[j];
            if (visit.x == query.x && visit.y == query.y)
            {
				alreadyVisited = YES;
                break;
            }
        }
		if (alreadyVisited == YES) { continue; }
        // Add candidate
        results[numResults] = dir;
        numResults += 1;
    }
    return numResults;
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
    // Clear stencil tags.
    // Cells that have no valid exits are tagged. and are not
    // added to the visits list.
    if (stencil != NULL)
    {
        ZPG_Grid_ClearAllTags(stencil);
    }

    ZPGCellTypeDef* def = ZPG_GetType(cfg->typeToPaint);
    ZPGWalkInfo info;
    info.numPoints = 0;
    info.maxPoints = grid->width * grid->height;
    info.points = (ZPGPoint*)malloc(sizeof(ZPGPoint) * info.maxPoints);
    i32 tilesPlaced = 0;
    i32 bPainting = YES;
    i32 iterations = 0;
    const i32 max_iterations = 99999;
    ///////////////////////////////////////////////
    // Painting
    ///////////////////////////////////////////////
    while(bPainting == YES)
    {
        // paint current
         
        if (ZPG_SetCellTypeGeometry(
            grid, cursor.x, cursor.y, def->value, def->geometryType) == YES)
        {
            tilesPlaced++;
        }

        // record visit if stencil tag is clear.
        if (ZPG_GetTagAt(stencil, cursor.x, cursor.y) == 0)
        {
            info.points[info.numPoints] = cursor;
            info.numPoints++;
            printf("Walk recording %d/%d - num points %d\n", cursor.x, cursor.y, info.numPoints);
        }
        else
        {
            printf("Walk not recording %d/%d - it is tagged\n", cursor.x, cursor.y);
        }
        
        
        // select next
        // try and walk... if walk fails, start searching backward
        ZPGPoint nextPos;
        #if 1 // Randomly change dir
        f32 r = ZPG_Randf32(*seed);
        *seed += 1;
        if (r < 0.3f)
        {
            dir = ZPG_RandomThreeWayDir(seed, dir);
        }
        #endif
        nextPos.x = cursor.x + dir.x;
        nextPos.y = cursor.y + dir.y;
        printf("Walk trying %d/%d\n", nextPos.x, nextPos.y);
        if (ZPG_CheckStencilOccupied(stencil, nextPos.x, nextPos.y) == YES)
        {
            ///////////////////////////////////////////////
            // Blocked
            ///////////////////////////////////////////////
            // Step failed, search back through visited tiles
            // for a few cell to continue from.
            i32 bScanForNewTile = YES;
            i32 selectIterations = 0;
            while(bScanForNewTile)
            {
                if (info.numPoints == 0)
                {
                    // Bugger
                }
                // Search for a viable direction
                i32 pointIndex = info.numPoints - 1;
                ZPGPoint searchPos = info.points[pointIndex];
                //searchPos.x = cursor.x + dir.x;
                //searchPos.y = cursor.y + dir.y;
                ZPGPoint dirs[ZPG_NUM_DIRECTIONS];
                i32 numDirs = ZPG_FindAvailableDirectionsAt(
                    grid, stencil, searchPos.x, searchPos.y, info.points, info.numPoints, dirs);
                if (numDirs == 0)
                {
                    // Pop tile from points record.
                    // tag tile so it cannot be re-added to the point list
                    ZPG_GetCellAt(stencil, searchPos.x, searchPos.y)->tile.tag = 1;
                    info.numPoints--;
                }
                else
                {
                    bScanForNewTile = NO;
                    // we have options. proceed.
                    dir = dirs[ZPG_RandArrIndex(numDirs, cfg->seed++)];
                    nextPos.x = searchPos.x + dir.x;
                    nextPos.y = searchPos.y + dir.y;
                }
                selectIterations++;
                if (selectIterations > max_iterations)
                {
                    bPainting = NO;
                    printf("ERROR - select new tile ran away\n");
                }
            }
        }
        // step
        cursor = nextPos;
        if (ZPG_Grid_IsPositionSafe(grid, cursor.x, cursor.y) == NO)
        {
            printf("ERROR - walk escaped grid\n");
            break;
        }
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