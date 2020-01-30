#ifndef ZPG_H
#define ZPG_H

#ifdef ZPG_WINDOWS_DLL 
#define ZPG_EXPORT extern "C" __declspec(dllexport)
#else
#define ZPG_EXPORT extern "C"
#endif

#include "zpg/zpg_common.h"
#include <stdlib.h>

#define ZPG_API_FLAG_PRINT_RESULT (1 << 0)
#define ZPG_API_FLAG_PRINT_WORKING (1 << 1)
#define ZPG_API_FLAG_STEP_THROUGH (1 << 2)
#define ZPG_API_FLAG_NO_ENTITIES (1 << 3)
#define ZPG_API_FLAG_PRINT_GREYSCALE (1 << 4)

//#define ZPG_CHAR_CODE_SOLID_BLOCK 35
#define ZPG_CHAR_CODE_SOLID_BLOCK 219

#define ZPG_GEOMETRY_TYPE_PATH 0
#define ZPG_GEOMETRY_TYPE_SOLID 1
#define ZPG_GEOMETRY_TYPE_VOID 2

#define ZPG_CELL_CATEGORY_NONE 0
#define ZPG_CELL_CATEGORY_OBJECTIVE 1
#define ZPG_CELL_CATEGORY_ENEMY 2

#define ZPG_CELL_TYPE_PATH 0
#define ZPG_CELL_TYPE_WALL 1
#define ZPG_CELL_TYPE_VOID 2

#define ZPG_CELL_TYPE_START 3
#define ZPG_CELL_TYPE_END 4
#define ZPG_CELL_TYPE_KEY 5
#define ZPG_CELL_TYPE_ENEMY 6

#define ZPG_STENCIL_TYPE_EMPTY 0
#define ZPG_STENCIL_TYPE_FULL 1

#define ZPG_CELL_CHANNEL_R 0
#define ZPG_CELL_CHANNEL_G 1
#define ZPG_CELL_CHANNEL_B 2
#define ZPG_CELL_CHANNEL_A 3

#define ZPG_CAVE_GEN_SEED_CHANCE_LOW 0.45f
#define ZPG_CAVE_GEN_SEED_CHANCE_DEFAULT 0.55f
#define ZPG_CAVE_GEN_SEED_CHANCE_HIGH 0.6f

#define ZPG_CAVE_GEN_CRITICAL_NEIGHBOURS_DEFAULT 4


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
    u8 bPlaceObjectives;
    u8 bStepThrough;
};

// TODO: This could get very messy as more functionality is piled in.
#pragma pack(push, 1)
union ZPGCell
{
    u8 arr[4];
    // Main type, stores main output values.
    struct
    {
        u8 type;
        // heightmap. 
        u8 height;
        // rotation degrees divided by two
        // to fit within one byte
        u8 halfDegrees;
        // Extra working value
        // eg recording that a cell has been visited
        u8 tag; // TODO: Move into separate working grid like ent data
    } tile;
    // Store additional data about another grid for placing entities 
    struct
    {
        u8 type;
        // immediate neighbour tile types
        u8 neighbours;
        // count of concentric rings of neighbours
        u8 rings;
    } entData;
    // Treat the grid as a 32bit texture
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

#define ZPG_MAX_PREFAB_EXITS 4
struct ZPGGridPrefab
{
    char* label;
    ZPGGrid* grid;
    i32 numExits;
    ZPGPoint exits[ZPG_MAX_PREFAB_EXITS];
    ZPGPoint exitDirs[ZPG_MAX_PREFAB_EXITS];
};

// void *(__cdecl *)(size_t)
typedef void* (*zpg_allocate_fn)(size_t size);
typedef void (*zpg_free_fn)(void* ptr);

//////////////////////////////////////////
// Functions
//////////////////////////////////////////
// API version is for external users to check the interface they are calling
ZPG_EXPORT i32 ZPG_ApiVersion();
// On error init returns a none 0 code
// TODO: Init is called automatically anyway in run functions
ZPG_EXPORT i32 ZPG_Init(zpg_allocate_fn ptrAlloc, zpg_free_fn ptrFree);
// Shutdown clears any still allocated memory on error returns none 0
ZPG_EXPORT i32 ZPG_Shutdown();
//ZPG_EXPORT void ZPG_RunPreset(i32 mode, char* outputPath, i32 apiFlags,
//    u8** resultPtr, i32* resultWidth, i32* resultHeight);
ZPG_EXPORT void ZPG_RunPresetCLI(
    i32 argc, char** argv,
    u8** resultPtr, i32* resultWidth, i32* resultHeight);
ZPG_EXPORT i32 ZPG_RunScript(u8* text, i32 textLength, i32 apiFlags);
ZPG_EXPORT void ZPG_PrintPrefabs();
ZPG_EXPORT void ZPG_PrintTileTypes();

#endif // ZPG_H