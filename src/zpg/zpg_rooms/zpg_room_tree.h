#ifndef ZPG_ROOM_TREE_H
#define ZPG_ROOM_TREE_H

#include "../zpg_internal.h"

static void ZPG_SeedRooms(
    ZPGGrid* target,
    ZPGGrid* stencil,
    u8 minValue,
    u8 maxValue,
    u8 healThreshold,
    i32 healMode,
    i32 healIterations,
    i32* seed,
    i32 bVerbose)
{
    i32 numDigits = 3;
    if (maxValue < 10)
    {
        numDigits = 1;
    }
    else if (maxValue < 100)
    {
        numDigits = 2;
    }
    
    ZPG_Grid_FillRandom(target, stencil, minValue, maxValue, seed);
    if (bVerbose)
    {
        printf("vals %d to %d\n",
            minValue, maxValue);
        ZPG_Grid_PrintValues(target, numDigits, YES);
    }
    for (i32 i = 0; i < healIterations; ++i)
    {
        i32 bFourWay = YES;
        i32 bCorners = YES;
        if (healMode == 1)
        {
            bFourWay = NO;
        }
        else if (bCorners == 2)
        {
            bCorners = NO;
        }
        //ZPG_HealRoomScatter2(target, YES, YES, healThreshold);
        //ZPG_HealRoomScatter2(target, NO, YES, healThreshold);
        ZPG_HealRoomScatter2(target, bFourWay, bCorners, healThreshold);
    }
    ZPG_ZeroOutLoneValues(target);
    if (bVerbose)
    {
        printf("After heal: threshold %d, mode %d, iterations %d\n",
            healThreshold, healMode, healIterations);
        ZPG_Grid_PrintValues(target, numDigits, YES);
    }
}

/**
 * Test settings for randomly laying out groups of numbers
 */
static ZPGPresetOutput ZPG_Preset_TestRoomSeeding(ZPGPresetCfg* cfg)
{
    i32 bVerbose = ((cfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0);
    i32 w = 10, h = 10;
    if (cfg->width > 0) { w = cfg->width; }
    if (cfg->height > 0) { h = cfg->height; }

    printf("--- Room Seeding test ---\n");
    ZPGGrid* a = ZPG_CreateGrid(w, h);
    ZPG_SeedRooms(a, NULL, 1, 9, 1, 0, 1, &cfg->seed, YES);
    ZPGGrid* b = ZPG_CreateGrid(w, h);
    ZPG_SeedRooms(b, NULL, 1, 3, 5, 0, 2, &cfg->seed, YES);
    ZPGGrid* c = ZPG_CreateGrid(w, h);
    ZPG_SeedRooms(c, NULL, 1, 4, 5, 0, 1, &cfg->seed, YES);
    ZPGGrid* d = ZPG_CreateGrid(w, h);
    ZPG_SeedRooms(d, NULL, 1, 50, 5, 0, 1, &cfg->seed, YES);
    ZPGGrid* e = ZPG_CreateGrid(w, h);
    ZPG_SeedRooms(e, NULL, 1, 10, 100, 0, 2, &cfg->seed, YES);

    ZPGGrid* perlinA = ZPG_CreateGrid(w, h);
    zpg_perlin_cfg perlin = {};
    ZPG_Perlin_SetCfgPreset(&perlin, 1);
    ZPG_DrawPerlinGrid(perlinA, NULL, &cfg->seed, &perlin);
    ZPG_Grid_PrintValues(perlinA, 2, YES);

    return ZPG_OutputFromAsciiGrid(a);
}

static ZPGPresetOutput ZPG_Preset_TestConnectRooms(ZPGPresetCfg* cfg)
{
    i32 bVerbose = ((cfg->flags & ZPG_API_FLAG_PRINT_WORKING) != 0);
    i32 w = 10, h = 10;
    if (cfg->width > 0) { w = cfg->width; }
    if (cfg->height > 0) { h = cfg->height; }

    const i32 roomVolumesIndex = 0;
    const i32 tagGridIndex = 1;
    const i32 doorwayFlagsIndex = 2;
    const i32 roomFlagsIndex = 3;
    const i32 caveStencilIndex = 4;
    //const i32 originalGrid = 4;
    const i32 stackSize = 24;

    // Stack grid
    ZPGGridStack* stack = ZPG_CreateGridStack(w, h, stackSize);

    ZPGGrid* roomVolumes = stack->grids[roomVolumesIndex];
    ZPGGrid* tagGrid = stack->grids[tagGridIndex];
    ZPGGrid* doorFlags = stack->grids[doorwayFlagsIndex];
    ZPGGrid* roomFlags = stack->grids[roomFlagsIndex];
    ZPGGrid* caveStencil = stack->grids[caveStencilIndex];
    ZPG_Grid_SetAll(roomVolumes, 0);
    ZPG_Grid_SetAll(tagGrid, 0);
    ZPG_Grid_SetAll(doorFlags, 0);
    ZPG_Grid_SetAll(roomFlags, 0);
	
    // Generate the initial room volumes grid:
    #if 0
    i32 hw = w / 2;
    i32 hh = h / 2;
    ZPG_FillRectWithStencil(roomVolumes, NULL, { 0, 0 }, { hw - 1, hh - 1 }, 1);
    ZPG_FillRectWithStencil(roomVolumes, NULL, { hw, 0 }, { w - 1, hh - 1 }, 2);
    ZPG_FillRectWithStencil(roomVolumes, NULL, { 0, hh }, { hw - 1, h - 1 }, 3);
    ZPG_FillRectWithStencil(roomVolumes, NULL, { hw, hh }, { w - 1, h - 1 }, 4);
    #endif
    #if 0
    u8 minType = 1;
    u8 maxType = 5;
    u8 healThreshold = 5;
    ZPG_Grid_FillRandom(roomVolumes, minType, maxType, &cfg->seed);
    ZPG_HealRoomScatter2(roomVolumes, YES, YES, healThreshold);
    #endif
    #if 1
    // ZPG_SeedRooms(roomVolumes, 1, 5, 5, 1, &cfg->seed); // okayish results.
    //ZPG_SeedRooms(roomVolumes, 1, 5, 5, 2, &cfg->seed, bVerbose);
    //ZPG_CreateHorizontalBisectStencil(caveStencil);
    ZPG_CreateVerticalBisectStencil(caveStencil);
    ZPG_Grid_PrintValues(caveStencil, 1, YES);
    ZPG_FillCaves(roomVolumes, caveStencil, ZPG_DefaultCaveRules(), &cfg->seed, bVerbose);
    ZPG_Grid_PrintValues(roomVolumes, 1, YES);
    // pass volumes as stencil so it doesn't overwrite itself
    ZPG_SeedRooms(roomVolumes, roomVolumes, 1, 4, 5, 1, 1, &cfg->seed, bVerbose);
    #endif
    ZPG_ZeroOutLoneValues(roomVolumes);
	
    ////////////////////////////////////////////////////////
    // find rooms by flood fill
    ZPGRoom* rooms = NULL;
    i32 numRooms = ZPG_Grid_FindRooms(roomVolumes, tagGrid, &rooms);

    //ZPG_ListRooms(rooms, numRooms);
    ZPG_Rooms_MergeOneDimensionalRooms(roomVolumes, roomFlags, rooms, numRooms);

    ZPG_Room_BuildNeighbourFlags(roomVolumes, roomFlags);
    if (bVerbose)
    {
        printf("Room neighbour flags:\n");
        ZPG_Grid_PrintValues(roomFlags, 3, YES);
    }

    // find all potential doorways
	ZPGDoorwaySet doors = ZPG_FindAllRoomConnectionPoints(
		roomVolumes, rooms, numRooms, bVerbose);
    #if 0
    if (bVerbose)
    {
        ZPG_ListRooms(rooms, numRooms);
        printf("Door to door pairs:\n");
        i32 l = doors.numPairs;
        for (i32 i = 0; i < l; ++i)
        {
            printf("%d: Room Id %d to %d\n",
                i + 1, doors.roomPairs[i].a, doors.roomPairs[i].b);
        }
		l = doors.numDoors;
		for (i32 i = 0; i < l; ++i)
		{
			ZPGDoorway* d = &doors.doors[i];
			printf("%d: Door between %d and %d, cells %d, %d to %d, %d\n",
				i + 1, d->idA, d->idB, d->posA.x, d->posA.y, d->posB.x, d->posB.y);
		}
    }
    #endif
    // select one doorway per connection
    ZPG_Rooms_AssignDoorways(doorFlags, doors, &cfg->seed, bVerbose);
    
    if (bVerbose)
    {
        printf("Source room grid\n");
        ZPG_Grid_PrintValues(roomVolumes, 1, YES);
        printf("Doorway direction flags:\n");
        ZPG_Grid_PrintValues(doorFlags, 3, YES);
    }

    ZPGGrid* connectionGrid = ZPG_Rooms_BuildConnectionsGrid(
        roomVolumes, rooms, numRooms, bVerbose);
    
    ZPGGrid* canvas = ZPG_CreateGrid(roomVolumes->width * 4, roomVolumes->height * 4);

    ZPG_Rooms_PaintGeometry(
        roomVolumes, canvas, roomFlags, doorFlags, rooms, numRooms, NO);
    
    // ZPGGridStack* canvas = ZPG_GenerateRoomBorder(
    //     roomVolumes, connectionGrid, rooms, numRooms, YES);
    // ZPG_Grid_SetAll(canvas->grids[0], ZPG_CELL_TYPE_WALL);
    // ZPG_Grid_SetAllWithStencil(canvas->grids[0], canvas->grids[2], ZPG_CELL_TYPE_PATH);

    ZPGGrid* result = ZPG_Grid_CreateClone(canvas);
    //ZPG_Grid_PrintValues(result, 1, YES);
    printf("Cleanup...\n");
    printf("Free rooms\n");
    ZPG_FreeRooms(rooms, numRooms);
    printf("Free doors list\n");
	ZPG_Free(doors.doors);
    printf("Free grid stack\n");
	ZPG_FreeGridStack(stack);    
    return ZPG_OutputFromAsciiGrid(result);
}

static ZPGPresetOutput ZPG_Preset_RoomTreeTest(ZPGPresetCfg* cfg)
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
    ZPG_Grid_FillRandom(stack->grids[0], NULL, minType, maxType, &cfg->seed);
    
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
    #if 0
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
    #endif
    
	
    // if (bVerbose)
    // {
    //     printf("Channel 3 values:\n");
    //     ZPG_Grid_PrintChannelValues(grid, ZPG_CELL_CHANNEL_3, YES);
    // }
    
    ////////////////////////////////////////////////////////
    // build room connections tree
    ZPGGrid* tagGrid = stack->grids[tagGridIndex];
    ZPG_Grid_SetAll(tagGrid, 0);
    ZPGRoom* rooms = NULL;
    i32 numRooms = ZPG_Grid_FindRooms(grid, tagGrid, &rooms);
    ZPG_ConnectRooms(grid, rooms, numRooms, bVerbose);
    
	// List rooms
    if (bVerbose)
    {
        ZPG_ListRooms(rooms, numRooms);
        #if 0
        printf("-- List (%d) rooms and connections --\n", numRooms);
	    for (i32 i = 0; i < numRooms; ++i)
	    {
	    	ZPGRoom* room = &rooms[i];
	    	if (room->tileType == ZPG_CELL_EMPTY) { continue; }
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
        #endif
    }
    
    /////////////////////////////////////////////////////
    // Create doorways between rooms
    
    // record room-to-room bitmask grid
    ZPGGrid* connectionGrid = ZPG_Rooms_BuildConnectionsGrid(grid, rooms, numRooms, bVerbose);
    
    //ZPG_Rooms_AssignDoorways(grid, connectionGrid, rooms, numRooms, bVerbose);

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
        if (rooms[startIndex].tileType == ZPG_CELL_EMPTY)
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
        if (rooms[endIndex].tileType == ZPG_CELL_EMPTY)
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

    /////////////////////////////////////////////////////
    // draw geometry
    
    ZPGGridStack* canvas = ZPG_GenerateRoomBorder(grid, connectionGrid, rooms, numRooms, YES);
    ZPG_Grid_SetAll(canvas->grids[0], ZPG_CELL_TYPE_WALL);
    ZPG_Grid_SetAllWithStencil(canvas->grids[0], canvas->grids[2], ZPG_CELL_TYPE_PATH);

	ZPG_Room_PaintIds(grid, rooms, numRooms);
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

    /////////////////////////////////////////////////////
    // Cleanup
    ZPGGrid* result = ZPG_Grid_CreateClone(canvas->grids[0]);

    //ZPG_PrintAllocations();
    printf("Free grids\n");
    ZPG_FreeGridStack(stack);
    ZPG_FreeGridStack(canvas);
    //ZPG_FreeGrid(canvas);
	//ZPG_FreeGrid(connectionGrid);
    //ZPG_PrintAllocations();
    ZPG_FreeRooms(rooms, numRooms);
    printf("Room tree test finished\n");
    return ZPG_OutputFromAsciiGrid(result);
}

#endif // ZPG_ROOM_TREE_H