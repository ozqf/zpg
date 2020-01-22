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

static void* ZPG_Alloc(i32 numBytes)
{
    ZPGAlloc* record = &g_allocs[g_numAllocs++];
    record->size = numBytes;
    record->ptr = malloc(numBytes);
    return record->ptr;
}

ZPG_EXPORT i32 ZPG_Free(void* ptr)
{
    i32 lastIndex = g_numAllocs - 1;
    if (lastIndex == 0)
    {
        g_numAllocs--;
        return 0;
    }
    for (i32 i = 0; i <= lastIndex; ++i)
    {
        if (g_allocs[i].ptr == ptr)
        {
            if (i == lastIndex)
            {
                g_numAllocs--;
                return 0;
            }
            //swap and decrement
            g_allocs[i] = g_allocs[lastIndex];
            g_numAllocs--;
            return 0;
        }
    }
    return 1;
}



#endif // ZPG_ALLOC_H