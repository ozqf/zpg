#ifndef ZPG_CELL_TYPES_H
#define ZPG_CELL_TYPES_H

#include "zpg_internal.h"
#include "zpg.cpp"

static u8 ZPG_GetGeometryType(u8 cellTypeIndex)
{
    if (cellTypeIndex < 0 || cellTypeIndex >= 256)
    { return ZPG_GEOMETRY_TYPE_PATH; }
    return g_types[cellTypeIndex].geometryType;
}

static u8 ZPG_GetCategory(u8 cellTypeIndex)
{
    if (cellTypeIndex < 0 || cellTypeIndex >= 256)
    { return ZPG_CELL_CATEGORY_NONE; }
    return g_types[cellTypeIndex].category;
}

static ZPGCellTypeDef* ZPG_GetDefaultType()
{
    return &g_types[0];
}

static ZPGCellTypeDef* ZPG_GetType(u8 cellTypeIndex)
{
    if (cellTypeIndex < 0 || cellTypeIndex >= 256)
    { return &g_types[0]; }
    return &g_types[cellTypeIndex];
}

static ZPGCellTypeDef* ZPG_GetTypeByAsci(u8 asciChar)
{
    for (i32 i = 0; i < ZPG_NUM_TYPES; ++i)
    {
        if (g_types[i].asciChar == asciChar)
        {
            return &g_types[i];
        }
    }
    return ZPG_GetDefaultType();
}

static void ZPG_AddCellType(
    u8 index,
    u8 geometryType,
    u8 category,
    u8 asciChar,
    ZPGColour colour,
    char* label
    )
{
    ZPGCellTypeDef* def = &g_types[index];
    def->value = index;
    def->geometryType = geometryType;
    def->category = category;
    def->asciChar = asciChar;
    def->colour = colour;
    def->label = label;
    //printf("Init cell def %d (%s) geo %d, cat %d, display %c\n",
    //    index, label, geometryType, category, asciChar);
    g_numTypes++;
}

static void ZPG_InitCellTypes()
{
    memset(g_types, 0, sizeof(g_types));
    ZPG_AddCellType(ZPG_CELL_TYPE_PATH, ZPG_GEOMETRY_TYPE_PATH,
        ZPG_CELL_CATEGORY_NONE, ' ', ZPG_DARK_GREY, "Path");
    ZPG_AddCellType(ZPG_CELL_TYPE_WALL, ZPG_GEOMETRY_TYPE_SOLID,
        ZPG_CELL_CATEGORY_NONE, '#', ZPG_LIGHT_GREY, "Solid");
    ZPG_AddCellType(ZPG_CELL_TYPE_VOID, ZPG_GEOMETRY_TYPE_VOID,
        ZPG_CELL_CATEGORY_NONE, '.', ZPG_BLUE, "Void");

    ZPG_AddCellType(ZPG_CELL_TYPE_START, ZPG_GEOMETRY_TYPE_PATH,
        ZPG_CELL_CATEGORY_OBJECTIVE, 's', ZPG_GREEN, "Start");
    ZPG_AddCellType(ZPG_CELL_TYPE_END, ZPG_GEOMETRY_TYPE_PATH,
        ZPG_CELL_CATEGORY_OBJECTIVE, 'e', ZPG_GREEN, "End");
    ZPG_AddCellType(ZPG_CELL_TYPE_KEY, ZPG_GEOMETRY_TYPE_PATH,
        ZPG_CELL_CATEGORY_OBJECTIVE, 'k', ZPG_GREEN, "Key");
    ZPG_AddCellType(ZPG_CELL_TYPE_ENEMY, ZPG_GEOMETRY_TYPE_PATH,
        ZPG_CELL_CATEGORY_ENEMY, 'x', ZPG_RED, "Enemy");


    //printf("%d Cell Types defined\n", g_numTypes);
}

#endif // ZPG_CELL_TYPES_H