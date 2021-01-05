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
			ZPG_Grid_SetCellTypeAt(grid, p.x, p.y, (u8)room->id, NULL);
		}
	}
}

static void ZPG_PaintRoomCell4x4(ZPGGrid* grid, ZPGGrid* src, ZPGRoom* rooms, i32 numRooms, i32 x, i32 y)
{
	i32 destX = x * 4;
	i32 destY = y * 4;
	i32 type = ZPG_Grid_GetCellAt(grid, x, y)->tile.type;
	if (type == 0) { return; }
	ZPGRoom* r = NULL;
	//for (i32 i = 0; i < )
}

static ZPGGrid* ZPG_GenerateRoomBorder(
	ZPGGrid* src, ZPGRoom* rooms, i32 numRooms)
{
	ZPGGrid* result = ZPG_CreateGrid(src->width * 4, src->height * 4);
	ZPG_Grid_SetCellTypeAll(result, 0);
	
	for (i32 i = 0; i < numRooms; ++i)
	{
		ZPGRoom* room = &rooms[i];
		if (room->tileType == 0) { continue; }
		for (i32 j = 0; j < room->numPoints; ++j)
		{
			ZPGPoint p = room->points[j];
			ZPGPoint dest;
			dest.x = p.x * 4;
			dest.y = p.y * 4;
			ZPGNeighbours nbs = ZPG_Grid_CountNeighboursAt(src, p.x, p.y);
			//
			if ((nbs.flags & ZPG_FLAG_ABOVE) == 0)
			{
				ZPG_Grid_SetCellTypeAt(result, dest.x, dest.y, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 1, dest.y, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 2, dest.y, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 3, dest.y, (u8)room->tileType, NULL);
			}
			if ((nbs.flags & ZPG_FLAG_BELOW) == 0)
			{
				ZPG_Grid_SetCellTypeAt(result, dest.x, dest.y + 3, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 1, dest.y + 3, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 2, dest.y + 3, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 3, dest.y + 3, (u8)room->tileType, NULL);
			}
			if ((nbs.flags & ZPG_FLAG_LEFT) == 0)
			{
				ZPG_Grid_SetCellTypeAt(result, dest.x, dest.y, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x, dest.y + 1, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x, dest.y + 2, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x, dest.y + 3, (u8)room->tileType, NULL);
			}
			if ((nbs.flags & ZPG_FLAG_RIGHT) == 0)
			{
				ZPG_Grid_SetCellTypeAt(result, dest.x + 3, dest.y, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 3, dest.y + 1, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 3, dest.y + 2, (u8)room->tileType, NULL);
				ZPG_Grid_SetCellTypeAt(result, dest.x + 3, dest.y + 3, (u8)room->tileType, NULL);
			}
		}
	}

	return result;
}