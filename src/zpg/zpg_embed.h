#ifndef ZPG_EMBED_H
#define ZPG_EMBED_H

static const char* embed_8x8_grid =
"########\r\n"
"#      #\r\n"
"#      #\r\n"
"#      #\r\n"
"#      #\r\n"
"#      #\r\n"
"#      #\r\n"
"########";

static const char* embed_8x8_grid_pillars =
"#### ####\r\n"
"#   e   #\r\n"
"# #   # #\r\n"
"# x   x #\r\n"
"         \r\n"
"# x   x #\r\n"
"# #   # #\r\n"
"#   s   #\r\n"
"#### ####";

static const char* embed_16x16_grid_pillars =
"######### #########\r\n"
"###             ###\r\n"
"##     #####     ##\r\n"
"#                 #\r\n"
"                   \r\n"
"#                 #\r\n"
"##     #####     ##\r\n"
"###             ###\r\n"
"######### #########";

static const char* embed_16x16_grid_left_start =
"##################\r\n"
"###            ###\r\n"
"##     ####     ##\r\n"
"#                #\r\n"
"#                 \r\n"
"#                #\r\n"
"##     ####     ##\r\n"
"###            ###\r\n"
"##################";

static const char* embed_16x16_grid_right_start =
"##################\r\n"
"###            ###\r\n"
"##     ####     ##\r\n"
"#                #\r\n"
"                 #\r\n"
"#                #\r\n"
"##     ####     ##\r\n"
"###            ###\r\n"
"##################";

static const char* embed_dead_simple = 
"                      \r\n"
" #################### \r\n"
" #################### \r\n"
" #################### \r\n"
" #################### \r\n"
" #####  ######  ##### \r\n"
" #####  ######  ##### \r\n"
" #################### \r\n"
" #################### \r\n"
"######################\r\n"
"######################\r\n"
" #################### \r\n"
" #################### \r\n"
" #####  ######  ##### \r\n"
" #####  ######  ##### \r\n"
" #################### \r\n"
" #################### \r\n"
" #################### \r\n"
" #################### \r\n"
" #################### \r\n"
"          ##          ";
static void ZPG_ScanRowForPrefabExits(
    ZPGGridPrefab* prefab,
    ZPGPoint dir,
    i32 startX,
    i32 startY,
    i32 iterateX,
    i32 iterateY,
    i32 limit)
{
    if (iterateX == 0 && iterateY == 0) { return; }
    if (prefab->numExits >= ZPG_MAX_PREFAB_EXITS) { return; }
    for (i32 i = 0; i < limit; ++i)
    {
        ZPGPoint p = { startX + (iterateX * i), startY + (iterateY * i) };
        //printf("\tCheck %d/%d\n", p.x, p.y);
        ZPGCellTypeDef* def = ZPG_GetCellTypeAt(prefab->grid, p.x, p.y);
        if (def->geometryType != ZPG_GEOMETRY_TYPE_PATH) { continue; }
        prefab->exits[prefab->numExits] = { p.x, p.y };
        prefab->exitDirs[prefab->numExits] = dir;
        prefab->numExits++;
        if (prefab->numExits >= ZPG_MAX_PREFAB_EXITS) { return; }
    }
}

static void ZPG_ScanPrefabForExits(ZPGGridPrefab* prefab, i32 bPrintExits)
{
    ZPGPoint min, max;
    min.x = 0;
    min.y = 0;
    max.x = prefab->grid->width - 1;
    max.y = prefab->grid->height - 1;
    printf("Scanning prefab for exits\n");
    // horizontal edges
    ZPG_ScanRowForPrefabExits(prefab, { 0, -1 }, min.x, min.y, 1, 0, prefab->grid->width);
    ZPG_ScanRowForPrefabExits(prefab, { 0, 1 }, min.x, max.y, 1, 0, prefab->grid->width);
    // vertical edges
    ZPG_ScanRowForPrefabExits(prefab, { -1, 0 }, min.x, min.y, 0, 1, prefab->grid->height);
    ZPG_ScanRowForPrefabExits(prefab, { 1, 0 }, max.x, min.y, 0, 1, prefab->grid->height);
    if (bPrintExits == YES)
    {
        for (i32 i = 0; i < prefab->numExits; ++i)
        {
            ZPGPoint p = prefab->exits[i];
            printf("Prefab exit at %d/%d\n", p.x, p.y);
        }
    }
}

static ZPGGridPrefab* ZPG_GetPrefabByIndex(i32 i)
{
   if (i < 0 || i >= g_numPrefabs) { return &g_prefabs[0]; }
   return &g_prefabs[i];
}

static void ZPG_SetupPrefab(ZPGGridPrefab* prefab, char* label, const char* asci)
{
    prefab->label = "embed_8x8_grid_pillars";
    i32 len = strlen(asci);
    prefab->grid = ZPG_ReadGridAsci((u8*)asci, len);
    ZPG_ScanPrefabForExits(prefab, NO);
}

static void ZPG_InitPrefabs()
{
    if (g_numPrefabs > 0) { return; }
    ZPG_SetupPrefab(&g_prefabs[g_numPrefabs++], "embed_16x16_grid_pillars", embed_16x16_grid_pillars);
    ZPG_SetupPrefab(&g_prefabs[g_numPrefabs++], "embed_8x8_grid_pillars", embed_8x8_grid_pillars);
    ZPG_SetupPrefab(&g_prefabs[g_numPrefabs++], "embed_16x16_grid_left_start", embed_16x16_grid_left_start);
    ZPG_SetupPrefab(&g_prefabs[g_numPrefabs++], "embed_16x16_grid_right_start", embed_16x16_grid_right_start);
    
    #if 0
    ZPGGridPrefab* prefab = NULL;
    prefab = &g_prefabs[g_numPrefabs];
    prefab->label = "embed_8x8_grid_pillars";
    g_numPrefabs++;
    const char* str = embed_8x8_grid_pillars;
    i32 len = strlen(str);
    prefab->grid = ZPG_ReadGridAsci((u8*)str, len);
    ZPG_ScanPrefabForExits(prefab, NO);
    #endif
}

#endif // ZPG_EMBED_H