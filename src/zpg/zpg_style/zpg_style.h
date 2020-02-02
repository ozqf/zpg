#ifndef ZPG_STYLE_H
#define ZPG_STYLE_H

#include "../../zpg.h"
#include "../zpg_internal.h"

#define ZPG_STYLE_CELL_TOP_LEFT (1 << 0)
#define ZPG_STYLE_CELL_TOP_MIDDLE (1 << 1)
#define ZPG_STYLE_CELL_TOP_RIGHT (1 << 2)

#define ZPG_STYLE_CELL_LEFT (1 << 3)
#define ZPG_STYLE_CELL_RIGHT (1 << 4)

#define ZPG_STYLE_CELL_BOTTOM_LEFT (1 << 5)
#define ZPG_STYLE_CELL_BOTTOM_MIDDLE (1 << 6)
#define ZPG_STYLE_CELL_BOTTOM_RIGHT (1 << 7)

static void ZPG_Style_IfMatchAppendFlag(
    ZPGGrid* grid, i32 x, i32 y, u8 queryType, u8* flags, u8 flag)
{
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { *flags |= flag; return; }
    if (cell->tile.type == queryType)
    { *flags |= flag; return; }
}

/**
 * Create a grid of styling data for the given
 * source grid
 */
static ZPGGrid* ZPG_CreateStylingGrid(ZPGGrid* source)
{
    ZPG_PARAM_NULL(source, NULL)

    ZPGGrid* grid = ZPG_CreateGrid(
        source->width, source->height);
    
    ZPG_BEGIN_GRID_ITERATE(grid)

    ZPGCell* cell = ZPG_Grid_GetCellAt(source, x, y);
    // Calculate tilemap indices
    u8 t = cell->tile.type;
    u8 flags = 0;
    ZPG_Style_IfMatchAppendFlag(source, x-1, y-1, t, &flags, ZPG_STYLE_CELL_TOP_LEFT);
    ZPG_Style_IfMatchAppendFlag(source, x, y-1, t, &flags, ZPG_STYLE_CELL_TOP_MIDDLE);
    ZPG_Style_IfMatchAppendFlag(source, x+1, y-1, t, &flags, ZPG_STYLE_CELL_TOP_RIGHT);
    
    ZPG_Style_IfMatchAppendFlag(source, x-1, y, t, &flags, ZPG_STYLE_CELL_LEFT);
    ZPG_Style_IfMatchAppendFlag(source, x+1, y, t, &flags, ZPG_STYLE_CELL_RIGHT);

    ZPG_Style_IfMatchAppendFlag(source, x-1, y+1, t, &flags, ZPG_STYLE_CELL_BOTTOM_LEFT);
    ZPG_Style_IfMatchAppendFlag(source, x, y+1, t, &flags, ZPG_STYLE_CELL_BOTTOM_MIDDLE);
    ZPG_Style_IfMatchAppendFlag(source, x+1, y+1, t, &flags, ZPG_STYLE_CELL_BOTTOM_RIGHT);

    ZPGCell* paintCell = ZPG_Grid_GetCellAt(grid, x, y);
    paintCell->style.neighbourFlags = flags; 

    ZPG_END_GRID_ITERATE

    ZPG_Grid_PrintValues(grid, YES);
    return grid;
}

#endif // ZPG_STYLE_H