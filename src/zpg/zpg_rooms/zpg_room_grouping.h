#ifndef ZPG_ROOM_GROUPING_H
#define ZPG_ROOM_GROUPING_H

#include "../zpg_internal.h"


static i32 ZPG_CheckPointsConnected(ZPGPoint a, ZPGPoint b)
{
    if (ZPG_ArePointsEqual({ a.x - 1, a.y }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x + 1, a.y }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x, a.y - 1 }, b)) { return YES; }
    if (ZPG_ArePointsEqual({ a.x, a.y + 1 }, b)) { return YES; }
    return NO;
}

static i32 ZPG_Rooms_FindConnectionsBetweenRooms(
    ZPGRoom* a, ZPGRoom* b, i32 bVerbose)
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
    //ZPGInt32Pair* pairs = (ZPGInt32Pair*)ZPG_Alloc(sizeof(ZPGInt32Pair) * maxPairs);
    *connections = ZPG_ALLOC_ARRAY(i32, maxPairs, ZPG_MEM_TAG_INTS);
    if (bVerbose)
    {
        printf("Find connections for room %d (%d) points\n",
            queryRoomIndex, room->numPoints);
    }
    for (i32 i = 0; i < numRooms; ++i)
    {
        if (i == queryRoomIndex) { continue; }
        if (bVerbose)
        {
            printf("Checking connections between rooms %d and %d\n",
                room->id, rooms[i].id);
        }
        // grab query room area
        //ZPGPoint* queryPoints = rooms[i].points;
        //i32 numQueryPoints = rooms[i].numPoints;
        if (rooms[i].tileType == 0) { continue; }
        // Find ajoining rooms
        i32 numRoomConnections = ZPG_Rooms_FindConnectionsBetweenRooms(
            room, &rooms[i], bVerbose);
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

static void ZPG_ConnectRooms(ZPGGrid* roomVolumes, ZPGRoom* rooms, i32 numRooms, i32 bVerbose)
{
    if (bVerbose)
    {
        printf("Build room connections for %d rooms\n", numRooms);
    }

    for (i32 i = 0; i < numRooms; ++i)
    {
        ZPGRoom* r = &rooms[i];
        printf("Check room %d\n", r->id);
        if (r->tileType == 0) { continue; }
        i32* connections;
        r->numConnections = ZPG_Rooms_FindConnectionsForRoom(
            rooms, numRooms, i, &connections, bVerbose);
        r->connections = connections;
        
        if (bVerbose)
        {
		    printf("%d has %d connections\n", i, r->numConnections);
        }
    }
}

static i32 ZPG_IndexOfDoorByCells(
	ZPGDoorway* doors, i32 numDoors, ZPGPoint queryA, ZPGPoint queryB)
{
	for (i32 i = 0; i < numDoors; ++i)
	{
		ZPGPoint doorA = doors[i].posA;
		ZPGPoint doorB = doors[i].posB;
		if (ZPG_ArePointsEqual(doorA, queryA) && ZPG_ArePointsEqual(doorB, queryB))
		{
			return i;
		}
		if (ZPG_ArePointsEqual(doorB, queryA) && ZPG_ArePointsEqual(doorA, queryB))
		{
			return i;
		}
	}
	return -1;
}

static ZPGDoorwaySet ZPG_FindAllRoomConnectionPoints(
    ZPGGrid* roomVolumes, ZPGRoom* rooms, i32 numRooms, i32 bVerbose)
{
    // > for every point in each room, search every point
	//		in other rooms.
    // > if a connection entry already exists for these two
    // 		cells, 
	ZPGDoorwaySet doorSet = {};
	// 
    i32 w = roomVolumes->width;
    i32 h = roomVolumes->height;

    // TODO: grow array when maxed out. also use a hash table!
	doorSet.maxDoors = w * h * 2;
	doorSet.doors = (ZPGDoorway*)ZPG_Alloc(
		sizeof(ZPGDoorway) * doorSet.maxDoors, ZPG_MEM_TAG_DOORS);
	doorSet.numDoors = 0;

    // TODO: also just grow this array and use a hash table!
    doorSet.maxPairs = numRooms * numRooms;
    doorSet.numPairs = 0;
    doorSet.roomPairs = (ZPGInt32Pair*)ZPG_Alloc(
        sizeof(ZPGInt32Pair) * doorSet.maxPairs, ZPG_MEM_TAG_INDEX_PAIRS);

	if (bVerbose)
	{
		printf("Find all doorways between %d rooms (%d doors max)\n",
			numRooms, doorSet.maxDoors);
	}
    for (i32 i = 0; i < numRooms; ++i)
    {
        ZPGRoom* roomA = &rooms[i];
        for (i32 j = 0; j < roomA->numPoints; ++j)
        {
            ZPGPoint a = roomA->points[j];
			for (i32 k = 0; k < numRooms; ++k)
			{
				if (i == k) { continue; }
				ZPGRoom* roomB = &rooms[k];
				for (i32 l = 0; l < roomB->numPoints; ++l)
				{
					ZPGPoint b = roomB->points[l];
					if (!ZPG_ArePointsCardinalNeighbours(a, b))
					{
						continue;
					}
					i32 current = ZPG_IndexOfDoorByCells(
						doorSet.doors, doorSet.numDoors, a, b);
					if (current >= 0)
					{
						continue;
					}
                    if (bVerbose)
                    {
                        printf("Doorway connection between %d and %d\n",
						    roomA->id, roomB->id);
                    }
					ZPGDoorway* door = &doorSet.doors[doorSet.numDoors];
					doorSet.numDoors++;
					door->idA = roomA->id;
					door->idB = roomB->id;
					door->posA = a;
					door->posB = b;

                    // Check if this room to room pair has been recorded
                    i32 pairIndex = -1;
                    for (i32 m = 0; m < doorSet.numPairs; ++m)
                    {
                        ZPGInt32Pair* pair = &doorSet.roomPairs[m];
                        if ((pair->a == door->idA && pair->b == door->idB)
                            || (pair->a == door->idB && pair->b == door->idA))
                        {
                            pairIndex = m;
                            break;
                        }
                    }
                    if (pairIndex == -1)
                    {
                        if (doorSet.numPairs < doorSet.maxPairs)
                        {
                            ZPGInt32Pair* pair = &doorSet.roomPairs[doorSet.numPairs];
                            pair->a = door->idA;
                            pair->b = door->idB;
                            doorSet.numPairs += 1;
                        }
                        else
                        {
                            printf("SKIP ROOM PAIR - no capacity\n");
                        }
                        
                    }

                    if (doorSet.numDoors >= doorSet.maxDoors)
                    {
                        printf("END DOOR SEARCH - max %d of %d doors allocated\n",
                            doorSet.numDoors, doorSet.maxDoors);
                        // drop out of loops
                        i = numRooms;
                        j = roomA->numPoints;
                        k = numRooms;
                        l = roomB->numPoints;
                    }
				}
			}
        }
    }
    if (bVerbose)
    {
        printf("\tFound %d room pairs and %d doorways\n",
            doorSet.numPairs, doorSet.numDoors);
    }
	return doorSet;
}

/**
 * Generate rooms by flood filling to group adjacent cells
 * of the same value in the 'roomsSeed' grid
 * 
 * Returns number of rooms found
 */
static i32 ZPG_Grid_FindRooms(
    ZPGGrid* roomsSeed,
    ZPGGrid* tagGrid,
    ZPGRoom** resultRooms)
{
    ZPGPoint* points;
    i32 maxPoints = ZPG_Grid_CreatePointsArray(roomsSeed, &points);
    ZPGRoom* rooms = (ZPGRoom*)ZPG_Alloc(sizeof(ZPGRoom) * maxPoints, ZPG_MEM_TAG_ROOMS);

    for (i32 i = 0; i < maxPoints; ++i)
    {
        rooms[i] = {};
    }
    i32 numRooms = 0;

    ZPG_BEGIN_GRID_ITERATE(roomsSeed)
        //cell = ZPG_Grid_GetCellAt(grid, x, y);
        // skip if cell is marked
        if (ZPG_GRID_GET(tagGrid, x, y) > 0) { continue; }
        // start a room on this tile
        u8 val = ZPG_GRID_GET(roomsSeed, x, y);
        // flood search for attached cells
        i32 numPoints = ZPG_Grid_FloodSearch(roomsSeed, tagGrid, x, y, points, maxPoints);
        // tag flood searched cells to avoid overwriting
        for (i32 j = 0; j < numPoints; ++j)
        {
            ZPG_GRID_SET(tagGrid, points[j].x, points[j].y, 1);
            // ZPGCell* pointCell = ZPG_Grid_GetCellAt(grid, points[j].x, points[j].y);
            // pointCell->tile.tag = 1;
        }
        // alloc a room and copy points
        ZPGRoom* room = &rooms[numRooms];
        numRooms++;
        room->id = numRooms;
        // default weight for now
        room->weight = 1;
        room->tileType = val;
        //room->tileType = cell->tile.type;
        room->points = ZPG_AllocAndCopyPoints(points, numPoints);
        room->numPoints = numPoints;
        // printf("Did room %d, %d tiles, tag grid:\n", nextRoom, numPoints);
        // ZPG_Grid_PrintValues(tagGrid, YES);

    ZPG_END_GRID_ITERATE
    printf("Free points\n");
    ZPG_Free(points);
    *resultRooms = rooms;
    return numRooms;
}

static ZPGRoom* ZPG_FindRoom(ZPGRoom* rooms, i32 numRooms, i32 id)
{
	for (i32 i = 0; i < numRooms; ++i)
	{
		if (rooms[i].id == id) { return &rooms[i]; }
	}
	return NULL;
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
	ZPGGrid* src, ZPGRoom* rooms, i32 numRooms, i32 bVerbose)
{
    ZPGGrid* result = ZPG_CreateGrid(src->width, src->height);
    ZPG_Grid_Clear(result);
    
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
    if (bVerbose)
    {
        printf("Created connections bitmask\n");
        ZPG_Grid_PrintValues(result, 3, NO);
    }
    return result;
}
/*
static void ZPG_Rooms_AssignDoorway(ZPGRoom* roomA, ZPGRoom* roomB, i32 bVerbose)
{
	printf("Assign doorway between room %d and room %d\n", roomA->id, roomB->id);
	// ZPGDoorway door;
	// roomA->numDoorways
}
*/

/**
 * Select one doorway for each room in the list of doors.
 * Write these connections as left/right/above/below flags in grid
 */
static void ZPG_Rooms_AssignDoorways(
	//ZPGGrid* roomVolumes, ZPGGrid* roomLinkFlags, ZPGRoom* rooms, i32 numRooms, i32 bVerbose)
	ZPGGrid* doorwayFlags, ZPGDoorwaySet doors, i32* seed, i32 bVerbose)
{
	/*
	> iterate room connections
	> for each room, gather all doorways and select a doorway to use.
	*/
	if (bVerbose)
	{
		printf("--- Select active doorways from %d pairs and %d doorways\n",
			doors.numPairs, doors.numDoors);
	}

    // iterate known room to room links
	for (i32 i = 0; i < doors.numPairs; ++i)
	{
        const i32 maxPotentialDoors = 64;
        i32 nextDoor = 0;
        ZPGDoorway potentialDoors[maxPotentialDoors];
		ZPGInt32Pair pair = doors.roomPairs[i];
        // iterate doorways, grabbing each that matches the pair
		for (i32 j = 0; j < doors.numDoors; ++j)
		{
            if (nextDoor >= maxPotentialDoors)
            { break; }

			ZPGDoorway* door = &doors.doors[j];
			if (!ZPG_PairMatches({ pair.a, pair.b }, { door->idA, door->idB }))
			{ continue; }

            ZPGDoorway* potentialDoor = &potentialDoors[nextDoor++];
            *potentialDoor = *door;
		}
        i32 numPotentialDoors = nextDoor;
        nextDoor = 0;
        // if (bVerbose)
        // {
        //     printf("Found %d connection points between rooms %d and %d\n",
        //         numPotentialDoors, pair.a, pair.b);
        // }
        // choose a door
        if (numPotentialDoors == 0) { continue; }
        i32 doorIndex = ZPG_RandArrIndex(numPotentialDoors, *seed++);
        ZPGDoorway newDoor = potentialDoors[doorIndex];
        // figure out direction each point is facing
        ZPGPoint dir;
        dir.x = newDoor.posB.x - newDoor.posA.x;
        dir.y = newDoor.posB.y - newDoor.posA.y;
        i32 gridIndexA = ZPG_GRID_POS_TO_INDEX(doorwayFlags, newDoor.posA.x, newDoor.posA.y);
        i32 gridIndexB = ZPG_GRID_POS_TO_INDEX(doorwayFlags, newDoor.posB.x, newDoor.posB.y);
        u8 flagsA = doorwayFlags->cells[gridIndexA];
        u8 flagsB = doorwayFlags->cells[gridIndexB];
        if (dir.x > 0)
        {
            // left to right
            flagsA |= ZPG_FLAG_RIGHT;
            flagsB |= ZPG_FLAG_LEFT;
        }
        else if (dir.x < 0)
        {
            // right to left
            flagsA |= ZPG_FLAG_LEFT;
            flagsB |= ZPG_FLAG_RIGHT;
        }
        else if (dir.y > 0)
        {
            // above to below
            flagsA |= ZPG_FLAG_BELOW;
            flagsB |= ZPG_FLAG_ABOVE;
        }
        else if (dir.y < 0)
        {
            // below to above
            flagsA |= ZPG_FLAG_ABOVE;
            flagsB |= ZPG_FLAG_BELOW;
        }
        doorwayFlags->cells[gridIndexA] = flagsA;
        doorwayFlags->cells[gridIndexB] = flagsB;

        if (bVerbose)
        {
            printf("Selected %d: of %d - from %d, %d to %d, %d (dir %d, %d)\n",
                doorIndex, numPotentialDoors,
                newDoor.posA.x, newDoor.posA.y,
                newDoor.posB.x, newDoor.posB.y,
                dir.x, dir.y);
        }
	}
	/*
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
	*/
}


#endif // ZPG_ROOM_GROUPING_H