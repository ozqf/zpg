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

/**
 * For the given room in an array of rooms, create a list of
 * all of its connections to other rooms.
 */
static void ZPG_Rooms_FindConnectionsForRoom(ZPGRoom* rooms, i32 numRooms, i32 queryRoomIndex)
{
    ZPGPoint* roomPoints = rooms[queryRoomIndex].points;
    i32 numRoomPoints = rooms[queryRoomIndex].numPoints;
    for (i32 i = 0; i < numRooms; ++i)
    {
        if (i == queryRoomIndex) { continue; }
        ZPGPoint* queryPoints = rooms[i].points;
        i32 numQueryPoints = rooms[i].numPoints;
        // Find ajoining rooms

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
    i32 w = 8, h = 8;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    //printf("Assign random values\n");
    ZPG_SetRandomGridValues(grid, 1, 6, &cfg->seed);
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
    // show results
    ZPG_Grid_PrintValues(grid, YES);

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
        room->tileType = cell->tile.type;
        room->points = ZPG_AllocAndCopyPoints(points, numPoints);
        room->numPoints = numPoints;
        nextRoom++;


    ZPG_END_GRID_ITERATE
    ZPG_Grid_PrintChannelValues(grid, ZPG_CELL_CHANNEL_3, YES);

    // List rooms
    for (i32 i = 0; i < nextRoom; ++i)
    {
        ZPGRoom* room = &rooms[i];
        printf("Room %d. cells: %d. type: %d\n",
            room->id, room->numPoints, room->tileType);
    }

    // build room connections tree


    // Cleanup
    ZPG_Free(points);
    ZPG_FreeRooms(rooms, numRooms);

    return grid;
}

#endif // ZPG_ROOM_TREE_H