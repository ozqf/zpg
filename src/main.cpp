
#include <stdio.h>

#include "zpg.h"

int main(int argc, char** argv)
{
	printf("Zealous Procedural Generator Init\n");
	ZPG_Init();
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
	#if 1
	i32 dashes = 80;
	while (dashes-- > 0) { printf("-"); }
	printf("\n");
	char* path = "test_script.txt";
	FILE* f;
	fopen_s(&f, path, "r");
	if (f == NULL)
	{
		printf("Failed to open script %s\n", path);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	i32 size = ftell(f);
	printf("Reading %d chars\n", size);
	fseek(f, 0, SEEK_SET);
	u8* buf = (u8*)malloc(size);
	fread(buf, 1, size, f);
	fclose(f);
	printf("Read script:\n%s\n", buf);
	ZPG_RunScript(buf, size);
	free(buf);

	#endif

	printf("...Done\n");
	return 0;
}