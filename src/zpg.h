#ifndef ZPG_H
#define ZPG_H

#ifdef ZPG_WINDOWS_DLL 
#define ZPG_EXPORT extern "C" __declspec(dllexport)
#else
#define ZPG_EXPORT extern "C"
#endif

#include "zpg/zpg_common.h"
#include <stdlib.h>

#define ZPG_ERROR_NONE 0
#define ZPG_ERROR_UNKNOWN 1
#define ZPG_ERROR_UNRECOGNISED_OPTION 2
#define ZPG_ERROR_MISSING_PARAMETER 3
#define ZPG_ERROR_NULL_PARAMETER 4
#define ZPG_ERROR_TARGET_IS_NOT_SET 5
#define ZPG_ERROR_OUT_OF_BOUNDS 6
#define ZPG_ERROR_FAILED_TO_OPEN_FILE 7

#define ZPG_OUTPUT_FORMAT_EMPTY 0
#define ZPG_OUTPUT_FORMAT_ASCI_GRID 1

#define ZPG_API_FLAG_PRINT_RESULT (1 << 0)
#define ZPG_API_FLAG_PRINT_WORKING (1 << 1)
#define ZPG_API_FLAG_STEP_THROUGH (1 << 2)
#define ZPG_API_FLAG_NO_ENTITIES (1 << 3)
#define ZPG_API_FLAG_PRINT_GREYSCALE (1 << 4)
#define ZPG_API_FLAG_PRINT_FINAL_ALLOCS (1 << 5)
#define ZPG_API_FLAG_SOLID_BORDER (1 << 6)

typedef int zpgError;
typedef i32 zpgHandle;

// void *(__cdecl *)(size_t)
typedef void* (*zpg_allocate_fn)(size_t size);
typedef void (*zpg_free_fn)(void* ptr);
typedef void (*zpg_fatal_fn)(const char* msg);

//////////////////////////////////////////
// Functions
//////////////////////////////////////////
// API version is for external users to check the interface they are calling
ZPG_EXPORT i32 ZPG_ApiVersion();
// On error init returns a none 0 code
// TODO: Init is called automatically anyway in run functions
ZPG_EXPORT i32 ZPG_Init(zpg_allocate_fn ptrAlloc, zpg_free_fn ptrFree, zpg_fatal_fn fatal);
// Shutdown clears any still allocated memory on error returns none 0
ZPG_EXPORT i32 ZPG_Shutdown();
//ZPG_EXPORT void ZPG_RunPreset(i32 mode, char* outputPath, i32 apiFlags,
//    u8** resultPtr, i32* resultWidth, i32* resultHeight);
ZPG_EXPORT void ZPG_RunPresetCLI(
    i32 argc, char** argv,
    u8** resultPtr, i32* resultWidth, i32* resultHeight);
ZPG_EXPORT i32 ZPG_BeginREPL();
ZPG_EXPORT zpgHandle ZPG_CreateContext();
ZPG_EXPORT i32 ZPG_RunScript(u8* text, i32 textLength, i32 apiFlags);

// Reading results from scripts
ZPG_EXPORT zpgHandle ZPG_AddOutput(
    zpgHandle context, zpgHandle output, i32 format, void* ptr, zpgSize numBytes);
ZPG_EXPORT i32          ZPG_GetOutputsCount();
ZPG_EXPORT zpgSize      ZPG_GetOutputSize(zpgHandle context, zpgHandle output);
ZPG_EXPORT i32          ZPG_GetOutputFormat(zpgHandle context, zpgHandle output);
ZPG_EXPORT void*        ZPG_GetOutputData(zpgHandle context, zpgHandle output);
ZPG_EXPORT zpgError     ZPG_FreeOutput(zpgHandle context, zpgHandle index);
ZPG_EXPORT void         ZPG_FreeAllOutputs(zpgHandle context);

ZPG_EXPORT void ZPG_PrintPrefabs();
ZPG_EXPORT void ZPG_PrintTileTypes();

#endif // ZPG_H