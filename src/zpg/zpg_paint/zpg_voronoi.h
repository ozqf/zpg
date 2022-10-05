#ifndef ZPG_VORONOI_H
#define ZPG_VORONOI_H

#include "zpg_internal.h"

static void ZPG_SeedVoronoi(ZPGGrid* grid, i32 regionTotal, i32* randSeed)
{
    if (grid == NULL) { return; }
    ZPG_Grid_SetAll(grid, 0);
    
    i32 numPoints = 0;
    ZPGPoint* points = (ZPGPoint*)ZPG_Alloc(sizeof(ZPGPoint) * regionTotal, 0);
    
    while (numPoints < regionTotal)
    {
        i32 x = ZPG_RandArrIndex(grid->width, *randSeed);
        *randSeed += 1;
        i32 y = ZPG_RandArrIndex(grid->height, *randSeed);
        *randSeed += 1;
        if (ZPG_GRID_GET(grid, x, y) == 1)
        {
            continue;
        }
        numPoints += 1;
        ZPG_GRID_SET(grid, x, y, 1);
        points[numPoints++] = { x, y };
    }
    
    ZPG_PrintPointsAsGrid(points, numPoints, grid->width, grid->height);
    ZPG_Grid_SetAll(grid, 0);
    ZPG_BEGIN_GRID_ITERATE(grid)
        // find the nearest point
        f32 bestDist = 9999999;
        i32 nearest = -1;
        for (i32 i = 0; i < numPoints; ++i)
        {
            ZPGPoint regionPos = points[i];
            ZPGPoint queryPos = { x, y };
            f32 queryDist = ZPG_Distance(regionPos, queryPos);
            if (nearest < 0)
            {
                nearest = i;
                continue;
            }
            if (queryDist < bestDist)
            {
                bestDist = queryDist;
                nearest = i;
            }
            ZPG_GRID_SET(grid, x, y, nearest & 0xFF);
        }
    ZPG_END_GRID_ITERATE

    // seed a random number of regions
    #if 0
    i32 totalCells = grid->width * grid->height;
    printf("Voronoi seeding %d cells\n", totalCells);
    for (i32 i = 0; i < (totalCells - 1); ++i)
    {
        f32 r = ZPG_Randf32(*randSeed);
        *randSeed += 1;
        // f32 r = 0.1f;
        if (r < perCellRegionChance)
        {
            grid->cells[i] = 1;
            totalRegions += 1;
        }
    }
    #endif
    printf("Seeded %d regons\n", numPoints);
    ZPG_Free(points);
}

#endif // ZPG_VORONOI_H