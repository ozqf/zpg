
#include <stdio.h>

#include "zpg.h"

int main(int argc, char** argv)
{
	printf("Zealous Procedural Generator Init\n");
	ZPG_Init();
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
		return 0;
	}
}