#ifndef ZPG_INTERNAL_H
#define ZPG_INTERNAL_H

#include "../zpg.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ZPG_ABS(value) (value = (value >= 0 ? value : -value))
#define ZPG_MIN(x, y) ((x) < (y) ? (x) : (y))
#define ZPG_MAX(x, y) ((x) > (y) ? (x) : (y))

// Standard lib calls
#define ZPG_STRLEN(ptrToCharArray) \
strlen(ptrToCharArray)

#define ZPG_STRCMP(stringA, stringB) \
strcmp(##stringA##, stringB##)

#define ZPG_MEMSET(ptrToMemory, valueToSet, numBytesToSet) \
memset(##ptrToMemory##, valueToSet##, numBytesToSet##)

#define ZPG_MEMCPY(destinationPtr, sourcePtr, numBytesToCopy) \
memcpy(##destinationPtr##, sourcePtr##, numBytesToCopy##)

#define ZPG_QSORT(list, lengthOfList, listItemDataType, qSortcompareFunction) \
qsort(list, lengthOfList, sizeof(listItemDataType), qSortcompareFunction)


// Param checking
#define ZPG_PARAM_NULL(paramPtr, failureReturnVal) \
if (##paramPtr == NULL) { printf("Param %s was null\n", #paramPtr##); return failureReturnVal; }

#define ZPG_PARAM_GRIDS_EQUAL_SIZE(gridAPtr, gridBPtr, failureReturnVal) \
if (gridAPtr->width != gridBPtr->width || gridAPtr->height != gridBPtr->height) \
{ printf("Grid size mismatch %d - %s\n", __LINE__, __FILE__); return failureReturnVal; }

#define ZPG_ALLOC_ARRAY(dataType, arraySize) \
(##dataType##*)ZPG_Alloc(sizeof(##dataType##) * arraySize##)

//////////////////////////////////////////
// Iterating grid cells
#ifndef ZPG_BEGIN_GRID_ITERATE
#define ZPG_BEGIN_GRID_ITERATE(ptrToGrid) \
for (i32 y = 0; y < ptrToGrid##->height; ++y) \
{ \
    for (i32 x = 0; x < ptrToGrid##->width; ++x) \
    {
#endif

#ifndef ZPG_END_GRID_ITERATE
#define ZPG_END_GRID_ITERATE \
    } \
}
#endif

//////////////////////////////////////////
// Globals
//////////////////////////////////////////

// Grid type is 1 byte so hard limit on types
#define ZPG_TYPES_LIST_SIZE 255
#define ZPG_NUM_TYPES 256
static u8 g_bInitialised = NO;
static ZPGCellTypeDef g_types[ZPG_TYPES_LIST_SIZE];
static u8 g_numTypes = 0;

#define ZPG_MAX_PREFABS 255
static ZPGGridPrefab g_prefabs[ZPG_MAX_PREFABS];
static i32 g_numPrefabs = 0;

#define ZPG_NUM_DIRECTIONS 4
#define ZPG_DIR_RIGHT 0
#define ZPG_DIR_UP 1
#define ZPG_DIR_LEFT 2
#define ZPG_DIR_DOWN 3
static ZPGPoint g_directions[ZPG_NUM_DIRECTIONS];

static zpg_allocate_fn g_ptrAlloc = NULL;
static zpg_free_fn g_ptrFree = NULL;

//////////////////////////////////////////
// Preset types and list of functions
//////////////////////////////////////////
struct ZPGPresetCfg
{
    i32 seed;
    i32 preset;
    i32 flags;
    char* asciOutput;
    char* imageOutput;
};

typedef ZPGGrid* (*zpg_preset_fn)(ZPGPresetCfg* cfg);

typedef i32 (*zpg_param_fn)(i32 argc, char** argv, ZPGPresetCfg* cfg);

#define ZPG_MAX_PRESETS 64
static zpg_preset_fn g_presets[ZPG_MAX_PRESETS];
static char* g_presetLabels[ZPG_MAX_PRESETS];
static i32 g_nextPreset = 0;

//////////////////////////////////////////
// Reading command line
//////////////////////////////////////////

struct ZPGParam
{
    i32 type;
    char asciChar;
    char* helpText;
    // discriminated union on type field
    union
    {
        i32 flag;
        i32 integerOffsetBytes;
        zpg_param_fn func;
    } data;
};

static i32 g_numParamTypes = 0;
static ZPGParam g_paramTypes[64];


//////////////////////////////////////////
// Room datatypes
//////////////////////////////////////////

struct ZPGIndexPair
{
    i32 a;
    i32 b;
};

struct ZPGRoom
{
    i32 id;
    i32 tileType;
    i32 weight;
    // If is stored as a set of points
    ZPGPoint* points;
    i32 numPoints;
    i32* connections;
    i32 numConnections;
};

//////////////////////////////////////////
// Functions
//////////////////////////////////////////
static void ZPG_DrawOuterBorder(ZPGGrid* grid, ZPGGrid* stencil, u8 typeToPaint);

// Grid manipulation
static i32 ZPG_Grid_PositionToIndex(ZPGGrid* grid, i32 x, i32 y);
static ZPGCell* ZPG_Grid_GetCellAt(ZPGGrid* grid, i32 x, i32 y);
static ZPGCellTypeDef* ZPG_Grid_GetCellTypeAt(ZPGGrid* grid, i32 x, i32 y);
static void ZPG_Grid_Clear(ZPGGrid* grid);
static i32 ZPG_Grid_CheckStencilOccupied(ZPGGrid* grid, i32 x, i32 y);
static i32 ZPG_Grid_GetTagAt(ZPGGrid* grid, i32 x, i32 y);
static void ZPG_Grid_SetCellTypeAt(ZPGGrid* grid, i32 x, i32 y, u8 type, ZPGGrid* stencil);
static i32 ZPG_Grid_SetCellTypeGeometry(ZPGGrid* grid, i32 x, i32 y, u8 typeToPaint, u8 geometryType);
static void ZPG_Grid_SetCellTypeAll(ZPGGrid* grid, u8 type);
static void ZPG_Grid_ClearAllTags(ZPGGrid* grid);
static void ZPG_Grid_SetCellChannelAll(ZPGGrid* grid, u8 type, i32 channel);
static ZPGGrid* ZPG_Grid_CreateClone(ZPGGrid* original);
static void ZPG_Grid_CalcStats(ZPGGrid* grid);
static i32 ZPG_Grid_CountNeighboursAt(ZPGGrid* grid, i32 x, i32 y);
static u8 ZPG_Grid_CountNeighourRingsAt(ZPGGrid* grid, i32 x, i32 y);
static void ZPG_Grid_CountNeighourRings(ZPGGrid* grid, ZPGGrid* result);
static void ZPG_Grid_PrintValues(ZPGGrid* grid, i32 bBlankZeroes);
static i32 ZPG_Grid_IsPositionSafe(ZPGGrid* grid, i32 x, i32 y);
static void ZPG_Grid_PrintChars(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY);
static void ZPG_Grid_PerlinToGreyscale(
    ZPGGrid* source, ZPGGrid* destination, u8 sourceChannel, u8 destChannel, i32 bSetAlpha);
static ZPGGrid* ZPG_CreateGrid(i32 width, i32 height);
static void ZPG_FreeGrid(ZPGGrid* grid);
static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height);
static i32 ZPG_ArePointsEqual(ZPGPoint a, ZPGPoint b);

// random numbers...
// TODO: Tidy this crap up...
static f32 ZPG_Randf32(i32 index);
static i32 ZPG_RandArrIndex(i32 len, i32 seed);
static u8 ZPG_RandU8InRange(i32 index, u8 min, u8 max);
static f32 ZPG_Randf32InRange(i32 index, f32 min, f32 max);
static i32 ZPG_STDRandI32();
static u8 ZPG_STDRandU8();
static f32 ZPG_STDRandf32();
static f32 ZPG_STDRandomInRange(f32 min, f32 max);

// misc
static i32 ZPG_ExecSet(char** tokens, i32 numTokens);

#endif // ZPG_INTERNAL_H