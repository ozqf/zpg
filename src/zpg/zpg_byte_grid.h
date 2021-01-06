#include "zpg_internal.h"

#define ZPG_BGRID_GET(byteGridPtr, gridPosX, gridPosY) \
byteGridPtr->cells[ZPG_POS_TO_INDEX(byteGridPtr->width, gridPosX, gridPosY)]

#define ZPG_BGRID_SET(byteGridPtr, gridPosX, gridPosY, newCellValueU8) \
byteGridPtr->cells[ZPG_POS_TO_INDEX(byteGridPtr->width, gridPosX, gridPosY)] = newCellValueU8
#if 0
/////////////////////////////////////////////////////////////
// Grid Allocation/Deallocation
/////////////////////////////////////////////////////////////
static ZPGByteGrid* ZPG_CreateByteGrid(i32 width, i32 height)
{
    i32 totalCells = width * height;
    i32 memForGrid = (sizeof(u8) * totalCells);
    i32 memTotal = sizeof(ZPGByteGrid) + memForGrid;
    // In-place grid and cells
    u8* ptr = (u8*)ZPG_Alloc(memTotal, ZPG_MEM_TAG_BYTE_GRID);
    ZPGByteGrid* grid = (ZPGByteGrid*)(ptr);
    *grid = {};
	grid->cells = (u8*)(ptr + sizeof(ZPGByteGrid));
    for (i32 i = 0; i < totalCells; ++i)
    {
        grid->cells[i] = 0;
    }
    grid->width = width;
    grid->height = height;
    return grid;
}

static void ZPG_FreeByteGrid(ZPGByteGrid* grid)
{
    ZPG_Free(grid);
}

static void ZPG_ByteGrid_Copy(ZPGByteGrid* src, ZPGByteGrid* dest)
{
	if (src->width != dest->width || src->height != dest->height)
	{
		return;
	}
	i32 len = src->width * src->height;
	for (i32 i = 0; i < len; ++i)
	{
		dest->cells[i] = src->cells[i];
	}
}

static void ZPG_ByteGrid_FillRandom(ZPGByteGrid* grid, u8 min, u8 max, i32* seed)
{
	i32 len = grid->width * grid->height;
	for (i32 i = 0; i < len; ++i)
	{
		grid->cells[i] = ZPG_RandU8InRange(*seed++, min, max);
	}
    // ZPG_BEGIN_GRID_ITERATE(grid)
	// 	i32 i = ZPG_POS_TO_INDEX(grid->width, x, y);
    //     grid->cells[i] = ZPG_RandU8InRange(*seed++, min, max);
    // ZPG_END_GRID_ITERATE
}

static void ZPG_BGrid_SetCellTypeAt(ZPGByteGrid* grid, i32 x, i32 y, u8 val, ZPGByteGrid* stencil)
{
	if (stencil != NULL && ZPG_BGRID_GET(stencil, x, y) != 0) { return; }
    ZPG_BGRID_SET(grid, x, y, val);
}

////////////////////////////////////////////
// manipulation
////////////////////////////////////////////

static void ZPG_ZeroOutLoneValues(ZPGByteGrid* grid)
{
    i32 w = grid->width;
    i32 h = grid->height;
    ZPG_BEGIN_GRID_ITERATE(grid)
        i32 neighbours = 0;
        u8 val = ZPG_BGRID_GET(grid, x, y);
        // left
        if (ZPG_IS_POS_SAFE(w, h, x - 1, y) && ZPG_BGRID_GET(grid, x - 1, y) == val)
        { neighbours++; }
        if (ZPG_IS_POS_SAFE(w, h, x + 1, y) && ZPG_BGRID_GET(grid, x + 1, y) == val)
        { neighbours++; }
        if (ZPG_IS_POS_SAFE(w, h, x, y - 1) && ZPG_BGRID_GET(grid, x, y - 1) == val)
        { neighbours++; }
        if (ZPG_IS_POS_SAFE(w, h, x, y + 1) && ZPG_BGRID_GET(grid, x, y + 1) == val)
        { neighbours++; }
        
        if (neighbours == 0)
        {
            ZPG_BGRID_SET(grid, x, y, 0);
        }
    ZPG_END_GRID_ITERATE
}

static void ZPG_BGrid_HealRoomScatter2(ZPGByteGrid* grid, ZPGByteGrid* stencil, i32 bFourWay, i32 bCorners)
{
	i32 w = grid->width;
	i32 h = grid->height;
    for (int y = 0; y < grid->height; ++y)
    {
        for (int x = 0; x < grid->width; ++x)
        {
            u8 above = 0;
            u8 below = 0;
            u8 left = 0;
            u8 right = 0;
            if (ZPG_IS_POS_SAFE(w, h, x, y - 1))
			{ above = ZPG_BGRID_GET(grid, x, y - 1); }
			if (ZPG_IS_POS_SAFE(w, h, x, y + 1))
			{ below = ZPG_BGRID_GET(grid, x, y + 1); }

			if (ZPG_IS_POS_SAFE(w, h, x - 1, y))
			{ left = ZPG_BGRID_GET(grid, x - 1, y); }
			if (ZPG_IS_POS_SAFE(w, h, x + 1, y))
			{ right = ZPG_BGRID_GET(grid, x + 1, y); }

			if (bFourWay)
			{
				if (above != 0 && above == below) { ZPG_BGrid_SetCellTypeAt(grid, x, y, below, stencil); continue; }
				if (left != 0 && left == right) { ZPG_BGrid_SetCellTypeAt(grid, x, y, right, stencil); continue; }
			}

			if (bCorners)
			{
				if (left != 0 && left == above) { ZPG_BGrid_SetCellTypeAt(grid, x, y, above, stencil); continue; }
            	if (right != 0 && right == above) { ZPG_BGrid_SetCellTypeAt(grid, x, y, above, stencil); continue; }
            	if (left != 0 && left == below) { ZPG_BGrid_SetCellTypeAt(grid, x, y, below, stencil); continue; }
            	if (right != 0 && right == below) { ZPG_BGrid_SetCellTypeAt(grid, x, y, below, stencil); continue; }
			}
        }
    }
}

#endif