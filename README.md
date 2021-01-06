# zpg - Zealous Procedural Generator

Library to hold various routines for procedural level generation I plan to use in various computer game projects.

Output is purely grid based so far. Currently features random walk for creating paths and cellular automata/perlin noise for creating random caves.

Plan to replace the current grid data structure with a grid 'stack' instead,
where each grid cell has a single byte data value and extra grid layers are
added when new fields are necessary.

An example grid stack might be something like this:
0 Seed value - initial random number (could be overwritten)
1 Adjusted seed value (eg after some iteration process)
2 object rooms stencil (stencil prevents other processes from overwriting)
3 Room Id
4 Room neighbour flags (1 bit for eight neighbours)
5 Room to room flags (1 bit for eight neighbours)
6 height map value
7 cave backdrop
8 styling flags
... etc
