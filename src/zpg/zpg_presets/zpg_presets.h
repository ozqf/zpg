#ifndef ZPG_PRESETS_H
#define ZPG_PRESETS_H

#include "zpg_internal.h"


static ZPGPresetOutput ZPG_TestDrunkenWalk_FromCentre(ZPGPresetCfg* presetCfg)
{
    printf("Generate: Drunken walk - start from centre\n");
    i32 bStepThrough = presetCfg->flags & ZPG_API_FLAG_STEP_THROUGH;

    i32 bVerbose = ((presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0);
    i32 width = 72, height = 32;
    if (presetCfg->width > 0) { width = presetCfg->width; }
    if (presetCfg->height > 0) { height = presetCfg->height; }

    ZPGGrid* grid = ZPG_CreateGrid(width, height);
    ZPG_Grid_SetAll(grid, 1);
    ZPGGrid* stencil = ZPG_CreateBorderStencil(width, height);
    // ZPGPoint centreMin = { 16, 8 };
    // ZPGPoint centreMax = { 48, 24 };
    ZPGPoint centreMin = { 20, 12 };
    ZPGPoint centreMax = { 40, 20 };
    ZPG_FillRectWithStencil(stencil, NULL, centreMin, centreMax, ZPG_STENCIL_TYPE_FULL );
    // debug check stencil
    //ZPG_Grid_PrintValues(stencil, 1, YES;
    ZPGWalkCfg cfg = {};
    cfg.seed = presetCfg->seed;
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
    cfg.typeToPaint = ZPG_CELL_TYPE_VOID;
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
        if (bStepThrough)
        {
            printf("River %d:\n", i);
            ZPG_Grid_PrintValues(grid, 0, YES);
            printf("Press ENTER to continue\n");
            getchar();
        }
    }
    // Draw "tunnels"
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
    cfg.bPlaceObjectives = YES;
    //cfg.charToPlace = '#';
    for (i32 i = 0; i < numPaths; ++i)
    {
        //ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        ZPGPoint dir = directions[i % numDirections];
        ZPG_GridRandomWalk(grid, stencil, NULL, &cfg, dir);
        if (bStepThrough)
        {
            printf("Path %d:\n", i);
            ZPG_Grid_PrintValues(grid, 0, YES);
            printf("Press ENTER to continue\n");
            getchar();
        }
    }
    //printf("Final seed value: %d\n", cfg.seed);
    ZPG_FreeGrid(stencil);
    ZPGGrid* style = ZPG_CreateStylingGrid(grid);
    ZPG_FreeGrid(style);
    return ZPG_OutputFromAsciiGrid(grid);
}

/**
 * Define some rects within a grid and random walk within
 */
static ZPGPresetOutput ZPG_TestDrunkenWalk_WithinSpace(ZPGPresetCfg* presetCfg)
{
    printf("Generate: Drunken walk - Within borders\n");
    i32 bVerbose = ((presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0);
    i32 w = 72, h = 32;
    if (presetCfg->width > 0) { w = presetCfg->width; }
    if (presetCfg->height > 0) { h = presetCfg->height; }

    ZPGGrid* grid = ZPG_CreateGrid(w, h);
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
    cfg.seed = presetCfg->seed;
    cfg.tilesToPlace = 120;//256;
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
    for (i32 i = 0; i < numSquares; ++i)
    {
        ZPGRect rect = squares[i];
        //ZPGRect rect = squares[1];
        printf("Draw in rect %d/%d to %d/%d\n", rect.min.x, rect.min.y, rect.max.x, rect.max.y);
        ZPG_DrawRect(grid, NULL, rect.min, rect.max, ZPG_CELL_TYPE_VOID);
        ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        ZPGPoint centre = ZPG_Rect_Centre(rect);
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
    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestDrunkenWalk_Scattered(ZPGPresetCfg* presetCfg)
{
    printf("Generate: Drunken walk - scattered starting points\n");
    i32 bVerbose = ((presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0);
    i32 w = 64, h = 32;
    if (presetCfg->width > 0) { w = presetCfg->width; }
    if (presetCfg->height > 0) { h = presetCfg->height; }

    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    ZPG_Grid_SetAll(grid, 1);
    ZPGWalkCfg cfg = {};
    cfg.seed = presetCfg->seed;
    cfg.tilesToPlace = 80;//256;
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
    
    i32 numRivers = 4;
    i32 numPaths = 4;
    printf("Drawing %d rivers and %d paths\n", numRivers, numPaths);
    // Rivers
    cfg.typeToPaint = ZPG_CELL_TYPE_VOID;
    for (i32 i = 0; i < numPaths; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        cfg.startX = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 63);
        cfg.startY = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 31);
        ZPG_GridRandomWalk(grid, NULL, NULL, &cfg, dir);
    }
    // Paths
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
    for (i32 i = 0; i < numPaths; ++i)
    {
        ZPGPoint dir = ZPG_RandomFourWayDir(&cfg.seed);
        cfg.startX = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 63);
        cfg.startY = (i32)ZPG_Randf32InRange(cfg.seed++, 0, 31);
        ZPG_GridRandomWalk(grid, NULL, NULL, &cfg, dir);
    }
    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestCaveGen(ZPGPresetCfg* presetCfg)
{
    i32 bVerbose = ((presetCfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0);
    i32 w = 72, h = 32;
    if (presetCfg->width > 0) { w = presetCfg->width; }
    if (presetCfg->height > 0) { h = presetCfg->height; }

    ZPGCellRules rules = ZPG_DefaultCaveRules();
    
    // Create canvas
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    ZPGGrid* stencil = ZPG_CreateGrid(w, h);
    if (presetCfg->flags & ZPG_API_FLAG_SOLID_BORDER)
    {
        ZPG_DrawOuterBorder(stencil, NULL, ZPG_STENCIL_TYPE_FULL);
    }
    // ZPG_Draw_HorizontalBisectStencil(stencil);
    // ZPG_Draw_VerticalBisectStencil(stencil);

    ZPG_Draw_RandomVerticalBisectStencil(stencil, 0.8f, &presetCfg->seed);
    ZPG_Draw_RandomHorizontalBisectStencil(stencil, 0.8f, &presetCfg->seed);
    //ZPG_Grid_SetAll(grid, rules.emptyValue);
    
    // Stencil grid - blocks writing of tiles
    // ZPGGrid* stencil = ZPG_CreateBorderStencil(w, h);
	ZPG_Grid_Copy(stencil, grid);

    if (bVerbose)
    {
        printf("Stencil\n");
        ZPG_Grid_PrintValues(stencil, 1, YES);
    }
	
	#if 1
	ZPG_FillCaves(grid, stencil, rules, &presetCfg->seed, bVerbose);
	#endif // use fill caves function
	
	#if 0
    ZPG_SeedCaves(
        grid,
        stencil,
        ZPG_CELL_TYPE_WALL,
        ZPG_CAVE_GEN_SEED_CHANCE_DEFAULT,
        &presetCfg->seed);
    
    if (bVerbose)
    {
        printf("Cave seed:\n");
        ZPG_Grid_PrintCellDefChars(grid, '\0', 0, 0);
    }
    for (i32 i = 0; i < 2; ++i)
    {
        ZPG_IterateCaves_defunct(
            grid,
            stencil,
            ZPG_CELL_TYPE_WALL,
            ZPG_CELL_TYPE_PATH,
            ZPG_CAVE_GEN_CRITICAL_NEIGHBOURS_DEFAULT);
    }
	
	ZPG_Grid_FlipBinary(grid, NULL);
	#endif
	
    ZPG_FreeGrid(stencil);
    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestCaveLayers(ZPGPresetCfg* presetCfg)
{
    // TODO: output leaks here, no cleanup!
    ZPGGrid* grid = ZPG_TestCaveGen(presetCfg).asciiGrid;
    ZPG_Grid_PrintValues(grid, 0, YES);
    ZPGGrid* gridB = ZPG_TestCaveGen(presetCfg).asciiGrid;
    ZPG_Grid_PrintValues(gridB, 0, YES);
    ZPG_AddGridsOfSameSize(grid, gridB);
    ZPG_AddGridsOfSameSize(grid, gridB);
    ZPG_FreeGrid(gridB);

    ZPG_Grid_PrintValues(grid, 0, YES);
    ZPG_Grid_PerlinToGreyscale(grid, NULL);
    //ZPG_Grid_PrintTexture(grid, YES);
	//ZPG_Grid_PrintValues(grid, 0, YES);
    if (presetCfg->flags & ZPG_API_FLAG_PRINT_GREYSCALE)
    {
		printf("Draw grid as texture\n");
        ZPG_Grid_PrintTexture(grid, NO);
    }

    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestDrawOffsetLines(ZPGPresetCfg* presetCfg)
{
    const i32 width = 72;
    const i32 height = 32;
    ZPGGrid* grid = ZPG_CreateGrid(width, height);
    //ZPG_Grid_SetCellTypeAll(grid, ZPG_CELL_TYPE_WALL);
    ZPG_Grid_SetAll(grid, ZPG_CELL_TYPE_WALL);

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
    cfg.typeToPaint = ZPG_CELL_TYPE_VOID;
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
    ZPGPoint* points = (ZPGPoint*)ZPG_Alloc(sizeof(ZPGPoint) * numPoints, ZPG_MEM_TAG_POINTS);
    ZPG_PlotSegmentedPath_Old(grid, &seed, points, numPoints, bVertical, NO);
    ZPG_DrawSegmentedLine(grid, NULL, points, numPoints, ZPG_CELL_TYPE_PATH, 0.2f);

    // Draw side paths
    cfg.tilesToPlace = numTilesPerPath;
    cfg.typeToPaint = ZPG_CELL_TYPE_PATH;
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
        ZPG_Grid_SetValueWithStencil(grid, end.x, end.y, ZPG_CELL_TYPE_KEY, NULL);
    }
    // play start/end points
    ZPG_Grid_SetValueWithStencil(grid, points[0].x, points[0].y, ZPG_CELL_TYPE_START, NULL);
    ZPG_Grid_SetValueWithStencil(grid, points[numPoints - 1].x, points[numPoints - 1].y, ZPG_CELL_TYPE_END, NULL);

    // cleanup
    ZPG_Free(points);
    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestDrawLines(ZPGPresetCfg* presetCfg)
{
    ZPGGrid* grid = ZPG_CreateGrid(72, 32);
    //ZPG_Grid_SetCellTypeAll(grid, ZPG_CELL_TYPE_WALL);
    ZPG_Grid_SetAll(grid, ZPG_CELL_TYPE_WALL);

    ZPG_DrawOuterBorder(grid, NULL, ZPG_CELL_TYPE_PATH);
    ZPG_DrawLine(grid, NULL, 0, 0, 71, 31, ZPG_CELL_TYPE_PATH, 0);
    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestLoadAsciFile(ZPGPresetCfg* presetCfg)
{
    char* fileName = "test_grid.txt";
    ZPGGrid* grid = NULL;
    #if 1
    zpgSize len;
    u8* chars = ZPG_StageFile(fileName, &len);
    printf("Read %lld bytes from %s\n", len, fileName);
    //ZPG_MeasureGridInString(chars, &size, len);
    grid = ZPG_ReadGridAsci(chars, len);
    #endif

    // clean up
    ZPG_Free(chars);
    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestEmbed(ZPGPresetCfg* presetCfg)
{
    ZPGGrid* grid = NULL;
    const char* str = embed_8x8_grid_pillars;
    zpgSize len = ZPG_STRLEN(str);
    grid = ZPG_ReadGridAsci((u8*)str, len);

    return ZPG_OutputFromAsciiGrid(grid);
}

static ZPGPresetOutput ZPG_TestBlit(ZPGPresetCfg* presetCfg)
{
    printf("*** TEST GRID BLIT ***\n");
    ZPGGrid* grid = NULL;
    ZPGGrid* source = NULL;
    ZPGGrid* stencil = NULL;
    const char* str = embed_8x8_grid_pillars;
    zpgSize len = ZPG_STRLEN(str);
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

    return ZPG_OutputFromAsciiGrid(grid);
}


#endif // ZPG_PRESETS_H