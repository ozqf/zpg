#ifndef ZPG_PRESETS_H
#define ZPG_PRESETS_H

#include "zpg_internal.h"


static ZPGGrid* ZPG_TestDrunkenWalk_FromCentre(i32 seed, i32 bbStepThrough)
{
    printf("Generate: Drunken walk - start from centre\n");
    const i32 width = 64;
    const i32 height = 32;
    ZPGGrid* grid = ZPG_CreateGrid(width, height);
    ZPGGrid* stencil = ZPG_CreateBorderStencil(width, height);
    ZPG_FillRectWithStencil(stencil, NULL, { 16, 8 }, { 48, 24 }, ZPG_STENCIL_TYPE_FULL );
    // debug check stencil
    //ZPG_Grid_PrintValues(stencil);
    ZPGWalkCfg cfg = {};
    cfg.seed = seed;
    cfg.startX = 31;
    cfg.startY = 15;
    cfg.bigRoomChance = 0.01f;
    cfg.tilesToPlace = 40;//256;

    i32 numRivers = 4;
    i32 numPaths = 4;
    printf("Drawing %d rivers and %d paths\n", numRivers, numPaths);
    const i32 numDirections = 4;
    ZPGPoint directions[] =
    {
        { -1, 0 },
        { 1, 0 },
        { 0, -1 },
        { 0, 1 }
    };
    // Draw "rivers"
    cfg.typeToPaint = ZPG2_CELL_TYPE_VOID;
    cfg.bPlaceObjectives = NO;
    cfg.bigRoomChance = 0.3f;
    cfg.bStepThrough = NO;
    //cfg.bigRoomChance = 0;
    //cfg.charToPlace = '.';
    for (i32 i = 0; i < numRivers; ++i)
    {
        //ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        ZPGPoint dir = directions[i % numDirections];
        ZPG_GridRandomWalk(grid, stencil, NULL, &cfg, dir);
        if (bbStepThrough)
        {
            printf("River %d:\n", i);
            ZPG_Grid_PrintValues(grid, YES);
            printf("Press ENTER to continue\n");
            getchar();
        }
    }
    // Draw "tunnels"
    cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
    cfg.bPlaceObjectives = YES;
    //cfg.charToPlace = '#';
    for (i32 i = 0; i < numPaths; ++i)
    {
        //ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        ZPGPoint dir = directions[i % numDirections];
        ZPG_GridRandomWalk(grid, stencil, NULL, &cfg, dir);
        if (bbStepThrough)
        {
            printf("Path %d:\n", i);
            ZPG_Grid_PrintValues(grid, YES);
            printf("Press ENTER to continue\n");
            getchar();
        }
    }
    //printf("Final seed value: %d\n", cfg.seed);
    ZPG_FreeGrid(stencil);
    return grid;
}

/**
 * Define some rects within a grid and random walk within
 */
static ZPGGrid* ZPG_TestDrunkenWalk_WithinSpace(i32 seed)
{
    printf("Generate: Drunken walk - Within borders\n");
    ZPGGrid* grid = ZPG_CreateGrid(72, 32);
    i32 halfWidth = grid->width / 2;
    i32 halfHeight = grid->height / 2;
    const i32 numSquares = 4;
    ZPGRect squares[numSquares];
    i32 borderSize = 1;
    // top left
    squares[0].min.x = borderSize;
    squares[0].min.y = borderSize;
    squares[0].max.x = halfWidth - (borderSize + 1);
    squares[0].max.y = halfHeight - (borderSize + 1);
    // top right
    squares[1].min.x = halfWidth + borderSize;
    squares[1].min.y = (0) + borderSize;
    squares[1].max.x = grid->width - (borderSize + 1);
    squares[1].max.y = halfHeight - (borderSize + 1);
    // bottom left
    squares[2].min.x = (0) + borderSize;
    squares[2].min.y = halfHeight + borderSize;
    squares[2].max.x = halfWidth - (borderSize + 1);
    squares[2].max.y = grid->height - (borderSize + 1);
    // bottom right
    squares[3].min.x = halfWidth + borderSize;
    squares[3].min.y = halfHeight + borderSize;
    squares[3].max.x = grid->width - (borderSize + 1);
    squares[3].max.y = grid->height - (borderSize + 1);

    ZPGWalkCfg cfg = {};
    cfg.seed = seed;
    cfg.tilesToPlace = 120;//256;
    cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
    for (i32 i = 0; i < numSquares; ++i)
    {
        ZPGRect rect = squares[i];
        //ZPGRect rect = squares[1];
        printf("Draw in rect %d/%d to %d/%d\n", rect.min.x, rect.min.y, rect.max.x, rect.max.y);
        ZPG_DrawRect(grid, NULL, rect.min, rect.max, ZPG2_CELL_TYPE_VOID);
        ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        ZPGPoint centre = rect.Centre();
        cfg.startX = centre.x;
        cfg.startY = centre.y;
        ZPG_GridRandomWalk(grid, NULL, &rect, &cfg, dir);
    }
    /*
    ZPGRect rect;
    rect.min.x = quarterWidth;
    rect.min.y = quarterHeight;
    rect.max.x = quarterWidth * 3;
    rect.max.y = quarterHeight * 3;
    ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
    ZPGPoint centre = rect.Centre();
    printf("Quarter size: %d/%d\n", quarterWidth, quarterHeight);
    printf("Draw from %d/%d\n", centre.x, centre.y);
    cfg.startX = (i32)ZPG_Randf32InRange(cfg.seed++, 0, (f32) centre.x);
    cfg.startY = (i32)ZPG_Randf32InRange(cfg.seed++, 0, (f32) centre.y);
    cfg.startX = centre.x;
    cfg.startY = centre.y;
    ZPG_GridRandomWalk(grid, &rect, &cfg, dir);
    */
   return grid;
}

static ZPGGrid* ZPG_TestDrunkenWalk_Scattered(i32 seed)
{
    printf("Generate: Drunken walk - scattered starting points\n");
    ZPGGrid* grid = ZPG_CreateGrid(64, 32);
    ZPGWalkCfg cfg = {};
    cfg.seed = seed;
    cfg.tilesToPlace = 80;//256;
    cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
    
    i32 numRivers = 4;
    i32 numPaths = 4;
    printf("Drawing %d rivers and %d paths\n", numRivers, numPaths);
    // Rivers
    cfg.typeToPaint = ZPG2_CELL_TYPE_VOID;
    for (i32 i = 0; i < numPaths; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        cfg.startX = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 63);
        cfg.startY = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 31);
        ZPG_GridRandomWalk(grid, NULL, NULL, &cfg, dir);
    }
    // Paths
    cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
    for (i32 i = 0; i < numPaths; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        cfg.startX = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 63);
        cfg.startY = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 31);
        ZPG_GridRandomWalk(grid, NULL, NULL, &cfg, dir);
    }
    return grid;
}

static ZPGGrid* ZPG_TestCaveGen(i32 seed)
{
    // Create canvas
    ZPGGrid* grid = ZPG_CreateGrid(72, 32);
    ZPG_Grid_SetCellTypeAll(grid, ZPG2_CELL_TYPE_WALL);
    // Stencil grid - blocks writing of tiles
    ZPGGrid* stencil = ZPG_CreateBorderStencil(72, 32);
    //ZPGGrid* stencil = ZPG_CreateGrid(72, 32);
    //stencil->SetCellTypeAll(ZPG_CELL_TYPE_NONE);
    //ZPG_DrawOuterBorder(stencil, ZPG_CELL_TYPE_FLOOR);

    ZPG_SeedCaves(grid, stencil, ZPG2_CELL_TYPE_PATH, &seed);
    ZPG_Grid_PrintChars(grid, '\0', 0, 0);
    i32 numIterations = 2;
    for (i32 i = 0; i < numIterations; ++i)
    {
        ZPG_IterateCaves(grid, stencil, ZPG2_CELL_TYPE_WALL, ZPG2_CELL_TYPE_PATH);
    }
    ZPG_FreeGrid(stencil);
    return grid;
}

static ZPGGrid* ZPG_TestDrawOffsetLines()
{
    const i32 width = 72;
    const i32 height = 32;
    ZPGGrid* grid = ZPG_CreateGrid(width, height);
    ZPG_Grid_SetCellTypeAll(grid, ZPG2_CELL_TYPE_WALL);

    i32 seed = 0;
    const i32 numPoints = 6;
    i32 numLines = numPoints - 1;
    i32 numRivers = 8;
    i32 numTilesPerRiver = 80;
    i32 numTilesPerPath = 40;

    // Draw rivers
    ZPGWalkCfg cfg = {};
    cfg.seed = seed;
    cfg.tilesToPlace = numTilesPerRiver;
    cfg.typeToPaint = ZPG2_CELL_TYPE_VOID;
    cfg.bigRoomChance = 0;

    for (i32 i = 0; i < numRivers; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        ZPGPoint p = ZPG_RandomGridCell(grid, &cfg.seed);
        cfg.startX = p.x;
        cfg.startY = p.y;
        ZPG_GridRandomWalk(grid, NULL, NULL, &cfg, dir);
    }

    // Draw main path
    i32 bVertical = NO;
    ZPGPoint* points = (ZPGPoint*)ZPG_Alloc(sizeof(ZPGPoint) * numPoints);
    ZPG_PlotSegmentedPath_Old(grid, &seed, points, numPoints, bVertical, NO);
    ZPG_DrawSegmentedLine(grid, NULL, points, numPoints, ZPG2_CELL_TYPE_PATH, 0.2f);

    // Draw side paths
    cfg.tilesToPlace = numTilesPerPath;
    cfg.typeToPaint = ZPG2_CELL_TYPE_PATH;
    cfg.bigRoomChance = 0.1f;
    for (i32 i = 1; i < numLines; ++i)
    {
        ZPGPoint dir = {};
        if (bVertical == YES)
        {
            dir.x = ZPG_RandomDir(&cfg.seed);
        }
        else
        {
            dir.y = ZPG_RandomDir(&cfg.seed);
        }
        cfg.startX = points[i].x;
        cfg.startY = points[i].y;
        ZPGPoint end = ZPG_GridRandomWalk(grid, NULL, NULL, &cfg, dir);
        // place objective at the end
        ZPG_Grid_SetCellTypeAt(grid, end.x, end.y, ZPG2_CELL_TYPE_KEY, NULL);
    }
    // play start/end points
    ZPG_Grid_SetCellTypeAt(grid, points[0].x, points[0].y, ZPG2_CELL_TYPE_START, NULL);
    ZPG_Grid_SetCellTypeAt(grid, points[numPoints - 1].x, points[numPoints - 1].y, ZPG2_CELL_TYPE_END, NULL);

    // cleanup
    ZPG_Free(points);
    return grid;
}

static ZPGGrid* ZPG_TestDrawLines()
{
    ZPGGrid* grid = ZPG_CreateGrid(72, 32);
    ZPG_Grid_SetCellTypeAll(grid, ZPG2_CELL_TYPE_WALL);

    ZPG_DrawOuterBorder(grid, NULL, ZPG2_CELL_TYPE_PATH);
    ZPG_DrawLine(grid, NULL, 0, 0, 71, 31, ZPG2_CELL_TYPE_PATH, 0);
    return grid;
}

static ZPGGrid* ZPG_TestLoadAsciFile()
{
    char* fileName = "test_grid.txt";
    ZPGGrid* grid = NULL;
    #if 1
    i32 len;
    u8* chars = ZPG_StageFile(fileName, &len);
    printf("Read %d bytes from %s\n", len, fileName);
    //ZPG_MeasureGridInString(chars, &size, len);
    grid = ZPG_ReadGridAsci(chars, len);
    #endif

    // clean up
    ZPG_Free(chars);
    return grid;
}

static ZPGGrid* ZPG_TestEmbed(i32 seed)
{
    ZPGGrid* grid = NULL;
    const char* str = embed_8x8_grid_pillars;
    i32 len = ZPG_STRLEN(str);
    grid = ZPG_ReadGridAsci((u8*)str, len);

    return grid;
}

static ZPGGrid* ZPG_TestBlit(i32 seed)
{
    printf("*** TEST GRID BLIT ***\n");
    ZPGGrid* grid = NULL;
    ZPGGrid* source = NULL;
    ZPGGrid* stencil = NULL;
    const char* str = embed_8x8_grid_pillars;
    i32 len = ZPG_STRLEN(str);
    source = ZPG_ReadGridAsci((u8*)str, len);
 
    grid = ZPG_CreateGrid(source->width * 2, source->height * 2);
    ZPGPoint topLeft;
    topLeft.x = 0;//source->width / 4;
    topLeft.y = 0;//source->height / 4;
    ZPG_BlitGrids(grid, source, topLeft, NULL);

    topLeft.x = grid->width - source->width;
    topLeft.x = grid->height - source->height;
    ZPG_BlitGrids(grid, source, topLeft, NULL);

    ZPG_FreeGrid(stencil);
    ZPG_FreeGrid(source);

    return grid;
}


#endif // ZPG_PRESETS_H