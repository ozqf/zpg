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
    i32 tag;
};

#define ZPG_MAX_ALLOCATIONS 256
static ZPGAlloc g_allocs[ZPG_MAX_ALLOCATIONS];
static i32 g_numAllocs = 0;

static i32 ZPG_GetNumAllocs()
{
    return g_numAllocs;
}

static void* ZPG_Alloc(i32 numBytes, i32 tag)
{
    //printf("Alloc %d at %d\n", numBytes, g_numAllocs);
    ZPGAlloc* record = &g_allocs[g_numAllocs++];
    record->size = numBytes;
    record->ptr = g_ptrAlloc(numBytes);
    record->tag = tag;
    return record->ptr;
}

ZPG_EXPORT i32 ZPG_Free(void* ptr)
{
    if (ptr == NULL) { return 0; }
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
    printf("FREE %d failed, no matching alloc found\n", (u32)ptr);
    return 1;
}

/**
 * Will free *every* remaining allocation
 */
static void ZPG_FreeAll()
{
    printf("Free all\n");
    for (i32 i = g_numAllocs - 1; i >= 0; --i)
    {
        ZPG_Free(g_allocs[i].ptr);
        g_allocs[i] = {};
    }
    g_numAllocs = 0;
}

/**
 * Clean up all attached memory on rooms
 * then the rooms list itself
 */
static void ZPG_FreeRooms(ZPGRoom* rooms, i32 numRooms)
{
    printf("Free %d rooms\n", numRooms);
    for (i32 i = 0; i < numRooms; ++i)
    {
        ZPG_Free(rooms[i].points);
        ZPG_Free(rooms[i].connections);
        ZPG_Free(rooms[i].doorways);
    }
    ZPG_Free(rooms);
}

static void ZPG_PrintAllocations()
{
    printf("=== ZPG ALLOCATIONS ===\n");
    for (i32 i = 0; i < g_numAllocs; ++i)
    {
        f32 kb = (f32)g_allocs[i].size / 1024.f;
        printf("%d: tag %d, addr %d, %.3fKB\n", i, g_allocs[i].tag, (u32)g_allocs[i].ptr, kb);
    }
}

#endif // ZPG_ALLOC_H