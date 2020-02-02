#ifndef ZPG_PATH_H
#define ZPG_PATH_H

#include "../zpg_internal.h"

struct ZPGPathNode
{
    i32 room;
    i32 parent;
    f32 f; // sum of g and h
    f32 g; // move cost from start to this node
    f32 h; // estimated cost from this node to exit
};

static i32 ZPG_Path_CompareNodes(const void* a, const void* b)
{
    return ((ZPGPathNode*)a)->f > ((ZPGPathNode*)b)->f ? 1 : -1;
}

static void ZPG_Path_SearchRooms(
    ZPGRoom* rooms, i32 numRooms, i32 startIndex, i32 endIndex)
{
    printf("Path search room %d to %d\n",
        startIndex, endIndex);
    ZPGRoom* start = &rooms[startIndex];
    ZPGRoom* end = &rooms[endIndex];

    // Create open and closed lists
    
    ZPGPathNode* closed = ZPG_ALLOC_ARRAY(ZPGPathNode, numRooms);
    i32 numClosed = 0;
    ZPGPathNode* open = ZPG_ALLOC_ARRAY(ZPGPathNode, numRooms);
    i32 numOpen = 0;
    open[numOpen].room = startIndex;
    numOpen++;

    // Sort open list by f
    qsort(open, numOpen, sizeof(ZPGPathNode), ZPG_Path_CompareNodes);
}

#endif // ZPG_PATH_H