#ifndef ZPG_SCRIPT_H
#define ZPG_SCRIPT_H

#include "zpg_internal.h"

#define ZPG_MAX_COMMANDS 32
static ZPGCommand g_commands[ZPG_MAX_COMMANDS];
static i32 g_numCommands = 0;
static i32 g_bCommandsInit = NO;

static void ZPG_RegisterCommand(char* name, zpg_command_fn func)
{
    if (g_numCommands >= ZPG_MAX_COMMANDS)
    {
        printf("No capacity for cmd %s\n", name);
        return;
    }
    ZPGCommand* cmd = &g_commands[g_numCommands++];
    cmd->name = name;
    cmd->function = func;
}

static ZPGCommand* ZPG_FindCommand(char* name)
{
    for (i32 i = 0; i < g_numCommands; ++i)
    {
        ZPGCommand* cmd = &g_commands[i];
        if (ZPG_STRCMP(name, cmd->name) == 0)
        {
            return cmd;
        }
    }
    return NULL;
}

static void ZPG_InitScripts()
{
    if (g_bCommandsInit)
    {
        return;
    }
    g_bCommandsInit = YES;
    ZPG_RegisterCommand("init_stack", ZPG_ExecInitStack);
    ZPG_RegisterCommand("grid_set_all", ZPG_ExecGridSetAll);

    ZPG_RegisterCommand("grid_print", ZPG_ExecGridPrint);
    ZPG_RegisterCommand("grid_copy_specific", ZPG_ExecGridCopyValue);

    ZPG_RegisterCommand("stencil", ZPG_ExecStencil);
    ZPG_RegisterCommand("drunk", ZPG_ExecRandomWalk);
    ZPG_RegisterCommand("caves", ZPG_ExecCaves);
}

static i32 ZPG_ReadTokens(
    char* source, char* destination, char** tokens);

static i32 ZPG_ExecuteCommand(ZPGContext* ctx, char** tokens, i32 numTokens)
{
    if (numTokens == 0) { return 1; }

    // trim off the first token as commands don't need to know that
    char* key = tokens[0];
    tokens = &tokens[1];
    numTokens -= 1;

    ZPGCommand* cmd = ZPG_FindCommand(key);
    if (cmd == NULL)
    {
        printf("No command '%s' found\n", key);
        return 0;
    }
    return cmd->function(ctx, tokens, numTokens);
    /*
    // grid manipulation
    if (ZPG_STRCMP(key, "init_stack") == 0)
    { return ZPG_ExecInitStack(ctx, tokens, numTokens); }
    if (ZPG_STRCMP(key, "grid_set_all") == 0)
    { return ZPG_ExecGridSetAll(ctx, tokens, numTokens); }

    // painting


    // procedures
    if (ZPG_STRCMP(key, "drunk") == 0)
    { return ZPG_ExecRandomWalk(ctx, tokens, numTokens); }
    if (ZPG_STRCMP(key, "caves_seed") == 0)
    { return ZPG_ExecCaves(ctx, tokens, numTokens); }

    if (ZPG_STRCMP(key, "set") == 0)
    { return ZPG_ExecSet(ctx, tokens, numTokens); }
    if (ZPG_STRCMP(key, "grid") == 0) { return 0; }
    if (ZPG_STRCMP(key, "fill") == 0) { return 0; }
    return 1;
    */
}

/*
Read - Eval - Print - Loop
*/
ZPG_EXPORT i32 ZPG_BeginREPL()
{
    ZPG_InitScripts();
    printf("Enter commands (enter exit to quit program):\n");
    ZPGContext ctx = {};
	char str[256];
    // ZPGGridStack* stack = NULL;
	for(;;)
	{
        printf("> ");
		char* cmd = gets_s(str, 256);
		if (cmd == NULL)
		{
			printf("No command was read!\n");
			return 1;
		}
		if (strcmp(cmd, "exit") == 0)
		{
			break;
		}
		// printf("You entered %s\n", cmd);
		
		const i32 maxTokens = 24;
		const int tempBufferSize = 256;
		
		u8 tokensBuf[tempBufferSize];
		ZPG_MEMSET(tokensBuf, 0, tempBufferSize);
		char* tokens[maxTokens];
		i32 numTokens = ZPG_ReadTokens(
			(char*)cmd, (char*)tokensBuf, tokens);
		// printf("\tTokens:\t");
		// for (i32 i = 0; i < numTokens; ++i)
		// {
		// 	printf("%s, ", tokens[i]);
		// }
		// printf("\n");
		i32 result = ZPG_ExecuteCommand(&ctx, tokens, numTokens);
	}
	
	return 0;
}

static void ZPG_PrintTokens(char** tokens, i32 numTokens)
{
	printf("\tTokens:\t");
	for (i32 i = 0; i < numTokens; ++i)
	{
		printf("%s, ", tokens[i]);
	}
	printf("\n");
}
	

static i32 ZPG_CheckSignature(char* sig, char** tokens, i32 numTokens)
{
	size_t len = strlen(sig);
	if (len != numTokens)
	{
		printf("Command params length mismatch\n");
		printf("%s\n", sig);
		ZPG_PrintTokens(tokens, numTokens);
		return NO;
	}
	return YES;
}

///////////////////////////////////////////////////////
// reading from a file
///////////////////////////////////////////////////////
#if 1
static i32 ZPG_ReadTokens(
    char* source, char* destination, char** tokens)
{
    zpgSize len = ZPG_STRLEN(source);
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

static i32 ZPG_ExecuteLine(ZPGContext* ctx, u8* cursor, u8* end, i32 lineNumber)
{
    const int tempBufferSize = 256;
    zpgSize lineLength = end - cursor;
    u8 tokensBuf[tempBufferSize];
    ZPG_MEMSET(tokensBuf, 0, tempBufferSize);

    u8 workBuf[tempBufferSize];
    ZPG_MEMCPY(workBuf, cursor, lineLength);
    workBuf[lineLength] = '\0';
    workBuf[lineLength + 1] = '\0';

    printf("Read line %d (%lld chars): \"%s\"\t",
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
    i32 result = ZPG_ExecuteCommand(ctx, tokens, numTokens);
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
    if (g_bZPGInitialised == false) { return 1; }
    ZPG_InitScripts();
    printf("Running script (%d chars)\n", textLength);
    u8* end = text + textLength;
    u8* cursor = text;
    u8* cursorEnd;
    i32 line = 1;
    ZPGContext ctx = {};
    while (cursor < end)
    {
        i32 lineEndSize;
        cursorEnd = ZPG_ScanForLineEnd(cursor, end, &lineEndSize);
        zpgSize len = cursorEnd - cursor;
        if (len >= 2)
        {
            i32 err = ZPG_ExecuteLine(&ctx, cursor, cursorEnd, line);
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