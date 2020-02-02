/**
 * ZPG Grid manipulation functions
 */
#ifndef ZPG_GRID_H
#define ZPG_GRID_H

#include "zpg_internal.h"

static i32 ZPG_ArePointsEqual(ZPGPoint a, ZPGPoint b)
{
    if (a.x == b.x && a.y == b.y) { return YES; }
    return NO;
}

static i32 ZPG_Grid_PositionToIndex(ZPGGrid* grid, i32 x, i32 y)
{
    if (x < 0 || x >= grid->width) { return -1; }
    if (y < 0 || y >= grid->height) { return -1; }
    return x + (y * grid->width);
}

static i32 ZPG_Grid_IsPositionSafe(ZPGGrid* grid, i32 x, i32 y)
{
    if (x < 0 || x >= grid->width) { return false; }
    if (y < 0 || y >= grid->height) { return false; }
    return true;
}

static ZPGCell* ZPG_Grid_GetCellAt(ZPGGrid* grid, i32 x, i32 y)
{
    i32 i = ZPG_Grid_PositionToIndex(grid, x, y);
    if (i == -1) { return NULL; }
    return &grid->cells[i];
}

static ZPGCellTypeDef* ZPG_Grid_GetCellTypeAt(ZPGGrid* grid, i32 x, i32 y)
{
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { return ZPG_GetDefaultType(); }
    return ZPG_GetType(cell->tile.type);
}

static i32 ZPG_Grid_CheckTypeAt(
    ZPGGrid* grid, i32 x, i32 y, u8 queryType, i32 bYesIfOutOfBounds)
{
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { return bYesIfOutOfBounds ? YES : NO; }
    return (cell->tile.type == queryType);
}

static i32 ZPG_Grid_CheckTagSetAt(
    ZPGGrid* grid, i32 x, i32 y, u8 queryType, i32 bYesIfOutOfBounds)
{
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { return bYesIfOutOfBounds ? YES : NO; }
    return (cell->tile.tag == queryType);
}

static void ZPG_Grid_Clear(ZPGGrid* grid)
{
    i32 len = grid->width * grid->height;
    for (i32 i = 0; i < len; ++i) { grid->cells[i] = {}; }
}
/**
 * returns NO if type at given cell is 0 (or no stencil was supplied),
 * YES otherwise
 */
static i32 ZPG_Grid_CheckStencilOccupied(ZPGGrid* grid, i32 x, i32 y)
{
    if (grid == NULL) { return NO; }
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { return NO; }
    return (cell->tile.type != ZPG_STENCIL_TYPE_EMPTY);
}

static i32 ZPG_Grid_GetTagAt(ZPGGrid* grid, i32 x, i32 y)
{
    if (grid == NULL) { return NO; }
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { return NO; }
    return cell->tile.tag;
}

static void ZPG_Grid_SetCellTypeAt(ZPGGrid* grid, i32 x, i32 y, u8 type, ZPGGrid* stencil)
{
    if (ZPG_Grid_CheckStencilOccupied(stencil, x, y) == YES) { return; }
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { return; }
    cell->tile.type = type;
}

/**
 * Paint the given type on this cell only if the current value
 * has a different geometry type
 * returns true if it performed a change
 */
static i32 ZPG_Grid_SetCellTypeGeometry(
    ZPGGrid* grid, i32 x, i32 y, u8 typeToPaint, u8 geometryType)
{
    ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL) { return NO; }
    ZPGCellTypeDef* def = ZPG_GetType(cell->tile.type);
    if (def->geometryType == geometryType) { return NO; }
    cell->tile.type = typeToPaint;
    return YES;
}

static void ZPG_Grid_SetCellTypeAll(ZPGGrid* grid, u8 type)
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

static void ZPG_Grid_SetCellChannelAll(ZPGGrid* grid, u8 type, i32 channel)
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

static void ZPG_Grid_CalcStats(ZPGGrid* grid)
{
    grid->stats.numFloorTiles = 0;
    grid->stats.numObjectiveTags = 0;
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_Grid_GetCellTypeAt(grid, x, y);
            if (def->geometryType == ZPG_GEOMETRY_TYPE_PATH)
            {
                grid->stats.numFloorTiles++;
            }
            if (def->category == ZPG_CELL_CATEGORY_OBJECTIVE)
            {
                grid->stats.numObjectiveTags++;
            }
        }
    }
}

static i32 ZPG_Grid_CountNeighboursAt(ZPGGrid* grid, i32 x, i32 y)
{
    ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
    if (cell == NULL)
    {
        return 0;
    }
    u8 matchType = cell->tile.type;
    i32 neighbours = 0;
    cell = ZPG_Grid_GetCellAt(grid, x - 1, y - 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_Grid_GetCellAt(grid, x, y - 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_Grid_GetCellAt(grid, x + 1, y - 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }

    cell = ZPG_Grid_GetCellAt(grid, x - 1, y);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_Grid_GetCellAt(grid, x + 1, y);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }

    cell = ZPG_Grid_GetCellAt(grid, x - 1, y + 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_Grid_GetCellAt(grid, x, y + 1);
    if (cell != NULL && cell->tile.type == matchType)
    {
        neighbours++;
    }
    cell = ZPG_Grid_GetCellAt(grid, x + 1, y + 1);
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
static u8 ZPG_Grid_CountNeighourRingsAt(ZPGGrid* grid, i32 x, i32 y)
{
    ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
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
                ZPGCell *queryCell = ZPG_Grid_GetCellAt(grid, plotX, plotY);
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

/**
 * result must be the same size as grid!
 */
static void ZPG_Grid_CountNeighourRings(ZPGGrid* grid, ZPGGrid* result)
{
    ZPG_PARAM_NULL(grid, );
    ZPG_PARAM_NULL(result, );
    ZPG_PARAM_GRIDS_EQUAL_SIZE(grid, result, )
    printf("Calc rings\n");
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            //ZPGCell *source = ZPG_Grid_GetCellAt(grid, x, y);
            ZPGCell *target = ZPG_Grid_GetCellAt(grid, x, y);
            target->entData.rings = ZPG_Grid_CountNeighourRingsAt(grid, x, y);
        }
    }
}

/**
 * If no destination is supplied, write the new values back into the source;
 * source channel can be 0-3. dest can only be 0-2 (alpha is always 255)
 */
static void ZPG_Grid_PerlinToGreyscale(
    ZPGGrid* source, ZPGGrid* destination, u8 sourceChannel, u8 destChannel, i32 bSetAlpha)
{
    ZPG_PARAM_NULL(source, )
    if (sourceChannel > 3) { return; }
    if (destChannel > 2) { return; }
    u8 highest = 0;
    i32 numPixels = source->width * source->height;
    for (i32 i = 0; i < numPixels; ++i)
    {
        u8 cellValue = source->cells[i].arr[sourceChannel];
        if (cellValue > highest)
        {
            highest = cellValue;
        }
    }
    if (destination == NULL) { destination = source; }
    u8 step = 255 / highest;
    printf("Grayscale conversion highest value %d - step %d\n", highest, step);
    for (i32 i = 0; i < numPixels; ++i)
    {
        u8 cellValue = source->cells[i].arr[sourceChannel];
        destination->cells[i].arr[destChannel] = cellValue * step;
        if (bSetAlpha)
        {
            source->cells[i].colour.a = 255;
        }
    }
}

/////////////////////////////////////////////////////////////
// Grid Allocation/Deallocation
/////////////////////////////////////////////////////////////
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
        grid->cells[i].tile.type = ZPG_CELL_TYPE_WALL;
    }
    grid->width = width;
    grid->height = height;
    return grid;
}

static void ZPG_FreeGrid(ZPGGrid* grid)
{
    ZPG_Free(grid->cells);
}


/////////////////////////////////////////////////////////////
// Grid stack
/////////////////////////////////////////////////////////////

// returns new grid index
static i32 ZPG_AddGridToStack(ZPGGridStack* stack)
{
    if (stack->numGrids >= stack->maxGrids)
    {
        return -1;
    }
    i32 i = stack->numGrids;
    ZPGGrid* grid = ZPG_CreateGrid(
        stack->width, stack->height);
    if (grid == NULL) { return -1; }
    grid->id = i;
    stack->grids[i] = grid;
    stack->numGrids++;
    return i;
}

static ZPGGridStack* ZPG_CreateGridStack(i32 width, i32 height)
{
    ZPGGridStack* stack = (ZPGGridStack*)ZPG_Alloc(sizeof(ZPGGridStack));
    stack->maxGrids = ZPG_MAX_GRID_STACKS;
    stack->grids[0] = ZPG_CreateGrid(width, height);
    stack->numGrids = 1;
    stack->width = width;
    stack->height = height;
    return stack;
}

/**
 * Create an array of points equal to the size of the grid
 * (for searches etc)
 */
static i32 ZPG_Grid_CreatePointsArray(ZPGGrid* grid, ZPGPoint** points)
{
    i32 numCells = grid->width * grid->height;
    i32 numBytes = sizeof(ZPGPoint) * numCells;
    *points = (ZPGPoint*)ZPG_Alloc(numBytes);
    return numCells;
}

static ZPGPoint* ZPG_AllocAndCopyPoints(ZPGPoint* source, i32 numPoints)
{
    i32 bytes = sizeof(ZPGPoint) * numPoints;
    ZPGPoint* points = (ZPGPoint*)ZPG_Alloc(bytes);
    if (points == NULL) { return NULL; }
    ZPG_MEMCPY(points, source, bytes);
    return points;
}

static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height)
{
    ZPGGrid* stencil = ZPG_CreateGrid(width, height);
    ZPG_Grid_SetCellTypeAll(stencil, ZPG_STENCIL_TYPE_EMPTY);
    ZPG_DrawOuterBorder(stencil, NULL, ZPG_STENCIL_TYPE_FULL);
    return stencil;
}

#endif // ZPG_GRID_H