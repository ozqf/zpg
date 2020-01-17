#ifndef ZPG_SCRIPT_H
#define ZPG_SCRIPT_H

#include "zpg_internal.h"

static void ZPG_ExecuteLine(u8* cursor, u8* end)
{
    i32 lineLength = end - cursor;
    printf("Exec line (%d chars)\n", lineLength + 1);
    u8* lineBuf = (u8*)malloc(lineLength + 1);
    memset(lineBuf, 0, lineLength);
    memcpy(lineBuf, cursor, lineLength);
    printf("Read line \"%s\"\n", lineBuf);
    free(lineBuf);
}

static u8* ZPG_ScanForLineEnd(u8* buf, u8* end)
{
    u8* result = buf;
    while (result < end)
    {
        if (*result == '\n') { break; }
        result++;
    }
    return result;
}

ZPG_EXPORT i32 ZPG_RunScript(u8* text, i32 textLength)
{
    printf("Running script (%d chars)\n", textLength);
    u8* end = text + textLength;
    u8* cursor = text;
    u8* cursorEnd;
    while (cursor < end)
    {
        cursorEnd = ZPG_ScanForLineEnd(cursor, end);
        i32 len = cursorEnd - cursor;
        if (len >= 2)
        {
            ZPG_ExecuteLine(cursor, cursorEnd);
        }
        else
        {
            printf("Skip line\n");
        }
        cursor = cursorEnd + 1;
    }
    return 0;
}


#endif // ZPG_SCRIPT_H