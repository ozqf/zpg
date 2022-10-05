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

static i32 ZPG_ExecSet(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (numTokens < 2)
    {
        printf("Set needs name of what to set and a value!\n");
        return 1;
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
        ctx->grid = GetParamAsGrid(ctx, value, tokens, numTokens);
        if (ctx->verbosity > 0)
        {
            if (ctx->grid != NULL)
            { printf("Set target grid to index %d\n", value); }
            else
            { printf("Grid index %d out of bounds\n", value); }
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
    return 0;
}

static i32 ZPG_ExecPrintPrefabs(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPG_PrintPrefabs();
    return 0;
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
    printf("Seed: %d\n", ctx->seed);
    srand(ctx->seed);
    return 0;
}

static i32 ZPG_ExecGridPrint(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (grid == NULL)
    {
        printf("Grid specified not found in stack\n");
        return 1;
    }
    ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
    return 0;
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
    return 0;
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
    printf("Grid Set All\n");
	i32 gridIndex = atoi(tokens[0]);
    if (gridIndex < 0 || gridIndex >= ctx->gridStack->numGrids)
    {
        printf("Grid index out of bounds\n");
        return 1;
    }
    ZPGGrid* grid = ctx->gridStack->grids[gridIndex];
	u8 value = (u8)(atoi(tokens[1]) & 0xFF);
	ZPG_Grid_SetAll(grid, value);
    if (ctx->verbosity > 0)
	{
		ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
	}
    return 0;
}

static i32 ZPG_ExecGridCopyValue(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* source = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (source == NULL) { return 1; }
    ZPGGrid* target = GetParamAsGrid(ctx, 1, tokens, numTokens);
    if (target == NULL) { return 1; }
    u8 sourceValue = (u8)GetParamAsInt(2, tokens, numTokens, 1);
    u8 targetValue = (u8)GetParamAsInt(3, tokens, numTokens, 0);
    ZPG_Grid_CopySpecificValue(source, target, sourceValue, targetValue);
    if (ctx->verbosity > 0)
	{
		ZPG_Grid_PrintCellDefChars(target, 0, 0, 0);
	}
	return 0;
}

static i32 ZPG_ExecGridToBinary(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL)
    {
        return 1;
    }
    ZPG_Grid_CollapseToBinary(ctx->grid, ctx->stencil, 1);
    return 0;
}

static i32 ZPG_ExecGridFlipBinary(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (ctx->grid == NULL) { return 1; }
    ZPG_Grid_FlipBinary(ctx->grid, ctx->stencil);
    return 0;
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
	}
    printf("Stopped at %d, %d\n", ctx->lastStop.x, ctx->lastStop.y);
    return 0;
}

static i32 ZPG_ExecCaves(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (grid == NULL) { return 1; }
    ZPGGrid* stencil = GetParamAsGrid(ctx, 1, tokens, numTokens);
    u8 solid = (u8)GetParamAsInt(2, tokens, numTokens, 1);
    u8 empty = (u8)GetParamAsInt(3, tokens, numTokens, 0);
    ZPGCellRules rules = ZPG_DefaultCaveRules();
    rules.filledValue = solid;
    rules.emptyValue = empty;
    printf("Seed before cave fill: %d\n", ctx->seed);
    ZPG_FillCaves(grid, stencil, rules, &ctx->seed, NO);
    printf("Seed after cave fill: %d\n", ctx->seed);
    if (ctx->verbosity > 0)
	{
		ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
	}
    return 0;
}

static i32 ZPG_ExecVoronoi(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    // ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (ctx->grid == NULL) { return 1; }
    i32 numPoints = GetParamAsInt(0, tokens, numTokens, 24);
    ZPG_SeedVoronoi(ctx->grid, ctx->stencil, numPoints, &ctx->seed);
    if (ctx->verbosity > 0)
	{
        ZPG_Grid_PrintChannelValues(ctx->grid, YES);
        ZPG_Grid_PrintRegionEdges(ctx->grid);
	}
    return 0;
}

static i32 ZPG_ExecStencil(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
    if (grid == NULL) { return 1; }
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
	if (ctx->verbosity > 0)
	{
		ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
	}
    
    return 0;
}

static i32 ZPG_ExecPlotSegmentedPath(ZPGContext* ctx, char** tokens, i32 numTokens)
{
	return 0;
}

static i32 ZPG_ExecSaveGridToTextFile(ZPGContext* ctx, char** tokens, i32 numTokens)
{
	if (!ZPG_CheckSignature("it", tokens, numTokens))
	{ return 1; }
	ZPGGrid* grid = GetParamAsGrid(ctx, 0, tokens, numTokens);
	if (grid == NULL)
	{ return 1; }
	char* fileName = GetParamAsString(1, tokens, numTokens, "output.txt");
	ZPG_WriteGridAsAsci(grid, fileName);
	return 0;
}

#endif // ZPG_COMMAND_H