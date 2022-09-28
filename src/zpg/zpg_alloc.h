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
    zpgSize size;
    i32 tag;
};

#define ZPG_MAX_ALLOCATIONS 256
static ZPGAlloc g_allocs[ZPG_MAX_ALLOCATIONS];
static i32 g_numAllocs = 0;

static i32 ZPG_GetNumAllocs()
{
    return g_numAllocs;
}

ZPG_EXPORT void* ZPG_Alloc(zpgSize numBytes, i32 tag)
{
    #ifdef ZPG_DEBUG
    printf("Alloc tag %d num bytes %d at index %d of %d from fn 0X%X\n",
        tag, numBytes, g_numAllocs, ZPG_MAX_ALLOCATIONS, (u32)g_ptrAlloc);
    #endif
    ZPGAlloc* record = &g_allocs[g_numAllocs++];
    record->tag = tag;
    record->size = numBytes;
    record->ptr = g_ptrAlloc(numBytes);
    #ifdef ZPG_DEBUG
    printf("\tallocated at 0x%X\n", (u32)record->ptr);
    #endif
    return record->ptr;
}

ZPG_EXPORT i32 ZPG_Free(void* ptr)
{
    if (ptr == NULL) { return 0; }
    i32 lastIndex = g_numAllocs - 1;
    if (lastIndex == 0)
    {
        #ifdef ZPG_DEBUG
        printf("ZPG Free at %d\n", lastIndex);
        #endif
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
    #ifdef ZPG_DEBUG
    printf("FREE 0x%X failed, no matching alloc found\n", (u32)ptr);
    #endif
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
        //ZPG_Free(rooms[i].doorways);
    }
    ZPG_Free(rooms);
}

static void ZPG_PrintAllocations()
{
    printf("=== ZPG ALLOCATIONS ===\n");
    for (i32 i = 0; i < g_numAllocs; ++i)
    {
        f32 kb = (f32)g_allocs[i].size / 1024.f;
        printf(
			"%d: tag %d, addr %lld, %.3fKB\n",
			i,
			g_allocs[i].tag,
			(u64)g_allocs[i].ptr,
			kb);
    }
}

#endif // ZPG_ALLOC_H