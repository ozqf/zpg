#ifndef ZPG_BUILD_PREFAB_H
#define ZPG_BUILD_PREFAB_H

#include "zpg_internal.h"

static ZPGGrid* ZPG_Test_PrefabBuildA(ZPGPresetCfg* presetCfg)
{
    //i32 w = 48, h = 48;
    i32 w = 96, h = 48;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    ZPGGrid* tagGrid = ZPG_CreateGrid(w, h);
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
    //ZPG_Grid_PrintValues(stencil, 1, YES);

    i32 numRivers = 4;
    for (i32 i = 0; i < numRivers; ++i)
    {
        printf("Draw river %d\n", i + 1);
        ZPGPoint riverStart = ZPG_RandomGridCellOutsideStencil(stencil, &presetCfg->seed);
        ZPGWalkCfg river = {};
        river.bigRoomChance = 0.02f;
        river.startX = riverStart.x;
        river.startY = riverStart.y;
        river.tilesToPlace = 80;
        river.typeToPaint = ZPG_CELL_TYPE_VOID;
        ZPGPoint dir = ZPG_RandomFourWayDir(&presetCfg->seed);
        ZPG_RandomWalkAndFill(grid, tagGrid, stencil, &river, dir, &presetCfg->seed);
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
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
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
        //ZPG_Grid_PrintCellDefChars(grid, '\0', 0, 0);
        printf("Walk starting at %d/%d\n", start.x, start.y);
        ZPGWalkCfg cfg = {};
        cfg.bigRoomChance = 0.02f;
        cfg.startX = topLeft.x + (start.x + dir.x);
        cfg.startY = topLeft.y + (start.y + dir.y);
        cfg.tilesToPlace = 80;
        cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
        ZPGPoint end = ZPG_RandomWalkAndFill(grid, tagGrid, stencil, &cfg, dir, &presetCfg->seed);
        ZPG_Grid_SetValueWithStencil(grid, end.x, end.y, ZPG_CELL_TYPE_KEY, NULL);
    }
    #endif

    ZPG_FreeGrid(stencil);
    return grid;
}

static ZPGGrid* ZPG_Test_WalkBetweenPrefabs(ZPGPresetCfg* presetCfg)
{
    i32 w = 96, h = 32;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    // fill the grid solid
    //ZPG_Grid_SetAll(grid, 1);

    ZPGGrid* tagGrid = ZPG_CreateGrid(w, h);
    ZPGGrid* stencil = ZPG_CreateBorderStencil(w, h);
    i32 prefabIndex = 1;

    /*
    TODO: Automation of room selection and exit linking
    */
    ZPGGridPrefab* leftRoom = ZPG_GetPrefabByIndex(prefabIndex);
    i32 roomYMax = h - leftRoom->grid->height;
    ZPGPoint blitPosA = {};
    blitPosA.x = 0;
    blitPosA.y = (i32)ZPG_RandArrIndex(roomYMax, presetCfg->seed++);
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
    blitPosB.y = (i32)ZPG_RandArrIndex(roomYMax, presetCfg->seed++);
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
    cfg.typeToPaint = ZPG_CELL_TYPE_VOID;
    for (i32 i = 1; i < 16; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&presetCfg->seed);
        ZPGPoint pos = ZPG_RandomGridCellOutsideStencil(stencil, &presetCfg->seed);
        cfg.startX = pos.x;
        cfg.startY = pos.y;
        ZPG_RandomWalkAndFill(grid, tagGrid, stencil, &cfg, dir, &presetCfg->seed);
    }
    #endif

    if ((presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0)
    {
        printf("Draw line segment between prefabs - Grid:\n");
        ZPG_Grid_PrintValues(grid, 1, YES);
        printf("Stencil:\n");
        ZPG_Grid_PrintValues(stencil, 1, YES);
    }
    
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
    ZPG_PlotSegmentedPath(grid, stencil, &presetCfg->seed, nodes, numNodes, lineNodeOffsetMax);
    if ((presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0)
    {
        printf("--- Plotted segemented path (%d nodes) ---\n", numNodes);
        for (i32 i = 0; i < numNodes; ++i)
        {
            printf("(%d, %d) ", nodes[i].x, nodes[i].y);
		}
        printf("\n");
	}
    ZPG_DrawSegmentedLine(grid, stencil, nodes, numNodes, ZPG_CELL_TYPE_PATH, bigRoomChance);
    if ((presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0)
    {
        ZPG_Grid_PrintCellDefChars(grid, '\0', 0, 0);
    }
    // random walk from nodes along the line
    #if 1
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
    cfg.tilesToPlace = 40;
    cfg.bigRoomChance = 0.3f;
    cfg.bPlaceObjectives = YES;
    for (i32 i = 1; i < numNodesMinusOne; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&presetCfg->seed);
        cfg.startX = nodes[i].x;
        cfg.startY = nodes[i].y;
        ZPGPoint end = ZPG_RandomWalkAndFill(grid, tagGrid, stencil, &cfg, dir, &presetCfg->seed);
        if (cfg.bPlaceObjectives == YES)
        {
            ZPG_Grid_SetValueWithStencil(grid, end.x, end.y, ZPG_CELL_TYPE_KEY, NULL);
        }
    }
    #endif
    return grid;
}

static void ZPG_PlotAndDrawSegmentedPath(ZPGGrid* grid, ZPGGrid* tagGrid, ZPGGrid* stencil, ZPGPoint a, ZPGPoint b)
{
    const i32 numNodes = 8;
    i32 numNodesMinusOne = numNodes - 1;
    ZPGPoint nodes[numNodes];
    nodes[0].x = a.x;
    nodes[0].y = a.y;
    nodes[numNodesMinusOne].x = b.x;
    nodes[numNodesMinusOne].y = b.y;

    f32 nodeOffsetMax = 5;
    i32 seed = 0;
    i32 err = ZPG_PlotSegmentedPath(grid, stencil, &seed, nodes, numNodes, nodeOffsetMax);
    if (err != ZPG_ERROR_NONE)
    {
        printf("ABORT %d PlotAndDrawSeg from %d, %d to %d, %d\n", err, a.x, a.y, b.x, b.y);
        return;
    }
    ZPG_DrawSegmentedLine(grid, stencil, nodes, numNodes, ZPG_CELL_TYPE_PATH, 0);

    ZPGWalkCfg cfg = {};
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
    cfg.tilesToPlace = 10;
    cfg.bigRoomChance = 0.3f;
    cfg.bPlaceObjectives = YES;
    for (i32 i = 1; i < numNodesMinusOne; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&seed);
        cfg.startX = nodes[i].x;
        cfg.startY = nodes[i].y;
        ZPGPoint end = ZPG_RandomWalkAndFill(grid, tagGrid, stencil, &cfg, dir, &seed);
        if (cfg.bPlaceObjectives == YES)
        {
            ZPG_Grid_SetValueWithStencil(grid, end.x, end.y, ZPG_CELL_TYPE_KEY, NULL);
        }
    }
}

static i32 ZPG_FindRoomConnectionPoints(
    ZPGGridPrefab* prefabA, ZPGGridPrefab* prefabB,
    ZPGPoint topLeftA, ZPGPoint topLeftB,
    ZPGPoint* resultStart, ZPGPoint* resultEnd)
{
    ZPG_PARAM_NULL(prefabA, 1);
    ZPG_PARAM_NULL(prefabB, 1);
    ZPG_PARAM_NULL(resultStart, 1);
    ZPG_PARAM_NULL(resultEnd, 1);
    // decide on direction of connection
    f32 dx = (f32)topLeftB.x - (f32)topLeftA.x;
    f32 dy = (f32)topLeftB.y - (f32)topLeftA.y;
    //printf("Rooms DX/DY: %.3f, %.3f\n", dx, dy);
    ZPGPoint connectionDir = {};
    if (dx > 0) { connectionDir.x = 1; }
    if (dx < 0) { connectionDir.x = -1; }
    if (dy > 0) { connectionDir.y = 1; }
    if (dy < 0) { connectionDir.y = -1; }

    ZPGPoint dirFlip;
    dirFlip.x = -connectionDir.x;
    dirFlip.y = -connectionDir.y;
    i32 exitIndexA = ZPG_Prefab_GetExitIndexByDirection(prefabA, connectionDir);
    if (exitIndexA == -1)
    {
        printf("No suitable exit from prefab A\n");
        return 1;
    }
    i32 exitIndexB = ZPG_Prefab_GetExitIndexByDirection(prefabB, dirFlip);
    if (exitIndexB == -1)
    {
        printf("No suitable exit from prefab B\n");
        return 1;
    }

    resultStart->x = topLeftA.x + prefabA->exits[exitIndexA].x;
    resultStart->y = topLeftA.y + prefabA->exits[exitIndexA].y;

    resultEnd->x = topLeftB.x + prefabB->exits[exitIndexB].x;
    resultEnd->y = topLeftB.y + prefabB->exits[exitIndexB].y;
    //printf("Connection points %d/%d to %d/%d\n",
    //    resultStart->x, resultStart->y, resultEnd->x, resultEnd->y);
    return 0;
}

static ZPGGrid* ZPG_Preset_PrefabsLinesCaves(ZPGPresetCfg* presetCfg)
{
    const i32 w = 96;
    const i32 h = 64;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    ZPGGrid* tagGrid = ZPG_CreateGrid(w, h);
    ZPGGrid* stencil = ZPG_CreateBorderStencil(w, h);
    ZPG_FillRectWithStencil(grid, stencil, { 1, 1 }, { w - 1, h - 1}, ZPG_CELL_TYPE_VOID);
    if (presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    { ZPG_Grid_PrintCellDefChars(grid, '\0', 0, 0); }
    ZPG_SeedCaves(
        grid,
        stencil,
        ZPG_CELL_TYPE_WALL,
        ZPG_CAVE_GEN_SEED_CHANCE_LOW,
        &presetCfg->seed);
    if (presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    { ZPG_Grid_PrintCellDefChars(grid, '\0', 0, 0); }

    for (i32 i = 0; i < 5; ++i)
    {
        ZPG_IterateCaves(
            grid,
            stencil,
            ZPG_CELL_TYPE_WALL,
            ZPG_CELL_TYPE_VOID,
            ZPG_CAVE_GEN_CRITICAL_NEIGHBOURS_DEFAULT);
    }
    if (presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    { ZPG_Grid_PrintCellDefChars(grid, '\0', 0, 0); }

    ZPGGridPrefab* room = ZPG_GetPrefabByIndex(0);
    i32 step = 5;
    i32 pointX = w - (room->grid->width) - step;
    i32 pointY = h - (room->grid->height) - step;
    ZPGPoint blitNorthWest = { step, step };
    ZPGPoint blitNorthEast = { pointX, step };
    ZPGPoint blitSouthWest = { step, pointY };
    ZPGPoint blitSouthEast = { pointX, pointY };
    ZPG_BlitGrids(grid, room->grid, blitNorthWest, stencil);
    ZPG_BlitGrids(grid, room->grid, blitNorthEast, stencil);
    ZPG_BlitGrids(grid, room->grid, blitSouthWest, stencil);
    ZPG_BlitGrids(grid, room->grid, blitSouthEast, stencil);
    
    ZPGPoint a, b;
    ZPG_FindRoomConnectionPoints(
        room, room, blitNorthWest, blitNorthEast, &a, &b);
    ZPG_PlotAndDrawSegmentedPath(grid, tagGrid, stencil, a, b);

    ZPG_FindRoomConnectionPoints(
        room, room, blitNorthEast, blitSouthEast, &a, &b);
    ZPG_PlotAndDrawSegmentedPath(grid, tagGrid, stencil, a, b);

    ZPG_FindRoomConnectionPoints(
        room, room, blitSouthEast, blitSouthWest, &a, &b);
    ZPG_PlotAndDrawSegmentedPath(grid, tagGrid, stencil, a, b);
    
    ZPG_FindRoomConnectionPoints(
        room, room, blitSouthWest, blitNorthWest, &a, &b);
    ZPG_PlotAndDrawSegmentedPath(grid, tagGrid, stencil, a, b);

    return grid;
}

#endif // ZPG_BUILD_PREFAB_H