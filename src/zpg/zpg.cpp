#ifndef ZE_PROC_GEN_CPP
#define ZE_PROC_GEN_CPP

/*
Research material
https://www.gamasutra.com/blogs/HermanTulleken/20161005/282629/Algorithms_for_making_more_interesting_mazes.php
https://www.boristhebrave.com/2019/07/28/dungeon-generation-in-enter-the-gungeon/

Noise and non-uniform random numbers
https://www.redblobgames.com/articles/noise/introduction.html

*/

// Internally shared functionality
#include "zpg_internal.h"

// implementations
#include "zpg_random_table.h"
#include "zpg_cell_types.h"
#include "zpg_alloc.h"
#include "zpg_grid.h"
#include "zpg_utils.h"
#include "zpg_file.h"
#include "zpg_embed.h"
#include "zpg_print.h"

// grid painting and individual generation functions
#include "zpg_paint/zpg_draw_grid_primitives.h"
#include "zpg_paint/zpg_paint_stencil.h"
#include "zpg_paint/zpg_perlin_draw.h"
#include "zpg_paint/zpg_drunken_walk.h"
#include "zpg_paint/zpg_grid_walk_and_fill.h"
#include "zpg_paint/zpg_cave_gen.h"
#include "zpg_paint/zpg_voronoi.h"
#include "zpg_path/zpg_path.h"
#include "zpg_entities/zpg_entities.h"

// Combined generation functions
#include "zpg_presets/zpg_build_prefab.h"
#include "zpg_presets/zpg_build_perlin.h"
#include "zpg_rooms/zpg_paint_rooms.h"
#include "zpg_rooms/zpg_room_grouping.h"
#include "zpg_rooms/zpg_room_tree.h"

// styling
#include "zpg_style/zpg_style.h"

// preset building
#include "zpg_presets/zpg_presets.h"
#include "zpg_presets/zpg_params.h"

// scripted building
#include "zpg_script/zpg_command.h"
#include "zpg_script/zpg_script.h"

static void ZPG_AddPresetFunction(
	zpg_preset_fn funcPtr,
	char* selector,
	char* description)
{
    if (g_nextPreset == ZPG_MAX_PRESETS)
    {
        printf("ERROR - no preset slot left for \"%s\"\n", selector);
        return;
    }
	ZPGPreset* p = &g_presets[g_nextPreset++];
	p->selector = selector;
	p->description = description;
	p->funcPtr = funcPtr;
    // g_presets[g_nextPreset] = funcPtr;
    // g_presetLabels[g_nextPreset] = label;
    // g_nextPreset++;
}

ZPG_EXPORT void ZPG_Fatal(const char* msg)
{
    g_ptrFatal(msg);
}

extern "C" ZPG_EXPORT i32 ZPG_Init(zpg_allocate_fn ptrAlloc, zpg_free_fn ptrFree, zpg_fatal_fn ptrFatal)
{
    if (g_bZPGInitialised == YES) { return 0; }
    
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
    g_bZPGInitialised = YES;

    g_ptrAlloc = ptrAlloc;
    g_ptrFree = ptrFree;
    //printf("Allocate/Free set at 0X%X and 0X%X\n", (u64)g_ptrAlloc, (u64)g_ptrFree);

    g_directions[ZPG_DIR_RIGHT] = { 1, 0 };
    g_directions[ZPG_DIR_UP] = { 0, -1 };
    g_directions[ZPG_DIR_LEFT] = { -1, 0 };
    g_directions[ZPG_DIR_DOWN] = { 0, 1 };
    
    ZPG_InitCellTypes();
    ZPG_InitPrefabs();
    ZPG_InitParams();

    //////////////////////////////////////////////////
    // Build preset list
    // 0
    ZPG_AddPresetFunction(ZPG_TestDrunkenWalk_FromCentre, "walk_from_centre", "Drunken Walk From Centre");
    ZPG_AddPresetFunction(ZPG_TestDrunkenWalk_Scattered, "scattered_walks", "Scattered drunken walks");
    ZPG_AddPresetFunction(ZPG_TestCaveGen, "test_caves", "Test Cellular Caves");
    ZPG_AddPresetFunction(ZPG_TestDrawOffsetLines, "test_offset_lines", "Test Draw Offset Lines");
    ZPG_AddPresetFunction(ZPG_TestDrawLines, "test_lines", "Test Draw Lines");
    // 5
    ZPG_AddPresetFunction(ZPG_TestDrunkenWalk_WithinSpace, "test_walk_in_space", "Test Drunken walk within space");
    ZPG_AddPresetFunction(ZPG_Preset_Perlin, "test_perlin_noise", "Test Perlin noise");
    ZPG_AddPresetFunction(ZPG_TestLoadAsciFile, "test_asci_load", "Test asci file load");
    ZPG_AddPresetFunction(ZPG_TestEmbed, "test_embed", "Test Embed");
    ZPG_AddPresetFunction(ZPG_TestBlit, "test_blit", "Test blit");
    // 10
    ZPG_AddPresetFunction(ZPG_Test_PrefabBuildA, "prefab_build_a", "Prefab Build A");
    ZPG_AddPresetFunction(ZPG_Test_WalkBetweenPrefabs, "offset_between_2_prefabs", "Offset path between two prefabs");
    ZPG_AddPresetFunction(ZPG_Preset_PrefabsLinesCaves, "offset_around_4_prefabs", "Offset path around four prefabs");
    ZPG_AddPresetFunction(ZPG_Preset_RoomTreeTest, "test_room_generate", "Test room tree generate");
    ZPG_AddPresetFunction(ZPG_Preset_TestConnectRooms, "test_room_connections", "Test room volume connections");
    // 15
    ZPG_AddPresetFunction(ZPG_TestCaveLayers, "test_cave_layering", "Test Cave Layering");
    ZPG_AddPresetFunction(ZPG_Preset_TestRoomSeeding, "test_room_seeding", "Test Room Seeding");

    //printf("Init complete - %d allocs\n", ZPG_GetNumAllocs());
    return 0;
}

static ZPGOutput ZPG_OutputFromAsciiGrid(ZPGGrid* grid)
{
    ZPGOutput output = {};
    output.typeFlags |= ZPG_OUTPUT_TYPE_ASCI_GRID;
    output.asciiGrid = grid;
    return output;
}

static ZPGOutput ZPG_OutputFromGreyscaleGrid(ZPGGrid* grid)
{
    ZPGOutput output = {};
    output.typeFlags |= ZPG_OUTPUT_TYPE_GREYSCALE;
    output.greyGrid = grid;
    return output;
}

static ZPGPreset* ZPG_GetPreset(const char* selector)
{
	for (i32 i = 0; i < g_nextPreset; ++i)
	{
		ZPGPreset* p = &g_presets[i];
		if (ZPG_STRCMP(selector, p->selector) == 0)
		{
			return p;
		}
	}
	return NULL;
}

static void ZPG_PrintPresetsList()
{
    printf("--- PRESET MODES ---\n");
    for (i32 i = 0; i < g_nextPreset; ++i)
    {
        printf("%d: %s: \"%s\"\n", i, g_presets[i].selector, g_presets[i].description);
    }
}

static void ZPG_PrintPresetHelp(char* exeName)
{
    printf("------------------------\n");
    printf("--- Preset Mode Help ---\n");
    printf("------------------------\n");
	printf("Run a preset generator function with given settings.\n");
	printf("preset <preset_mode> <options>\n");
    printf("eg: %s preset walk_from_centre -p -i output.txt\n\n", exeName);
    ZPG_PrintPresetsList();
    ZPG_Params_PrintHelp();
}

static void ZPG_PrintArgvs(i32 argc, char** argv)
{
	printf("Options: ");
	for (i32 i = 0; i < argc; ++i)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");
}

static i32 ZPG_GenerateSeed()
{
    i32 seed = (i32)time(NULL);
    seed ^= (u64)(&seed);
    return seed;
}

ZPG_EXPORT
void ZPG_RunPresetCLI(
    i32 argc, char** argv,
    u8** resultPtr, i32* resultWidth, i32* resultHeight)
{
    printf("argc %d\n", argc);
    if (argc <= 2)
    {
        printf("No preset settings received\n");
        ZPG_PrintPresetHelp(argv[0]);
        return;
    }
	ZPG_PrintArgvs(argc, argv);
    ZPGPresetCfg cfg = {};
    // Seed randomly - param may override
    cfg.seed = (i32)time(NULL);
    cfg.seed ^= (u64)&cfg;
    // read params
    ZPGError err = ZPG_Params_ReadForPreset(&cfg, argc, argv);
	if (err != 0)
	{
		printf("ABORT with error code %d\n", err);
        return;
	}
	ZPGPreset* preset = ZPG_GetPreset(cfg.preset);
	if (preset == NULL)
	{
		printf("ABORT no preset \"%s\" found\n", cfg.preset);
		return;
	}
    // if (cfg.preset < 0 || cfg.preset >= g_nextPreset)
    // {
    //     printf("ABORT Unrecognised preset type %d\n", cfg.preset);
    //     return;
    // }
    if (g_bZPGInitialised == false) { return; }
    
    /////////////////////////////////////////////
    // Run
	
    printf("=== Preset %s ===\n", preset->description);
    printf("Seed: %d\n", cfg.seed);
	i32 originalSeed = cfg.seed;
    srand(cfg.seed);
    ZPGOutput output = preset->funcPtr(&cfg);
    ZPGGrid* grid = output.asciiGrid;
    if (grid == NULL)
    {
        printf("ERROR - no grid was returned...\n");
        return;
    }

    ZPG_GenerateEntites(&cfg, grid);

    if (cfg.flags & ZPG_API_FLAG_PRINT_RESULT)
    {
        ZPG_Grid_PrintCellDefChars(grid, '\0', 0, 0);
    }
    if (cfg.asciOutput != NULL)
    {
        ZPG_WriteGridAsAsci(grid, cfg.asciOutput);
    }
    if (cfg.imageOutput != NULL)
    {
        ZPG_WriteGridAsPNG(grid, cfg.imageOutput, NO);
    }
    if (cfg.pictureOutput != NULL)
    {
        ZPG_WriteGridAsPNG(grid, cfg.pictureOutput, YES);
    }

    if (resultPtr != NULL 
        && resultWidth != NULL
        && resultHeight != NULL)
    {
        *resultPtr = (u8*)grid->cells;
        *resultWidth = grid->width;
        *resultHeight = grid->height;
    }

    if (cfg.flags & ZPG_API_FLAG_PRINT_FINAL_ALLOCS)
    {
        ZPG_PrintAllocations();
    }
    printf("Free all\n");
    ZPG_FreeAll();
    if (cfg.flags & ZPG_API_FLAG_PRINT_FINAL_ALLOCS)
    {
        ZPG_PrintAllocations();
    }
	
    printf("Initial seed was %d\n", originalSeed);
	printf("Cmdline: ");
	i32 i = 0;
	while (++i < argc)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");

    // TODO: Assumes caller will free grid memory
    // (and that they passed in malloc/free functions)
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