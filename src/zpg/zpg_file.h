#ifndef ZPG_FILE_H
#define ZPG_FILE_H

#include "zpg_internal.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib/stb_image_write.h"

static zpgError ZPG_WriteGridBinary(ZPGGrid* grid, u8** resultPtr, zpgSize* resultSize)
{
	/*
	4 bytes magic value
	4 bytes grid width
	4 bytes grid height
	Cells
	*/
	// one byte per cell.
	zpgSize totalCells = grid->width * grid->height;
	*resultSize = 4 + 4 + 4 + totalCells;
	*resultPtr = (u8*)ZPG_Alloc(*resultSize, 0);
	u8* cursor = *resultPtr;
	
	*cursor = 'G';
	cursor++;
	*cursor = 'R';
	cursor++;
	*cursor = 'I';
	cursor++;
	*cursor = 'D';
	cursor++;
	
	*(u32*)cursor = grid->width;
	cursor += 4;
	*(u32*)cursor = grid->height;
	cursor += 4;
	
	for (i32 i = 0; i < totalCells; ++i)
	{
		*cursor = grid->cells[i];
		cursor += 1;
	}
	return ZPG_ERROR_NONE;
}
/*
static void ZPG_WriteGridCSV(ZPGGrid* grid, u8** resultPtr, zpgSize* resultLength)
{
    // chars are grid + a heights' worth of line endings + null terminator
    i32 totalCells = grid->width * grid->height;
    i32 totalCommas = totalCells - grid->height;

    i32 totalChars = (grid->width * grid->height) + grid->height + 1;
    u8* buf = (u8*)ZPG_Alloc(totalChars, 0);
    
    u8* cursor = buf;
    ZPG_MEMSET(buf, 0, totalChars);
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_Grid_GetTypeDefAt(grid, x, y);
            if (def != NULL)
            {
                *cursor = def->asciChar;
            }
            else
            {
                *cursor = '#';
            }
            cursor += sizeof(u8);
        }
        *cursor = '\n';
        cursor += sizeof(u8);
    }
    *cursor = '\0';
    cursor += sizeof(u8);
    
    *resultPtr = buf;
    *resultLength = totalChars;
}
*/
static void ZPG_WriteGridAscii(ZPGGrid* grid, u8** resultPtr, zpgSize* resultLength)
{
    // chars are grid + a heights' worth of line endings + null terminator
    i32 totalChars = (grid->width * grid->height) + grid->height + 1;
    u8* buf = (u8*)ZPG_Alloc(totalChars, 0);
    
    u8* cursor = buf;
    ZPG_MEMSET(buf, 0, totalChars);
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCellTypeDef* def = ZPG_Grid_GetTypeDefAt(grid, x, y);
            if (def != NULL)
            {
                *cursor = def->asciChar;
            }
            else
            {
                *cursor = '#';
            }
            cursor += sizeof(u8);
        }
        *cursor = '\n';
        cursor += sizeof(u8);
    }
    *cursor = '\0';
    cursor += sizeof(u8);
    
    *resultPtr = buf;
    *resultLength = totalChars;
}

extern "C" void ZPG_WriteGridAciiToFile(ZPGGrid* grid, char* fileName)
{
    if (grid == NULL) { return; }
    if (fileName == NULL) { return; }
    FILE* f;
    i32 err = fopen_s(&f, fileName, "w");
    if (err != 0)
    {
        printf("Cound not open file %s for writing\n", fileName);
        return;
    }
    for (i32 y = 0; y < grid->height; ++y)
    {
        for (i32 x = 0; x < grid->width; ++x)
        {
            // Note: Keep to regular non-extended asci here to avoid
            // garbling output in some text editors
            ZPGCellTypeDef* def = ZPG_Grid_GetTypeDefAt(grid, x, y);
            if (def != NULL)
            {
                fprintf(f, "%c", def->asciChar);
            }
            else
            {
                fprintf(f, "#");
            }
        }
        if (y < (grid->height - 1)) { fprintf(f, "\n"); }
    }
    printf("Wrote %d chars to %s\n", ftell(f), fileName);
    fclose(f);
}

static void ZPG_WriteGridAsPNG(ZPGGrid* grid, char* fileName, i32 bUseCellTypeColours)
{
	i32 w = grid->width, h = grid->height;
    i32 bitmapBytes = w * h * sizeof(ZPGColour);
	ZPGColour* pixels = (ZPGColour*)ZPG_Alloc(bitmapBytes, 0);
	i32 numPixels = w * h;
    printf("Saving %.3fKB of bitmap to PNG %s\n", ((f32)bitmapBytes / 1024.f), fileName);
	for (i32 i = 0; i < numPixels; ++i)
	{
		ZPGColour* pix = &pixels[i];
        if (bUseCellTypeColours == YES)
        {
            ZPGCellTypeDef* def = ZPG_GetType(grid->cells[i]);
            pix->arr[0] = def->colour.channels.r;
		    pix->arr[1] = def->colour.channels.g;
		    pix->arr[2] = def->colour.channels.b;
		    pix->arr[3] = def->colour.channels.a;
        }
        else
        {
            pix->arr[0] = grid->cells[i];
		    pix->arr[1] = grid->cells[i];
		    pix->arr[2] = grid->cells[i];
		    pix->arr[3] = 255;
        }
	}
    i32 err = stbi_write_png(fileName, grid->width, grid->height, 4, pixels, 0);
	ZPG_Free(pixels);
    if (err == 1)
    {
        return;
    }
    printf("ERROR %d saving to png \"%s\"\n", err, fileName);
}

static void ZPG_MeasureGridInString(u8* str, ZPGPoint* size, zpgSize numChars)
{
    i32 width = 0, height = 0, currentWidth = 0;
    char c = '\0';
    i32 i = 0;
    while(i < numChars)
    {
        c = str[i];
        currentWidth++;
        switch (c)
        {
            case '\r':
                //printf("CR");
                break;
            case '\n':
                //printf("LF\n");
                height++;
                currentWidth = 0;
                break;
            default:
            {
                if (height < 1)
                {
                    width++;
                }
                //printf("%c", c);
            } break;
        }
        i++;
    }
    if (currentWidth > 0)
    {
        height++;
    }
    size->x = width;
    size->y = height;
}

static zpgError ZPG_WriteBlob(char* fileName, u8* data, zpgSize numBytes)
{
    if (fileName == NULL) { return ZPG_ERROR_MISSING_PARAMETER; }
    FILE* f;
    i32 err = fopen_s(&f, fileName, "wb");
    if (err != 0)
    {
        printf("Err %d opening file %s for writing\n", err, fileName);
        return ZPG_ERROR_FAILED_TO_OPEN_FILE;
    }
    fseek(f, 0, SEEK_SET);
    fwrite(data, numBytes, 1, f);
    fclose(f);
    return ZPG_ERROR_NONE;
}

#if 1
static u8* ZPG_StageFile(char* fileName, zpgSize* bytesRead)
{
    FILE* f;
    i32 err = fopen_s(&f, fileName, "rb");
    if (err != 0)
    {
        printf("Err %d opening file %s\n", err, fileName);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    *bytesRead = ftell(f);
    fseek(f, 0, SEEK_SET);
    void* mem = ZPG_Alloc(*bytesRead, ZPG_MEM_TAG_FILE);
    if (mem == NULL)
    {
        fclose(f);
        printf("Failed to allocate %lld bytes for file\n", *bytesRead);
        return NULL;
    }

    // Read
    fread(mem, *bytesRead, 1, f);
    fclose(f);
    return (u8*)mem;
}
#endif
static void ZPG_CloseFile(FILE* f)
{
    fclose(f);
}

static ZPGGrid* ZPG_ReadGridAsci(u8* chars, zpgSize len)
{
    ZPGGrid* grid = NULL;
    ZPGPoint size;
    ZPG_MeasureGridInString(chars, &size, len);
    //printf("\nMeasured asci grid %d chars, size %d/%d\n",
    //    len, size.x, size.y);
    #if 0 // print a random row
    i32 rowNum = 3;
    i32 readIndex = rowNum * (size.x + 2); // +2 for new line
    i32 endIndex = readIndex + size.x;
    for (i32 i = readIndex; i < endIndex; ++i)
    {
        char c = chars[i];
        printf("%c", c);
    }
    printf("\n");
    #endif

    grid = ZPG_CreateGrid(size.x, size.y);
    for (i32 y = 0; y < grid->height; ++y)
    {
        // Read line
        i32 readIndex = y * (size.x + 2); // +2 for new line \r\n
        i32 endIndex = readIndex + size.x;
        i32 x = 0;
        for (i32 i = readIndex; i < endIndex; ++i, ++x)
        {
            u8 c = chars[i];
            ZPGCellTypeDef* def = ZPG_GetTypeByAsci(c);
            grid->cells[ZPG_GRID_POS_TO_INDEX(grid, x, y)] = def->value;
            // ZPGCell* cell = ZPG_Grid_GetCellAt(grid, x, y);
            // *cell = {};
            // cell->tile.type = def->value;
        }
    }

    return grid;
}

#endif // ZPG_FILE_H