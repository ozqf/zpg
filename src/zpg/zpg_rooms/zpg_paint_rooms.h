#include "../zpg_internal.h"

static ZPGRoom* ZPG_FindRoom(ZPGRoom* rooms, i32 numRooms, i32 id)
{
	for (i32 i = 0; i < numRooms; ++i)
	{
		if (rooms[i].id == id) { return &rooms[i]; }
	}
	return NULL;
}

static void ZPG_Room_PaintIds(ZPGGrid* grid, ZPGRoom* rooms, i32 numRooms)
{
	for (i32 i = 0; i < numRooms; ++i)
	{
		ZPGRoom* room = &rooms[i];
		if (room->tileType == 0) { continue; }
		for (i32 j = 0; j < room->numPoints; ++j)
		{
			ZPGPoint p = room->points[j];
			ZPG_Grid_SetValueWithStencil(grid, p.x, p.y, (u8)room->id, NULL);
		}
	}
}

static i32 ZPG_Rooms_IsPointOtherRoom(
    ZPGGrid* grid, i32 originRoomType, ZPGPoint origin, ZPGPoint query)
{
    if (!ZPG_GRID_POS_SAFE(grid, query.x, query.y)) { return NO; }
    u8 val = ZPG_GRID_GET(grid, query.x, query.y);
    if (val == 0 || val == originRoomType) { return NO; }
    return YES;
}

static ZPGGridStack* ZPG_GenerateRoomBorder(
	ZPGGrid* src, ZPGGrid* roomConnectionFlags, ZPGRoom* rooms, i32 numRooms, i32 bPaintRoomId)
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
	ZPG_Grid_SetAll(geometryGrid, 0);
	ZPG_Grid_SetAll(roomVolumes, 0);
	ZPG_Grid_SetAll(roomBoarder, 0);
	
	for (i32 i = 0; i < numRooms; ++i)
	{
		ZPGRoom* room = &rooms[i];
		if (room->tileType == 0) { continue; }
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

/**
 * build a new grid of bitmasks storing connections to other rooms.
 */
static ZPGGrid* ZPG_Rooms_BuildConnectionsGrid(
	ZPGGrid* src, ZPGRoom* rooms, i32 numRooms)
{
    ZPGGrid* result = ZPG_CreateGrid(src->width, src->height);
    printf("Create bitmask grid\n");
    ZPG_Grid_Clear(result);
    //ZPG_Grid_SetCellTypeAll(result, 0);
    
    // iterate rooms
    for (i32 i = 0; i < numRooms; ++i)
    {
        ZPGRoom* room = &rooms[i];
        if (room->tileType == 0) { continue; }
        // iterate points in this room
        for (i32 j = 0; j < room->numPoints; ++j)
        {
			u8 flags = 0;
            ZPGPoint p = room->points[j];
            // check above
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x, p.y - 1}))
            { flags |= ZPG_FLAG_ABOVE; }
            // below
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x, p.y + 1}))
            { flags |= ZPG_FLAG_BELOW; }
            // left
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x - 1, p.y}))
            { flags |= ZPG_FLAG_LEFT; }
            // right
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x + 1, p.y}))
            { flags |= ZPG_FLAG_RIGHT; }
            //if (flags > 0) { printf("Flags! %d\n", flags); }
            i32 tileIndex = ZPG_Grid_PositionToIndexSafe(src, p.x, p.y);
            result->cells[tileIndex] = flags;
        }
    }
    
    return result;
}
