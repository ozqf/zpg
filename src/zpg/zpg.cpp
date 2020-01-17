#ifndef ZE_PROC_GEN_CPP
#define ZE_PROC_GEN_CPP

//#ifndef ZPG_EXPORT
//#define ZPG_EXPORT extern "C"
//#endif

#include "zpg_internal.h"

/*
Research material
https://www.boristhebrave.com/2019/07/28/dungeon-generation-in-enter-the-gungeon/

*/

// Grid type is 1 byte so hard limit on types
#define ZPG_TYPES_LIST_SIZE 255
#define ZPG_NUM_TYPES 256
static u8 g_bInitialised = NO;
static ZPGCellTypeDef g_types[ZPG_TYPES_LIST_SIZE];
static u8 g_numTypes = 0;

#define ZPG_MAX_PREFABS 255
static ZPGGridPrefab g_prefabs[ZPG_MAX_PREFABS];
static i32 g_numPrefabs = 0;

#define ZPG_NUM_DIRECTIONS 4
#define ZPG_DIR_RIGHT 0
#define ZPG_DIR_UP 1
#define ZPG_DIR_LEFT 2
#define ZPG_DIR_DOWN 3
static ZPGPoint g_directions[ZPG_NUM_DIRECTIONS];

extern "C" ZPGGrid* ZPG_CreateGrid(i32 width, i32 height);
static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height);

#include "string.h"
#include "time.h"
#include "zpg_random_table.h"
#include "zpg_cell_types.h"
#include "zpg_grid.h"
#include "zpg_utils.h"
#include "zpg_file.h"
#include "zpg_draw_grid_primitives.h"
#include "zpg_embed.h"

// individual generation functions
#include "zpg_perlin_draw.h"
#include "zpg_drunken_walk.h"
#include "zpg_grid_walk_and_fill.h"
#include "zpg_cave_gen.h"
#include "zpg_entities.h"

// Combined generation functions
#include "zpg_build_prefab.h"
#include "zpg_build_perlin.h"

#include "zpg_script.h"

extern "C" ZPGGrid* ZPG_CreateGrid(i32 width, i32 height)
{
    i32 totalCells = width * height;
    i32 memForGrid = (sizeof(ZPGCell) * totalCells);
    i32 memTotal = sizeof(ZPGGrid) + memForGrid;
    //printf("Make grid %d by %d (%d cells, %d bytes)\n",
    //    width, height, (width * height), memTotal);
    u8* ptr = (u8*)malloc(memTotal);
    // Create grid struct
    ZPGGrid* grid = (ZPGGrid*)ptr;
    *grid = {};
    // init grid memory
    ptr += sizeof(ZPGGrid);
    //memset(ptr, ' ', memForGrid);
    grid->cells = (ZPGCell*)ptr;
    for (i32 i = 0; i < totalCells; ++i)
    {
        grid->cells[i] = {};
        grid->cells[i].tile.type = ZPG2_CELL_TYPE_WALL;
    }
    grid->width = width;
    grid->height = height;
    return grid;
}

static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height)
{
    ZPGGrid* stencil = ZPG_CreateGrid(width, height);
    ZPG_Grid_SetCellTypeAll(stencil, ZPG_STENCIL_TYPE_EMPTY);
    ZPG_DrawOuterBorder(stencil, NULL, ZPG_STENCIL_TYPE_FULL);
    return stencil;
}

extern "C" ZPGGrid* ZPG_TestDrunkenWalk_FromCentre(i32 seed, i32 bbStepThrough)
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
    return grid;
}

/**
 * Define some rects within a grid and random walk within
 */
extern "C" ZPGGrid* ZPG_TestDrunkenWalk_WithinSpace(i32 seed)
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

extern "C" ZPGGrid* ZPG_TestDrunkenWalk_Scattered(i32 seed)
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

extern "C" ZPGGrid* ZPG_TestCaveGen(i32 seed)
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
    return grid;
}

extern "C" ZPGGrid* ZPG_TestDrawOffsetLines()
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
    ZPGPoint* points = (ZPGPoint*)malloc(sizeof(ZPGPoint) * numPoints);
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
    free(points);
    return grid;
}

extern "C" ZPGGrid* ZPG_TestDrawLines()
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
    free(chars);
    return grid;
}

static ZPGGrid* ZPG_TestEmbed(i32 seed)
{
    ZPGGrid* grid = NULL;
    const char* str = embed_8x8_grid_pillars;
    i32 len = strlen(str);
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
    i32 len = strlen(str);
    source = ZPG_ReadGridAsci((u8*)str, len);

    grid = ZPG_CreateGrid(source->width * 2, source->height * 2);
    ZPGPoint topLeft;
    topLeft.x = 0;//source->width / 4;
    topLeft.y = 0;//source->height / 4;
    ZPG_BlitGrids(grid, source, topLeft, NULL);

    topLeft.x = grid->width - source->width;
    topLeft.x = grid->height - source->height;
    ZPG_BlitGrids(grid, source, topLeft, NULL);

    return grid;
}

extern "C" void ZPG_Init()
{
    if (g_bInitialised == YES) { return; }
    g_bInitialised = YES;
    g_directions[ZPG_DIR_RIGHT] = { 1, 0 };
    g_directions[ZPG_DIR_UP] = { 0, -1 };
    g_directions[ZPG_DIR_LEFT] = { -1, 0 };
    g_directions[ZPG_DIR_DOWN] = { 0, 1 };

    // // above
    // g_neighbours[0] = { -1, -1 };
    // g_neighbours[1] = { 0, -1 };
    // g_neighbours[2] = { 1, -1 };
    // // beside
    // g_neighbours[3] = { -1, 0 };
    // g_neighbours[4] = { 1, 0 };
    // // below
    // g_neighbours[5] = { -1, 1 };
    // g_neighbours[6] = { 0, 1 };
    // g_neighbours[7] = { 1, 1 };

    ZPG_InitCellTypes();
    ZPG_InitPrefabs();
}

extern "C" ZPG_EXPORT
void ZPG_RunPreset(i32 mode)
{
    ZPG_Init();
    i32 srandSeed;
    // Seed randomly
    srandSeed = (i32)time(NULL);
    // seed specifically
    //srandSeed = 1578760952;
    srand(srandSeed);

    i32 seed = 0;
    char titleBorder[81];
    titleBorder[80] = '\0';
    memset(titleBorder, '-', 80);
    printf("%s\n", titleBorder);
    printf("-- ZPG RUN PRESET mode %d seed %d--\n", mode, srandSeed);
    printf("%s\n", titleBorder);
    ZPGGrid* grid = NULL;
    i32 bPlaceEntities = YES;
    i32 bPrintValues = NO;
    i32 bPrintChars = YES;
    i32 bSaveGridAsci = YES;
    i32 bSaveGridPNG = NO;
    
    //////////////////////////////////////////
    // Generate geometry
    //////////////////////////////////////////
    switch (mode)
    {
        case 1: grid = ZPG_TestDrunkenWalk_FromCentre(0, NO); break;
        case 2: grid = ZPG_TestDrunkenWalk_Scattered(0); break;
        case 3: grid = ZPG_TestCaveGen(seed); break;
        case 4: grid = ZPG_TestDrawOffsetLines(); break;
        case 5: grid = ZPG_TestDrawLines(); break;
        case 6: grid = ZPG_TestDrunkenWalk_WithinSpace(seed); break;
        case 7:
            grid = ZPG_TestPerlin(seed);
            bPrintValues = NO;
            bPrintChars = NO;
            bPlaceEntities = NO;
            bSaveGridAsci = NO;
            bSaveGridPNG = YES;
            break;
        case 8:
            grid = ZPG_TestLoadAsciFile();
            bSaveGridAsci = NO;
            break;
        case 9: grid = ZPG_TestEmbed(seed); break;
        case 10: grid = ZPG_TestBlit(seed); break;
        case 11:
            grid = ZPG_Test_PrefabBuildA(seed); 
            bPlaceEntities = NO;
            break;
        case 12:
            grid = ZPG_Test_WalkBetweenPrefabs(seed);
            bPlaceEntities = YES;
            break;
        default: printf("Did not recognise test mode %d\n", mode); break;
    }
    
    //////////////////////////////////////////
    // Generate entities and save
    //////////////////////////////////////////
    if (grid != NULL)
    {
        if (bPlaceEntities)
        {
            printf("-- Grid Loaded --\ncreating entities\n");
            ZPG_Grid_CountNeighourRings(grid);
            ZPGGrid* entData = ZPG_CreateGrid(grid->width, grid->height);
            ZPG_AnalyseForEntities(grid, entData, &seed);
            ZPG_PlaceScatteredEntities(grid, &seed);
        }

        if (bPrintValues)
        {
            ZPG_Grid_PrintValues(grid, YES);
        }
        if (bPrintChars)
        {
            ZPG_Grid_PrintChars(grid, '\0', 0, 0);    
        }
        if (bSaveGridAsci)
        {
            ZPG_WriteGridAsAsci(grid, "test_grid.txt");
        }
        if (bSaveGridPNG)
        {
            ZPG_WriteGridAsPNG(grid, "test_grid.png");
        }
        free(grid);
    }
    else
    {
        printf("No grid was generated.\n");
    }
    

    //ZPG_TestDrunkenWalk(876987);
    //ZPG_TestDrunkenWalk(1993);
}

#endif // ZE_PROC_GEN_CPP