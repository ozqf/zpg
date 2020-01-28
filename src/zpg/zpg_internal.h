#ifndef ZPG_INTERNAL_H
#define ZPG_INTERNAL_H

#include "../zpg.h"
#include <stdio.h>

#define ZPG_ABS(value) (value = (value >= 0 ? value : -value))
#define ZPG_MIN(x, y) ((x) < (y) ? (x) : (y))
#define ZPG_MAX(x, y) ((x) > (y) ? (x) : (y))

#define ZPG_STRLEN(ptrToCharArray) \
strlen(ptrToCharArray)

#define ZPG_STRCMP(stringA, stringB) \
strcmp(##stringA##, stringB##)

#define ZPG_MEMSET(ptrToMemory, valueToSet, numBytesToSet) \
memset(##ptrToMemory##, valueToSet##, numBytesToSet##)

#define ZPG_MEMCPY(destinationPtr, sourcePtr, numBytesToCopy) \
memcpy(##destinationPtr##, sourcePtr##, numBytesToCopy##)

static zpg_allocate_fn g_ptrAlloc = NULL;
static zpg_free_fn g_ptrFree = NULL;

#define ZPG_PARAM_NULL(paramPtr, failureReturnVal) \
if (##paramPtr == NULL) { printf("Param %s was null\n", #paramPtr##); return failureReturnVal; }

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
    union
    {
        i32 flag;
        zpg_param_fn func;    
    } data;
};

static i32 g_numParamTypes = 0;
static ZPGParam g_paramTypes[64];

//////////////////////////////////////////
// Functions
//////////////////////////////////////////
static void ZPG_DrawOuterBorder(ZPGGrid* grid, ZPGGrid* stencil, u8 typeToPaint);

static ZPGGrid* ZPG_CreateGrid(i32 width, i32 height);
static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height);
static void ZPG_FreeGrid(ZPGGrid* grid);

static i32 ZPG_ExecSet(char** tokens, i32 numTokens);

#endif // ZPG_INTERNAL_H