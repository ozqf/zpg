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

static void ZPG_PrintCommands()
{
	printf("--- Commands Help ---\n");
    printf("help - list commands\n");
    printf("exit - end session and quit program\n");
	for (i32 i = 0; i < g_numCommands; ++i)
    {
        ZPGCommand* cmd = &g_commands[i];
        printf("%s\n", cmd->name);
    }
	printf("\n");
}

static void ZPG_InitScripts()
{
    if (g_bCommandsInit)
    {
        return;
    }
    g_bCommandsInit = YES;
    ZPG_RegisterCommand("grid_set_all", ZPG_ExecGridSetAll);
    ZPG_RegisterCommand("grid_ascii", ZPG_ExecGridPrintAscii);
    ZPG_RegisterCommand("grid_values", ZPG_ExecGridPrintValues);
	ZPG_RegisterCommand("grid_save_ascii_file", ZPG_ExecSaveGridAsciiToTextFile);
    ZPG_RegisterCommand("grid_save_bytes", ZPG_ExecSaveGridBytes);
    ZPG_RegisterCommand("grid_copy_from", ZPG_ExecGridCopy);
    ZPG_RegisterCommand("grid_copy_value", ZPG_ExecGridCopyValue);
    ZPG_RegisterCommand("grid_replace_value", ZPG_ExecGridReplaceValue);
	ZPG_RegisterCommand("grid_print_prefabs", ZPG_ExecPrintPrefabs);
    ZPG_RegisterCommand("grid_write_to_output", ZPG_ExecAsciiGridToOutput);
    ZPG_RegisterCommand("grid_scatter", ZPG_ExecGridScatter);
    ZPG_RegisterCommand("grid_draw_points", ZPG_ExecGridDrawPoints);
    ZPG_RegisterCommand("grid_to_binary", ZPG_ExecGridToBinary);
    ZPG_RegisterCommand("grid_flip_binary", ZPG_ExecGridFlipBinary);
    ZPG_RegisterCommand("grid_fill_rect", ZPG_ExecGridDrawRect);

    ZPG_RegisterCommand("stencil", ZPG_ExecStencil);
    ZPG_RegisterCommand("drunk", ZPG_ExecRandomWalk);
    ZPG_RegisterCommand("caves", ZPG_ExecCaves);
    ZPG_RegisterCommand("voronoi", ZPG_ExecVoronoi);
    ZPG_RegisterCommand("rooms", ZPG_ExecBuildRooms);

    ZPG_RegisterCommand("init_stack", ZPG_ExecInitStack);
	ZPG_RegisterCommand("seed", ZPG_ExecSetSeed);
    ZPG_RegisterCommand("set", ZPG_ExecSet);
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
}

static i32 ExecLine(ZPGContext* ctx, char* cmd)
{
	const i32 maxTokens = 24;
	const int tempBufferSize = 256;
	
	u8 tokensBuf[tempBufferSize];
	ZPG_MEMSET(tokensBuf, 0, tempBufferSize);
	char* tokens[maxTokens];
	i32 numTokens = ZPG_ReadTokens(
		(char*)cmd, (char*)tokensBuf, tokens);
	return ZPG_ExecuteCommand(ctx, tokens, numTokens);
}

static ZPGContext CreateContext()
{
    ZPGContext ctx = {};
    // apply some default properties
    ctx.walkCfg.bigRoomChance = 0.01f;
    ctx.walkCfg.tilesToPlace = 40;
    ctx.walkCfg.typeToPaint = 0;
    ctx.walkCfg.bPlaceObjectives = YES;
    ctx.walkCfg.bStepThrough = NO;

    ctx.cellCfg = ZPG_DefaultCaveRules();
    // generate a new seed - script can override if it wants.
    ctx.seed = ZPG_GenerateSeed();
    return ctx;
}

static void FreeContext(ZPGContext* ctx)
{

}

/*
Read - Eval - Print - Loop
*/
ZPG_EXPORT i32 ZPG_BeginREPL()
{
    ZPG_InitScripts();
    printf("Enter commands (enter exit to quit program):\n");
    ZPGContext ctx = CreateContext();
    ExecLine(&ctx, "set verbosity 2");
    // create a default grid stack - running init_stack
    // again will override it if needed
    ExecLine(&ctx, "init_stack 8 48 24");
    // set active grids
    ExecLine(&ctx, "set grid 0");
    ExecLine(&ctx, "set stencil 1");
	ctx.verbosity = 2;
	char str[256];
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
		if (strcmp(cmd, "help") == 0)
		{
			ZPG_PrintCommands();
			continue;
		}
		// printf("You entered %s\n", cmd);
		
		i32 result = ExecLine(&ctx, cmd);
		if (result != 0)
		{
			printf("Error code %d\n", result);
		}
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

    // printf("Read line %d (%lld chars): \"%s\"\t",
        // lineNumber, lineLength + 1, workBuf);

    const i32 maxTokens = 24;
    char* tokens[maxTokens];
    i32 numTokens = ZPG_ReadTokens(
        (char*)workBuf, (char*)tokensBuf, tokens);
    // printf("\tTokens:\t");
    // for (i32 i = 0; i < numTokens; ++i)
    // {
    //     printf("%s, ", tokens[i]);
    // }
    // printf("\n");
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
    // printf("EoL code %d\n", *result);
    return result;
}

static zpgSize ExtractLine(u8* buffer, zpgSize bufferSize, u8* cursor, u8* end)
{
    zpgSize lineLength = end - cursor;
    ZPG_MEMCPY(buffer, cursor, lineLength);
    buffer[lineLength] = '\0';
    buffer[lineLength + 1] = '\0';
    return lineLength;
}

ZPG_EXPORT i32 ZPG_RunScript(u8* text, i32 textLength, i32 apiFlags)
{
    if (g_bZPGInitialised == false) { return 1; }
    i32 result = 0;
    ZPG_InitScripts();
    printf("Running script (%d chars)\n", textLength);
    u8* end = text + textLength;
    u8* cursor = text;
    u8* cursorEnd;
    i32 line = 1;
    ZPGContext ctx = CreateContext();
    while (cursor < end)
    {
        i32 lineEndSize;
        cursorEnd = ZPG_ScanForLineEnd(cursor, end, &lineEndSize);
        
        zpgSize len = cursorEnd - cursor;
        if ((char)*cursor != '#' && len >= 2)
        {
            const zpgSize bufSize = 256;
            u8 buf[bufSize];
            ExtractLine(buf, bufSize, cursor, cursorEnd);
            // i32 err = ZPG_ExecuteLine(&ctx, cursor, cursorEnd, line);
            i32 err = ExecLine(&ctx, (char*)buf);
            if (err != 0)
            {
                printf("ABORT Error %d executing line '%s'\n", err, (char*)buf);
                cursor = end;
                result = err;
                break;
            }
        }
        cursor = cursorEnd + lineEndSize;
        line++;
    }
    FreeContext(&ctx);
    // ZPG_PrintAllocations();
    return result;
}


#endif // ZPG_SCRIPT_H