/*
TODO: This class should also hold contexts.
outputs should be assigned to contexts, allowing multiple contexts to be run.
In fact, all allocations should be assigned to a context.
*/
#include "../zpg_internal.h"

#define ZPG_MAX_DATA_ITEMS 64
// static ZPGContext g_contexts[ZPG_MAX_DATA_ITEMS];
// static i32 g_numContexts = 0;
static ZPGOutput g_outputs[ZPG_MAX_DATA_ITEMS];
// static i32 g_numOutputs = 0;


// Reading results from scripts
ZPG_EXPORT i32 ZPG_GetOutputsCount(zpgHandle context)
{
    i32 total = 0;
    for (i32 i = 0; i < ZPG_MAX_DATA_ITEMS; ++i)
    {
        if (g_outputs[i].format != 0)
        {
            total++;
        }
    }
    return total;
}

ZPG_EXPORT zpgSize ZPG_GetOutputSize(zpgHandle context, zpgHandle output)
{
    if (output < 0 || output > ZPG_MAX_DATA_ITEMS) { return 0; }
    return g_outputs[output].size;
}

ZPG_EXPORT i32 ZPG_GetOutputFormat(zpgHandle context, zpgHandle output)
{
    if (output < 0 || output > ZPG_MAX_DATA_ITEMS) { return ZPG_OUTPUT_FORMAT_EMPTY; }
    return g_outputs[output].format;
}

ZPG_EXPORT void* ZPG_GetOutputData(zpgHandle context, zpgHandle output)
{
    if (output < 0 || output > ZPG_MAX_DATA_ITEMS) { return NULL; }
    return g_outputs[output].ptr;
}

ZPG_EXPORT zpgError ZPG_FreeOutput(zpgHandle context, zpgHandle index)
{
    if (index < 0 || index > ZPG_MAX_DATA_ITEMS) { return 1; }
    if (g_outputs[index].format == ZPG_OUTPUT_FORMAT_EMPTY)
    { return 1; }
    ZPG_Free(g_outputs[index].ptr);
    g_outputs[index].size = 0;
    g_outputs[index].format = 0;
    return 0;
}

ZPG_EXPORT void ZPG_FreeAllOutputs(zpgHandle context)
{
    for (i32 i = 0; i < ZPG_MAX_DATA_ITEMS; ++i)
    {
        if (g_outputs[i].format == ZPG_OUTPUT_FORMAT_EMPTY)
        { continue; }
        ZPG_Free(g_outputs[i].ptr);
        g_outputs[i].size = 0;
        g_outputs[i].format = ZPG_OUTPUT_FORMAT_EMPTY;
    }
}

static ZPGOutput* AssignOutput(zpgHandle context, zpgHandle index, i32 format)
{
    if (format == ZPG_OUTPUT_FORMAT_EMPTY) { return NULL; }

    ZPGOutput* o = &g_outputs[index];
    if (o->format != ZPG_OUTPUT_FORMAT_EMPTY)
    {
        // delete!
        ZPG_FreeOutput(context, index);
    }

    o->id = index;
    o->format = format;
    return o;
}

ZPG_EXPORT zpgError ZPG_AddOutput(
    zpgHandle context, zpgHandle output, i32 format, void* ptr, zpgSize numBytes)
{
    if (output < 0 || output > ZPG_MAX_DATA_ITEMS) { return 1; }
    // zpgHandle handle = g_numOutputs++;
    // ZPGOutput* o = &g_outputs[handle];
    ZPGOutput* o = AssignOutput(context, output, format);
    if (o == NULL)
    {
        printf("Output %d not found\n", output);
    }
    o->format = format;
    o->ptr = ptr;
    o->size = numBytes;
    return 0;
}
