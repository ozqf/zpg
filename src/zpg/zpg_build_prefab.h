#ifndef ZPG_BUILD_PREFAB_H
#define ZPG_BUILD_PREFAB_H

#include "../zpg.h"

static ZPGGrid* ZPG_Test_PrefabBuildA(i32 seed)
{
    //i32 w = 48, h = 48;
    i32 w = 96, h = 48;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    ZPGGrid* stencil = ZPG_CreateBorderStencil(w, h);
    i32 gridHalfWidth = w / 2, gridHalfHeight = h / 2;

    // Draw prefabs and write stencil to prevent walks from
    // overwriting them.
    ZPGGridPrefab* prefab = ZPG_GetPrefabByIndex(0);
    i32 prefabHalfWidth = prefab->grid->width / 2;
    i32 prefabHalfHeight = prefab->grid->height / 2;

    ZPGPoint topLeft;
    topLeft.x = gridHalfWidth - prefabHalfWidth;
    topLeft.y = gridHalfHeight - prefabHalfHeight;
    ZPG_BlitGrids(grid, prefab->grid, topLeft, stencil);

    //printf("Stencil after blit:\n");
    //ZPG_Grid_PrintValues(stencil);

    i32 numRivers = 4;
    for (i32 i = 0; i < numRivers; ++i)
    {
        printf("Draw river %d\n", i + 1);
        ZPGPoint riverStart = ZPG_RandomGridCellOutsideStencil(grid, stencil, &seed);
        ZPGWalkCfg river = {};
        river.bigRoomChance = 0.02f;
        river.startX = riverStart.x;
        river.startY = riverStart.y;
        river.tilesToPlace = 80;
        river.typeToPaint = ZPG2_CELL_TYPE_VOID;
        ZPGPoint dir = ZPG_RandomFourWayDir(&seed);
        ZPG_RandomWalkAndFill(grid, stencil, &river, dir, &seed);
    }
    

    #if 0 // debug - single walk

    i32 exit = 3; // to right
    ZPGPoint start = prefab->exits[exit];
    ZPGPoint dir = prefab->exitDirs[exit];

    ZPGWalkCfg cfg = {};
    cfg.bigRoomChance = 0.02f;
    cfg.startX = topLeft.x + (start.x + dir.x);
    cfg.startY = topLeft.y + (start.y + dir.y);
    cfg.tilesToPlace = 80;
    cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
    //ZPGPoint end = ZPG_RandomWalkAndFill(grid, stencil, &cfg, dir, &seed);
    ZPGPoint end = ZPG_RandomWalkAndFill(grid, stencil, &cfg, dir, &seed);
    #endif

    #if 1 // Walk from all exits

    printf("Walking from prefab's %d exits\n", prefab->numExits);
    for (i32 i = 0; i < prefab->numExits; ++i)
    {
        ZPGPoint start = prefab->exits[i];
        ZPGPoint dir = prefab->exitDirs[i];
        //printf("Grid before random walk:\n");
        //ZPG_PrintChars(grid);
        printf("Walk starting at %d/%d\n", start.x, start.y);
        ZPGWalkCfg cfg = {};
        cfg.bigRoomChance = 0.02f;
        cfg.startX = topLeft.x + (start.x + dir.x);
        cfg.startY = topLeft.y + (start.y + dir.y);
        cfg.tilesToPlace = 80;
        cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
        ZPGPoint end = ZPG_RandomWalkAndFill(grid, stencil, &cfg, dir, &seed);
        ZPG_SetCellTypeAt(grid, end.x, end.y, ZPG2_CELL_TYPE_KEY, NULL);
    }
    #endif

    free(stencil);
    return grid;
}

static ZPGGrid* ZPG_Test_WalkBetweenPrefabs(i32 seed)
{
    i32 w = 96, h = 24;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    ZPGGrid* stencil = ZPG_CreateBorderStencil(w, h);

    ZPGGridPrefab* leftRoom = ZPG_GetPrefabByIndex(2);
    i32 roomYMax = h - leftRoom->grid->height;
    ZPGPoint blitPosA = {};
    blitPosA.x = 0;
    blitPosA.y = (i32)ZPG_RandArrIndex(roomYMax, seed++);
    ZPG_BlitGrids(grid, leftRoom->grid, blitPosA, stencil);
    ZPGPoint leftExit = leftRoom->exits[0];
    ZPGPoint leftExitDir = leftRoom->exitDirs[0];

    ZPGGridPrefab* rightRoom = ZPG_GetPrefabByIndex(3);
    roomYMax = h - rightRoom->grid->height;
    ZPGPoint blitPosB = {};
    blitPosB.x = grid->width - rightRoom->grid->width;
    blitPosB.y = (i32)ZPG_RandArrIndex(roomYMax, seed++);
    ZPG_BlitGrids(grid, rightRoom->grid, blitPosB, stencil);
    ZPGPoint rightExit = rightRoom->exits[0];
    ZPGPoint rightExitDir = rightRoom->exitDirs[0];

    printf("Draw line segment between prefabs - Stencil state:\n");
    ZPG_Grid_PrintValues(stencil, YES);
    const i32 numNodes = 12;
    ZPGPoint nodes[numNodes];
    nodes[0].x = (leftExit.x + blitPosA.x) + leftExitDir.x;
    nodes[0].y = (leftExit.y + blitPosA.y) + leftExitDir.y;
    nodes[numNodes - 1].x = (rightExit.x + blitPosB.x) + rightExitDir.x;
    nodes[numNodes - 1].y = (rightExit.y + blitPosB.y) + rightExitDir.y;
    //ZPG_PlotSegmentedPath_Old(grid, &seed, nodes, numNodes, NO, YES);
    f32 lineNodeOffsetMax = 10;//1.5f;
    f32 bigRoomChance = 0;//0.2f;
    ZPG_PlotSegmentedPath(grid, stencil, &seed, nodes, numNodes, lineNodeOffsetMax);
    ZPG_DrawSegmentedLine(grid, stencil, nodes, numNodes, ZPG2_CELL_TYPE_PATH, bigRoomChance);
    #if 0
    for (i32 i = 0; i < numNodes; ++i)
    {
        ZPG_SetCellTypeAt(grid, nodes[i].x, nodes[i].y, ZPG2_CELL_TYPE_ENEMY, NULL);
    }
    #endif
    return grid;
}

#endif // ZPG_BUILD_PREFAB_H