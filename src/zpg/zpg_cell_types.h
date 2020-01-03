#ifndef ZPG_CELL_TYPES_H
#define ZPG_CELL_TYPES_H

#include "../zpg.h"
#include "zpg.cpp"

static void ZPG_AddCellType(
    u8 index,
    u8 geometryType,
    u8 category,
    u8 asciChar,
    char* label
    )
{
    ZPGCellTypeDef* def = &g_types[index];
    def->value = index;
    def->geometryType = geometryType;
    def->category = category;
    def->asciChar = asciChar;
    def->label = label;
    printf("Init cell def %d as %s\n", index, label);
    g_numTypes++;
}

static void ZPG_InitCellTypes()
{
    memset(g_types, 0, sizeof(g_types));
    ZPG_AddCellType(0, 1, 1, 1, NULL);

    printf("%d Cell Types defined\n", g_numTypes);
}

#endif // ZPG_CELL_TYPES_H