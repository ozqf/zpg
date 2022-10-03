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

static char* GetParamAsString(i32 index, char** tokens, i32 numTokens, char* fail)
{
	if (index < 0 || index >= numTokens)
	{
		return fail;
	}
	return tokens[index];
}

static i32 ZPG_ExecSet(ZPGContext* ctx, char** tokens, i32 numTokens)
{
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
    ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
    return 0;
}

static i32 ZPG_ExecRandomWalk(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    // i32 gridIndex = -1;
    // i32 
    if (ctx->gridStack == NULL)
    {
        printf("Grid stack has not been initialised\n");
        return 1;
    }
	// if (!ZPG_CheckSignature("iii", tokens, numTokens))
	// {
		// return 1;
	// }
	
    i32 gridIndex = GetParamAsInt(0, tokens, numTokens, 0);
    ZPGGrid* grid = ctx->gridStack->grids[gridIndex];
    ZPGWalkCfg cfg {};
	cfg.typeToPaint = GetParamAsInt(1, tokens, numTokens, ZPG_CELL_TYPE_VOID) & 0xFF;
	cfg.startX = GetParamAsInt(2, tokens, numTokens, grid->width / 2);
    cfg.startY = GetParamAsInt(3, tokens, numTokens,  grid->height / 2);
	cfg.tilesToPlace = GetParamAsInt(4, tokens, numTokens,  40);
	ZPGPoint dir = {
		GetParamAsInt(5, tokens, numTokens, 1),
		GetParamAsInt(6, tokens, numTokens, 0)
	};
    cfg.seed = 1;
    cfg.bPlaceObjectives = YES;
    cfg.bigRoomChance = 0.3f;
    cfg.bStepThrough = NO;
    ZPG_GridRandomWalk(grid, NULL, NULL, &cfg, dir);

    ZPG_Grid_PrintCellDefChars(grid, '*', cfg.startX, cfg.startY);
    return 0;
}

static i32 ZPG_ExecCaves(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    ZPGGrid* grid = ctx->gridStack->grids[0];
    ZPGCellRules rules = ZPG_DefaultCaveRules();
    ZPG_FillCaves(grid, NULL, rules, &ctx->seed, NO);
    ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
    return 0;
}

static i32 ZPG_ExecStencil(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (!ZPG_CheckSignature("it", tokens, numTokens))
	{
		return 1;
	}
    i32 gridIndex = GetParamAsInt(0, tokens, numTokens, 0);
    char* type = GetParamAsString(1, tokens, numTokens, "border");
    ZPGGrid* grid = ctx->gridStack->grids[gridIndex];
    if (ZPG_STRCMP(type, "bisect") == 0)
    {
        ZPG_Draw_HorizontalBisectStencil(grid);
    }
    else
    {
        ZPG_DrawOuterBorder(grid, NULL, 1);
    }
    ZPG_Grid_PrintCellDefChars(grid, 0, 0, 0);
    return 0;
}

static i32 ZPG_ExecPlotSegmentedPath(ZPGContext* ctx, char** tokens, i32 numTokens)
{
	return 0;
}


#endif // ZPG_COMMAND_H