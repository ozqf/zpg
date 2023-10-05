#ifndef ZPG_COMMAND_H
#define ZPG_COMMAND_H

#include "zpg_internal.h"

static i32 GetParamAsInt(i32 index, char** tokens, i32 numTokens, i32 fail)
{
	if (index < 0 || index >= numTokens)
	{
		return fail;
	}
	return atoi(tokens[index]);
}

static f32 GetParamAsFloat(i32 index, char** tokens, i32 numTokens, f32 fail)
{
	if (index < 0 || index >= numTokens)
	{
		return fail;
	}
	return (f32)atof(tokens[index]);
}

static char* GetParamAsString(i32 index, char** tokens, i32 numTokens, char* fail)
{
	if (index < 0 || index >= numTokens)
	{
		return fail;
	}
	return tokens[index];
}

static ZPGGrid* GetParamAsGrid(
    ZPGContext* ctx, i32 index, char** tokens, i32 numTokens)
{
    i32 i = GetParamAsInt(index, tokens, numTokens, -1);
    if (i < 0 || i >= ctx->gridStack->numGrids)
    {
        return NULL;
    }
    return ctx->gridStack->grids[i];
}

static void VerboseGridValues(ZPGContext* c)
{
    if (c == NULL || c->grid == NULL) { return; }
    ZPG_Grid_PrintChannelValues(c->grid, YES);
}

static void VerboseGridAscii(ZPGContext* c)
{
    if (c == NULL || c->grid == NULL) { return; }
    if (c->verbosity > 0) { ZPG_Grid_PrintCellDefChars(c->grid, 0, 0, 0); }
}

static i32 ZPG_ExecSet(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (numTokens < 2)
    {
        printf("Set needs name of what to set and a value!\n");
        return ZPG_ERROR_MISSING_PARAMETER;
    }
    char* name = GetParamAsString(0, tokens, numTokens, "");
    i32 value = GetParamAsInt(1, tokens, numTokens, 0);
    if (ZPG_STREQL(name, "verbosity"))
    {
        ctx->verbosity = value;
        printf("Set %s to %d\n", name, value);
    }
    else if (ZPG_STREQL(name, "walk_start_x"))
    {
        ctx->walkCfg.startX = value;
    }
    else if (ZPG_STREQL(name, "walk_start_y"))
    {
        ctx->walkCfg.startY = value;
    }
    else if (ZPG_STREQL(name, "walk_start_y"))
    {
        ctx->walkCfg.startY = value;
    }
    else if (ZPG_STREQL(name, "walk_dir_x"))
    {
        ctx->walkCfg.dirX = value;
    }
    else if (ZPG_STREQL(name, "walk_dir_y"))
    {
        ctx->walkCfg.dirY = value;
    }
    else if (ZPG_STREQL(name, "walk_paint_value"))
    {
        ctx->walkCfg.typeToPaint = value & 0xFF;
    }
    else if (ZPG_STREQL(name, "walk_count"))
    {
        ctx->walkCfg.tilesToPlace = value;
    }
    else if (ZPG_STREQL(name, "walk_big_chance"))
    {
        ctx->walkCfg.bigRoomChance = GetParamAsFloat(1, tokens, numTokens, 0);
    }
    else if (ZPG_STREQL(name, "grid"))
    {
        i32 i = GetParamAsInt(value, tokens, numTokens, -1);
        if (i == -1) { return ZPG_ERROR_INVALID_PARAMETER; }
        i32 numGrids = ctx->gridStack->numGrids;
        if (i < numGrids || i >= numGrids)
        {
            if (ctx->verbosity > 0)
            {
                printf("Grid index %d out of bounds\n", value);
                return ZPG_ERROR_INVALID_PARAMETER;
            }
        }
        ctx->gridIndex = i;
        ctx->grid = ctx->gridStack->grids[ctx->gridIndex];
        if (ctx->verbosity > 0)
        {
            printf("Set target grid to index %d\n", ctx->gridIndex);
        }
    }
    else if (ZPG_STREQL(name, "stencil"))
    {
        ctx->stencil = GetParamAsGrid(ctx, value, tokens, numTokens);
        if (ctx->verbosity > 0)
        { printf("Set stencil grid to index %d\n", value); }
    }
    else
    {
        printf("Unknown set target '%s'\n", name);
    }
    return ZPG_ERROR_NONE;
}

static ZPGGrid* GetGridByIndex(ZPGContext* ctx, i32 i)
{
    return NULL;
}

static i32 ZPG_ExecPrintPrefabs(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPG_PrintPrefabs();
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecSetSeed(ZPGContext* ctx, char** tokens, i32 numTokens)
{
	if (numTokens < 1)
	{
        ctx->seed = ZPG_GenerateSeed();
	}
    else
    {
        ctx->seed = GetParamAsInt(0, tokens, numTokens, 0);
    }
    if (ctx->verbosity > 0)
    {
        printf("Seeding srand: %d\n", ctx->seed);
    }
    srand(ctx->seed);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridScatter(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    i32 seedCount = GetParamAsInt(0, tokens, numTokens, 12);
    ZPGPointList points = ZPG_SeedByCount(ctx->grid, ctx->stencil, seedCount, &ctx->seed);
    if (ctx->points.points != NULL)
    {
        if (ctx->verbosity > 0)
        {
            printf("Freeing %d points\n", ctx->points.max);
        }
        ZPG_Free(ctx->points.points);
    }
    ctx->points = points;
    if (ctx->verbosity > 0)
    {
        i32 total = ctx->grid->width * ctx->grid->height;
        f32 ratio = (f32)ctx->points.count / (f32)total;
        printf("Drew %d points over %d cells. Ratio to grid size of %.3f\n",
            ctx->points.count, total, ratio);
        ZPG_PrintPointsAsGrid(points.points, points.count, ctx->grid->width, ctx->grid->height);
    }
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridPrintAscii(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (grid == NULL)
    {
        printf("Grid specified not found in stack\n");
        return 1;
    }
    ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridPrintValues(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (grid == NULL)
    {
        printf("Grid specified not found in stack\n");
        return 1;
    }
    ZPG_Grid_PrintChannelValues(grid, YES);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecAsciiGridToOutput(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL) { return 1; }
    // by default will write to output 0 if none was specified!
    zpgHandle outputIndex = GetParamAsInt(0, tokens, numTokens, 0);
    u8* ptr;
    zpgSize numBytes;
    ZPG_WriteGridAscii(ctx->grid, &ptr, &numBytes);
    if (ctx->verbosity > 0)
    {
        printf("Writing %lld bytes to output %d\n", numBytes, outputIndex);
    }
    zpgError err = ZPG_AddOutput(0, outputIndex, ZPG_OUTPUT_FORMAT_ASCI_GRID, ptr, numBytes);
    if (err != 0)
    {
        if (ctx->verbosity > 0)
        {
            printf("Failed to write output. Code %d\n", err);
        }
        return 1;
    }
    if (ctx->verbosity > 0)
    {
        printf("Created output %d with %lld bytes\n", outputIndex, numBytes);
    }
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecInitStack(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    printf("Init grid stack\n");
    if (numTokens < 3)
    {
        printf("Missing args for gridstack\n");
        printf("<stack_size> <grid width> <grid height>\n");
        return 1;
    }
	if (ctx->gridStack != NULL)
	{
		printf("Freeing existing stack!\n");
		ZPG_FreeGridStack(ctx->gridStack);
		ctx->gridStack = NULL;
	}
    i32 size = atoi(tokens[0]);
    i32 width = atoi(tokens[1]);
    i32 height = atoi(tokens[2]);
    ctx->gridStack = ZPG_CreateGridStack(width, height, size);
    printf("Created stack size %d, w/h %d/%d\n", size, width, height);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecCreateStack(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (numTokens < 2)
    {
        if (ctx->verbosity > 0)
        {
            printf("Must specify width, height, stack name and at least one grid name\n");
        }
        return ZPG_ERROR_MISSING_PARAMETER;
    }
    if (ctx->verbosity > 0)
    {
        printf("Created stack %s with grids: ", tokens[0]);
        for (i32 i = 1; i < numTokens; ++i) { printf("%s, ", tokens[i]); }
        printf("\n");
    }
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridSetAll(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->gridStack == NULL)
    {
        printf("Grid stack has not been initialiseda\n");
        return 1;
    }
	if (!ZPG_CheckSignature("ii", tokens, numTokens))
	{
		return 1;
    }
	i32 gridIndex = atoi(tokens[0]);
    if (gridIndex < 0 || gridIndex >= ctx->gridStack->numGrids)
    {
        printf("Grid index out of bounds\n");
        return ZPG_ERROR_OUT_OF_BOUNDS;
    }
    ZPGGrid* grid = ctx->gridStack->grids[gridIndex];
	u8 value = (u8)(atoi(tokens[1]) & 0xFF);
	ZPG_Grid_SetAll(grid, value);
    if (ctx->verbosity > 1)
	{
		ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
	}
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridCopy(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* source = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (ctx->grid == NULL || source == NULL)
    { return ZPG_ERROR_MISSING_PARAMETER; }
    ZPG_Grid_CopyWithStencil(source, ctx->grid, ctx->stencil);
    if (ctx->verbosity > 0) { printf("Copying grid\n"); }
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridCopyValue(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (numTokens < 4)
    {
        if (ctx->verbosity > 0)
        {
            printf("Grid copy params: sourceGridIndex, targetGridIndex, sourceValue, targetValue");
        }
        return ZPG_ERROR_MISSING_PARAMETER;
    }
    ZPGGrid* source = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (source == NULL) { return 1; }
    ZPGGrid* target = GetParamAsGrid(ctx, 1, tokens, numTokens);
    if (target == NULL) { return 1; }
    u8 sourceValue = (u8)GetParamAsInt(2, tokens, numTokens, 1);
    u8 targetValue = (u8)GetParamAsInt(3, tokens, numTokens, 0);
    ZPG_Grid_CopySpecificValue(source, target, sourceValue, targetValue);

    VerboseGridAscii(ctx);
	return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridReplaceValue(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (numTokens < 3)
    {
        if (ctx->verbosity > 0)
        {
            printf("Grid replace params: sourceGridIndex, sourceValue, targetValue\n");
        }
        return ZPG_ERROR_MISSING_PARAMETER;
    }
    ZPGGrid* source = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (source == NULL) { return ZPG_ERROR_MISSING_PARAMETER; }
    u8 sourceValue = (u8)GetParamAsInt(1, tokens, numTokens, 1);
    u8 targetValue = (u8)GetParamAsInt(2, tokens, numTokens, 0);
    ZPG_Grid_ReplaceValue(source, ctx->stencil, sourceValue, targetValue);
    VerboseGridAscii(ctx);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridToBinary(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }
    u8 cutoff = GetParamAsInt(0, tokens, numTokens, 1) & 0xff;
    printf("Collapse to binary, cutoff value is %d\n", cutoff);
    ZPG_Grid_CollapseToBinary(ctx->grid, ctx->stencil, cutoff);
    VerboseGridAscii(ctx);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridFlipBinary(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }
    ZPG_Grid_FlipBinary(ctx->grid, ctx->stencil);
    VerboseGridAscii(ctx);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridDrawRect(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGRect r;
    r.min.x = GetParamAsInt(0, tokens, numTokens, 0);
    r.min.y = GetParamAsInt(1, tokens, numTokens, 0);
    r.max.x = GetParamAsInt(2, tokens, numTokens, 4);
    r.max.y = GetParamAsInt(3, tokens, numTokens, 4);
    u8 value = GetParamAsInt(4, tokens, numTokens, 1) & 0xff;
    ZPG_FillRectWithStencil(ctx->grid, ctx->stencil, r.min, r.max, value);
    VerboseGridValues(ctx);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecRandomWalk(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->gridStack == NULL)
    {
        printf("Grid stack has not been initialised\n");
        return 1;
    }
    if (ctx->grid == NULL)
    {
        if (ctx->verbosity > 0)
        {
            printf("No working grid set! use set grid <index>\n");
        }
        return 1;
    }
	
    ZPGWalkCfg* cfg = &ctx->walkCfg;
    cfg->seed = ctx->seed;
    ZPGPoint dir = { cfg->dirX, cfg->dirY };
    ctx->lastStop = ZPG_GridRandomWalk(ctx->grid, NULL, NULL, cfg, dir);
    ctx->seed = ctx->walkCfg.seed;
    // ctx->seed = cfg.seed;
    if (ctx->verbosity > 0)
	{
		ZPG_Grid_PrintCellDefChars(ctx->grid, '*', cfg->startX, cfg->startY);
        printf("Stopped at %d, %d\n", ctx->lastStop.x, ctx->lastStop.y);
	}
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecGridDrawPoints(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }
    if (ctx->points.points == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }
    if (ctx->points.count <= 0) { return 1; }
    ZPG_Grid_DrawPoints(
        ctx->grid, ctx->points.points, ctx->points.count, 1);
    if (ctx->verbosity > 0)
    {
        ZPG_Grid_PrintChannelValues(ctx->grid, YES);
        i32 total = ctx->grid->width * ctx->grid->height;
        f32 ratio = (f32)ctx->points.count / (f32)total;
        printf("Drawing %d points. Ratio to grid size of %.3f\n",
            ctx->points.count, ratio);
    }
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecCaves(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }

    u8 solid = (u8)GetParamAsInt(2, tokens, numTokens, 1);
    u8 empty = (u8)GetParamAsInt(3, tokens, numTokens, 0);
    ZPGCellRules rules = ZPG_DefaultCaveRules();
    rules.filledValue = solid;
    rules.emptyValue = empty;
    ZPG_IterateCaves(ctx->grid, ctx->stencil, NULL, rules);
    VerboseGridAscii(ctx);
    return ZPG_ERROR_NONE;

    // ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    // if (grid == NULL) { return 1; }
    // ZPGGrid* stencil = GetParamAsGrid(ctx, 1, tokens, numTokens);
    // u8 solid = (u8)GetParamAsInt(2, tokens, numTokens, 1);
    // u8 empty = (u8)GetParamAsInt(3, tokens, numTokens, 0);
    // ZPGCellRules rules = ZPG_DefaultCaveRules();
    // rules.filledValue = solid;
    // rules.emptyValue = empty;
    // printf("Seed before cave fill: %d\n", ctx->seed);
    // ZPG_FillCaves(grid, stencil, rules, &ctx->seed, NO);
    // printf("Seed after cave fill: %d\n", ctx->seed);
    // if (ctx->verbosity > 0)
	// {
	// 	ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
	// }
    // return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecVoronoi(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    // ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (ctx->grid == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }
    // i32 numPoints = GetParamAsInt(0, tokens, numTokens, 24);
    // ZPG_SeedVoronoi(ctx->grid, ctx->stencil, numPoints, &ctx->seed);
    if (ctx->points.points == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }
    ZPG_Voronoi(ctx->grid, ctx->stencil, ctx->points.points, ctx->points.count);
    if (ctx->verbosity > 0)
	{
        ZPG_Grid_PrintChannelValues(ctx->grid, YES);
        ZPG_Grid_PrintRegionEdges(ctx->grid);
	}
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecBuildRooms(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL) { return ZPG_ERROR_MISSING_PARAMETER; }
    i32 w = ctx->grid->width;
    i32 h = ctx->grid->height;
    ZPGGrid* tagGrid = ZPG_CreateGrid(w, h);
    ZPGRoom* rooms = NULL;
    i32 numRooms = ZPG_Grid_FindRooms(ctx->grid, tagGrid, &rooms);
    ZPGGrid* roomFlags = ZPG_CreateGrid(w, h);
    
    //ZPG_ListRooms(rooms, numRooms);
    ZPG_Rooms_MergeOneDimensionalRooms(ctx->grid, roomFlags, rooms, numRooms);

    if (ctx->verbosity > 0)
    { printf("Found %d rooms\n", numRooms); }
    
    ZPGGrid* doorFlags = ZPG_CreateGrid(w, h);
    
    ZPG_Room_BuildNeighbourFlags(ctx->grid, roomFlags);
    if (ctx->verbosity > 0)
    {
        printf("Room neighbour flags:\n");
        ZPG_Grid_PrintValues(roomFlags, 3, YES);
    }

    // find all potential doorways
	ZPGDoorwaySet doors = ZPG_FindAllRoomConnectionPoints(
		ctx->grid, rooms, numRooms, ctx->verbosity);
    
    // select one doorway per connection
    ZPG_Rooms_AssignDoorways(doorFlags, doors, &ctx->seed, ctx->verbosity);
    VerboseGridValues(ctx);
    
	const i32 scale = 4;
    ZPGGrid* canvas = ZPG_CreateGrid(w * scale, h * scale);

    ZPG_Rooms_PaintGeometry(
        ctx->grid, canvas, roomFlags, doorFlags, rooms, numRooms, NO, scale);
    
    if (ctx->verbosity > 0) { ZPG_Grid_PrintCellDefChars(canvas, 0, 0, 0); }
    
    // cleanup
    ZPG_FreeGrid(tagGrid);
    ZPG_FreeGrid(doorFlags);
    ZPG_FreeGrid(roomFlags);
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecStencil(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (grid == NULL) { return ZPG_ERROR_TARGET_IS_NOT_SET; }
    ZPGGrid* stencil = GetParamAsGrid(ctx, 1, tokens, numTokens);

    char* type = GetParamAsString(2, tokens, numTokens, "border");
    if (ZPG_STRCMP(type, "bisect") == 0)
    {
        ZPG_Draw_HorizontalBisectStencil(grid);
    }
    else
    {
        ZPG_DrawOuterBorder(grid, stencil, 1);
    }
	VerboseGridAscii(ctx);
    
    return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecPlotSegmentedPath(ZPGContext* ctx, char** tokens, i32 numTokens)
{
	return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecSaveGridAsciiToTextFile(ZPGContext* ctx, char** tokens, i32 numTokens)
{
	if (!ZPG_CheckSignature("it", tokens, numTokens))
	{ return 1; }
	ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
	if (grid == NULL)
	{ return 1; }
	char* fileName = GetParamAsString(1, tokens, numTokens, "output.txt");
	ZPG_WriteGridAciiToFile(grid, fileName);
	return ZPG_ERROR_NONE;
}

static i32 ZPG_ExecSaveGridBytes(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    char* fileName = GetParamAsString(0, tokens, numTokens, "output.bin");
    u8* ptr;
    zpgSize size;
    zpgError err;
    err = ZPG_WriteGridBinary(ctx->grid, &ptr, &size);
    if (err != ZPG_ERROR_NONE)
    {
        if (ctx->verbosity > 0) { printf("Failed to write grid data: %dn", err); }
        return ZPG_ERROR_UNKNOWN;
    }
    err = ZPG_WriteBlob(fileName, ptr, size);
    if (err != ZPG_ERROR_NONE)
    {
        if (ctx->verbosity > 0)
        { printf("Failed to write %lld grid bytes to %s. Code: %d\n", size, fileName, err); }
        return ZPG_ERROR_UNKNOWN;
    }
    ZPG_Free(ptr);
    return ZPG_ERROR_NONE;
}

#endif // ZPG_COMMAND_H