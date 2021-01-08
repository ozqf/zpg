#ifndef ZPG_BUILD_PERLIN_H
#define ZPG_BUILD_PERLIN_H

#include "zpg_internal.h"

static ZPGGrid* ZPG_Preset_Perlin(ZPGPresetCfg* presetCfg)
{
    i32 w = 96, h = 48;
    //i32 w = 1024, h = 1024;
    //i32 w = 8, h = 8;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    //ZPG_Grid_SetCellTypeAll(grid, ZPG_CELL_TYPE_WALL);
    ZPG_Grid_SetAll(grid, ZPG_CELL_TYPE_WALL);

    ZPGGrid* stencil = ZPG_CreateGrid(w, h);
    //ZPG_Grid_SetCellTypeAll(stencil, ZPG_CELL_TYPE_PATH);
    ZPG_Grid_SetAll(stencil, ZPG_CELL_TYPE_PATH);
    ZPG_DrawOuterBorder(stencil, NULL, ZPG_CELL_TYPE_WALL);
    zpg_perlin_cfg cfg = {};
    ZPG_Perlin_SetCfgPreset(&cfg, 1);
    ZPG_DrawPerlinGrid(grid, stencil, &presetCfg->seed, &cfg);
    i32 caveIterations = 0;
    for (i32 i = 0; i < caveIterations; ++i)
    {
        ZPG_IterateCaves(
            grid,
            stencil,
            ZPG_CELL_TYPE_WALL,
            ZPG_CELL_TYPE_PATH,
            ZPG_CAVE_GEN_CRITICAL_NEIGHBOURS_DEFAULT);
    }
    //ZPG_Grid_PrintValues(grid, 1, YES);
    ZPG_Grid_PerlinToGreyscale(grid, NULL);
    //ZPG_Grid_PrintTexture(grid, YES);
    if (w <= 96 && h <= 96
        && (presetCfg->flags & ZPG_API_FLAG_PRINT_GREYSCALE))
    {
        ZPG_Grid_PrintTexture(grid, NO);
    }
    return grid;
}

#endif // ZPG_BUILD_PERLIN_H