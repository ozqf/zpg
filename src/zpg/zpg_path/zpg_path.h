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

static i32 ZPG_Path_CompareNodesDesc(const void* a, const void* b)
{
    //return ((ZPGPathNode*)a)->f > ((ZPGPathNode*)b)->f ? 1 : -1;
    // want list to be descending, so pop gets least expensive
    return ((ZPGPathNode*)a)->f > ((ZPGPathNode*)b)->f ? -1 : 1;
}

static ZPGPathNode* ZPG_Path_PushNode(ZPGPathNode* list, i32* num)
{
    ZPGPathNode* node = &list[*num];
    *num += 1;
    return node;
}

static ZPGPathNode* ZPG_Path_PopNode(ZPGPathNode* list, i32* num)
{
    if (*num == 0) { return NULL; }
    i32 i = *num - 1;
    ZPGPathNode* node = &list[i];
    *num -= 1;
    return node;
}

/**
 * TODO: Implement me!
 */
static void ZPG_Path_SearchRooms(
    ZPGRoom* rooms, i32 numRooms, i32 startIndex, i32 endIndex)
{
    printf("Path search room %d to %d\n",
        startIndex, endIndex);
    ZPGRoom* start = &rooms[startIndex];
    ZPGRoom* end = &rooms[endIndex];

    // Create open and closed lists
    ZPGPathNode* closed = ZPG_ALLOC_ARRAY(ZPGPathNode, numRooms, ZPG_MEM_TAG_PATHNODES);
    i32 numClosed = 0;
    ZPGPathNode* open = ZPG_ALLOC_ARRAY(ZPGPathNode, numRooms, ZPG_MEM_TAG_PATHNODES);
    i32 numOpen = 0;

    // add start node for inspection
    ZPGPathNode* startRoom = ZPG_Path_PushNode(open, &numOpen);
    startRoom->room = startIndex;

    i32 bSearching = YES;
    i32 escapeCounter = 0;
    while (bSearching)
    {
        // Sort open list by f
        ZPG_QSORT(open, numOpen, ZPGPathNode, ZPG_Path_CompareNodesDesc);
        // pop top node
        ZPGPathNode* node = ZPG_Path_PopNode(open, &numOpen);
        if (node == NULL)
        {
            printf("ABORT room path search ran out of open nodes! (%d iterations)\n",
                escapeCounter);
            bSearching = NO;
            break;
        }
        
        ZPGRoom* room = &rooms[node->room];
        printf("Popped node %d (%d connections)\n",
            node->room, room->numConnections);

        // Inspect and add connected rooms
        for (i32 i = 0; i < room->numConnections; ++i)
        {

        }

        escapeCounter++;
        if (escapeCounter > 999999)
        {
            bSearching = NO;
            printf("ABORT room path search ran away with %d iterations\n",
                escapeCounter);
        }
    }
}

#endif // ZPG_PATH_H