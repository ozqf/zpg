#ifndef ZPG_CAVE_GEN_H
#define ZPG_CAVE_GEN_H

#include "zpg_internal.h"

// 0.45 very little paint chance. 0.55 much higher.
//#define ZPG_CAVE_GEN_DEFAULT_CELL_SEED_CHANCE 0.45f
//#define ZPG_CAVE_GEN_DEFAULT_CELL_SEED_CHANCE 0.55f
//#define ZPG_CAVE_GEN_CRITICAL_NEIGHBOUR_COUNT 4

/**
 * Randomly paint tiles for growing caves
 * Any Stencil cell whose value is not 0 will be skipped
 */
static void ZPG_SeedCaves(
    ZPGGrid* grid,
    ZPGGrid* stencil,
    u8 paintType,
    f32 seedChance,
    i32* seed)
{
    //const f32 seedChance = ZPG_CAVE_GEN_DEFAULT_CELL_SEED_CHANCE;
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            if (ZPG_Grid_CheckStencilOccupied(stencil, x, y) == YES)
            { continue; }
            f32 rand = ZPG_Randf32(*seed);
            *seed += 1;
            if (rand < seedChance)
            {
                ZPG_GRID_SET(grid, x, y, paintType);
                // ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
                // cell->tile.type = paintType;
            }
        }
    }
}

/**
 * Add/Remove tiles based on their neighbour count
 * Any Stencil cell whose value is not 0 will be skipped
 */
static void ZPG_IterateCaves(
    ZPGGrid* grid,
    ZPGGrid* stencil,
    u8 solidType,
    u8 emptyType,
    i32 criticalNeighbours)
{
    ZPG_BEGIN_GRID_ITERATE(grid)
        if (ZPG_Grid_CheckStencilOccupied(stencil, x, y) == YES)
        { continue; }
        //ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
        u8* val = ZPG_GRID_GET_ADDR(grid, x, y);
        i32 neighbours = ZPG_Grid_CountNeighboursAt(grid, x, y).count;
        if (neighbours < criticalNeighbours)
        {
            // if (cell->tile.type == solidType)
            // {
            //     cell->tile.type = emptyType;
            // }
            // else if (cell->tile.type == emptyType)
            // {
            //     cell->tile.type = solidType;
            // }
            if (*val == solidType)
            {
                *val = emptyType;
            }
            else if (*val == emptyType)
            {
                *val = solidType;
            }
        }
    ZPG_END_GRID_ITERATE
}

#endif // ZPG_CAVE_GEN_H