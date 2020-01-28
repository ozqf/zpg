#ifndef ZE_PROC_GEN_CPP
#define ZE_PROC_GEN_CPP

#include "zpg_internal.h"

/*
Research material
https://www.boristhebrave.com/2019/07/28/dungeon-generation-in-enter-the-gungeon/

*/

#include "string.h"
#include "time.h"
#include "zpg_random_table.h"
#include "zpg_cell_types.h"
#include "zpg_alloc.h"
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

#include "zpg_command.h"
#include "zpg_script.h"
#include "zpg_presets.h"

static ZPGGrid* ZPG_CreateGrid(i32 width, i32 height)
{
    i32 totalCells = width * height;
    i32 memForGrid = (sizeof(ZPGCell) * totalCells);
    i32 memTotal = sizeof(ZPGGrid) + memForGrid;
    //printf("Make grid %d by %d (%d cells, %d bytes)\n",
    //    width, height, (width * height), memTotal);
    u8* ptr = (u8*)ZPG_Alloc(memTotal);
    // Create grid struct at END of cells array
    ZPGGrid* grid = (ZPGGrid*)(ptr + memForGrid);
    *grid = {};
    // init grid memory
    //ptr += sizeof(ZPGGrid);
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

static void ZPG_FreeGrid(ZPGGrid* grid)
{
    ZPG_Free(grid->cells);
}

static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height)
{
    ZPGGrid* stencil = ZPG_CreateGrid(width, height);
    ZPG_Grid_SetCellTypeAll(stencil, ZPG_STENCIL_TYPE_EMPTY);
    ZPG_DrawOuterBorder(stencil, NULL, ZPG_STENCIL_TYPE_FULL);
    return stencil;
}

extern "C" ZPG_EXPORT void ZPG_SetMemoryFunctions(
    zpg_allocate_fn ptrAlloc, zpg_free_fn ptrFree)
{
    
}

extern "C" ZPG_EXPORT i32 ZPG_Init(zpg_allocate_fn ptrAlloc, zpg_free_fn ptrFree)
{
    if (g_bInitialised == YES) { return 0; }
    
    if (ptrAlloc == NULL || ptrFree == NULL)
    {
        #if 0
        printf("ZPG INIT FAILED - null allocator functions\n");
        return 1;
        #endif
        #if 1
        ptrAlloc = malloc;
        ptrFree = free;
        #endif
    }
    g_bInitialised = YES;

    g_ptrAlloc = ptrAlloc;
    g_ptrFree = ptrFree;

    g_directions[ZPG_DIR_RIGHT] = { 1, 0 };
    g_directions[ZPG_DIR_UP] = { 0, -1 };
    g_directions[ZPG_DIR_LEFT] = { -1, 0 };
    g_directions[ZPG_DIR_DOWN] = { 0, 1 };
    
    ZPG_InitCellTypes();
    ZPG_InitPrefabs();
    printf("Init complete - %d allocs\n", ZPG_GetNumAllocs());
    return 0;
}

ZPG_EXPORT
void ZPG_RunPreset(
    i32 mode, char* outputPath, i32 apiFlags,
    u8** resultPtr, i32* resultWidth, i32* resultHeight)
{

    if (g_bInitialised == false) { return; }
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
        case 13:
            grid = ZPG_Preset_PrefabsLinesCaves(seed);
            bPlaceEntities = YES;
            break;
        default: printf("Did not recognise test mode %d\n", mode); break;
    }

    if (outputPath == NULL
        || *outputPath == '\0'
        || ZPG_STRCMP(outputPath, "none") == 0)
    {
        bSaveGridAsci = NO;
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
            ZPG_WriteGridAsAsci(grid, outputPath);
        }
        if (bSaveGridPNG)
        {
            ZPG_WriteGridAsPNG(grid, "test_grid.png");
        }
    }
    else
    {
        printf("No grid was generated.\n");
    }

    if (resultPtr != NULL && resultWidth != NULL && resultHeight != NULL)
    {
        *resultPtr = (u8*)grid->cells;
        *resultWidth = grid->width;
        *resultHeight = grid->height;
    }
    else if (grid != NULL)
    {
        ZPG_FreeGrid(grid);
    }
    

    //ZPG_PrintAllocations();
    //ZPG_TestDrunkenWalk(876987);
    //ZPG_TestDrunkenWalk(1993);
}

static void ZPG_PrintPresetHelp(char* exeName)
{
    printf("--- Preset Mode Help ---\n");
	printf("\nRun a generator preset:\n");
	printf("preset <preset_mode> <output_file_name>\n");
	printf("\tPreset modes are currently 1-12\n");
    printf("eg:\n");
    printf("\t%s preset 12 output.txt\n", exeName);
}

ZPG_EXPORT
void ZPG_RunPresetCLI(
    i32 argc, char** argv,
    u8** resultPtr, i32* resultWidth, i32* resultHeight)
{
    if (argc <= 2)
    {
        printf("No preset settings received\n");
        ZPG_PrintPresetHelp(argv[0]);
        return;
    }
    printf("Run present params:\n");
    for (i32 i = 2; i < argc; ++i)
    {
        printf("%d: %s\n", i, argv[i]);
    }
    i32 preset = atoi(argv[2]);

}

ZPG_EXPORT i32 ZPG_Shutdown()
{
    return 0;
}

ZPG_EXPORT i32 ZPG_ApiVersion()
{
    return 1;
}

#endif // ZE_PROC_GEN_CPP