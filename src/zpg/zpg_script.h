#ifndef ZPG_SCRIPT_H
#define ZPG_SCRIPT_H

#include "zpg_internal.h"
/**
 * TODO: Restore this tokenise code.
 */
#if 0
i32 COM_ReadTokens(char* source, char* destination, char** tokens)
{
    i32 len = COM_StrLen(source);
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


char* COM_RunToNewLine(char* buffer)
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
static void ZPG_script_tokenise(
    u8* str, i32 length, char** tokens, i32* numTokens, i32 maxTokens)
{
    printf("Tokenise - \n");
    // TODO: See unfinished tokenise code above!
}

static void ZPG_ExecuteLine(u8* cursor, u8* end, i32 lineNumber)
{
    i32 lineLength = end - cursor;
    //printf("Exec line (%d chars)\n", lineLength + 1);
    u8* lineBuf = (u8*)malloc(lineLength + 1);
    memset(lineBuf, 0, lineLength + 1);
    memcpy(lineBuf, cursor, lineLength);
    printf("Read line %d (%d chars): \"%s\"\t", lineNumber, lineLength + 1, lineBuf);

    const i32 maxTokens = 24;
    char* tokens[maxTokens];
    i32 numTokens = 0;
    ZPG_script_tokenise(lineBuf, lineLength, tokens, &numTokens, maxTokens);

    free(lineBuf);
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

ZPG_EXPORT i32 ZPG_RunScript(u8* text, i32 textLength)
{
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
    return 0;
}


#endif // ZPG_SCRIPT_H