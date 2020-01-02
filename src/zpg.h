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

    void CalcStats()
    {
        stats.numFloorTiles = 0;
        stats.numObjectiveTags = 0;
        for (i32 y = 0; y < height; ++y)
        {
            for (i32 x = 0; x < width; ++x)
            {
                ZPGCell* cell = GetCellAt(x, y);
                #if 1
                if (cell->tile.type != ZPG_CELL_TYPE_FLOOR) { continue; }
                stats.numFloorTiles++;
                if (cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_START)
                {
                    printf("Found start tag at %d/%d\n", x, y);
                    stats.numObjectiveTags++;
                }
                if (cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_END)
                {
                    printf("Found end tag at %d/%d\n", x, y);
                    stats.numObjectiveTags++;
                }
                #endif
                #if 0
                if (cell->tile.type == ZPG_CELL_TYPE_FLOOR)
                {
                    stats.numFloorTiles++;
                }
                if (cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_START
                    || cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_END)
                {
                    stats.numObjectiveTags++;
                }
                #endif
            }
        }
    }

    i32 PositionToIndex(i32 x, i32 y)
    {
        if (x < 0 || x >= width)
        {
            return -1;
        }
        if (y < 0 || y >= height)
        {
            return -1;
        }
        return x + (y * width);
    }

    ZPGCell *GetCellAt(i32 x, i32 y)
    {
        i32 i = PositionToIndex(x, y);
        if (i == -1)
        {
            return NULL;
        }
        return &cells[i];
    }

    void SetCellTypeAt(i32 x, i32 y, u8 type)
    {
        ZPGCell* cell = GetCellAt(x, y);
        if (cell == NULL) { return; }
        cell->tile.type = type;
    }

    void SetCellTypeAll(u8 type)
    {
        i32 totalCells = width * height;
        for (i32 i = 0; i < totalCells; ++i)
        {
            cells[i].tile.type = type;
        }
    }

    void SetCellChannelAll(u8 type, i32 channel)
    {
        if (channel < 0 || channel >= 4) { return; }
        i32 totalCells = width * height;
        for (i32 i = 0; i < totalCells; ++i)
        {
            cells[i].arr[channel] = type;
        }
    }

    void SetCellTagAt(i32 x, i32 y, u8 tag)
    {
        ZPGCell *cell = GetCellAt(x, y);
        if (cell == NULL)
        {
            return;
        }
        if (cell->tile.tag != ZPG_CELL_TAG_NONE)
        {
            printf("WARN: Cannot tag %d/%d as %d, already %d!\n",
                x, y, tag, cell->tile.tag);
            return;
        }
        cell->tile.tag = tag;
        printf("Tagging cell %d/%d as %d\n", x, y, tag);
    }

    i32 CountNeighboursAt(i32 x, i32 y)
    {
        ZPGCell *cell = GetCellAt(x, y);
        if (cell == NULL)
        {
            return 0;
        }
        u8 matchType = cell->tile.type;
        i32 neighbours = 0;
        cell = GetCellAt(x - 1, y - 1);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }
        cell = GetCellAt(x, y - 1);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }
        cell = GetCellAt(x + 1, y - 1);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }

        cell = GetCellAt(x - 1, y);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }
        cell = GetCellAt(x + 1, y);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }

        cell = GetCellAt(x - 1, y + 1);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }
        cell = GetCellAt(x, y + 1);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }
        cell = GetCellAt(x + 1, y + 1);
        if (cell != NULL && cell->tile.type == matchType)
        {
            neighbours++;
        }

        return neighbours;
    }

    /*
    eg:
    0   1     2
    ### ##### #######
    #p# #...# #.....#
    ### #.p.# #.....#
        #...# #..p..#
        ##### #.....#
              #.....#
              #######
    */
    u8 CountNeighourRingsAt(i32 x, i32 y)
    {
        ZPGCell *cell = GetCellAt(x, y);
        if (cell == NULL)
        {
            printf("Cannot Plot ringss at %d/%d - Cell is null\n",
                   x, y);
            return 0;
        }
        u8 result = 0;
        u8 ringTest = 1;
        i32 plotX, plotY;
        i32 bScanning = YES;
        // Count until a test fails
        for (;;)
        {
            for (i32 iY = -ringTest; iY <= ringTest; ++iY)
            {
                for (i32 iX = -ringTest; iX <= ringTest; ++iX)
                {
                    plotX = x + iX;
                    plotY = y + iY;
                    //printf("  Test %d/%d\n", plotX, plotY);
                    ZPGCell *queryCell = GetCellAt(plotX, plotY);
                    if (queryCell == NULL)
                    {
                        bScanning = NO;
                        break;
                    }
                    if (queryCell == cell)
                    {
                        continue;
                    }
                    if (queryCell->tile.type != cell->tile.type)
                    {
                        //printf("  Char mismatch (%c vs %c)\n",
                        //    cell->c, queryCell->c);
                        bScanning = NO;
                        break;
                    }
                }
                if (bScanning == NO)
                {
                    break;
                }
            }
            if (bScanning == NO)
            {
                break;
            }
            else
            {
                result++;
                ringTest++;
            }
        }
        return result;
    }

    void CountNeighourRings()
    {
        printf("Calc rings\n");
        for (i32 y = 0; y < height; ++y)
        {
            for (i32 x = 0; x < width; ++x)
            {
                ZPGCell *cell = GetCellAt(x, y);
                cell->tile.rings = CountNeighourRingsAt(x, y);
            }
        }
    }

    char CellToChar(ZPGCell* cell)
    {
        char c = ' ';
        switch (cell->tile.type)
        {
            case ZPG_CELL_TYPE_WALL:
                c = (u8)ZPG_CHAR_CODE_SOLID_BLOCK;
                break;
            case ZPG_CELL_TYPE_FLOOR:
                c = ' ';
                switch (cell->tile.entType)
                {
                    case ZPG_ENTITY_TYPE_NONE:
                        c = ' ';
                        break;
                    case ZPG_ENTITY_TYPE_ENEMY:
                        c = 'x';
                        break;
                    case ZPG_ENTITY_TYPE_START:
                        c = 's';
                        break;
                    case ZPG_ENTITY_TYPE_END:
                        c = 'e';
                        break;
                    case ZPG_ENTITY_TYPE_OBJECTIVE:
                        c = 'k';
                        break;
                    default:
                        // unknown non-zero
                        c = '?';
                        break;
                }
                break;
        case ZPG_CELL_TYPE_WATER:
            c = '.';
            break;
        default:
            c = '?';
            break;
        }
        return c;
    }

    void PrintChars()
    {
        printf("------ Grid %d/%d (%d path tiles, %d objectives)------\n",
            width, height, stats.numFloorTiles, stats.numObjectiveTags);
        for (i32 y = 0; y < height; ++y)
        {
            printf("|");
            for (i32 x = 0; x < width; ++x)
            {
                ZPGCell *cell = GetCellAt(x, y);
                char c = CellToChar(cell);
                //printf("%c%d", cell->c, cell->rings);

                printf("%c", c);
            }
            printf("|\n");
        }
        printf("------------------\n");
    }
};

extern "C" void ZPG_RunTest(i32 mode);
extern "C" ZPGGrid *ZPG_CreateGrid(i32 width, i32 height);

extern "C" void ZPG_GridRandomWalk(ZPGGrid* grid, ZPGGrid* stencil, ZPGRect* borderRect, ZPGWalkCfg* cfg, ZPGPoint dir);
extern "C" void ZPG_SeedCaves(ZPGGrid *grid, ZPGGrid *stencil, u8 paintType, i32 *seed);
extern "C" void ZPG_IterateCaves(ZPGGrid *grid, ZPGGrid *stencil, u8 solidType, u8 emptyType);

#endif // ZPG_H