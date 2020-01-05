#ifndef ZPG_GRID_H
#define ZPG_GRID_H

#include "../zpg.h"
static i32 ZPG_PositionToIndex(ZPGGrid* grid, i32 x, i32 y)
{
    if (x < 0 || x >= grid->width)
    {
        return -1;
    }
    if (y < 0 || y >= grid->height)
    {
        return -1;
    }
    return x + (y * grid->width);
}

static ZPGCell* ZPG_GetCellAt(ZPGGrid* grid, i32 x, i32 y)
{
    i32 i = ZPG_PositionToIndex(grid, x, y);
    if (i == -1)
    {
        return NULL;
    }
    return &grid->cells[i];
}

static ZPGCellTypeDef* ZPG_GetCellTypeAt(ZPGGrid* grid, i32 x, i32 y)
{
    ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL)
    {
        return ZPG_GetDefaultType();
    }
    return ZPG_GetType(cell->tile.type);
}

/**
 * returns NO if type at given cell is 0 (or no stencil was supplied),
 * YES otherwise
 */
static i32 ZPG_CheckStencilOccupied(ZPGGrid* grid, i32 x, i32 y)
{
    if (grid == NULL) { return NO; }
    ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL) { return NO; }
    return (cell->tile.type != ZPG_STENCIL_TYPE_EMPTY);
}

static i32 ZPG_GetTagAt(ZPGGrid* grid, i32 x, i32 y)
{
    if (grid == NULL) { return NO; }
    ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL) { return NO; }
    return cell->tile.tag;
}

static void ZPG_SetCellTypeAt(ZPGGrid* grid, i32 x, i32 y, u8 type, ZPGGrid* stencil)
{
    if (ZPG_CheckStencilOccupied(stencil, x, y) == YES) { return; }
    ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL) { return; }
    cell->tile.type = type;
}

/**
 * Paint the given type on this cell only if the current value
 * has a different geometry type
 * returns true if it performed a change
 */
static i32 ZPG_SetCellTypeGeometry(
    ZPGGrid* grid, i32 x, i32 y, u8 typeToPaint, u8 geometryType)
{
    ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL) { return NO; }
    ZPGCellTypeDef* def = ZPG_GetType(cell->tile.type);
    if (def->geometryType == geometryType) { return NO; }
    cell->tile.type = typeToPaint;
    return YES;
}

static void ZPG_SetCellTypeAll(ZPGGrid* grid, u8 type)
{
    i32 totalCells = grid->width * grid->height;
    for (i32 i = 0; i < totalCells; ++i)
    {
        grid->cells[i].tile.type = type;
    }
}

static void ZPG_Grid_ClearAllTags(ZPGGrid* grid)
{
    i32 len = grid->width * grid->height;
    for (i32 i = 0; i < len; ++i)
    {
        grid->cells[i].tile.tag = 0;
    }
}

static void ZPG_SetCellChannelAll(ZPGGrid* grid, u8 type, i32 channel)
{
    if (channel < 0 || channel >= 4) { return; }
    i32 totalCells = grid->width * grid->height;
    for (i32 i = 0; i < totalCells; ++i)
    {
        grid->cells[i].arr[channel] = type;
    }
}

static ZPGGrid* ZPG_Grid_CreateClone(ZPGGrid* original)
{
    ZPGGrid* clone = ZPG_CreateGrid(original->width, original->height);
    i32 totalCells = original->width * original->height;
    memcpy(clone->cells, original->cells, sizeof(ZPGCell) * totalCells);
}

/*
static void ZPG_SetCellTagAt(ZPGGrid* grid, i32 x, i32 y, u8 tag)
{
    ZPGCell *cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL)
    {
        return;
    }
    if (cell->tile.tag != ZPG_CELL_TAG_NONE)
    {
        //printf("WARN: Cannot tag %d/%d as %d, already %d!\n",
        //    x, y, tag, cell->tile.tag);
        return;
    }
    cell->tile.tag = tag;
    //printf("Tagging cell %d/%d as %d\n", x, y, tag);
}
*/
static void ZPG_CalcStats(ZPGGrid* grid)
{
    grid->stats.numFloorTiles = 0;
    grid->stats.numObjectiveTags = 0;
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_GetCellTypeAt(grid, x, y);
            if (def->geometryType == ZPG_GEOMETRY_TYPE_PATH)
            {
                grid->stats.numFloorTiles++;
            }
            if (def->category == ZPG_CELL_CATEGORY_OBJECTIVE)
            {
                grid->stats.numObjectiveTags++;
            }
            #if 0
            ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
            if (cell->tile.type != ZPG_CELL_TYPE_FLOOR) { continue; }
            grid->stats.numFloorTiles++;
            if (cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_START)
            {
                printf("Found start tag at %d/%d\n", x, y);
                grid->stats.numObjectiveTags++;
            }
            if (cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_END)
            {
                printf("Found end tag at %d/%d\n", x, y);
                grid->stats.numObjectiveTags++;
            }
            #endif
            #if 0
            ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
            if (cell->tile.type == ZPG_CELL_TYPE_FLOOR)
            {
                stats.numFloorTiles++;
            }
            if (cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_START
                || cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_END)
            {
                stats.numObjectiveTags++;
            }
            #endif
        }
    }
}

static i32 ZPG_CountNeighboursAt(ZPGGrid* grid, i32 x, i32 y)
{
    ZPGCell *cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL)
    {
        return 0;
    }
    u8 matchType = cell->tile.type;
    i32 neighbours = 0;
    cell = ZPG_GetCellAt(grid, x - 1, y - 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_GetCellAt(grid, x, y - 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_GetCellAt(grid, x + 1, y - 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }

    cell = ZPG_GetCellAt(grid, x - 1, y);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_GetCellAt(grid, x + 1, y);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }

    cell = ZPG_GetCellAt(grid, x - 1, y + 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_GetCellAt(grid, x, y + 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_GetCellAt(grid, x + 1, y + 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    return neighbours;
}


/*
eg:
0   1     2
### ##### #######
#p# #...# #.....#
### #.p.# #.....#
    #...# #..p..#
    ##### #.....#
          #.....#
          #######
*/
static u8 ZPG_CountNeighourRingsAt(ZPGGrid* grid, i32 x, i32 y)
{
    ZPGCell *cell = ZPG_GetCellAt(grid, x, y);
    if (cell == NULL)
    {
        printf("Cannot Plot ringss at %d/%d - Cell is null\n",
               x, y);
        return 0;
    }
    u8 result = 0;
    u8 ringTest = 1;
    i32 plotX, plotY;
    i32 bScanning = YES;
    // Count until a test fails
    for (;;)
    {
        for (i32 iY = -ringTest; iY <= ringTest; ++iY)
        {
            for (i32 iX = -ringTest; iX <= ringTest; ++iX)
            {
                plotX = x + iX;
                plotY = y + iY;
                //printf("  Test %d/%d\n", plotX, plotY);
                ZPGCell *queryCell = ZPG_GetCellAt(grid, plotX, plotY);
                if (queryCell == NULL)
                {
                    bScanning = NO;
                    break;
                }
                if (queryCell == cell)
                {
                    continue;
                }
                if (queryCell->tile.type != cell->tile.type)
                {
                    //printf("  Char mismatch (%c vs %c)\n",
                    //    cell->c, queryCell->c);
                    bScanning = NO;
                    break;
                }
            }
            if (bScanning == NO)
            {
                break;
            }
        }
        if (bScanning == NO)
        {
            break;
        }
        else
        {
            result++;
            ringTest++;
        }
    }
    return result;
}

static void ZPG_CountNeighourRings(ZPGGrid* grid)
{
    printf("Calc rings\n");
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCell *cell = ZPG_GetCellAt(grid, x, y);
            cell->tile.rings = ZPG_CountNeighourRingsAt(grid, x, y);
        }
    }
}

static void ZPG_Grid_PrintValues(ZPGGrid* grid)
{
    if (grid == NULL) { return; }
    printf("------ Grid %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCell *cell = ZPG_GetCellAt(grid, x, y);
            printf("%d", cell->tile.type);
        }
        printf("|\n");
    }
    printf("------------------\n");
}

static i32 ZPG_Grid_IsPositionSafe(ZPGGrid* grid, i32 x, i32 y)
{
    if (x < 0 || x >= grid->width) { return false; }
    if (y < 0 || y >= grid->height) { return false; }
    return true;
}

/*
static char ZPG_CellToChar(ZPGCell* cell)
{
    char c = ' ';
    switch (cell->tile.type)
    {
        case ZPG_CELL_TYPE_WALL:
            c = (u8)ZPG_CHAR_CODE_SOLID_BLOCK;
            break;
        case ZPG_CELL_TYPE_FLOOR:
            c = ' ';
            switch (cell->tile.entType)
            {
                case ZPG_ENTITY_TYPE_NONE:
                    c = ' ';
                    break;
                case ZPG_ENTITY_TYPE_ENEMY:
                    c = 'x';
                    break;
                case ZPG_ENTITY_TYPE_START:
                    c = 's';
                    break;
                case ZPG_ENTITY_TYPE_END:
                    c = 'e';
                    break;
                case ZPG_ENTITY_TYPE_OBJECTIVE:
                    c = 'k';
                    break;
                default:
                    // unknown non-zero
                    c = '?';
                    break;
            }
            break;
    case ZPG_CELL_TYPE_WATER:
        c = '.';
        break;
    default:
        c = '?';
        break;
    }
    return c;
}
*/
static void ZPG_PrintChars(ZPGGrid* grid)
{
    printf("------ Grid %d/%d (%d total tiles, %d path tiles, %d objectives)------\n",
        grid->width,
        grid->height,
        grid->width * grid->height,
        grid->stats.numFloorTiles,
        grid->stats.numObjectiveTags);
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_GetCellTypeAt(grid, x, y);
            printf("%c", def->asciChar);
            //ZPGCell *cell = ZPG_GetCellAt(grid, x, y);
            //char c = ZPG_CellToChar(cell);
            //printf("%c%d", cell->c, cell->rings);
            //printf("%c", c);
        }
        printf("|\n");
    }
    printf("------------------\n");
}

#endif // ZPG_GRID_H