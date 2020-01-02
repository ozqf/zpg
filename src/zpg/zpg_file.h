#ifndef ZPG_FILE_H
#define ZPG_FILE_H

#include "../zpg.h"

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
            ZPGCell* cell = grid->GetCellAt(x, y);
            char c = grid->CellToChar(cell);
            fprintf(f, "%c", c);
        }
        if (y < (grid->height - 1)) { fprintf(f, "\n"); }
    }
    printf("Wrote %d chars to %s\n", ftell(f), fileName);
    fclose(f);
}

static FILE* ZPG_OpenAndMeasureFile(char* fileName, ZPGPoint* size, i32* numChars)
{
    FILE* f = NULL;
    i32 err = fopen_s(&f, fileName, "r");
    if (err != 0)
    {
        printf("Error %d opening %s\n", err, fileName);
        return NULL;
    }
    i32 width = 0, height = 0, currentWidth = 0;
    fseek(f, 0, SEEK_END);
    *numChars = ftell(f);
    fseek(f, 0, SEEK_SET);
    char c = '\0';
    i32 i = 0;
    while(i < *numChars)
    {
        c = (u8)fgetc(f);
        currentWidth++;
        switch (c)
        {
            case '\r': printf("CR"); break;
            case '\n': printf("LF\n");
                height++;
                currentWidth = 0;
                break;
            default:
            {
                if (height < 1)
                {
                    width++;
                }
                printf("%c", c);
            } break;
        }
        i++;
    }
    if (currentWidth > 0)
    {
        height++;
    }
    fseek(f, 0, SEEK_SET);
    size->x = width;
    size->y = height;
    return f;
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
    void* mem = malloc(*bytesRead);
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

static void ZPG_ReadGridAsci()
{

}

#endif // ZPG_FILE_H