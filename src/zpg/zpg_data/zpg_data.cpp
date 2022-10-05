
#include "../zpg_internal.h"

#define ZPG_MAX_DATA_ITEMS 64
static ZPGContext g_contexts[ZPG_MAX_DATA_ITEMS];
static i32 g_numContexts = 0;
static ZPGOutput g_outputs[ZPG_MAX_DATA_ITEMS];
static i32 g_numOutputs = 0;


// Reading results from scripts
ZPG_EXPORT i32 ZPG_GetOutputsCount()
{
    return g_numOutputs;
}

ZPG_EXPORT zpgSize ZPG_GetOutputSize(zpgHandle context, zpgHandle output)
{
    i32 i = output;
    if (i < 0 || i >= g_numOutputs) { return 0; }
    return g_outputs[i].size;
}

ZPG_EXPORT i32 ZPG_GetOutputFormat(zpgHandle context, zpgHandle output)
{
    i32 i = output;
    if (i < 0 || i >= g_numOutputs) { return 0; }
    return g_outputs[i].format;
}

ZPG_EXPORT void* ZPG_GetOutputData(zpgHandle context, zpgHandle output)
{
    i32 i = output;
    if (i < 0 || i >= g_numOutputs) { return NULL; }
    return g_outputs[i].ptr;
}

ZPG_EXPORT zpgHandle ZPG_AddOutput(i32 format, void* ptr, zpgSize numBytes)
{
    zpgHandle handle = g_numOutputs++;
    ZPGOutput* o = &g_outputs[handle];
    o->format = format;
    o->ptr = ptr;
    o->size = numBytes;
    return handle;
}

ZPG_EXPORT void ZPG_PurgeOutputs()
{
    for (i32 i = 0; i < g_numOutputs; ++i)
    {
        ZPG_Free(g_outputs[i].ptr);
        g_outputs[i].size = 0;
        g_outputs[i].format = 0;
    }
    g_numOutputs = 0;
}
