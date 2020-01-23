#ifndef ZPG_SCRIPT_H
#define ZPG_SCRIPT_H

#include "zpg_internal.h"

static i32 ZPG_ExecuteCommand(char** tokens, i32 numTokens)
{
    if (ZPG_STRCMP(tokens[0], "set") == 0)
    { return ZPG_ExecSet(tokens, numTokens); }
    if (ZPG_STRCMP(tokens[0], "grid") == 0) { return 0; }
    if (ZPG_STRCMP(tokens[0], "fill") == 0) { return 0; }
    return 1;
}

#if 1
static i32 ZPG_ReadTokens(
    char* source, char* destination, char** tokens)
{
    i32 len = ZPG_STRLEN(source);
    i32 tokensCount = 0;

    i32 readPos = 0;
    i32 writePos = 0;

    u8 reading = true;
    u8 readingToken = 0;
    while (reading)
    {
        char c = *(source + readPos);
        if (readingToken)
        {
            if (c == ' ')
            {
                *(destination + writePos) = '\0';
				readingToken = 0;
            }
            else if (c == '\0')
            {
                *(destination + writePos) = '\0';
				readingToken = 0;
                reading = false;
            }
            else
            {
                *(destination + writePos) = c;
            }
            readPos++;
            writePos++;
        }
        else
        {
            if (c == ' ' || c == '\t')
            {
                readPos++;
            }
            else if (c == '\0')
            {
                *(destination + writePos) = '\0';
                reading = false;
            }
            else
            {
                readingToken = 1;
                *(destination + writePos) = c;

                tokens[tokensCount++] = (destination + writePos);

                readPos++;
                writePos++;
            }
        }
    }

    return tokensCount;
}


char* ZPG_RunToNewLine(char* buffer)
{
    u8 reading = true;
    while (reading)
    {
        if (*buffer == '\n' || *buffer == EOF)
        {
            reading = false;
        }
        else
        {
            ++buffer;
        }
    }
    return buffer;
}

#endif

static i32 ZPG_ExecuteLine(u8* cursor, u8* end, i32 lineNumber)
{
    const int tempBufferSize = 256;
    i32 lineLength = end - cursor;
    u8 tokensBuf[tempBufferSize];
    ZPG_MEMSET(tokensBuf, 0, tempBufferSize);

    u8 workBuf[tempBufferSize];
    ZPG_MEMCPY(workBuf, cursor, lineLength);
    workBuf[lineLength] = '\0';
    workBuf[lineLength + 1] = '\0';

    printf("Read line %d (%d chars): \"%s\"\t",
        lineNumber, lineLength + 1, workBuf);

    const i32 maxTokens = 24;
    char* tokens[maxTokens];
    i32 numTokens = ZPG_ReadTokens(
        (char*)workBuf, (char*)tokensBuf, tokens);
    printf("\tTokens:\t");
    for (i32 i = 0; i < numTokens; ++i)
    {
        printf("%s, ", tokens[i]);
    }
    printf("\n");
    i32 result = ZPG_ExecuteCommand(tokens, numTokens);
    return result;
}

static u8* ZPG_ScanForLineEnd(u8* buf, u8* end, i32* lineEndSize)
{
    u8* result = buf;
    while (result < end)
    {
        if (*result == '\n') { *lineEndSize = 1; break; }
        else if (*result == '\r') { *lineEndSize = 2; break; }
        else if (*result == '\0') { break; }
        result++;
    }
    printf("EoL code %d\n", *result);
    return result;
}

ZPG_EXPORT i32 ZPG_RunScript(u8* text, i32 textLength, i32 apiFlags)
{
    if (g_bInitialised == false) { return 1; }
    printf("Running script (%d chars)\n", textLength);
    u8* end = text + textLength;
    u8* cursor = text;
    u8* cursorEnd;
    i32 line = 1;
    while (cursor < end)
    {
        i32 lineEndSize;
        cursorEnd = ZPG_ScanForLineEnd(cursor, end, &lineEndSize);
        i32 len = cursorEnd - cursor;
        if (len >= 2)
        {
            i32 err = ZPG_ExecuteLine(cursor, cursorEnd, line);
            if (err != 0)
            {
                printf("ABORT Error %d executing line\n", err);
                break;
            }
        }
        /*else
        {
            printf("Skip line\n");
        }*/
        cursor = cursorEnd + lineEndSize;
        line++;
    }
    ZPG_PrintAllocations();
    return 0;
}


#endif // ZPG_SCRIPT_H