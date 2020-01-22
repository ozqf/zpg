#ifndef ZPG_INTERNAL_H
#define ZPG_INTERNAL_H

#include "../zpg.h"
#include <stdio.h>

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

static ZPGGrid* ZPG_CreateGrid(i32 width, i32 height);
static ZPGGrid* ZPG_CreateBorderStencil(i32 width, i32 height);

#endif // ZPG_INTERNAL_H