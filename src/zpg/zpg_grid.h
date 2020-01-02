#ifndef ZPG_GRID_H
#define ZPG_GRID_H

#include "../zpg.h"


    
static void ZPG_Grid_PrintValues(ZPGGrid* grid)
{
    if (grid == NULL) { return; }
    printf("------ Grid %d/%d ------\n", grid->width, grid->height);
    for (i32 y = 0; y < grid->height; ++y)
    {
        printf("|");
        for (i32 x = 0; x < grid->width; ++x)
        {
            ZPGCell *cell = grid->GetCellAt(x, y);
            printf("%d", cell->tile.type);
        }
        printf("|\n");
    }
    printf("------------------\n");
}

#endif // ZPG_GRID_H