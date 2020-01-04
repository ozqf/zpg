#ifndef ZPG_H
#define ZPG_H

#include "zpg/zpg_common.h"
#include <stdio.h>
#include <stdlib.h>

//#define ZPG_CHAR_CODE_SOLID_BLOCK 35
#define ZPG_CHAR_CODE_SOLID_BLOCK 219

#define ZPG_GEOMETRY_TYPE_PATH 0
#define ZPG_GEOMETRY_TYPE_SOLID 1
#define ZPG_GEOMETRY_TYPE_VOID 2

#define ZPG_CELL_CATEGORY_NONE 0
#define ZPG_CELL_CATEGORY_OBJECTIVE 1
#define ZPG_CELL_CATEGORY_ENEMY 2

#define ZPG2_CELL_TYPE_PATH 0
#define ZPG2_CELL_TYPE_WALL 1
#define ZPG2_CELL_TYPE_VOID 2

#define ZPG2_CELL_TYPE_START 3
#define ZPG2_CELL_TYPE_END 4
#define ZPG2_CELL_TYPE_KEY 5
#define ZPG2_CELL_TYPE_ENEMY 6


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

//////////////////////////////////////////
// Data types
//////////////////////////////////////////

struct ZPGCellTypeDef
{
    u8 value;
    // Solid/floor/void
    u8 geometryType;
    // for grouping - eg none/enemy/objective/trap etc
    u8 category;
    // char to write when saving grid as text
    u8 asciChar;
    // display name
    char* label;
};

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

#pragma pack(push, 1)
union ZPGCell
{
    u8 arr[4];
    struct
    {
        u8 type,
            //tag,
            // rotation degrees divided by two
            // to fit without one byte
            halfDegrees,
            entType,
            rings;
    } tile;
    struct
    {
        u8 r, g, b, a;
    } colour;
};
#pragma pack(pop)

struct ZPGGridEntityStats
{
    i32 numFloorTiles;
    i32 numObjectiveTags;
};

struct ZPGGrid
{
    i32 width;
    i32 height;
    // id: plan is to use it to identify grids in
    // user created generate scripts
    //i32 id;
    // used for generating entities
    ZPGGridEntityStats stats;
    // row by row cell store
    ZPGCell *cells;
};

//////////////////////////////////////////
// Functions
//////////////////////////////////////////
extern "C" void ZPG_Init();
extern "C" void ZPG_RunPreset(i32 mode);
/**
 * allocates a new grid.
 * The struct and cells array are in a single allocation
 * so to free the grid, just call free(gridPointer)
 */
extern "C" ZPGGrid *ZPG_CreateGrid(i32 width, i32 height);

extern "C" void ZPG_GridRandomWalk(ZPGGrid* grid, ZPGGrid* stencil, ZPGRect* borderRect, ZPGWalkCfg* cfg, ZPGPoint dir);
extern "C" void ZPG_SeedCaves(ZPGGrid *grid, ZPGGrid *stencil, u8 paintType, i32 *seed);
extern "C" void ZPG_IterateCaves(ZPGGrid *grid, ZPGGrid *stencil, u8 solidType, u8 emptyType);

#endif // ZPG_H