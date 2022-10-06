#include "../zpg_internal.h"

static void ZPG_Grid_FillRandom(ZPGGrid* grid, ZPGGrid* stencil, u8 min, u8 max, i32* seed)
{
	ZPG_PARAM_NULL(grid, )
	if (stencil != NULL)
	{
		i32 len = grid->width * grid->height;
		ZPG_BEGIN_GRID_ITERATE(grid)
			if (ZPG_GRID_GET(stencil, x, y) != 0) { continue; }
			ZPG_GRID_SET(grid, x, y, ZPG_RandU8InRange(ZPG_INC_SEED_PTR(seed), min, max));
		ZPG_END_GRID_ITERATE
		return;
	}
	i32 len = grid->width * grid->height;
	for (i32 i = 0; i < len; ++i)
	{
		grid->cells[i] = ZPG_RandU8InRange(ZPG_INC_SEED_PTR(seed), min, max);
	}
}

static void ZPG_ZeroOutLoneValues(ZPGGrid* grid)
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

/**
 * returns -1 if pair is not found
 */
#if 0
static i32 ZPG_FindIndexPair(
    ZPGInt32Pair* pairs, i32 numPairs, ZPGInt32Pair query)
{
    for (i32 i = 0; i < numPairs; ++i)
    {
        if (pairs[i].a == query.a
            && pairs[i].b == query.b)
        { return i; }
    }
    return -1;
}
#endif

static void ZPG_ListRooms(ZPGRoom* rooms, i32 numRooms)
{
    printf("-- List (%d) rooms and connections --\n", numRooms);
	for (i32 i = 0; i < numRooms; ++i)
	{
		ZPGRoom* room = &rooms[i];
		if (room->tileType == ZPG_CELL_EMPTY) { continue; }
		i32 w = room->extents.max.x - room->extents.min.x;
		i32 h = room->extents.max.y - room->extents.min.y;
		printf("Room %d (%d by %d). type: %d. cells: %d connections (%d):",
			room->id, w, h, room->tileType, room->numPoints, room->numConnections);
		if (room->numConnections == 0) { printf("\n"); continue; }
        else { printf("\n\t"); }
		for (i32 j = 0; j < room->numConnections; ++j)
		{
			i32 index = room->connections[j];
			printf(" to room %d (type %d), ", rooms[index].id, rooms[index].tileType);
		}
		printf("\n");
	}
}

// Returns 1 if tile was altered
static i32 ZPG_CheckAndReplace(ZPGGrid* grid, int posX, int posY, int queryX, int queryY, u8 threshold)
{
    if (!ZPG_Grid_IsPositionSafe(grid, queryX, queryY))
    { return false; }
    u8 neighbourType = ZPG_GRID_GET(grid, queryX, queryY);
    //if (neighbourType == 0) { return false; }
    u8 selfType = ZPG_GRID_GET(grid, posX, posY);
    i32 diff = (i32)neighbourType - (i32)selfType;
    if (diff < 0) { diff = -diff; }
    if (diff > 0 && diff <= threshold)
    {
        printf("Replacing %d with %d at %d, %d\n", selfType, neighbourType, posX, posY);
        ZPG_GRID_SET(grid, posX, posY, neighbourType);
        //ZPG_Grid_SetValueWithStencil(grid, posX, posY, neighbourType, NULL);
        return true;
    }
    return false;
}

static void ZPG_HealRoomScatter(ZPGGrid* grid, u8 threshold)
{
    for (int y = 0; y < grid->height; ++y)
    {
        for (int x = 0; x < grid->width; ++x)
        {
            if (ZPG_CheckAndReplace(grid, x, y, x - 1, y, threshold)) { continue; }
            if (ZPG_CheckAndReplace(grid, x, y, x, y - 1, threshold)) { continue; }
            
        }
    }
}

static void ZPG_HealRoomScatter2(ZPGGrid* grid, i32 bFourWay, i32 bCorners, u8 threshold)
{
    for (int y = 0; y < grid->height; ++y)
    {
        for (int x = 0; x < grid->width; ++x)
        {
            u8 self = ZPG_GRID_GET(grid, x, y);
            u8 above = 0;
            u8 below = 0;
            u8 left = 0;
            u8 right = 0;
            if (ZPG_Grid_IsPositionSafe(grid, x, y - 1))
            { above = ZPG_GRID_GET(grid, x, y); }
            if (ZPG_Grid_IsPositionSafe(grid, x, y + 1))
            { below = ZPG_GRID_GET(grid, x, y + 1); }
            if (ZPG_Grid_IsPositionSafe(grid, x - 1, y))
            { left = ZPG_GRID_GET(grid, x - 1, y); }
            if (ZPG_Grid_IsPositionSafe(grid, x + 1, y))
            { right = ZPG_GRID_GET(grid, x + 1, y); }

            // check threshold is met
            if (!ZPG_WithinThreshold(self, above, threshold)) { above = 0; }
            if (!ZPG_WithinThreshold(self, below, threshold)) { below = 0; }
            if (!ZPG_WithinThreshold(self, left, threshold)) { left = 0; }
            if (!ZPG_WithinThreshold(self, right, threshold)) { right = 0; }

			if (bFourWay)
			{
				if (above != 0 && above == below) { ZPG_GRID_SET(grid, x, y, below); continue; }
				if (left != 0 && left == right) { ZPG_GRID_SET(grid, x, y, right); continue; }
			}

			if (bCorners)
			{
				if (left != 0 && left == above) { ZPG_GRID_SET(grid, x, y, above); continue; }
            	if (right != 0 && right == above) { ZPG_GRID_SET(grid, x, y, above); continue; }
            	if (left != 0 && left == below) { ZPG_GRID_SET(grid, x, y, below); continue; }
            	if (right != 0 && right == below) { ZPG_GRID_SET(grid, x, y, below); continue; }
			}
        }
    }
}

static void ZPG_Room_PaintIds(ZPGGrid* grid, ZPGRoom* rooms, i32 numRooms)
{
	for (i32 i = 0; i < numRooms; ++i)
	{
		ZPGRoom* room = &rooms[i];
		if (room->tileType == ZPG_CELL_EMPTY) { continue; }
		for (i32 j = 0; j < room->numPoints; ++j)
		{
			ZPGPoint p = room->points[j];
			ZPG_Grid_SetValueWithStencil(grid, p.x, p.y, (u8)room->id, NULL);
		}
	}
}

// TODO: Remove when 'rooms to geometry' is done
static ZPGGridStack* ZPG_GenerateRoomBorder(
	ZPGGrid* src,
	ZPGGrid* roomConnectionFlags,
	ZPGRoom* rooms,
	i32 numRooms,
	i32 bPaintRoomId)
{
	const i32 scale = 4;
	i32 w = src->width * scale;
	i32 h = src->height * scale;
	ZPGGridStack* stack = ZPG_CreateGridStack(w, h, 3);
	ZPGGrid* geometryGrid = stack->grids[0];
	ZPGGrid* roomVolumes = stack->grids[1];
	ZPGGrid* roomBoarder = stack->grids[2];

	//ZPGGrid* result = ZPG_CreateGrid(src->width * 4, src->height * 4);
	//ZPG_Grid_SetCellTypeAll(result, 0);
	ZPG_Grid_SetAll(geometryGrid, ZPG_CELL_EMPTY);
	ZPG_Grid_SetAll(roomVolumes, ZPG_CELL_EMPTY);
	ZPG_Grid_SetAll(roomBoarder, ZPG_CELL_EMPTY);
	
	for (i32 i = 0; i < numRooms; ++i)
	{
		ZPGRoom* room = &rooms[i];
		if (room->tileType == ZPG_CELL_EMPTY) { continue; }
		for (i32 j = 0; j < room->numPoints; ++j)
		{
			ZPGPoint p = room->points[j];
			// look up room-to-room flags:
			u8 connectionFlags = ZPG_GRID_GET(roomConnectionFlags, p.x, p.y);
			ZPGPoint dest;
			dest.x = p.x * 4;
			dest.y = p.y * 4;
			// calc self-room flags:
			ZPGNeighbours nbs = ZPG_Grid_CountNeighboursAt(src, p.x, p.y);
			u8 val = (u8)room->tileType;
			if (bPaintRoomId)
			{
				val = (u8)room->id;
			}
			// merge flags to decide what walls are painted:
			u8 flags = (u8)nbs.flags;// | ~connectionFlags;
			// fill in 
			ZPG_FillRectWithStencil(roomVolumes, NULL, dest, { dest.x + 3, dest.y + 3 }, val);
			//ZPG_Grid_SetValueWithStencil(roomVolumes, dest.x, dest.y, val, NULL);
			if ((flags & ZPG_FLAG_ABOVE) == 0 && (connectionFlags & ZPG_FLAG_ABOVE) == 0)
			{
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 1, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 2, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 3, dest.y, val, NULL);
			}
			if ((flags & ZPG_FLAG_BELOW) == 0 && (connectionFlags & ZPG_FLAG_BELOW) == 0)
			{
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x, dest.y + 3, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 1, dest.y + 3, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 2, dest.y + 3, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 3, dest.y + 3, val, NULL);
			}
			if ((flags & ZPG_FLAG_LEFT) == 0 && (connectionFlags & ZPG_FLAG_LEFT) == 0)
			{
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x, dest.y + 1, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x, dest.y + 2, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x, dest.y + 3, val, NULL);
			}
			if ((flags & ZPG_FLAG_RIGHT) == 0 && (connectionFlags & ZPG_FLAG_RIGHT) == 0)
			{
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 3, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 3, dest.y + 1, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 3, dest.y + 2, val, NULL);
				ZPG_Grid_SetValueWithStencil(roomBoarder, dest.x + 3, dest.y + 3, val, NULL);
			}
		}
	}

	return stack;
}

// TODO: Remove when 'rooms to geometry' is done
static void ZPG_Rooms_PaintGeometry(
	ZPGGrid* src,
	ZPGGrid* target, // MUST be scale * size of src grid
	ZPGGrid* roomConnectionFlags,
	ZPGGrid* doorwayFlags,
	ZPGRoom* rooms,
	i32 numRooms,
	i32 bPaintRoomId,
	i32 scale)
{
	scale = 4; // TODO Scale doesn't work. always assumes 4
	if (target->width != (src->width * scale))
	{
		printf("ABORT - bad canvas width\n");
		return;
	}
	if (target->height != (src->height * scale))
	{
		printf("ABORT - bad canvas height\n");
		return;
	}

	ZPG_Grid_SetAll(target, ZPG_CELL_TYPE_VOID);
	
	// iterate rooms and their points
	for (i32 i = 0; i < numRooms; ++i)
	{
		ZPGRoom* room = &rooms[i];
		if (room->tileType == ZPG_CELL_EMPTY) { continue; }

		for (i32 j = 0; j < room->numPoints; ++j)
		{
			ZPGPoint p = room->points[j];
			u8 roomFlags = ZPG_GRID_GET(roomConnectionFlags, p.x, p.y);
			u8 doorFlags = ZPG_GRID_GET(doorwayFlags, p.x, p.y);
			// create a wall if their i no doorway and no room in that direction
			//u8 flags = roomFlags;
			//u8 flags = doorFlags;
			u8 flags = roomFlags | doorFlags;
			//u8 flags = (~roomFlags) | doorFlags;
			//u8 flags = roomFlags | (~doorFlags);
			ZPGPoint dest = {};
			dest.x = p.x * scale;
			dest.y = p.y * scale;
			u8 val = ZPG_CELL_TYPE_WALL;
			u8 doorCellType = ZPG_CELL_TYPE_DOOR;
			
			// fill floor with path
			ZPG_FillRectWithStencil(
				target, NULL, dest, { dest.x + (scale - 1), dest.y + (scale - 1) }, ZPG_CELL_TYPE_PATH);

			///////////////////////////////////////
			// Draw doors
			if ((doorFlags & ZPG_FLAG_ABOVE) > 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 1, dest.y, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 2, dest.y, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y, val, NULL);
			}
			if ((doorFlags & ZPG_FLAG_BELOW) > 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 3, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 1, dest.y + 3, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 2, dest.y + 3, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 3, val, NULL);
			}
			if ((doorFlags & ZPG_FLAG_LEFT) > 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 1, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 2, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 3, val, NULL);
			}
			if ((doorFlags & ZPG_FLAG_RIGHT) > 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 1, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 2, doorCellType, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 3, val, NULL);
			}
			
			///////////////////////////////////////
			// Draw solid walls
			// Paint edges depending on flags
			if ((flags & ZPG_FLAG_ABOVE) == 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 1, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 2, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y, val, NULL);
			}
			if ((flags & ZPG_FLAG_BELOW) == 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 3, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 1, dest.y + 3, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 2, dest.y + 3, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 3, val, NULL);
			}
			if ((flags & ZPG_FLAG_LEFT) == 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 1, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 2, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x, dest.y + 3, val, NULL);
			}
			if ((flags & ZPG_FLAG_RIGHT) == 0)
			{
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 1, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 2, val, NULL);
				ZPG_Grid_SetValueWithStencil(target, dest.x + 3, dest.y + 3, val, NULL);
			}
		}
	}
}
