#ifndef ZPG_ROOM_TREE_H
#define ZPG_ROOM_TREE_H

#include "../zpg_internal.h"

static void ZPG_SetRandomGridValues(ZPGGrid* grid, u8 min, u8 max, i32* seed)
{
    ZPG_BEGIN_GRID_ITERATE(grid)
        u8 val = ZPG_RandU8InRange(*seed++, min, max);
        ZPG_Grid_SetCellTypeAt(grid, x, y, val, NULL);
    ZPG_END_GRID_ITERATE
}

static void ZPG_ZeroOutLoneValues(ZPGGrid* grid)
{
    ZPG_BEGIN_GRID_ITERATE(grid)
        i32 neighbours = 0;
        ZPGCell* current = ZPG_Grid_GetCellAt(grid, x, y);
        u8 val = current->tile.type;
        ZPGCell* cell = NULL;
        // left
        cell = ZPG_Grid_GetCellAt(grid, x - 1, y);
        if (cell != NULL && cell->tile.type == val) { neighbours++; }
        // right
        cell = ZPG_Grid_GetCellAt(grid, x + 1, y);
        if (cell != NULL && cell->tile.type == val) { neighbours++; }
        // above
        cell = ZPG_Grid_GetCellAt(grid, x, y - 1);
        if (cell != NULL && cell->tile.type == val) { neighbours++; }
        // below
        cell = ZPG_Grid_GetCellAt(grid, x, y + 1);
        if (cell != NULL && cell->tile.type == val) { neighbours++; }

        if (neighbours == 0)
        {
            current->tile.type = 0;
        }
    ZPG_END_GRID_ITERATE
}

static ZPGGrid* ZPG_Preset_RoomTreeTest(ZPGPresetCfg* cfg)
{
    ZPGGrid* grid = ZPG_CreateGrid(32, 4);
    printf("Assign random values\n");
    ZPG_SetRandomGridValues(grid, 2, 9, &cfg->seed);
    ZPG_Grid_PrintValues(grid, YES);
    printf("Group random values\n");
    /*
    > Clear any cell that is isolated (no 4 connected neighbours)
    > While any empty cell can be found
        > Flood fill that cell with 1
    */
    ZPG_ZeroOutLoneValues(grid);
    ZPG_Grid_PrintValues(grid, YES);
    printf("Flood fill space\n");
    i32 x, y;
    ZPGCell* cell = ZPG_Grid_FindFirstCellWithType(grid, 0, &x, &y);
    while (cell != NULL)
    {
        ZPG_Grid_FloodFill(grid, x, y, 1);
        cell = ZPG_Grid_FindFirstCellWithType(grid, 0, &x, &y);
    }
    ZPG_Grid_PrintValues(grid, YES);
    return grid;
}

#endif // ZPG_ROOM_TREE_H