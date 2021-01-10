#ifndef ZPG_ROOM_TREE_H
#define ZPG_ROOM_TREE_H

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
    i32 bVerbose = ((cfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0);
    i32 w = 10, h = 10;
    if (cfg->width > 0) { w = cfg->width; }
    if (cfg->height > 0) { h = cfg->height; }
    u8 minType = 1;
    u8 maxType = 5;
    u8 healThreshold = 1;

    const i32 mainGrid = 0;
    const i32 originalGrid = 1;
    const i32 tagGridIndex = 2;
    const i32 stackSize = 24;

    // Stack grid
    ZPGGridStack* stack = ZPG_CreateGridStack(w, h, stackSize);
    ZPGGrid* grid = stack->grids[mainGrid];
    //ZPG_FillRectWithStencil(grid, NULL, { 0, 0 }, { (w / 2) - 1, h }, 1);
    //ZPG_FillRectWithStencil(grid, NULL, { (w / 2), 0 }, { w - 1, h }, 2);
    ZPG_Grid_FillRandom(stack->grids[0], minType, maxType, &cfg->seed);
    
    if (bVerbose)
    {
        printf("Seed grid with random values from %d to %d\n", minType, maxType);
        ZPG_Grid_PrintValues(grid, 2, YES);
    }

    ZPG_Grid_Copy(stack->grids[0], stack->grids[1]);
    //ZPG_HealRoomScatter(grid, healThreshold);
	//ZPG_HealRoomScatter2(grid, NO, YES);
    //ZPG_HealRoomScatter2(grid, YES, NO);
    ZPG_HealRoomScatter2(grid, YES, YES, 5);

    ZPG_ZeroOutLoneValues(grid);
    ZPGGrid* clone = stack->grids[originalGrid];
    ZPG_Grid_Copy(grid, clone);
    
    if (bVerbose)
    {
        printf("Heal iteration (threshold %d)\n", healThreshold);
        ZPG_Grid_PrintValues(grid, 2, YES);
    }
    
    ZPG_ZeroOutLoneValues(grid);
    ZPG_ZeroOutLoneValues(clone);
    if (bVerbose)
    {
        printf("Zero out lone values\n");
        ZPG_Grid_PrintValues(grid, 2, YES);
        printf("Before heal:\n");
        ZPG_Grid_PrintValues(clone, 2, YES);
    }
    ZPG_FreeGrid(clone);
    
    //ZPGCell* cell;
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
    ZPGGrid* tagGrid = stack->grids[tagGridIndex];
    ZPG_Grid_SetAll(tagGrid, 0);
    //ZPG_Grid_ClearAllTags(grid);

    i32 numCells = grid->width * grid->height;
    i32 numRooms = 0;
    if (bVerbose)
    {
        printf("Gather rooms from:\n");
        ZPG_Grid_PrintValues(grid, 2, YES);
    }
    // allocate for worst case; everyone cell is a room
    // also zero out room memory.
    ZPGRoom* rooms = (ZPGRoom*)ZPG_Alloc(sizeof(ZPGRoom) * maxPoints, ZPG_MEM_TAG_ROOMS);
    for (i32 i = 0; i < maxPoints; ++i)
    {
        rooms[i] = {};
    }
    i32 nextRoom = 0;

    ZPG_BEGIN_GRID_ITERATE(grid)
        //cell = ZPG_Grid_GetCellAt(grid, x, y);
        // skip if cell is marked
        if (ZPG_GRID_GET(tagGrid, x, y) > 0) { continue; }
        // start a room on this tile
        u8 val = ZPG_GRID_GET(grid, x, y);
        numRooms++;
        // flood search for attached cells
        i32 numPoints = ZPG_Grid_FloodSearch(grid, tagGrid, x, y, points, maxPoints);
        // tag flood searched cells to avoid overwriting
        for (i32 j = 0; j < numPoints; ++j)
        {
            ZPG_GRID_SET(tagGrid, points[j].x, points[j].y, 1);
            // ZPGCell* pointCell = ZPG_Grid_GetCellAt(grid, points[j].x, points[j].y);
            // pointCell->tile.tag = 1;
        }
        // alloc a room and copy points
        ZPGRoom* room = &rooms[nextRoom];
        room->id = nextRoom + 1;
        // default weight for now
        room->weight = 1;
        room->tileType = val;
        //room->tileType = cell->tile.type;
        room->points = ZPG_AllocAndCopyPoints(points, numPoints);
        room->numPoints = numPoints;
        // printf("Did room %d, %d tiles, tag grid:\n", nextRoom, numPoints);
        // ZPG_Grid_PrintValues(tagGrid, YES);
        nextRoom++;


    ZPG_END_GRID_ITERATE

    
	
    // if (bVerbose)
    // {
    //     printf("Channel 3 values:\n");
    //     ZPG_Grid_PrintChannelValues(grid, ZPG_CELL_CHANNEL_3, YES);
    // }
    
    ////////////////////////////////////////////////////////
    // build room connections tree
    if (bVerbose)
    {
        printf("Build room connections\n");
    }

    for (i32 i = 0; i < nextRoom; ++i)
    {
        ZPGRoom* room = &rooms[i];
        if (room->tileType == 0) { continue; }
        i32* connections;
        room->numConnections = ZPG_Rooms_FindConnectionsForRoom(
            rooms, nextRoom, i, &connections, ((bVerbose) != 0));
		room->connections = connections;
        
        if (bVerbose)
        {
		    printf("%d has %d connections\n", i, room->numConnections);
        }
    }

	// List rooms
    if (bVerbose)
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

    /////////////////////////////////////////////////////
    // create room layout and draw geometry
    
    // record room-to-room bitmask grid
    ZPGGrid* connectionGrid = ZPG_Rooms_BuildConnectionsGrid(grid, rooms, nextRoom);
    if (bVerbose)
    {
        printf("Connections bitmask\n");
        ZPG_Grid_PrintValues(connectionGrid, 3, NO);
    }

    ZPGGridStack* canvas = ZPG_GenerateRoomBorder(grid, connectionGrid, rooms, nextRoom, YES);
    ZPG_Grid_SetAll(canvas->grids[0], ZPG_CELL_TYPE_WALL);
    ZPG_Grid_SetAllWithStencil(canvas->grids[0], canvas->grids[2], ZPG_CELL_TYPE_PATH);
#if 1
	ZPG_Room_PaintIds(grid, rooms, nextRoom);
	if (bVerbose)
    {
		printf("Paint Ids\n");
		ZPG_Grid_PrintValues(grid, 2, YES);
	}

	if (bVerbose)
    {
        printf("Room borders\n");
		ZPG_Grid_PrintValues(canvas->grids[2], 3, YES);
        printf("Room volumes\n");
        ZPG_Grid_PrintValues(canvas->grids[1], 3, YES);
        printf("Geometry\n");
        ZPG_Grid_PrintValues(canvas->grids[0], 3, YES);
	}
#endif

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
    ZPGGrid* result = ZPG_Grid_CreateClone(canvas->grids[0]);

    //ZPG_PrintAllocations();
    printf("Free points\n");
    ZPG_Free(points);
    printf("Free grids\n");
    ZPG_FreeGridStack(stack);
    ZPG_FreeGridStack(canvas);
    //ZPG_FreeGrid(canvas);
	//ZPG_FreeGrid(connectionGrid);
    //ZPG_PrintAllocations();
    ZPG_FreeRooms(rooms, numRooms);
    printf("Room tree test finished\n");
    return result;
}

#endif // ZPG_ROOM_TREE_H