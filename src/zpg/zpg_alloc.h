#ifndef ZPG_ALLOC_H
#define ZPG_ALLOC_H

/**
 * Primitive tracking of allocations so they can be bulk freed, or
 * freed on request of an external user.
 */
#include "zpg_internal.h"

struct ZPGAlloc
{
    void* ptr;
    i32 size;
};

#define ZPG_MAX_ALLOCATIONS 256
static ZPGAlloc g_allocs[ZPG_MAX_ALLOCATIONS];
static i32 g_numAllocs = 0;

static i32 ZPG_GetNumAllocs()
{
    return g_numAllocs;
}

static void* ZPG_Alloc(i32 numBytes)
{
    //printf("Alloc %d at %d\n", numBytes, g_numAllocs);
    ZPGAlloc* record = &g_allocs[g_numAllocs++];
    record->size = numBytes;
    record->ptr = g_ptrAlloc(numBytes);
    return record->ptr;
}

ZPG_EXPORT i32 ZPG_Free(void* ptr)
{
    i32 lastIndex = g_numAllocs - 1;
    if (lastIndex == 0)
    {
        printf("ZPG Free at %d\n", lastIndex);
        g_ptrFree(ptr);
        g_numAllocs--;
        return 0;
    }
    for (i32 i = 0; i <= lastIndex; ++i)
    {
        if (g_allocs[i].ptr == ptr)
        {
            // if last, just decrment
            // otherwise, swap what we're deleting
            // for last
            if (i != lastIndex)
            {
                g_allocs[i] = g_allocs[lastIndex];
            }
            g_ptrFree(ptr);
            g_numAllocs--;
            return 0;
        }
    }
    printf("FREE failed, no matching alloc found\n");
    return 1;
}

/**
 * Clean up all attached memory on rooms
 * then the rooms list itself
 */
static void ZPG_FreeRooms(ZPGRoom* rooms, i32 numRooms)
{
    for (i32 i = 0; i < numRooms; ++i)
    {
        if (rooms[i].points != NULL)
        {
            ZPG_Free(rooms[i].points);
            rooms[i].points = NULL;
            rooms[i].numPoints = 0;
        }
    }
    ZPG_Free(rooms);
}

static void ZPG_PrintAllocations()
{
    printf("=== ZPG ALLOCATIONS ===\n");
    for (i32 i = 0; i < g_numAllocs; ++i)
    {
        f32 kb = (f32)g_allocs[i].size / 1024.f;
        printf("%d: %d, %.3fKB\n", i, (u32)g_allocs[i].ptr, kb);
    }
}

#endif // ZPG_ALLOC_H