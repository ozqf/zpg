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

static i32 ZPG_Rooms_CheckConnected(ZPGRoom* a, ZPGRoom* b)
{
    for (i32 i = 0; i < a->numPoints; ++i)
    {
        ZPGPoint* pa = &a->points[i];
        for (i32 j = 0; j < b->numPoints; ++j)
        {
            ZPGPoint* pb = &b->points[j];
            if (ZPG_CheckPointsConnected(*pa, *pb) == YES)
            {
                return YES;
            }
        }
    }
    return NO;
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
    printf("Find connections for room %d (%d) points\n",
        queryRoomIndex, room->numPoints);
    for (i32 i = 0; i < numRooms; ++i)
    {
        if (i == queryRoomIndex) { continue; }
        // grab query room area
        //ZPGPoint* queryPoints = rooms[i].points;
        //i32 numQueryPoints = rooms[i].numPoints;
        // Find ajoining rooms
        if (ZPG_Rooms_CheckConnected(room, &rooms[i]) == YES)
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
    i32 w = 8, h = 8;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    //printf("Assign random values\n");
    ZPG_SetRandomGridValues(grid, 1, 9, &cfg->seed);
    ZPG_Grid_PrintValues(grid, YES);
    //printf("Group random values\n");
    
    ZPG_ZeroOutLoneValues(grid);
    ZPG_Grid_PrintValues(grid, YES);
    
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
        room->id = nextRoom;
        // default weight for now
        room->weight = 1;
        room->tileType = cell->tile.type;
        room->points = ZPG_AllocAndCopyPoints(points, numPoints);
        room->numPoints = numPoints;
        nextRoom++;


    ZPG_END_GRID_ITERATE

    ZPG_Grid_PrintChannelValues(grid, ZPG_CELL_CHANNEL_3, YES);
    
    // build room connections tree
    printf("Build room connections\n");
    for (i32 i = 0; i < nextRoom; ++i)
    {
        ZPGRoom* room = &rooms[i];
        i32* connections;
        room->numConnections = ZPG_Rooms_FindConnectionsForRoom(
            rooms, nextRoom, i, &connections);
		room->connections = connections;
		printf("%d has %d connections\n", i, room->numConnections);
    }

	// List rooms
#if 1
	for (i32 i = 0; i < nextRoom; ++i)
	{
		ZPGRoom* room = &rooms[i];
		printf("Room %d. cells: %d. type: %d\n",
			room->id, room->numPoints, room->tileType);
		printf("\tconnections (%d): ", room->numConnections);
		for (i32 j = 0; j < room->numConnections; ++j)
		{
			i32 index = room->connections[j];
			printf(" %d (%d), ", index, rooms[index].tileType);
		}
		printf("\n");
	}
#endif
    ///////////////////////////////////////
    // create a route

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

    // Cleanup
    ZPG_Free(points);
    ZPG_FreeRooms(rooms, numRooms);

    return grid;
}

#endif // ZPG_ROOM_TREE_H