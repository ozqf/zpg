
#include <stdio.h>
#include <string.h>

#include "zpg.h"

static i32 g_numAllocs = 0;
static u32 g_totalAllocated = 0;

//#define HEAP_ALLOC_TEST

// allocate a big heap
static u8* g_heap;
static u32 g_heapSize;
static u32 g_cursor = 0;

static void* track_malloc(size_t size)
{
#ifndef HEAP_ALLOC_TEST
	void* ptr = malloc(size);
	g_numAllocs += 1;
	g_totalAllocated += size;
	return ptr;
#else
	u8* result = &g_heap[g_cursor];
	g_cursor += (u32)size;
	return (void*)result;
#endif
}

static void track_free(void* ptr)
{
#ifndef HEAP_ALLOC_TEST
	free(ptr);
#endif
}

static void FatalHandler(const char* msg)
{
	printf("ZPG - fatal error\n");
    printf("%s\n", msg);
	printf("\nAborting\n");
	exit(1);
}

static void print_help(char* exeName)
{
	printf("--- Help ---\n");
	printf("Modes are preset, script or data\n");
	printf("eg:\n:");
	printf("\t%s preset <options>\n", exeName);
}

static void run_preset_cli(i32 argc, char** argv)
{
	ZPG_RunPresetCLI(argc, argv, NULL, NULL, NULL);
}

static void run_script(char* inputFileName, char* outputFileName)
{
	i32 dashes = 80;
	while (dashes-- > 0) { printf("-"); }
	printf("\n");
	char* path = inputFileName;
	FILE* f;
	fopen_s(&f, path, "r");
	if (f == NULL)
	{
		printf("Failed to open script %s\n", path);
		return;
	}
	fseek(f, 0, SEEK_END);
	i32 size = ftell(f);
	printf("Reading %d chars\n", size);
	fseek(f, 0, SEEK_SET);
	u8* buf = (u8*)malloc(size + 1);
	memset(buf, 0, size);
	buf[size] = '\0';
	fread(buf, 1, size, f);
	fclose(f);
	printf("Read script:\n%s\n", buf);
	i32 flags = ZPG_API_FLAG_PRINT_WORKING | ZPG_API_FLAG_PRINT_RESULT;
	ZPG_RunScript(buf, size, flags);
	free(buf);

}

int main(int argc, char** argv)
{
	printf("Zealous Procedural Generator build %s, %s\n",
		__DATE__, __TIME__);
	#if 1
	// ZPG_Init(NULL, NULL);
#ifdef HEAP_ALLOC_TEST
	g_heapSize = 1024 * 1024 * 128;
	g_heap = (u8*)malloc(g_heapSize);
#endif

	 // for debugging - force params
    #if 0
    char* arrArgs[32];
    argc = 0;
    arrArgs[argc++] = "zpg.exe";
    arrArgs[argc++] = "preset";
    arrArgs[argc++] = "13";
    arrArgs[argc++] = "-e";
    arrArgs[argc++] = "-p";
    arrArgs[argc++] = "-s";
    arrArgs[argc++] = "42";
    arrArgs[argc++] = "-v";
    arrArgs[argc++] = "-w";
    arrArgs[argc++] = "8";
    arrArgs[argc++] = "-h";
    arrArgs[argc++] = "8";
    argv = arrArgs;
    #endif
    #if 0
    char* arrArgs[32];
    argc = 0;
    arrArgs[argc++] = "zpg.exe";
    arrArgs[argc++] = "preset";
    arrArgs[argc++] = "12";
    arrArgs[argc++] = "-v";
    arrArgs[argc++] = "-e";
    arrArgs[argc++] = "-p";
    arrArgs[argc++] = "-s";
    arrArgs[argc++] = "42";
    argv = arrArgs;
    #endif
    #if 0
    char* arrArgs[32];
    argc = 0;
    arrArgs[argc++] = "zpg.exe";
    arrArgs[argc++] = "preset";
    arrArgs[argc++] = "11";
    arrArgs[argc++] = "-v";
    arrArgs[argc++] = "-p";
    arrArgs[argc++] = "-s";
    arrArgs[argc++] = "1610110326";
    argv = arrArgs;
    #endif
    
	// Run!
	ZPG_Init(track_malloc, track_free, FatalHandler);
	run_preset_cli(argc, argv);
	#endif

	printf("\nDone\n");
	return 0;
}