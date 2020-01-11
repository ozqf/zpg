#ifndef ZPG_GRID_WALK_AND_FILL
#define ZPG_GRID_WALK_AND_FILL

#include "../zpg.h"

struct ZPGWalkInfo
{
    ZPGPoint* points;
    i32 numPoints;
    i32 maxPoints;
};

static i32 ZPG_FillAreaForWalk()
{
    return 0;
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
        if (ZPG_Grid_CheckStencilOccupied(stencil, query.x, query.y) == YES)
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
    i32 pointArrayBytes = sizeof(ZPGPoint) * info.maxPoints;
    printf("%d bytes for points array\n", pointArrayBytes);
    info.points = (ZPGPoint*)malloc(pointArrayBytes);
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
        if (ZPG_Grid_SetCellTypeGeometry(
            grid, cursor.x, cursor.y, def->value, def->geometryType) == YES)
        {
            tilesPlaced++;
        }

        // record visit if stencil tag is clear.
        if (ZPG_Grid_GetTagAt(stencil, cursor.x, cursor.y) == 0)
        {
            info.points[info.numPoints] = cursor;
            info.numPoints++;
        }
        
        // select next
        // try and walk... if walk fails, start searching backward
        ZPGPoint nextPos;
        #if 1 // Randomly change dir
        f32 r = ZPG_Randf32(*seed);
        *seed += 1;
        f32 branchChance = 0.7f; //0.3f;
        if (r < branchChance)
        {
            dir = ZPG_RandomThreeWayDir(seed, dir);
        }
        #endif
        nextPos.x = cursor.x + dir.x;
        nextPos.y = cursor.y + dir.y;
        //printf("Walk trying %d/%d\n", nextPos.x, nextPos.y);
        if (ZPG_Grid_CheckStencilOccupied(stencil, nextPos.x, nextPos.y) == YES)
        {
            ///////////////////////////////////////////////
            // Blocked
            ///////////////////////////////////////////////
            // Step failed, search back through visited tiles
            // for a cell to continue from.
            i32 bScanForNewTile = YES;
            i32 selectIterations = 0;
            while(bScanForNewTile)
            {
                if (info.numPoints == 0)
                {
                    // Bugger
                    bPainting = NO;
                    printf("ABORT - Walk and fill ran out of potential cells\n");
                    bScanForNewTile = NO;
                    continue;
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
                    ZPG_Grid_GetCellAt(stencil, searchPos.x, searchPos.y)->tile.tag = 1;
                    info.numPoints--;
                }
                else
                {
                    bScanForNewTile = NO;
                    // we have options. proceed.
                    dir = dirs[ZPG_RandArrIndex(numDirs, cfg->seed++)];
                    nextPos.x = searchPos.x + dir.x;
                    nextPos.y = searchPos.y + dir.y;
                    break;
                }
                selectIterations++;
                if (selectIterations > max_iterations)
                {
                    bPainting = NO;
                    printf("ERROR - select new tile ran away\n");
                    break;
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

#endif // ZPG_GRID_WALK_AND_FILL