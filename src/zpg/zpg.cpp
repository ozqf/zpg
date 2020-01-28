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
#include "zpg_params.h"

extern "C" ZPG_EXPORT void ZPG_SetMemoryFunctions(
    zpg_allocate_fn ptrAlloc, zpg_free_fn ptrFree)
{
    
}

static void ZPG_AddPresetFunction(zpg_preset_fn funcPtr, char* label)
{
    if (g_nextPreset == ZPG_MAX_PRESETS)
    {
        printf("ERROR - no preset slot left for \"%s\"\n", label);
        return;
    }
    g_presets[g_nextPreset] = funcPtr;
    g_presetLabels[g_nextPreset] = label;
    g_nextPreset++;
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

    //////////////////////////////////////////////////
    // Build preset list
    ZPG_AddPresetFunction(ZPG_TestDrunkenWalk_FromCentre, "Drunken Walk From Centre");
    ZPG_AddPresetFunction(ZPG_TestDrunkenWalk_Scattered, "Scattered drunken walks");
    ZPG_AddPresetFunction(ZPG_TestCaveGen, "Test Cellular Caves");
    ZPG_AddPresetFunction(ZPG_TestDrawOffsetLines, "Test Draw Offset Lines");
    ZPG_AddPresetFunction(ZPG_TestDrawLines, "Test Draw Lines");
    ZPG_AddPresetFunction(ZPG_TestDrunkenWalk_WithinSpace, "Test Drunken walk within space");
    ZPG_AddPresetFunction(ZPG_TestPerlin, "Test Perlin noise");
    ZPG_AddPresetFunction(ZPG_TestLoadAsciFile, "Test asci file load");
    ZPG_AddPresetFunction(ZPG_TestEmbed, "Test Embed");
    ZPG_AddPresetFunction(ZPG_TestBlit, "Test blit");
    ZPG_AddPresetFunction(ZPG_Test_PrefabBuildA, "Prefab Build A");
    ZPG_AddPresetFunction(ZPG_Test_WalkBetweenPrefabs, "Offset path between two prefabs");
    ZPG_AddPresetFunction(ZPG_Preset_PrefabsLinesCaves, "Offset path around four prefabs");

    printf("Init complete - %d allocs\n", ZPG_GetNumAllocs());
    return 0;
}

static void ZPG_PrintPresetsList()
{
    printf("--- PRESET MODES ---\n");
    for (i32 i = 0; i < g_nextPreset; ++i)
    {
        printf("%d: \"%s\"\n", i, g_presetLabels[i]);
    }
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

    ZPGPresetCfg cfg = {};
    cfg.seed = seed;
    
    //////////////////////////////////////////
    // Generate geometry
    //////////////////////////////////////////
    switch (mode)
    {
        case 1: grid = ZPG_TestDrunkenWalk_FromCentre(&cfg); break;
        case 2: grid = ZPG_TestDrunkenWalk_Scattered(&cfg); break;
        case 3: grid = ZPG_TestCaveGen(&cfg); break;
        case 4: grid = ZPG_TestDrawOffsetLines(&cfg); break;
        case 5: grid = ZPG_TestDrawLines(&cfg); break;
        case 6: grid = ZPG_TestDrunkenWalk_WithinSpace(&cfg); break;
        case 7:
            grid = ZPG_TestPerlin(&cfg);
            bPrintValues = NO;
            bPrintChars = NO;
            bPlaceEntities = NO;
            bSaveGridAsci = NO;
            bSaveGridPNG = YES;
            break;
        case 8:
            grid = ZPG_TestLoadAsciFile(&cfg);
            bSaveGridAsci = NO;
            break;
        case 9: grid = ZPG_TestEmbed(&cfg); break;
        case 10: grid = ZPG_TestBlit(&cfg); break;
        case 11:
            grid = ZPG_Test_PrefabBuildA(&cfg); 
            bPlaceEntities = NO;
            break;
        case 12:
            grid = ZPG_Test_WalkBetweenPrefabs(&cfg);
            bPlaceEntities = YES;
            break;
        case 13:
            grid = ZPG_Preset_PrefabsLinesCaves(&cfg);
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
    printf("------------------------\n");
    printf("--- Preset Mode Help ---\n");
    printf("------------------------\n");
	printf("Run a preset generator function with given settings.\n");
	printf("preset <preset_mode> <output_file_name>\n");
    printf("eg: %s preset 12 output.txt\n\n", exeName);
    ZPG_PrintPresetsList();
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
    ZPG_Params_ReadForPreset(argc, argv);
    #if 0
    printf("Run present params:\n");
    for (i32 i = 2; i < argc; ++i)
    {
        printf("%d: %s\n", i, argv[i]);
    }

    if (ZPG_STRCMP(argv[2], "list") == 0)
    {
        ZPG_PrintPresetsList();
        return;
    }

    i32 preset = atoi(argv[2]);
    if (preset < 0 || preset >= g_nextPreset)
    {
        printf("ABORT Unrecognised preset type %d\n", preset);
        return;
    }
    printf("Running preset %d: \"%s\"\n", preset, g_presetLabels[preset]);
    ZPGPresetCfg cfg = {};
    ZPGGrid* grid = g_presets[preset](&cfg);
    if (grid == NULL)
    {
        printf("ERROR - no grid was returned...\n");
        return;
    }
    ZPG_Grid_PrintChars(grid, '\0', 0, 0);
    #endif
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