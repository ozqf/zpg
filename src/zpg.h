#ifndef ZPG_H
#define ZPG_H

extern "C" void ZPG_Hello();

#include "zpg/zpg_common.h"
#include <stdio.h>
#include <stdlib.h>

struct ZPGPoint
{
    i32 x;
    i32 y;
};

struct ZPGRect
{
    ZPGPoint min;
    ZPGPoint max;

    ZPGPoint Centre()
    {
        ZPGPoint p;
        p.x = min.x + ((max.x - min.x) / 2);
        p.y = min.y + ((max.y - min.y) / 2);
        return p;
    }
};

struct ZPGEntity
{
    ZPGPoint pos;
    i32 degrees;
    i32 type;
    i32 tag;
};

struct ZPGWalkCfg
{
    i32 seed;
    f32 bigRoomChance;
    i32 startX;
    i32 startY;
    i32 tilesToPlace;
    u8 typeToPaint;
    //char charToPlace;
};

//#define ZPG_CHAR_CODE_SOLID_BLOCK 35
#define ZPG_CHAR_CODE_SOLID_BLOCK 219

#define ZPG_ENTITY_TYPE_NONE 0
#define ZPG_ENTITY_TYPE_START 1
#define ZPG_ENTITY_TYPE_END 2
#define ZPG_ENTITY_TYPE_OBJECTIVE 3
#define ZPG_ENTITY_TYPE_ENEMY 4
#define ZPG_ENTITY_TYPE_ITEM 5

#define ZPG_CELL_TYPE_NONE 0
#define ZPG_CELL_TYPE_FLOOR 1
#define ZPG_CELL_TYPE_WALL 2
#define ZPG_CELL_TYPE_WATER 3

#define ZPG_STENCIL_TYPE_EMPTY 0
#define ZPG_STENCIL_TYPE_FULL 1

#define ZPG_CELL_TAG_NONE 0
#define ZPG_CELL_TAG_RANDOM_WALK_START 1
#define ZPG_CELL_TAG_RANDOM_WALK_END 2

#define ZPG_CELL_CHANNEL_R 0
#define ZPG_CELL_CHANNEL_G 1
#define ZPG_CELL_CHANNEL_B 2
#define ZPG_CELL_CHANNEL_A 3

#if 0
struct ZPGCell
{
    // specific class of this cell
    i32 type;
    // how many concentric rings of the same cell surround this cell
    i32 rings;
    // for additional categorisation
    i32 tag;
};
#endif

#if 1
#pragma pack(push, 1)
union ZPGCell
{
    u8 arr[4];
    struct
    {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    } colour;

    struct
    {
        u8 type;
        u8 tag;
        u8 entType;
        u8 rings;
    } tile;
    
};
#pragma pack(pop)
#endif

struct ZPGGrid
{
    i32 id;
    i32 width;
    i32 height;
    ZPGCell *cells;

    struct
    {
        i32 numFloorTiles;
        i32 numObjectiveTags;
    } stats;
};

extern "C" void ZPG_RunTest(i32 mode);
extern "C" ZPGGrid *ZPG_CreateGrid(i32 width, i32 height);

extern "C" void ZPG_GridRandomWalk(ZPGGrid* grid, ZPGGrid* stencil, ZPGRect* borderRect, ZPGWalkCfg* cfg, ZPGPoint dir);
extern "C" void ZPG_SeedCaves(ZPGGrid *grid, ZPGGrid *stencil, u8 paintType, i32 *seed);
extern "C" void ZPG_IterateCaves(ZPGGrid *grid, ZPGGrid *stencil, u8 solidType, u8 emptyType);

#endif // ZPG_H