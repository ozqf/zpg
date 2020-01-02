#ifndef ZPG_ENTITIES_H
#define ZPG_ENTITIES_H

struct ZPGEntityInfo
{
    ZPGPoint pos;
    i32 i;
    u8 tag;
    u8 entType;
    f32 avgDist;
};

static i32 ZPG_CompareEntsByDistance(const void* a, const void* b)
{
    return ((ZPGEntityInfo*)a)->avgDist > ((ZPGEntityInfo*)b)->avgDist ? 1 : -1;
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
    f32* avgDistWorking = (f32*)malloc(sizeof(f32) * totalDistances);

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
    qsort(ents, numEnts, sizeof(ZPGEntityInfo), ZPG_CompareEntsByDistance);
    
    // end of list is start
    ents[numEnts - 1].entType = ZPG_ENTITY_TYPE_START;
    ents[numEnts - 2].entType = ZPG_ENTITY_TYPE_END;
    // mark any remaining items to objectives
    i32 numRemainingEnts = numEnts - 2;
    for (i32 i = 0; i < numRemainingEnts; ++i)
    {
        ents[i].entType = ZPG_ENTITY_TYPE_OBJECTIVE;
    }

    // debug
    for (i32 i = 0; i < numEnts; ++i)
    {
        ZPGEntityInfo* info = &ents[i];
        printf("Ent %d type %d pos %d/%d avg dist %.3f\n",
            info->i, info->entType, info->pos.x, info->pos.y, info->avgDist);
    }


    // Cleanup
    free(avgDistWorking);
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
    grid->CalcStats();
    if (grid->stats.numObjectiveTags < 2)
    {
        printf("Abort: Only %d ents! Must have at least TWO for objectives\n",
            grid->stats.numObjectiveTags);
        return 1;
    }

    /////////////////////////////////////////////
    // allocate working arrays

    // empty tiles are all floor tiles with no objective
    ZPGPoint* emptyTiles = (ZPGPoint*)malloc(sizeof(ZPGPoint) * grid->stats.numFloorTiles);
    i32 numEmptyTiles = 0;
    // list of floor tiles with an 'objective'
    ZPGEntityInfo* objectives = (ZPGEntityInfo*)malloc(
        sizeof(ZPGEntityInfo) * grid->stats.numObjectiveTags);
    i32 numObjectives = 0;
    
    /////////////////////////////////////////////
    // build working arrays
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0;  x < grid->width; ++x)
        {
            ZPGCell* cell = grid->GetCellAt(x, y);
            if (cell->tile.type != ZPG_CELL_TYPE_FLOOR) { continue; }

            if (cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_START
                || cell->tile.tag == ZPG_CELL_TAG_RANDOM_WALK_END)
            {
                objectives[numObjectives].pos.x = x;
                objectives[numObjectives].pos.y = y;
                objectives[numObjectives].tag = cell->tile.tag;
                numObjectives++;
            }
            else
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
            ZPGCell* cell = grid->GetCellAt(info->pos.x, info->pos.y);
            cell->tile.entType = info->entType;
        }
    }
    // Randomly place enemies on remaining tiles:
    i32 numEnemies = numEmptyTiles / 4;
    i32 tilesCursor = numEmptyTiles;
    while ((numEnemies > 0))
    {
        //f32 r = ZPG_Randf32(*seed);
        //*seed += 1;
        //i32 randomIndex = (i32)(r % (f32)tilesCursor);
        i32 randomIndex = rand() % tilesCursor;

        // place enemy
        ZPGPoint* p = &emptyTiles[randomIndex];
        grid->GetCellAt(p->x, p->y)->tile.entType = ZPG_ENTITY_TYPE_ENEMY;
        tilesCursor--;

        // Reduce usable tiles
        if (randomIndex == (tilesCursor - 1))
        {
            // end of array
            tilesCursor--;
        }
        else
        {
            /* code */
            // swap last item with item just used
            emptyTiles[randomIndex] = emptyTiles[tilesCursor - 1];
            // reduce
            tilesCursor--;
        }
        

        numEnemies--;
    }

    // clean up working arrays
    free(emptyTiles);
    free(objectives);
    return 0;
}

#endif // ZPG_ENTITIES_H