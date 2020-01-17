#ifndef ZPG_RANDOM_TABLE_H
#define ZPG_RANDOM_TABLE_H
/*
TODO: How did this get so messy again...?
*/
#include "zpg_internal.h"
#include <time.h>
#include <math.h>

static f32 ZPG_STDRandf32();

static f32 ZPG_Randf32(i32 index)
{
	return ZPG_STDRandf32();
}

static i32 ZPG_RandArrIndex(i32 len, i32 seed)
{
    return rand() % len;
}

static f32 ZPG_Randf32InRange(i32 index, f32 min, f32 max)
{
	return ZPG_Randf32(index) * (max - min) + min;
}

// Wrap rand for now...
static i32 ZPG_STDRandI32()
{
    return rand();
}

static u8 ZPG_STDRandU8()
{
    return rand() % UINT8_MAX;
}

static f32 ZPG_STDRandf32()
{
    return (f32)rand() / (f32)RAND_MAX;
}

static f32 ZPG_STDRandomInRange(f32 min, f32 max)
{
    return ZPG_STDRandf32() * (max - min) + min;
}

#endif // ZPG_RANDOM_TABLE_H