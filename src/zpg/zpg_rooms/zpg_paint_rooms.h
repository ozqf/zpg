#include "../zpg_internal.h"

static void ZPG_Grid_FillRandom(ZPGGrid* grid, u8 min, u8 max, i32* seed)
{
	i32 len = grid->width * grid->height;
	for (i32 i = 0; i < len; ++i)
	{
		grid->cells[i] = ZPG_RandU8InRange(*seed++, min, max);
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
    ZPGIndexPair* pairs, i32 numPairs, ZPGIndexPair query)
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

static i32 ZPG_CheckPointsConnected(ZPGPoint a, ZPGPoint b)
{
    if (ZPG_ArePointsEqual({ a.x - 1, a.y }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x + 1, a.y }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x, a.y - 1 }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x, a.y + 1 }, b)) { return YES; }
    return NO;
}

static i32 ZPG_Rooms_FindConnections(ZPGRoom* a, ZPGRoom* b, i32 bVerbose)
{
    i32 count = 0;
    for (i32 i = 0; i < a->numPoints; ++i)
    {
        ZPGPoint* pa = &a->points[i];
        for (i32 j = 0; j < b->numPoints; ++j)
        {
            ZPGPoint* pb = &b->points[j];
            if (ZPG_CheckPointsConnected(*pa, *pb) == YES)
            {
				if (bVerbose)
				{
					printf("Connection between %d and %d\n", a->id, b->id);
					printf("\tFrom %d, %d to %d, %d\n", pa->x, pa->y, pb->x, pb->y);
                }
                count += 1;
                //return YES;
            }
        }
    }
    return count;
}

/**
 * For the given room in an array of rooms, create a list of
 * all of its connections to other rooms.
 */
static i32 ZPG_Rooms_FindConnectionsForRoom(
    ZPGRoom* rooms, i32 numRooms, i32 queryRoomIndex, i32** connections, i32 bVerbose)
{
    ZPGRoom* room = &rooms[queryRoomIndex];
    const i32 maxPairs = 16;
    i32 numPairs = 0;
    //ZPGIndexPair* pairs = (ZPGIndexPair*)ZPG_Alloc(sizeof(ZPGIndexPair) * maxPairs);
    *connections = ZPG_ALLOC_ARRAY(i32, maxPairs, ZPG_MEM_TAG_INTS);
    //printf("Find connections for room %d (%d) points\n",
    //    queryRoomIndex, room->numPoints);
    for (i32 i = 0; i < numRooms; ++i)
    {
        if (i == queryRoomIndex) { continue; }
        // grab query room area
        //ZPGPoint* queryPoints = rooms[i].points;
        //i32 numQueryPoints = rooms[i].numPoints;
        if (rooms[i].tileType == 0) { continue; }
        // Find ajoining rooms
        i32 numRoomConnections = ZPG_Rooms_FindConnections(room, &rooms[i], bVerbose);
        if (numRoomConnections > 0)
        {
            // add pair if necessary
			//i32* tar = connections[numPairs];
			//*tar = i;
            (*connections)[numPairs] = i;
			//(connections[numPairs]) = i;
            numPairs++;
            if (numPairs >= maxPairs)
            {
                printf("No more connection capacity\n");
                return numPairs;
            }
        }
    }
    return numPairs;
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

static void ZPG_Rooms_AssignDoorway(ZPGRoom* roomA, ZPGRoom* roomB, i32 bVerbose)
{
	printf("Assign doorway between room %d and room %d\n", roomA->id, roomB->id);
	// ZPGDoorway door;
	// roomA->numDoorways
}

static void ZPG_Rooms_AssignDoorways(
	ZPGGrid* roomVolumes, ZPGGrid* roomLinkFlags, ZPGRoom* rooms, i32 numRooms, i32 bVerbose)
{
	for (i32 ri = 0; ri < numRooms; ++ri)
	{
		ZPGRoom* room = &rooms[ri];
		i32 numConnections = room->numConnections;
		for (i32 ci = 0; ci < numConnections; ++ci)
		{
			ZPGRoom* other = &rooms[room->connections[ci]];
			ZPG_Rooms_AssignDoorway(room, other, bVerbose);
		}
	}
}

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
