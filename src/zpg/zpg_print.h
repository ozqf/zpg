#ifndef ZPG_PRINT_H
#define ZPG_PRINT_H

#include "zpg_internal.h"


static void ZPG_BGrid_PrintValues(ZPGByteGrid* grid, i32 bBlankZeroes)
{
    ZPG_PARAM_NULL(grid, )
	if (grid->width > 100 || grid->height > 100)
	{
		printf("SKIP: Grid size %d/%d is too big to print\n", grid->width, grid->height);
		return;
	}
    printf("------ Byte Grid %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < grid->width; ++x)
        {
            i32 val = ZPG_BGRID_GET(grid, x, y);
            if (bBlankZeroes && val == 0)
            {
                //printf(" ");
                printf("    ");
            }
            else
            {
                printf(" %03d", val);
            }
        }
        printf("|\n");
    }
    printf("------------------\n");
}


/////////////////////////////////////////////
// Grid printing
/////////////////////////////////////////////
static void ZPG_Grid_PrintValues(ZPGGrid* grid, i32 bBlankZeroes)
{
    ZPG_PARAM_NULL(grid, )
	if (grid->width > 100 || grid->height > 100)
	{
		printf("SKIP: Grid size %d/%d is too big to print\n", grid->width, grid->height);
		return;
	}
    printf("------ Grid %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
            if (bBlankZeroes && cell->tile.type == 0)
            {
                //printf(" ");
                printf("    ");
            }
            else
            {
                printf(" %03d", cell->tile.type);
            }
        }
        printf("|\n");
    }
    printf("------------------\n");
}

static void ZPG_Grid_PrintChannelValues(ZPGGrid* grid, i32 channel, i32 bBlankZeroes)
{
    ZPG_PARAM_NULL(grid, )
	if (grid->width > 100 || grid->height > 100)
	{
		printf("SKIP: Grid size %d/%d is too big to print\n", grid->width, grid->height);
		return;
	}
    if (channel < 0 || channel > 3) { channel = 0; }
    printf("------ Grid %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
            if (bBlankZeroes && cell->arr[channel] == 0)
            {
                printf(" ");
            }
            else
            {
                printf("%d", cell->arr[channel]);
            }
        }
        printf("|\n");
    }
    printf("------------------\n");
}

static void ZPG_PrintPointsAsGrid(
    ZPGPoint* points, i32 numPoints, i32 width, i32 height)
{
    //i32 pointsPrinted = 0;
    for (i32 y = 0; y < height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < width; ++x)
        {
            i32 bPrintPoint = NO;
            for (i32 i = 0; i < numPoints; ++i)
            {
                if (points[i].x == x && points[i].y == y)
                {
                    bPrintPoint = YES;
                    break;
                }
            }
            if (bPrintPoint == YES)
            {
                printf("X");
                continue;
            }
            printf(" ");
        }
        printf("|\n");
    }
}

/**
 * Send '\0' marker to place no special marker
 */
static void ZPG_Grid_PrintChars(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY)
{
	ZPG_PARAM_NULL(grid, )
	if (grid->width > 100 || grid->height > 100)
	{
		printf("SKIP: Grid size %d/%d is too big to print\n", grid->width, grid->height);
		return;
	}
    printf("------ Grid %d/%d (%d total tiles, %d path tiles, %d objectives)------\n",
        grid->width,
        grid->height,
        grid->width * grid->height,
        grid->stats.numFloorTiles,
        grid->stats.numObjectiveTags);
    i32 xNum = 0;
    i32 yNum = 0;
    printf(" ");
    for (i32 x = 0; x < grid->width; ++x)
    {
        printf("%d", xNum++);
        if (xNum >= 10) { xNum = 0; }
    }
    printf("\n");
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("%d", yNum++);
        if (yNum >= 10) { yNum = 0; }
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_Grid_GetCellTypeAt(grid, x, y);
            u8 c = def->asciChar;
            // Special case
            if (c == '#')
            {
                c = 219;
            }
            if (c == '.')
            {
                c = 176;
            }
            if (marker != '\0' && x == markerX && y == markerY)
            {
                c = marker;
            }
            printf("%c", c);
        }
        printf("\n");
    }
    printf("------------------\n");
}

/**
 * Goes by red channel only. assumes 0-255 range
 */
static void ZPG_Grid_PrintTexture(ZPGGrid* grid, i32 bColourIndices)
{
	ZPG_PARAM_NULL(grid, )
	if (grid->width > 100 || grid->height > 100)
	{
		printf("SKIP: Grid size %d/%d is too big to print\n", grid->width, grid->height);
		return;
	}
    const u8 white = 219;
    const u8 lightGrey = 178;
    const u8 middleGrey = 177;
    const u8 darkGrey = 176;
    const u8 black = ' ';
    //u8 colours[] = { white, lightGrey, middleGrey, darkGrey, black };
    u8 colours[] = { black, darkGrey, middleGrey, lightGrey, white };
    //u8 colours[] = { white, lightGrey, middleGrey, darkGrey };
    //u8 colours[] = { lightGrey, middleGrey, darkGrey, white };
    u8 numColours = sizeof(colours);

    u8 divider = (u8)(255 / (numColours - 1));
    //divider -= 1;
    printf("Sizeof(colours): %d divider %d\n", sizeof(colours), divider);
    printf("------ Grayscale texture %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            u8 r = grid->cells[x + (y * grid->width)].colour.r;
            u8 outputIndex = (u8)((f32)((f32)r / (f32)divider) + 0.5f);
            u8 ch = colours[outputIndex];
            //if (outputIndex >= numColours) { outputIndex = numColours - 1; }
            if (bColourIndices == YES)
            {
                printf("%3d: (%3d),", r, outputIndex);
            }
            else
            {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
    printf("----------------------\n");
}


#endif // ZPG_PRINT_H