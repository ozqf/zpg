#ifndef ZPG_COMMAND_HISTORY_H
#define ZPG_COMMAND_HISTORY_H

#include "zpg_internal.h"

// record of all commands executed, separated by '\n'.
#define ZPG_MAX_HISTORY_CHARS (1024 * 1024)
static char* g_commandHistory[ZPG_MAX_HISTORY_CHARS];

static void ZPG_HistoryClear(ZPGContext* ctx)
{
	
}

static void ZPG_HistoryAppend(ZPGContext* ctx, char* cmd)
{
    
}

#endif // ZPG_COMMAND_HISTORY_H