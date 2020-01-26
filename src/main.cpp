
#include <stdio.h>
#include <string.h>

#include "zpg.h"

static void print_help(char* exeName)
{
	printf("--- Help ---\n");
	printf("\nRun a generator preset:\n");
	printf("preset <preset_mode> <output_file_name>\n");
	printf("\tPreset modes are currently 1-12\n");
	printf("\nExecute a build script:\n");
	printf("script <script_file_name> <output_file_name>\n");
	printf("\teg\n%s script make_dungeon.txt dungeon.txt\n", exeName);
}

static void run_preset(char* modeStr, char* outputFileName)
{
	i32 mode = atoi(modeStr);
	i32 flags = ZPG_API_FLAG_PRINT_WORKING | ZPG_API_FLAG_PRINT_RESULT;
	u8* cells;
	i32 width, height;
	ZPG_RunPreset(mode, outputFileName, flags, &cells, &width, &height);
	printf("Main read grid size %d/%d\n", width, height);

	//ZPG_RunPreset(1); // standard drunken walk
	//ZPG_RunPreset(2); // scattered walks
	//ZPG_RunPreset(3); // cave gen
	//ZPG_RunPreset(4); // walks from line segment
	//ZPG_RunPreset(5); // test draw lines
	//ZPG_RunPreset(6); // walk with space
	//ZPG_RunPreset(7); // perlin caves
	//ZPG_RunPreset(8);
	//ZPG_RunPreset(9); // embed
	//ZPG_RunPreset(10); // blit
	//ZPG_RunPreset(11); // walk fnrom prefab
	//ZPG_RunPreset(12); // walk between prefabs via line segment
	#if 0
	//printf("Press ENTER to exit\n");
	//getchar();
	#endif
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
	if (argc != 4)
	{
		printf("No command specified\n");
		print_help(argv[0]);
		return 0;
	}
	ZPG_Init(NULL, NULL);
	if (strcmp(argv[1], "preset") == 0)
	{
		run_preset(argv[2], argv[3]);
	}
	else if (strcmp(argv[1], "script") == 0)
	{
		run_script(argv[2], argv[3]);
	}
	#if 0
	printf("Read %d params\n", argc);
	if (argc == 1)
	{
		printf("No options specified - running tests\n");
		//ZPG_RunPreset(1); // standard drunken walk
		//ZPG_RunPreset(2); // scattered walks
		//ZPG_RunPreset(3); // cave gen
		//ZPG_RunPreset(4); // walks from line segment
		//ZPG_RunPreset(5); // test draw lines
		//ZPG_RunPreset(6); // walk with space
		ZPG_RunPreset(7); // perlin caves
		//ZPG_RunPreset(8);
		//ZPG_RunPreset(9); // embed
		//ZPG_RunPreset(10); // blit
		//ZPG_RunPreset(11); // walk fnrom prefab
		ZPG_RunPreset(12); // walk between prefabs via line segment
		#if 0
		//printf("Press ENTER to exit\n");
		//getchar();
		#endif
	}
	#endif
	ZPG_PrintTileTypes();
	ZPG_PrintPrefabs();
	printf("...Done\n");
	return 0;
}