
#include <stdio.h>

#include "zpg.h"

int main(int argc, char** argv)
{
	printf("Read %d params\n", argc);
	ZPG_Hello();
	ZPG_RunPreset(1); // standard drunken walk
	//ZPG_RunPreset(8);
	//ZPG_RunPreset(9); // embed
	//ZPG_RunPreset(10); // blit
}