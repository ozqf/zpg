
#include "zpg_entities.h"

struct ZPGEntityInfo
{
    ZPGPoint pos;
    i32 i;
    u8 tag;
    u8 entType; // TODO: rename to just type
    f32 avgDist;
};

static i32 ZPG_CompareEntsByDistance(const void* a, const void* b)
{
    return ((ZPGEntityInfo*)a)->avgDist > ((ZPGEntityInfo*)b)->avgDist ? 1 : -1;
}

static i32 ZPG_Grid_CountCardinalNeighbours(
    ZPGGrid* grid, i32 x, i32 y, u8 type, ZPGPoint* results)
{
    i32 count = 0;
    if (ZPG_GRID_POS_SAFE(grid, x - 1, y) && ZPG_GRID_GET(grid, x - 1, y) == type)
    {
        results[count] = { -1, 0 };
        count++;
    }
    if (ZPG_GRID_POS_SAFE(grid, x + 1, y) && ZPG_GRID_GET(grid, x + 1, y) == type)
    {
        results[count] = { 1, 0 };
        count++;
    }
    if (ZPG_GRID_POS_SAFE(grid, x, y - 1) && ZPG_GRID_GET(grid, x, y - 1) == type)
    {
        results[count] = { 0, -1 };
        count++;
    }
    if (ZPG_GRID_POS_SAFE(grid, x, y + 1) && ZPG_GRID_GET(grid, x, y + 1) == type)
    {
        results[count] = { 0, 1 };
        count++;
    }
    /*
    ZPGCell* cell;
    // left
    cell = ZPG_Grid_GetCellAt(grid, x - 1, y);
    if (cell && cell->tile.type == type)
    { results[count] = { -1, 0 }; count++; }
    // right
    cell = ZPG_Grid_GetCellAt(grid, x + 1, y);
    if (cell && cell->tile.type == type)
    { results[count] = { 1, 0 }; count++; }
    // up
    cell = ZPG_Grid_GetCellAt(grid, x, y - 1);
    if (cell && cell->tile.type == type)
    { results[count] = { 0, -1 }; count++; }
    // down
    cell = ZPG_Grid_GetCellAt(grid, x, y + 1);
    if (cell && cell->tile.type == type)
    { results[count] = { 0, 1 }; count++; }
    */
    return count;
}

static void ZPG_AnalyseCellForEntities(
    ZPGGrid* grid, i32 x, i32 y, ZPGGrid* result, i32* seed)
{
    // ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
    // if (cell == NULL) { printf("Grid cell is null\n"); return; }
    // ZPGCell* target = ZPG_Grid_GetCellAt(result, x, y);
    // if (target == NULL) { printf("target cell is null\n"); return; }
    // ZPGCellTypeDef* def = ZPG_GetType(cell->tile.type);

    if (!ZPG_GRID_POS_SAFE(grid, x, y)) { return; }

    ZPGCellTypeDef* def = ZPG_GetType(ZPG_GRID_GET(grid, x, y));
    if (def == NULL) { printf("Cell type def is null\n");return;}
    ZPGPoint dirs[4];
    u8 count = 0;
    u32 iterations = 0;
    switch (def->geometryType)
    {
        case ZPG_GEOMETRY_TYPE_PATH:
        count = ZPG_Grid_CountNeighourRingsAt(grid, x, y, &iterations);
        ZPG_Grid_SetValueWithStencil(result, x, y, (u8)count, NULL);
        break;
        case ZPG_GEOMETRY_TYPE_SOLID:
        // TODO: How to record for later usage that this cell
        // could be used for a hidden monster trap
        count = (u8)ZPG_Grid_CountCardinalNeighbours(
            grid, x, y, ZPG_GEOMETRY_TYPE_PATH, dirs);
        ZPG_Grid_SetValueWithStencil(result, x, y, (u8)count, NULL);
        /*for (i32 i = 0; i < count; ++i)
        {
            ZPG_Grid_SetValueWithStencil(result, x, y, (u8)count, NULL);
        }*/
        break;
        case ZPG_GEOMETRY_TYPE_VOID:
        count = (u8)ZPG_Grid_CountCardinalNeighbours(
            grid, x, y, ZPG_GEOMETRY_TYPE_PATH, dirs);
        ZPG_Grid_SetValueWithStencil(result, x, y, (u8)count + 4, NULL);
        
        break;
        default: return;
    }
}

/**
 * -- Solid tiles --
 * > Count any neighbouring path tiles. Potentially makes this a good position for a trap
 * or ambush enemy.
 * -- Void tiles --
 * > If next to a path tile - could be a jump-onto-path ambush enemy
 * > If in cardinal line of sight of a path tile - could be a flying enemy.
 * -- Path tiles --
 * > If count neighbouring path tile rings. 1 ring == larger monster, 2+ could be a boss.
 */
static void ZPG_AnalyseForEntities(ZPGGrid* grid, ZPGGrid* result, i32* seed)
{
    printf("Analysing grid for entities\n");
    ZPG_Grid_Clear(result);
    ZPG_BEGIN_GRID_ITERATE(grid)
        ZPG_AnalyseCellForEntities(grid, x, y, result, seed);
    ZPG_END_GRID_ITERATE
    printf("Entity analysis result:\n");
    //ZPG_Grid_PrintValues(result, 1, YES);
}

/**
 * Objectives are the critical parts of the path from start to end.
 * other 'objective entities are places to place, say, loot or perhaps keys.
 * Return none zero if something went wrong
 */
static i32 ZPG_PlaceScatteredObjectives(ZPGGrid* grid, ZPGEntityInfo* ents, i32 numEnts)
{
    if (numEnts < 2)
    {
        printf("Abort: Only %d ents! Must have at least TWO for objectives\n", numEnts);
        return 1;
    }
    // record distances from each tile to every other tile here
    i32 totalDistances = numEnts * numEnts;
    f32* avgDistWorking = (f32*)ZPG_Alloc(sizeof(f32) * totalDistances, ZPG_MEM_TAG_FLOATS);

    // calculate distances from ent to ent
    for (i32 i = 0; i < numEnts; ++i)
    {
        for (i32 j = 0; j < numEnts; ++j)
        {
            i32 distIndex = (i * numEnts) + j;
            f32 dist = 0;
            if (i != j)
            {
                dist = ZPG_Distance(ents[i].pos, ents[j].pos);
            }
            avgDistWorking[distIndex] = dist;

        }
    }

    // calculate averages
    for (i32 i = 0; i < numEnts; ++i)
    {
        f32 avg = 0;
        for (i32 j = 0; j < numEnts; ++j)
        {
            i32 distIndex = (i * numEnts) + j;
            avg += avgDistWorking[distIndex];
        }
        avg /= numEnts;
        ents[i].avgDist = avg;
        ents[i].i = i;
    }
    // sort list lowest to highest average distance
    ZPG_QSORT(ents, numEnts, ZPGEntityInfo, ZPG_CompareEntsByDistance);
    
    // end of list is start
    ents[numEnts - 1].entType = ZPG_CELL_TYPE_START;// ZPG_ENTITY_TYPE_START;
    ents[numEnts - 2].entType = ZPG_CELL_TYPE_END;// ZPG_ENTITY_TYPE_END;
    // mark any remaining items to objectives
    i32 numRemainingEnts = numEnts - 2;
    for (i32 i = 0; i < numRemainingEnts; ++i)
    {
        ents[i].entType = ZPG_CELL_TYPE_KEY;
    }

    #if 0 // debug
    for (i32 i = 0; i < numEnts; ++i)
    {
        ZPGEntityInfo* info = &ents[i];
        printf("Ent %d type %d pos %d/%d avg dist %.3f\n",
            info->i, info->entType, info->pos.x, info->pos.y, info->avgDist);
    }
    #endif

    // Cleanup
    ZPG_Free(avgDistWorking);
    return 0;
}

/**
 * returns none zero if something went wrong
 */
static i32 ZPG_PlaceScatteredEntities(ZPGGrid* grid, i32* seed)
{
    /*
    Tasks:
    > Build stats of grid and alloc working arrays

    > Main route
        > Scan for objective tags
            > If none are found, randomly place some...somehow.
        > Measure distances between each. Item with greatest average distance is the start.
        > Randomly select one of the remaining objectives as the end.
        > Others become generic 'keys' for opening the end tile.
    > Other objects:
        > Build list of all floor tiles not within a specific range of the start tile
        > Pop a cell from this list and place an object there.
            > Repeat until desired count is placed.
    */
    /////////////////////////////////////////////
    // calculate capacity for working arrays
    ZPG_Grid_CalcStats(grid);
    if (grid->stats.numObjectiveTags < 2)
    {
        printf("Abort: Got %d ents - must have at least TWO for objectives\n",
            grid->stats.numObjectiveTags);
        return 1;
    }

    /////////////////////////////////////////////
    // allocate working arrays

    // empty tiles are all floor tiles with no objective
    ZPGPoint* emptyTiles = (ZPGPoint*)ZPG_Alloc(
        sizeof(ZPGPoint) * grid->stats.numFloorTiles, ZPG_MEM_TAG_POINTS);
    i32 numEmptyTiles = 0;
    // list of floor tiles with an 'objective'
    ZPGEntityInfo* objectives = (ZPGEntityInfo*)ZPG_Alloc(
        sizeof(ZPGEntityInfo) * grid->stats.numObjectiveTags, ZPG_MEM_TAG_ENTITIES);
    i32 numObjectives = 0;
    
    /////////////////////////////////////////////
    // build working arrays
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0;  x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_Grid_GetTypeDefAt(grid, x, y);
            if (def->category == ZPG_CELL_CATEGORY_OBJECTIVE)
            {
                objectives[numObjectives].pos.x = x;
                objectives[numObjectives].pos.y = y;
                numObjectives++;
            }
            else if (def->value == ZPG_CELL_TYPE_PATH)
            {
                emptyTiles[numEmptyTiles].x = x;
                emptyTiles[numEmptyTiles].y = y;
                numEmptyTiles++;
            }
        }
    }
    printf("Build entities found %d path tiles and %d objectives\n",
        numEmptyTiles, numObjectives);

    // generate objectives - if successful write entity types into grid
    if (ZPG_PlaceScatteredObjectives(grid, objectives, numObjectives) == NO)
    {
        for (i32 i = numObjectives - 1; i >= 0; --i)
        {
            ZPGEntityInfo* info = &objectives[i];
            ZPG_Grid_SetValueWithStencil(grid, info->pos.x, info->pos.y, info->entType, NULL);
        }
    }
    // Randomly place enemies on remaining tiles:
    i32 numEnemies = numEmptyTiles / 8;
    i32 tilesCursor = numEmptyTiles;
    while ((numEnemies > 0))
    {
        //f32 r = ZPG_Randf32(*seed);
        //*seed += 1;
        //i32 randomIndex = (i32)(r % (f32)tilesCursor);
        i32 randomIndex = rand() % tilesCursor;

        // place enemy
        ZPGPoint* p = &emptyTiles[randomIndex];
        ZPG_Grid_SetValueWithStencil(grid, p->x, p->y, ZPG_CELL_TYPE_ENEMY, NULL);
        tilesCursor--;

        // Reduce usable tiles
        if (randomIndex == (tilesCursor - 1))
        {
            // end of array
            tilesCursor--;
        }
        else
        {
            // swap last item with item just used
            emptyTiles[randomIndex] = emptyTiles[tilesCursor - 1];
            // reduce
            tilesCursor--;
        }
        

        numEnemies--;
    }

    // clean up working arrays
    ZPG_Free(emptyTiles);
    ZPG_Free(objectives);
    return 0;
}

extern "C" i32 ZPG_GenerateEntites(ZPGPresetCfg* cfg, ZPGGrid* target)
{
    
    if ((cfg->flags & ZPG_API_FLAG_NO_ENTITIES) != 0)
    {
        return NO;
    }
    i32 bVerbose = (cfg->flags & ZPG_API_FLAG_PRINT_WORKING);
    if (bVerbose)
    { printf("-- Grid Loaded --\ncreating entities\n"); }

    ZPGGrid* entData = ZPG_CreateGrid(target->width, target->height);
    ZPG_Grid_CountNeighourRings(target, entData, ZPG_CELL_TYPE_WALL, bVerbose);
    
    ZPG_AnalyseForEntities(target, entData, &cfg->seed);
    ZPG_PlaceScatteredEntities(target, &cfg->seed);
    return NO;
}

