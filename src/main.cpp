
#include <stdio.h>

#include "zpg.h"

int main(int argc, char** argv)
{
	printf("Read %d params\n", argc);
	ZPG_Hello();
	//ZPG_RunTest(1);
	//ZPG_RunTest(8);
	ZPG_RunTest(9);
}