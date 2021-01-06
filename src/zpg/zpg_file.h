#ifndef ZPG_FILE_H
#define ZPG_FILE_H

#include "zpg_internal.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib/stb_image_write.h"

extern "C" void ZPG_WriteGridAsAsci(ZPGGrid* grid, char* fileName)
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
            fprintf(f, "%c", ZPG_Grid_GetCellTypeAt(grid, x, y)->asciChar);
        }
        if (y < (grid->height - 1)) { fprintf(f, "\n"); }
    }
    printf("Wrote %d chars to %s\n", ftell(f), fileName);
    fclose(f);
}

static void ZPG_WriteGridAsPNG(ZPGGrid* grid, char* fileName)
{
    i32 err = stbi_write_png(fileName, grid->width, grid->height, 4, grid->cells, 0);
    if (err == 1)
    {
        return;
    }
    printf("ERROR %d saving to png \"%s\"\n", err, fileName);
}

static void ZPG_MeasureGridInString(u8* str, ZPGPoint* size, i32 numChars)
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

#if 1
static u8* ZPG_StageFile(char* fileName, i32* bytesRead)
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
        printf("Failed to allocate %d bytes for file\n", *bytesRead);
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

static ZPGGrid* ZPG_ReadGridAsci(u8* chars, i32 len)
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