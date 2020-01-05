
#include <stdio.h>

#include "zpg.h"

int main(int argc, char** argv)
{
	printf("Read %d params\n", argc);
	ZPG_Init();
	//ZPG_RunPreset(1); // standard drunken walk
	//ZPG_RunPreset(2); // scattered walks
	//ZPG_RunPreset(3); // cave gen
	//ZPG_RunPreset(4); // walks from line segment
	//ZPG_RunPreset(8);
	//ZPG_RunPreset(9); // embed
	//ZPG_RunPreset(10); // blit
	ZPG_RunPreset(11); // walk fnrom prefab
	printf("Press ENTER to exit\n");
	//getchar();
}