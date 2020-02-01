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

static ZPGGrid* ZPG_Preset_RoomTreeTest(ZPGPresetCfg* cfg)
{
    i32 w = 8, h = 8;
    ZPGGrid* grid = ZPG_CreateGrid(w, h);
    printf("Assign random values\n");
    ZPG_SetRandomGridValues(grid, 2, 9, &cfg->seed);
    ZPG_Grid_PrintValues(grid, YES);
    printf("Group random values\n");
    /*
    > Clear any cell that is isolated (no 4 connected neighbours)
    > While any empty cell can be found
        > Flood fill that cell with 1
    */
    ZPG_ZeroOutLoneValues(grid);
    ZPG_Grid_PrintValues(grid, YES);
    printf("Flood fill space\n");
    i32 posX, posY;
    ZPGCell* cell = ZPG_Grid_FindFirstCellWithType(grid, 0, &posX, &posY);
    while (cell != NULL)
    {
        ZPG_Grid_FloodFill(grid, posX, posY, 1);
        cell = ZPG_Grid_FindFirstCellWithType(grid, 0, &posX, &posY);
    }
    // working array of cell positions
    ZPGPoint* points;
    i32 maxPoints = ZPG_Grid_CreatePointsArray(grid, &points);
    // Stencil to mark visited cells
    //ZPGGrid* stencil = ZPG_CreateGrid(grid->width, grid->height);
    printf("Created %d points\n", maxPoints);
    // show results
    ZPG_Grid_PrintValues(grid, YES);

    /////////////////////////////////////////////
    // Search for rooms... somehow.

    // use tag to record that a cell has been added to a room already
    ZPG_Grid_ClearAllTags(grid);
    i32 numCells = grid->width * grid->height;
    i32 numRooms = 0;
    /*
    iterate grid cells. For each cell that is not tagged
    > flood fill for connected tiles of the same time
    > tag each cell found so that is not
        used as part of another room
    > create a room and store the cells
    */
   ZPGRoom* rooms = (ZPGRoom*)ZPG_Alloc(sizeof(ZPGRoom) * maxPoints);
   i32 nextRoom = 0;
    
    ZPG_BEGIN_GRID_ITERATE(grid)
        cell = ZPG_Grid_GetCellAt(grid, x, y);
        if (cell->tile.tag > 0) { continue; }
        //printf(" Start room value %d at %d/%d\n", cell->tile.type, x, y);
        // new room start gathering tiles
        numRooms++;
        i32 numPoints = ZPG_Grid_FloodSearch(grid, x, y, points, maxPoints);
        //printf("Room %d. value %d, %d cells: ",
        //    numRooms, cell->tile.type, numPoints);
        for (i32 j = 0; j < numPoints; ++j)
        {
            //printf("%d/%d, ", points[j].x, points[j].y);
            ZPGCell* pointCell = ZPG_Grid_GetCellAt(grid, points[j].x, points[j].y);
            pointCell->tile.tag = 1;
        }
        //printf("\n");
        //ZPG_PrintPointsAsGrid(points, numPoints, grid->width, grid->height);
        
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

    return grid;
}

#endif // ZPG_ROOM_TREE_H