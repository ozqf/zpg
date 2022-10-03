#ifndef ZPG_PRINT_H
#define ZPG_PRINT_H

#include "zpg_internal.h"

static void ZPG_PrintPointDirectionFlags(i32 x, i32 y, i32 val, u8 flags)
{
    printf("%d, %d: val %d, dir flags %d: ", x, y, val, flags);
    if (flags & ZPG_FLAG_LEFT) { printf("left, "); }
    if (flags & ZPG_FLAG_RIGHT) { printf("right, "); }
    if (flags & ZPG_FLAG_ABOVE) { printf("above, "); }
    if (flags & ZPG_FLAG_BELOW) { printf("below, "); }
    printf("\n");
}

static void ZPG_Grid_PrintValues(ZPGGrid* grid, i32 digitCount, i32 bBlankZeroes)
{
    ZPG_PARAM_NULL(grid, )
	if (grid->width > 100 || grid->height > 100)
	{
		printf("SKIP: Grid size %d/%d is too big to print\n", grid->width, grid->height);
		return;
	}
    char* valueFormat = "%d";
    char* blankFormat = " ";
    if (digitCount == 2)
    {
        valueFormat = " %02d";
        blankFormat = "   ";
    }
    else if (digitCount == 3)
    {
        valueFormat = " %03d";
        blankFormat = "    ";
    }
    printf("------ Grid %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < grid->width; ++x)
        {
            i32 val = ZPG_BGRID_GET(grid, x, y);
            if (bBlankZeroes && val == 0)
            {
                //printf(" ");
                printf(blankFormat);
            }
            else
            {
                //printf(" %03d", val);
                printf(valueFormat, val);
            }
        }
        printf("|\n");
    }
    printf("------------------\n");
}

static void ZPG_Grid_PrintPath(ZPGGrid* grid, ZPGPoint* points, i32 numPoints)
{
    if (grid == NULL) { return; }
    if (points == NULL) { return; }
    if (numPoints <= 0) { return; }
    printf("Print %d points, starting at %d, %d\n", numPoints, points[0].x, points[0].y);
    i32 w = grid->width, h = grid->height;
    ZPGGrid* temp = ZPG_CreateGrid(w, h);
    i32 numCells = w * h;
    for (i32 i = 0; i < numCells; ++i)
    {
        u8 val = grid->cells[i];
        u8 result = 0;
        if (val == 1) { result = '#'; }
        else if (val > 1) { result = '.'; }
        temp->cells[i] = result;
    }
    char c = '0';
    for (i32 i = 0; i < numPoints; ++i)
    {
        ZPGPoint p = points[i];
        if (!ZPG_GRID_POS_SAFE(grid, p.x, p.y)) { continue; }
        ZPG_GRID_SET(temp, p.x, p.y, c);
        c++;
        if (c > '9') { c = '1'; }
    }
    printf("Start\n");
    ZPG_Grid_PrintAsci(grid, 'X', points[0].x, points[0].y);
    printf("End\n");
    ZPG_Grid_PrintAsci(grid, 'X', points[numPoints - 1].x, points[numPoints - 1].y);
    ZPG_Grid_PrintAsci(temp, '\0', 0, 0);

    ZPG_FreeGrid(temp);
}

/////////////////////////////////////////////
// Grid printing
/////////////////////////////////////////////
#if 0
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
            //ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
            u8 val = ZPG_GRID_GET(grid, x, y);
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
#endif
#if 0
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
            //ZPGCell *cell = ZPG_Grid_GetCellAt(grid, x, y);
            u8 val = ZPG_GRID_GET(grid, x, y);
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
#endif
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
static void ZPG_Grid_PrintCellDefChars(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY)
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
        printf("%d_", xNum++);
        if (xNum >= 10) { xNum = 0; }
    }
    printf("\n");
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("%d|", yNum);
        
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_Grid_GetTypeDefAt(grid, x, y);
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
            // double up to make a 8x16 pixel character square in the console!
            printf("%c%c", c, c);
        }
        printf("|%d", yNum++);
        if (yNum >= 10) { yNum = 0; }
        printf("\n");
    }
    printf("------------------\n");
}

/**
 * Send '\0' marker to place no special marker
 */
static void ZPG_Grid_PrintAsci(ZPGGrid* grid, u8 marker, i32 markerX, i32 markerY)
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
            u8 c = ZPG_GRID_GET(grid, x, y);
            // Special case
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
    printf("Sizeof(colours): %lld divider %d\n", sizeof(colours), divider);
    printf("------ Grayscale texture %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            //u8 r = grid->cells[x + (y * grid->width)].colour.r;
            u8 r = grid->cells[x + (y * grid->width)];
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