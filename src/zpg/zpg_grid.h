/**
 * ZPG Grid manipulation functions
 */
#ifndef ZPG_GRID_H
#define ZPG_GRID_H

#include "zpg_internal.h"

static i32 ZPG_Grid_PositionToIndex(ZPGGrid* grid, i32 x, i32 y)
{
    if (x < 0 || x >= grid->width) { return -1; }
    if (y < 0 || y >= grid->height) { return -1; }
    return x + (y * grid->width);
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

static void ZPG_Grid_CountNeighourRings(ZPGGrid* grid)
{
    printf("Calc rings\n");
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
            cell->tile.rings = ZPG_Grid_CountNeighourRingsAt(grid, x, y);
        }
    }
}

static void ZPG_Grid_PrintValues(ZPGGrid* grid, i32 bBlankZeroes)
{
    if (grid == NULL) { return; }
    printf("------ Grid %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
            if (bBlankZeroes && cell->tile.type == 0)
            {
                printf(" ");
            }
            else
            {
                printf("%d", cell->tile.type);
            }
        }
        printf("\n");
    }
    printf("------------------\n");
}

static i32 ZPG_Grid_IsPositionSafe(ZPGGrid* grid, i32 x, i32 y)
{
    if (x < 0 || x >= grid->width) { return false; }
    if (y < 0 || y >= grid->height) { return false; }
    return true;
}

/**
 * Send '\0' marker to place no special marker
 */
static void ZPG_Grid_PrintChars(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY)
{
    printf("------ Grid %d/%d (%d total tiles, %d path tiles, %d objectives)------\n",
        grid->width,
        grid->height,
        grid->width * grid->height,
        grid->stats.numFloorTiles,
        grid->stats.numObjectiveTags);
    i32 xNum = 0;
    i32 yNum = 0;
    printf(" ");
    for (i32 x = 0; x < grid->width; ++x)
    {
        printf("%d", xNum++);
        if (xNum >= 10) { xNum = 0; }
    }
    printf("\n");
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("%d", yNum++);
        if (yNum >= 10) { yNum = 0; }
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_Grid_GetCellTypeAt(grid, x, y);
            u8 c = def->asciChar;
            // Special case
            if (c == '#')
            {
                c = 219;
            }
            if (c == '.')
            {
                c = 176;
            }
            if (marker != '\0' && x == markerX && y == markerY)
            {
                c = marker;
            }
            printf("%c", c);
        }
        printf("\n");
    }
    printf("------------------\n");
}

/**
 * Goes by red channel only. assumes 0-255 range
 */
static void ZPG_Grid_PrintTexture(ZPGGrid* grid, i32 bColourIndices)
{
    const u8 white = 219;
    const u8 lightGrey = 178;
    const u8 middleGrey = 177;
    const u8 darkGrey = 176;
    const u8 black = ' ';
    //u8 colours[] = { white, lightGrey, middleGrey, darkGrey, black };
    u8 colours[] = { black, darkGrey, middleGrey, lightGrey, white };
    //u8 colours[] = { white, lightGrey, middleGrey, darkGrey };
    //u8 colours[] = { lightGrey, middleGrey, darkGrey, white };
    u8 numColours = sizeof(colours);

    u8 divider = (u8)(255 / (numColours - 1));
    //divider -= 1;
    printf("Sizeof(colours): %d divider %d\n", sizeof(colours), divider);
    printf("------ Grayscale texture %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            u8 r = grid->cells[x + (y * grid->width)].colour.r;
            u8 outputIndex = (u8)((f32)((f32)r / (f32)divider) + 0.5f);
            u8 ch = colours[outputIndex];
            //if (outputIndex >= numColours) { outputIndex = numColours - 1; }
            if (bColourIndices == YES)
            {
                printf("%3d: (%3d),", r, outputIndex);
            }
            else
            {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
    printf("----------------------\n");
}

/**
 * If no destination is supplied, write the new values back into the source;
 */
static void ZPG_Grid_PerlinToGreyscale(ZPGGrid* source, ZPGGrid* destination)
{
    if (source == NULL) { return; }
    u8 highest = 0;
    i32 numPixels = source->width * source->height;
    for (i32 i = 0; i < numPixels; ++i)
    {
        u8 cellValue = source->cells[i].tile.type;
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
        u8 cellValue = source->cells[i].tile.type;
        source->cells[i].colour.r = cellValue * step;
        source->cells[i].colour.g = 0;//cellValue * step;
        source->cells[i].colour.b = 0;//cellValue * step;
        source->cells[i].colour.a = 255;
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

#endif // ZPG_GRID_H