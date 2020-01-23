#ifndef ZPG_SCRIPT_H
#define ZPG_SCRIPT_H

#include "zpg_internal.h"
/**
 * TODO: Restore this tokenise code.
 */
#if 1
static i32 ZPG_ReadTokens(char* source, char* destination, char** tokens)
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
// static void ZPG_script_tokenise(
//     u8* str, i32 length, char** tokens, i32* numTokens, i32 maxTokens)
// {
//     printf("Tokenise - \n");
//     // TODO: See unfinished tokenise code above!
//     i32 numTokens = ZPG_ReadTokens(str, )
// }

static void ZPG_ExecuteLine(u8* cursor, u8* end, i32 lineNumber)
{
    const int tempBufferSize = 256;
    i32 lineLength = end - cursor;
    u8 tokensBuf[tempBufferSize];
    ZPG_MEMSET(tokensBuf, 0, tempBufferSize);

    u8 workBuf[tempBufferSize];
    ZPG_MEMCPY(workBuf, cursor, lineLength);
    workBuf[lineLength + 1] = '\0';

    printf("Read line %d (%d chars): \"%s\"\t", lineNumber, lineLength + 1, workBuf);

    const i32 maxTokens = 24;
    char* tokens[maxTokens];
    i32 numTokens = ZPG_ReadTokens((char*)workBuf, (char*)tokensBuf, tokens);
    printf("\tTokens:\t");
    for (i32 i = 0; i < numTokens; ++i)
    {
        printf("%s, ", tokens[i]);
    }
    printf("\n");
    //ZPG_script_tokenise(tokensBuf, lineLength, tokens, &numTokens, maxTokens);
    //ZPG_Free(tokensBuf);
}

static u8* ZPG_ScanForLineEnd(u8* buf, u8* end, i32* lineEndSize)
{
    u8* result = buf;
    while (result < end)
    {
        if (*result == '\n') { *lineEndSize = 1; break; }
        else if (*result == '\r') { *lineEndSize = 2; break; }
        result++;
    }
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
            ZPG_ExecuteLine(cursor, cursorEnd, line);
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