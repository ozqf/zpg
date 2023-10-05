#ifndef ZPG_INTERNAL_H
#define ZPG_INTERNAL_H

#include "../zpg.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

//#define ZPG_DEBUG

#define ZPG_ABS(value) (value = (value >= 0 ? value : -value))
#define ZPG_MIN(x, y) ((x) < (y) ? (x) : (y))
#define ZPG_MAX(x, y) ((x) > (y) ? (x) : (y))

#define ZPG_INC_SEED_PTR(ptrToSeedInt) ((*ptrToSeedInt)++)

#define ZPG_MEM_TAG_NONE 0
#define ZPG_MEM_TAG_GRID 1
#define ZPG_MEM_TAG_POINTS 2
#define ZPG_MEM_TAG_ROOMS 3
#define ZPG_MEM_TAG_DOORS 4
#define ZPG_MEM_TAG_INTS 5
#define ZPG_MEM_TAG_FLOATS 6
#define ZPG_MEM_TAG_GRID_STACK 7
#define ZPG_MEM_TAG_FILE 8
#define ZPG_MEM_TAG_PATHNODES 9
#define ZPG_MEM_TAG_ENTITIES 10
#define ZPG_MEM_TAG_BYTE_GRID 11
#define ZPG_MEM_TAG_INDEX_PAIRS 12

#define ZPG_CHAR_WHITE 219
#define ZPG_CHAR_LIGHTGREY 178
#define ZPG_CHAR_MIDDLEGREY 177
#define ZPG_CHAR_DARKGREY 176
#define ZPG_CHAR_EMPTY ' '

#define ZPG_OUTPUT_TYPE_ASCI_GRID (1 << 0)
#define ZPG_OUTPUT_TYPE_GREYSCALE (1 << 1)

#define ZPG_FLAG_ABOVE_LEFT (1 << 0)
#define ZPG_FLAG_ABOVE (1 << 1)
#define ZPG_FLAG_ABOVE_RIGHT (1 << 2)
#define ZPG_FLAG_LEFT (1 << 3)
#define ZPG_FLAG_RIGHT (1 << 4)
#define ZPG_FLAG_BELOW_LEFT (1 << 5)
#define ZPG_FLAG_BELOW (1 << 6)
#define ZPG_FLAG_BELOW_RIGHT (1 << 7)

//#define ZPG_CHAR_CODE_SOLID_BLOCK 35
#define ZPG_CHAR_CODE_SOLID_BLOCK 219

#define ZPG_GEOMETRY_TYPE_PATH 0
#define ZPG_GEOMETRY_TYPE_SOLID 1
#define ZPG_GEOMETRY_TYPE_VOID 2

#define ZPG_CELL_CATEGORY_NONE 0
#define ZPG_CELL_CATEGORY_OBJECTIVE 1
#define ZPG_CELL_CATEGORY_ENEMY 2
#define ZPG_CELL_CATEGORY_DOOR 3

#define ZPG_CELL_EMPTY 0

#define ZPG_CELL_TYPE_PATH 0
#define ZPG_CELL_TYPE_WALL 1
#define ZPG_CELL_TYPE_VOID 2

#define ZPG_CELL_TYPE_START 3
#define ZPG_CELL_TYPE_END 4
#define ZPG_CELL_TYPE_KEY 5
#define ZPG_CELL_TYPE_ENEMY 6
#define ZPG_CELL_TYPE_DOOR 71

#define ZPG_STENCIL_TYPE_EMPTY 0
#define ZPG_STENCIL_TYPE_FULL 1

#define ZPG_CELL_CHANNEL_R 0
#define ZPG_CELL_CHANNEL_G 1
#define ZPG_CELL_CHANNEL_B 2
#define ZPG_CELL_CHANNEL_A 3

#define ZPG_CAVE_GEN_SEED_CHANCE_EVEN_LOWER 0.4f
#define ZPG_CAVE_GEN_SEED_CHANCE_LOW 0.45f
#define ZPG_CAVE_GEN_SEED_CHANCE_DEFAULT 0.55f
#define ZPG_CAVE_GEN_SEED_CHANCE_HIGH 0.6f

#define ZPG_CAVE_GEN_CRITICAL_NEIGHBOURS_DEFAULT 4

//////////////////////////////////////////
// Data types
//////////////////////////////////////////

struct ZPGPoint
{
    i32 x;
    i32 y;
};

struct ZPGPointList
{
    ZPGPoint* points;
    i32 count;
    i32 max;
};

struct ZPGPointPair
{
    ZPGPoint a;
    ZPGPoint b;
};

struct ZPGRect
{
    ZPGPoint min;
    ZPGPoint max;
};

struct ZPGNeighbours
{
	i32 count;
	u32 flags;
};

#pragma pack(push, 1)
union ZPGColour
{
	u8 arr[4];
	struct
	{
		u8 r;
        u8 g;
        u8 b;
        u8 a;
	} channels;
};
#pragma pack(pop)

#define ZPG_BLACK { 0, 0, 0, 255 }
#define ZPG_WHITE { 255, 255, 255, 255 }
#define ZPG_DARK_GREY { 50, 50, 50, 255 }
#define ZPG_LIGHT_GREY { 125, 125, 125, 255 }
#define ZPG_BLUE { 0, 0, 255, 255 }
#define ZPG_GREEN { 0, 255, 0, 255 }
#define ZPG_RED { 255, 0, 0, 255 }

struct ZPGCellTypeDef
{
    u8 value;
    // Solid/floor/void
    u8 geometryType;
    // for grouping - eg none/enemy/objective/trap etc
    u8 category;
    // char to write when saving grid as text
    u8 asciChar;
    // colour to draw when saving in picture mode
    ZPGColour colour;
    // display name
    char* label;
};

struct ZPGEntity
{
    ZPGPoint pos;
    i32 degrees;
    i32 type;
    i32 tag;
};

struct ZPGGridEntityStats
{
    i32 numFloorTiles;
    i32 numObjectiveTags;
};

struct ZPGWalkCfg
{
    i32 seed;
    f32 bigRoomChance;
    i32 startX;
    i32 startY;
    i32 dirX;
    i32 dirY;
    i32 tilesToPlace;
    u8 typeToPaint;
    u8 bPlaceObjectives;
    u8 bStepThrough;
};

struct ZPGCellRules
{
    f32 seedChance;
    i32 starveLimit;
    i32 overpopLimit;
    i32 birthLimit;
    i32 iterations;

    u8 emptyValue;
    u8 filledValue;
};

#define ZPG_CELL_CHANNEL_0 0
#define ZPG_CELL_CHANNEL_1 1
#define ZPG_CELL_CHANNEL_2 2
#define ZPG_CELL_CHANNEL_3 3
 
#define ZPG_IS_POS_SAFE(gridWidthI32, gridHeightI32, gridPosX, gridPosY) \
((gridPosX) >= 0 && (gridPosX) < (gridWidthI32) && (gridPosY) >= 0 && (gridPosY) < gridHeightI32)

#define ZPG_POS_TO_INDEX(gridWidthI32, gridPosX, gridPosY) \
((gridPosX) + ((gridPosY) * (gridWidthI32)))

#define ZPG_GRID_POS_SAFE(gridPtr, gridPosX, gridPosY) \
(ZPG_IS_POS_SAFE(gridPtr->width, gridPtr->height, (gridPosX), (gridPosY)))

#define ZPG_GRID_POS_TO_INDEX(gridPtr, gridPosX, gridPosY) \
(gridPosX + (gridPosY * gridPtr->width))

#define ZPG_GRID_GET(gridPtr, gridPosX, gridPosY) \
(gridPtr->cells[ZPG_POS_TO_INDEX((gridPtr->width), (gridPosX), (gridPosY))])

#define ZPG_GRID_GET_ADDR(gridPtr, gridPosX, gridPosY) \
(&gridPtr->cells[ZPG_POS_TO_INDEX(gridPtr->width, (gridPosX), (gridPosY))])

#define ZPG_GRID_SET(gridPtr, gridPosX, gridPosY, valToWrite) \
{ if ZPG_IS_POS_SAFE(gridPtr->width, gridPtr->height, gridPosX, gridPosY) \
{ gridPtr->cells[ZPG_POS_TO_INDEX(gridPtr->width, (gridPosX), (gridPosY))] = (valToWrite); }}

#define ZPG_BGRID_GET(byteGridPtr, gridPosX, gridPosY) \
byteGridPtr->cells[ZPG_POS_TO_INDEX(byteGridPtr->width, gridPosX, gridPosY)]

#define ZPG_BGRID_SET(byteGridPtr, gridPosX, gridPosY, newCellValueU8) \
byteGridPtr->cells[ZPG_POS_TO_INDEX(byteGridPtr->width, (gridPosX), (gridPosY))] = (newCellValueU8)

struct ZPGGrid
{
    i32 width;
    i32 height;
    // id: plan is to use it to identify grids in
    // user created generate scripts
    i32 id;
    // used for generating entities
    ZPGGridEntityStats stats;
    // row by row cell store
    u8* cells;
    //ZPGCell *cells;
    u8 GetValue(i32 x, i32 y)
    {
        return this->cells[ZPG_POS_TO_INDEX(this->width, x, y)];
    }

    void SetValue(i32 x, i32 y, u8 val)
    {
        this->cells[ZPG_POS_TO_INDEX(this->width, x, y)] = val;
    }

    i32 IsSafe(i32 x, i32 y)
    {
        return ZPG_IS_POS_SAFE(this->width, this->height, x, y);
    }
};

#define ZPG_MAX_GRID_STACKS 32
struct ZPGGridStack
{
    i32 numGrids;
    i32 maxGrids;
    i32 width;
    i32 height;
    ZPGGrid* grids[ZPG_MAX_GRID_STACKS];
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

// Standard lib calls
#define ZPG_STRLEN(ptrToCharArray) \
strlen(ptrToCharArray)

#define ZPG_STRCMP(stringA, stringB) \
strcmp(##stringA##, stringB##)

#define ZPG_STREQL(stringA, stringB) \
(strcmp(##stringA##, stringB##) == 0)

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

#define ZPG_ALLOC_ARRAY(dataType, arraySize, memAllocTag) \
(##dataType##*)ZPG_Alloc(sizeof(##dataType##) * arraySize##, memAllocTag##)

#define ZPG_ASSERT(expression, msg) if (!(expression)) \
{ \
    char assertBuf[1024]; \
    snprintf(assertBuf, 1024, "%s, %d: %s\n", __FILE__, __LINE__, msg); \
	ZPG_Fatal(assertBuf); \
}

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
static u8 g_bZPGInitialised = NO;
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
static zpg_fatal_fn g_ptrFatal = NULL;

//////////////////////////////////////////
// Preset types and list of functions
//////////////////////////////////////////
struct ZPGPresetCfg
{
    i32 seed;
    char* preset;
    i32 flags;
    i32 width;
    i32 height;
	i32 roomCount;
    char* asciOutput;
    char* imageOutput;
    char* pictureOutput;
};

struct ZPGOutput
{
    i32 id;
    i32 format; // format of zero means nothing is assigned
    void* ptr;
    zpgSize size;

};

struct ZPGPresetOutput
{
    i32 typeFlags;
    ZPGGrid* asciiGrid;
    ZPGGrid* greyGrid;
};

struct ZPGContext
{
    i32 seed;
    i32 verbosity;
    ZPGPoint lastStop;

    // additionally allocated items
    ZPGGridStack* gridStack;
    ZPGPointList points;

    char* history;
    zpgSize maxHistory;
    zpgSize historyCursor;

    // configurations
    ZPGWalkCfg walkCfg;
    ZPGCellRules cellCfg;
    
    // target objects
    i32 gridIndex;
    i32 stencilIndex;
    ZPGGrid* grid;
    ZPGGrid* stencil;
};

typedef ZPGPresetOutput (*zpg_preset_fn)(ZPGPresetCfg* cfg);
typedef i32 (*zpg_param_fn)(i32 argc, char** argv, ZPGPresetCfg* cfg);
typedef i32 (*zpg_command_fn)(ZPGContext* ctx, char** tokens, i32 numTokens);

struct ZPGCommand
{
    char* name;
    zpg_command_fn function;
};

struct ZPGPreset
{
	char* selector;
	char* description;
	zpg_preset_fn funcPtr;
	u32 outputFormatFlags;
	u32 defaultOutputFormat;
};

#define ZPG_MAX_PRESETS 64
static ZPGPreset g_presets[ZPG_MAX_PRESETS];
// static zpg_preset_fn g_presets[ZPG_MAX_PRESETS];
// static char* g_presetLabels[ZPG_MAX_PRESETS];
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

struct ZPGInt32Pair
{
    i32 a;
    i32 b;
};

struct ZPGDoorway
{
    // Ids of rooms
    i32 idA;
    i32 idB;
    // positions of tiles
    ZPGPoint posA;
    ZPGPoint posB;
};

struct ZPGDoorwaySet
{
	ZPGDoorway* doors;
	i32 numDoors;
	i32 maxDoors;

    ZPGInt32Pair* roomPairs;
    i32 numPairs;
    i32 maxPairs;
};

struct ZPGRoom
{
    i32 id;
    i32 tileType;
    i32 weight;
    ZPGRect extents;
    // If is stored as a set of points
    ZPGPoint* points;
    i32 numPoints;
    i32* connections;
    i32 numConnections;
    //ZPGDoorway* doorways;
    //i32 numDoorways;
};

//////////////////////////////////////////
// Functions
//////////////////////////////////////////
//static void* ZPG_Alloc(i32 numBytes, i32 tag);
//ZPG_EXPORT i32 ZPG_Free(void* ptr);
ZPG_EXPORT void ZPG_Fatal(const char* msg);

// resources
ZPG_EXPORT void* ZPG_Alloc(zpgSize numBytes, i32 tag);
ZPG_EXPORT i32 ZPG_Free(void* ptr);

// utility
ZPG_EXPORT f32 ZPG_Distance(ZPGPoint a, ZPGPoint b);

ZPG_EXPORT u8 ZPG_Grid_CountNeighourRingsAt(
    ZPGGrid* grid, i32 x, i32 y, u32* iterateCount);


static void ZPG_Grid_PrintPath(ZPGGrid* grid, ZPGPoint* points, i32 numPoints);
static void ZPG_Grid_PrintCellDefChars(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY);
static void ZPG_Grid_PrintAsci(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY);

static void ZPG_DrawOuterBorder(ZPGGrid* grid, ZPGGrid* stencil, u8 typeToPaint);

// Grid manipulation
static i32 ZPG_Grid_PositionToIndexSafe(ZPGGrid* grid, i32 x, i32 y);
static i32 ZPG_Grid_CheckStencilOccupied(ZPGGrid* grid, i32 x, i32 y);
static ZPGGrid* ZPG_Grid_CreateClone(ZPGGrid* original);
ZPG_EXPORT void ZPG_Grid_CalcStats(ZPGGrid* grid);
static ZPGNeighbours ZPG_Grid_CountNeighboursAt(ZPGGrid* grid, i32 x, i32 y);
static void ZPG_Grid_SetAll(ZPGGrid* grid, u8 val);

ZPG_EXPORT void ZPG_Grid_CountNeighourRings(
    ZPGGrid* grid, ZPGGrid* result, i32 ignoreValue, i32 bVerbose);
static void ZPG_SeedVoronoi(ZPGGrid* grid, i32 pointCount, i32* randSeed);

static void ZPG_Grid_PrintValues(ZPGGrid* grid, i32 digitCount, i32 bBlankZeroes);
static i32 ZPG_Grid_IsPositionSafe(ZPGGrid* grid, i32 x, i32 y);
static void ZPG_Grid_PrintCellDefChars(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY);
static void ZPG_Grid_PerlinToGreyscale(
    ZPGGrid* source, ZPGGrid* destination, u8 sourceChannel, u8 destChannel, i32 bSetAlpha);
ZPG_EXPORT ZPGGrid* ZPG_CreateGrid(i32 width, i32 height);
ZPG_EXPORT void ZPG_Grid_Clear(ZPGGrid* grid);
static void ZPG_FreeGrid(ZPGGrid* grid);
static void ZPG_Draw_BorderStencil(ZPGGrid* target);
static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height);
static i32 ZPG_ArePointsEqual(ZPGPoint a, ZPGPoint b);

ZPG_EXPORT void ZPG_Grid_SetValueWithStencil(ZPGGrid* grid, i32 x, i32 y, u8 val, ZPGGrid* stencil);

static ZPGPresetOutput ZPG_OutputFromAsciiGrid(ZPGGrid* grid);
static ZPGPresetOutput ZPG_OutputFromGreyscaleGrid(ZPGGrid* grid);

// Cell Types
ZPG_EXPORT ZPGCellTypeDef* ZPG_GetType(u8 cellTypeIndex);
ZPG_EXPORT ZPGCellTypeDef* ZPG_Grid_GetTypeDefAt(ZPGGrid* grid, i32 x, i32 y);

// random numbers...
// TODO: Tidy this crap up...
static i32 ZPG_GenerateSeed();
static f32 ZPG_Randf32(i32 index);
static i32 ZPG_RandArrIndex(i32 len, i32 seed);
static u8 ZPG_RandU8InRange(i32 index, u8 min, u8 max);
static f32 ZPG_Randf32InRange(i32 index, f32 min, f32 max);
static i32 ZPG_STDRandI32();
static u8 ZPG_STDRandU8();
static f32 ZPG_STDRandf32();
static f32 ZPG_STDRandomInRange(f32 min, f32 max);

static i32 ZPG_CheckSignature(char* sig, char** tokens, i32 numTokens);
// misc
// TODO: create via pointers like presets are.
static i32 ZPG_ExecSet(ZPGContext* ctx, char** tokens, i32 numTokens);
static i32 ZPG_ExecInitStack(ZPGContext* ctx, char** tokens, i32 numTokens);

static i32 ZPG_ExecStencil(ZPGContext* ctx, char** tokens, i32 numTokens);
static i32 ZPG_ExecRandomWalk(ZPGContext* ctx, char** tokens, i32 numTokens);
static i32 ZPG_ExecCaves(ZPGContext* ctx, char** tokens, i32 numTokens);

#endif // ZPG_INTERNAL_H