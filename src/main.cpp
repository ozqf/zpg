
#include <stdio.h>
#include <string.h>

#include "zpg.h"

static i32 g_numAllocs = 0;
static u32 g_totalAllocated = 0;

// allocate a big heap
static u8* g_heap;
static u32 g_heapSize;
static u32 g_cursor = 0;

static void* track_malloc(size_t size)
{
	u8* result = &g_heap[g_cursor];
	g_cursor += (u32)size;
	return (void*)result;
	/*void* ptr = malloc(size);
	g_numAllocs += 1;
	g_totalAllocated += size;
	return ptr;*/
}

static void track_free(void* ptr)
{
	//free(ptr);
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
	g_heapSize = 1024 * 1024 * 128;
	g_heap = (u8*)malloc(g_heapSize);
	ZPG_Init(track_malloc, track_free);
	run_preset_cli(argc, argv);
	#endif

	#if 0
	if (argc <= 1)
	{
		printf("No command specified\n");
		print_help(argv[0]);
		return 0;
	}
	if (strcmp(argv[1], "preset") == 0)
	{
		ZPG_Init(NULL, NULL);
		run_preset_cli(argc, argv);
	}
	else if (strcmp(argv[1], "script") == 0)
	{
		printf("Sorry, script mode disabled\n");
		#if 0
		ZPG_Init(NULL, NULL);
		run_script(argv[2], argv[3]);
		#endif
	}
	else if (strcmp(argv[1], "data") == 0)
	{
		ZPG_Init(NULL, NULL);
		printf("--- DATA ---\n");
		ZPG_PrintTileTypes();
		ZPG_PrintPrefabs();
	}
	else
	{
		printf("Unrecognised command \"%s\"\n", argv[1]);
		print_help(argv[0]);
		return 0;
	}
	#endif
	printf("\nDone\n");
	return 0;
}