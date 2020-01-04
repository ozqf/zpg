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
            fprintf(f, "%c", ZPG_GetCellTypeAt(grid, x, y)->asciChar);
            //ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
            //char c = ZPG_CellToChar(cell);
            //fprintf(f, "%c", c);
        }
        if (y < (grid->height - 1)) { fprintf(f, "\n"); }
    }
    printf("Wrote %d chars to %s\n", ftell(f), fileName);
    fclose(f);
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

static ZPGGrid* ZPG_ReadGridAsci(u8* chars, i32 len)
{
    ZPGGrid* grid = NULL;
    ZPGPoint size;
    ZPG_MeasureGridInString(chars, &size, len);
    printf("\nMeasured asci grid %d chars, size %d/%d\n",
        len, size.x, size.y);
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
        i32 readIndex = y * (size.x + 2); // +2 for new line
        i32 endIndex = readIndex + size.x;
        i32 x = 0;
        for (i32 i = readIndex; i < endIndex; ++i, ++x)
        {
            u8 c = chars[i];
            ZPGCellTypeDef* def = ZPG_GetTypeByAsci(c);
            ZPGCell* cell = ZPG_GetCellAt(grid, x, y);
            *cell = {};
            cell->tile.type = def->value;
            #if 0
            u8 type = ZPG_CELL_TYPE_NONE;
            u8 entType = ZPG_ENTITY_TYPE_NONE;
            u8 tag = ZPG_CELL_TAG_NONE;
            switch (c)
            {
                case ' ':
                    type = ZPG_CELL_TYPE_FLOOR;
                    break;
                case '.':
                    type = ZPG_CELL_TYPE_WATER;
                    break;
                case 'x':
                    type = ZPG_CELL_TYPE_FLOOR;
                    entType = ZPG_ENTITY_TYPE_ENEMY;
                    break;
                case 'k':
                    type = ZPG_CELL_TYPE_FLOOR;
                    entType = ZPG_ENTITY_TYPE_OBJECTIVE;
                    break;
                case 's':
                    type = ZPG_CELL_TYPE_FLOOR;
                    entType = ZPG_ENTITY_TYPE_START;
                    break;
                case 'e':
                    type = ZPG_CELL_TYPE_FLOOR;
                    entType = ZPG_ENTITY_TYPE_END;
                    break;
                case '#':
                case (u8)ZPG_CHAR_CODE_SOLID_BLOCK:
                    type = ZPG_CELL_TYPE_WALL;
                    break;
                default:
                    printf("Unknown char %c (%d)\n", c, c);
                    break;
            }
            cell->tile.type = type;
            cell->tile.entType = entType;
            cell->tile.tag = tag;
            #endif
        }
    }

    return grid;
}

#endif // ZPG_FILE_H