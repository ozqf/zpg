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

static i32 ZPG_CheckPointsConnected(ZPGPoint a, ZPGPoint b)
{
    if (ZPG_ArePointsEqual({ a.x - 1, a.y }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x + 1, a.y }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x, a.y - 1 }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x, a.y + 1 }, b)) { return YES; }
    return NO;
}

static i32 ZPG_Rooms_FindConnections(ZPGRoom* a, ZPGRoom* b)
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
                printf("Connection between %d and %d\n", a->id, b->id);
                printf("\tFrom %d, %d to %d, %d\n", pa->x, pa->y, pb->x, pb->y);
                
                count += 1;
                //return YES;
            }
        }
    }
    return count;
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

/**
 * For the given room in an array of rooms, create a list of
 * all of its connections to other rooms.
 */
static i32 ZPG_Rooms_FindConnectionsForRoom(
    ZPGRoom* rooms, i32 numRooms, i32 queryRoomIndex, i32** connections)
{
    ZPGRoom* room = &rooms[queryRoomIndex];
    const i32 maxPairs = 16;
    i32 numPairs = 0;
    //ZPGIndexPair* pairs = (ZPGIndexPair*)ZPG_Alloc(sizeof(ZPGIndexPair) * maxPairs);
    *connections = ZPG_ALLOC_ARRAY(i32, maxPairs);
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
        i32 numRoomConnections = ZPG_Rooms_FindConnections(room, &rooms[i]);
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

static i32 ZPG_Rooms_IsPointOtherRoom(
    ZPGGrid* grid, i32 originRoomType, ZPGPoint origin, ZPGPoint query)
{
    ZPGCell* cell = NULL;
    cell = ZPG_Grid_GetCellAt(grid, query.x, query.y);
    if (cell == NULL) { return NO; }
    u8 tileType = cell->tile.type;
    if (tileType == originRoomType)
    { return NO; }
    if (tileType == 0)
    { return NO; }

    return YES;
}

/**
 * build a new grid where each channel represents a connection to
 * a room left, right, up or down
 */
static ZPGGrid* ZPG_Rooms_BuildConnectionsGrid(ZPGGrid* src, ZPGRoom* rooms, i32 numRooms)
{
    ZPGGrid* result = ZPG_CreateGrid(src->width, src->height);
    printf("Create bitmask grid\n");
    ZPG_Grid_SetCellTypeAll(result, 0);
    
    // iterate rooms
    for (i32 i = 0; i < numRooms; ++i)
    {
        ZPGRoom* room = &rooms[i];
        u8 flags = 0;
        if (room->tileType == 0) { continue; }
        // iterate points in this room
        for (i32 j = 0; j < room->numPoints; ++j)
        {
            ZPGPoint p = room->points[j];
            // check above
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x, p.y - 1}))
            { flags |= (1 << 0); }
            // below
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x, p.y + 1}))
            { flags |= (1 << 1); }
            // left
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x - 1, p.y}))
            { flags |= (1 << 2); }
            // right
            if (ZPG_Rooms_IsPointOtherRoom(src, room->tileType, p, { p.x + 1, p.y}))
            { flags |= (1 << 3); }
            //if (flags > 0) { printf("Flags! %d\n", flags); }
            i32 tileIndex = ZPG_Grid_PositionToIndex(src, p.x, p.y);
            result->cells[tileIndex].tile.type = flags;
        }
    }
    
    return result;
}

// Returns 1 if tile was altered
static i32 ZPG_CheckAndReplace(ZPGGrid* grid, int posX, int posY, int queryX, int queryY, u8 threshold)
{
    if (!ZPG_Grid_IsPositionSafe(grid, queryX, queryY))
    { return false; }
    u8 neighbourType = ZPG_Grid_GetCellAt(grid, queryX, queryY)->tile.type;
    //if (neighbourType == 0) { return false; }
    u8 selfType = ZPG_Grid_GetCellAt(grid, posX, posY)->tile.type;
    i32 diff = (i32)neighbourType - (i32)selfType;
    if (diff < 0) { diff = -diff; }
    if (diff > 0 && diff <= threshold)
    {
        printf("Replacing %d with %d at %d, %d\n", selfType, neighbourType, posX, posY);
        ZPG_Grid_SetCellTypeAt(grid, posX, posY, neighbourType, NULL);
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

static void ZPG_HealRoomScatter2(ZPGGrid* grid, i32 bFourWay, i32 bCorners)
{
    for (int y = 0; y < grid->height; ++y)
    {
        for (int x = 0; x < grid->width; ++x)
        {
            u8 above = 0;
            u8 below = 0;
            u8 left = 0;
            u8 right = 0;
            if (ZPG_Grid_IsPositionSafe(grid, x, y - 1))
            { above = ZPG_Grid_GetCellAt(grid, x, y - 1)->tile.type; }
            if (ZPG_Grid_IsPositionSafe(grid, x, y + 1))
            { below = ZPG_Grid_GetCellAt(grid, x, y + 1)->tile.type; }
            if (ZPG_Grid_IsPositionSafe(grid, x - 1, y))
            { left = ZPG_Grid_GetCellAt(grid, x - 1, y)->tile.type; }
            if (ZPG_Grid_IsPositionSafe(grid, x + 1, y))
            { right = ZPG_Grid_GetCellAt(grid, x + 1, y)->tile.type; }

			if (bFourWay)
			{
				if (above != 0 && above == below) { ZPG_Grid_SetCellTypeAt(grid, x, y, below, NULL); continue; }
				if (left != 0 && left == right) { ZPG_Grid_SetCellTypeAt(grid, x, y, right, NULL); continue; }
			}

			if (bCorners)
			{
				if (left != 0 && left == above) { ZPG_Grid_SetCellTypeAt(grid, x, y, above, NULL); continue; }
            	if (right != 0 && right == above) { ZPG_Grid_SetCellTypeAt(grid, x, y, above, NULL); continue; }
            	if (left != 0 && left == below) { ZPG_Grid_SetCellTypeAt(grid, x, y, below, NULL); continue; }
            	if (right != 0 && right == below) { ZPG_Grid_SetCellTypeAt(grid, x, y, below, NULL); continue; }
			}
        }
    }
}

static ZPGGrid* ZPG_Preset_RoomTreeTest(ZPGPresetCfg* cfg)
{
    ///////////////////////////////////////////////////////////
    // Stage 1
    /*
    > Create rooms on a grid by randomly placing numbers
    > range of numbers skips 0, this will be used to
        fill in isolated tiles. Smaller the range,
        the larger the rooms.
    > group ajoining cells of the same number.
    > Clear any cell that is isolated (no 4 connected neighbours)
    */
   /*
    eg result:
      01234567
    0|  77    |
    1|    5   |
    2|883 5   |
    3|8 33377 |
    4|8   3 7 |
    5|  34    |
    6| 334    |
    7|   4    |
    */
    i32 w = 10, h = 10;
    if (cfg->width > 0) { w = cfg->width; }
    if (cfg->height > 0) { h = cfg->height; }
    u8 minType = 1;
    u8 maxType = 16;
    u8 healThreshold = 1;

    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    //printf("Assign random values\n");
    ZPG_SetRandomGridValues(grid, minType, maxType, &cfg->seed);

    if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
        printf("Seed grid with random values\n");
        ZPG_Grid_PrintValues(grid, YES);
    }
    ZPGGrid* clone = ZPG_Grid_CreateClone(grid);
    
    //ZPG_HealRoomScatter(grid, healThreshold);
	ZPG_HealRoomScatter2(grid, NO, YES);
    // ZPG_HealRoomScatter2(grid, YES, NO);
    // ZPG_HealRoomScatter2(grid, YES, YES);

    if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
        printf("Heal iteration (threshold %d)\n", healThreshold);
        ZPG_Grid_PrintValues(grid, YES);
    }
    /*
    ZPG_HealRoomScatter(grid, healThreshold);
    if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
        printf("Heal iteration (threshold %d)\n", healThreshold);
        ZPG_Grid_PrintValues(grid, YES);
    }
    */
    ZPG_ZeroOutLoneValues(grid);
    ZPG_ZeroOutLoneValues(clone);
    if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
        printf("Zero out lone values\n");
        ZPG_Grid_PrintValues(grid, YES);
        printf("Before heal:\n");
        ZPG_Grid_PrintValues(clone, YES);
    }
    ZPG_FreeGrid(clone);
    
    ZPGCell* cell;
    #if 0 // flood fill space with a different value if you like
    // printf("Flood fill space\n");
    cell = ZPG_Grid_FindFirstCellWithType(grid, 0, &posX, &posY);
    i32 posX, posY;
    // Flood empty space with 1 - not really necessary 
    while (cell != NULL)
    {
        ZPG_Grid_FloodFill(grid, posX, posY, 1);
        cell = ZPG_Grid_FindFirstCellWithType(grid, 0, &posX, &posY);
    }
    #endif

    //cfg->flags &= ~ZPG_API_FLAG_PRINT_WORKING;

    ///////////////////////////////////////////////////////////
    // Stage 2
    // Locate rooms
    /*
    iterate grid cells. For each cell that is not tagged
    > flood fill for connected tiles of the same type
    > tag each cell found so that is not
        used as part of another room
    > create a room and store the cells
    */
    // working array of cell positions
    ZPGPoint* points;
    i32 maxPoints = ZPG_Grid_CreatePointsArray(grid, &points);
    // Stencil to mark visited cells
    //ZPGGrid* stencil = ZPG_CreateGrid(grid->width, grid->height);

    // use tag to record that a cell has been added to a room already
    ZPG_Grid_ClearAllTags(grid);
    i32 numCells = grid->width * grid->height;
    i32 numRooms = 0;
    ZPGRoom* rooms = (ZPGRoom*)ZPG_Alloc(sizeof(ZPGRoom) * maxPoints);
    i32 nextRoom = 0;

    ZPG_BEGIN_GRID_ITERATE(grid)
        cell = ZPG_Grid_GetCellAt(grid, x, y);
        if (cell->tile.tag > 0) { continue; }
        numRooms++;
        i32 numPoints = ZPG_Grid_FloodSearch(grid, x, y, points, maxPoints);
        for (i32 j = 0; j < numPoints; ++j)
        {
            ZPGCell* pointCell = ZPG_Grid_GetCellAt(grid, points[j].x, points[j].y);
            pointCell->tile.tag = 1;
        }
        ZPGRoom* room = &rooms[nextRoom];
        room->id = nextRoom + 1;
        // default weight for now
        room->weight = 1;
        room->tileType = cell->tile.type;
        room->points = ZPG_AllocAndCopyPoints(points, numPoints);
        room->numPoints = numPoints;
        nextRoom++;


    ZPG_END_GRID_ITERATE

    
	
    if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
        printf("Channel 3 values:\n");
        ZPG_Grid_PrintChannelValues(grid, ZPG_CELL_CHANNEL_3, YES);
    }
    
    // build room connections tree
    if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
        printf("Build room connections\n");
    }

    for (i32 i = 0; i < nextRoom; ++i)
    {
        ZPGRoom* room = &rooms[i];
        if (room->tileType == 0) { continue; }
        i32* connections;
        room->numConnections = ZPG_Rooms_FindConnectionsForRoom(
            rooms, nextRoom, i, &connections);
		room->connections = connections;
		//printf("%d has %d connections\n", i, room->numConnections);
    }

	// List rooms
    if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
        printf("-- List (%d) rooms and connections --\n", nextRoom);
	    for (i32 i = 0; i < nextRoom; ++i)
	    {
	    	ZPGRoom* room = &rooms[i];
	    	if (room->tileType == 0) { continue; }
	    	printf("Room %d. type: %d. cells: %d connections (%d):",
	    		room->id, room->tileType, room->numPoints, room->numConnections);
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

	ZPG_Room_PaintIds(grid, rooms, nextRoom);
	if (cfg->flags & ZPG_API_FLAG_PRINT_WORKING)
    {
		printf("Paint Ids\n");
		ZPG_Grid_PrintValues(grid, YES);
	}

	ZPGGrid* canvas = ZPG_GenerateRoomBorder(grid, rooms, nextRoom, YES);
	ZPG_Grid_PrintValues(canvas, YES);
    
    printf("Connections bitmask\n");
    ZPGGrid* connectionGrid = ZPG_Rooms_BuildConnectionsGrid(grid, rooms, nextRoom);
    ZPG_Grid_PrintValues(connectionGrid, NO);

	ZPG_FreeGrid(canvas);

    ///////////////////////////////////////
    // create a route

#if 0
    printf("-- Create Route --\n");
    // Select start room
    i32 startIndex = -1;
    while (startIndex == -1)
    {
        startIndex = ZPG_RandArrIndex(nextRoom, cfg->seed++);
        // Reject filled tiles as start or end
        if (rooms[startIndex].tileType == 0)
        { startIndex = -1; }
    }
    ZPGRoom* start = &rooms[startIndex];

    // Select end room
    i32 endIndex = -1;
    while (endIndex == -1)
    {
        endIndex = ZPG_RandArrIndex(nextRoom, cfg->seed++);
        // cannot be same room as start!
        if (endIndex == startIndex) { endIndex = -1; continue; }
        // Reject filled tiles as start or end
        if (rooms[endIndex].tileType == 0)
        { endIndex = -1; continue;  }
        ZPGRoom* room = &rooms[endIndex];
        for (i32 i = 0; i < room->numConnections; ++i)
        {
            if (room->connections[i] == startIndex)
            { endIndex = -1; break; }
        }
    }
    ZPGRoom* end = &rooms[endIndex];

    printf("Start room is %d, end is %d\n", startIndex, endIndex);
    ZPG_PrintPointsAsGrid(
        start->points, start->numPoints, grid->width, grid->height);
    printf("----------------------\n");
    ZPG_PrintPointsAsGrid(
        end->points, end->numPoints, grid->width, grid->height);
    // show results
    ZPG_Grid_PrintValues(grid, YES);

    ////////////////////////////////////////
    // Build path
    ZPG_Path_SearchRooms(rooms, nextRoom, startIndex, endIndex);
#endif
    // Cleanup
    ZPG_Free(points);
    ZPG_FreeRooms(rooms, numRooms);
    printf("Room tree test finished\n");
    return grid;
}

#endif // ZPG_ROOM_TREE_H