#ifndef ZPG_BUILD_PREFAB_H
#define ZPG_BUILD_PREFAB_H

#include "zpg_internal.h"

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
        ZPGPoint riverStart = ZPG_RandomGridCellOutsideStencil(stencil, &seed);
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
        //ZPG_Grid_PrintChars(grid, '\0', 0, 0);
        printf("Walk starting at %d/%d\n", start.x, start.y);
        ZPGWalkCfg cfg = {};
        cfg.bigRoomChance = 0.02f;
        cfg.startX = topLeft.x + (start.x + dir.x);
        cfg.startY = topLeft.y + (start.y + dir.y);
        cfg.tilesToPlace = 80;
        cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
        ZPGPoint end = ZPG_RandomWalkAndFill(grid, stencil, &cfg, dir, &seed);
        ZPG_Grid_SetCellTypeAt(grid, end.x, end.y, ZPG2_CELL_TYPE_KEY, NULL);
    }
    #endif

    ZPG_FreeGrid(stencil);
    return grid;
}

static ZPGGrid* ZPG_Test_WalkBetweenPrefabs(i32 seed)
{
    i32 w = 96, h = 32;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    ZPGGrid* stencil = ZPG_CreateBorderStencil(w, h);
    i32 prefabIndex = 1;

    ZPGGridPrefab* leftRoom = ZPG_GetPrefabByIndex(prefabIndex);
    i32 roomYMax = h - leftRoom->grid->height;
    ZPGPoint blitPosA = {};
    blitPosA.x = 0;
    blitPosA.y = (i32)ZPG_RandArrIndex(roomYMax, seed++);
    ZPG_BlitGrids(grid, leftRoom->grid, blitPosA, stencil);
    // TODO Choose exit - assuming prefab has one (and only one)
    i32 leftExitIndex = ZPG_Prefab_GetExitIndexByDirection(leftRoom, { 1, 0 });
    if (leftExitIndex == -1) { printf("ABORT: No right exit on prefab\n"); return grid; }
    ZPGPoint leftExit = leftRoom->exits[leftExitIndex];
    ZPGPoint leftExitDir = leftRoom->exitDirs[leftExitIndex];

    ZPGGridPrefab* rightRoom = ZPG_GetPrefabByIndex(prefabIndex);
    roomYMax = h - rightRoom->grid->height;
    ZPGPoint blitPosB = {};
    blitPosB.x = grid->width - rightRoom->grid->width;
    blitPosB.y = (i32)ZPG_RandArrIndex(roomYMax, seed++);
    ZPG_BlitGrids(grid, rightRoom->grid, blitPosB, stencil);
    // TODO Choose exit - assuming prefab has one (and only one)
    i32 rightExitIndex = ZPG_Prefab_GetExitIndexByDirection(rightRoom, { -1, 0 });
    if (rightExitIndex == -1) { printf("ABORT: No right exit on prefab\n"); return grid; }
    ZPGPoint rightExit = rightRoom->exits[rightExitIndex];
    ZPGPoint rightExitDir = rightRoom->exitDirs[rightExitIndex];

    // draw rivers
    ZPGWalkCfg cfg = {};
    cfg.tilesToPlace = 30;
    cfg.bigRoomChance = 0.1f;
    #if 1
    cfg.typeToPaint = ZPG2_CELL_TYPE_VOID;
    for (i32 i = 1; i < 16; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&seed);
        ZPGPoint pos = ZPG_RandomGridCellOutsideStencil(stencil, &seed);
        cfg.startX = pos.x;
        cfg.startY = pos.y;
        ZPG_RandomWalkAndFill(grid, stencil, &cfg, dir, &seed);
    }
    #endif

    printf("Draw line segment between prefabs - Stencil state:\n");
    ZPG_Grid_PrintValues(stencil, YES);
    const i32 numNodes = 12;
    i32 numNodesMinusOne = numNodes - 1;
    ZPGPoint nodes[numNodes];
    nodes[0].x = (leftExit.x + blitPosA.x) + leftExitDir.x;
    nodes[0].y = (leftExit.y + blitPosA.y) + leftExitDir.y;
    nodes[numNodesMinusOne].x = (rightExit.x + blitPosB.x) + rightExitDir.x;
    nodes[numNodesMinusOne].y = (rightExit.y + blitPosB.y) + rightExitDir.y;
    //ZPG_PlotSegmentedPath_Old(grid, &seed, nodes, numNodes, NO, YES);
    f32 lineNodeOffsetMax = 10;//1.5f;
    f32 bigRoomChance = 0;//0.2f;
    ZPG_PlotSegmentedPath(grid, stencil, &seed, nodes, numNodes, lineNodeOffsetMax);
    ZPG_DrawSegmentedLine(grid, stencil, nodes, numNodes, ZPG2_CELL_TYPE_PATH, bigRoomChance);
    // random walk from nodes along the line
    #if 1
    cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
    cfg.tilesToPlace = 40;
    cfg.bigRoomChance = 0.3f;
    cfg.bPlaceObjectives = YES;
    for (i32 i = 1; i < numNodesMinusOne; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&seed);
        cfg.startX = nodes[i].x;
        cfg.startY = nodes[i].y;
        ZPGPoint end = ZPG_RandomWalkAndFill(grid, stencil, &cfg, dir, &seed);
        if (cfg.bPlaceObjectives == YES)
        {
            ZPG_Grid_SetCellTypeAt(grid, end.x, end.y, ZPG2_CELL_TYPE_KEY, NULL);
        }
    }
    #endif
    return grid;
}

#endif // ZPG_BUILD_PREFAB_H