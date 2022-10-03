# zpg - Zealous Procedural Generator

Library to hold various routines for procedural level generation I plan to use in various computer game projects.

Output is purely grid based so far. Currently features random walk for creating paths and cellular automata/perlin noise for creating random caves, and some experiments on creating rooms by joining randomly scattered numbers.

## Example command lines:

```zpg preset offset_around_4_prefabs -p -w 96 -h 48```

```zpg preset test_room_connections -p -w 20 -h 20 -v```

Also has a REPL mode that allows sequential commands to be entered manually

```zpg repl```

## Example output

![Drunken walks](https://pbs.twimg.com/media/FeA1h6JWQAABrAz?format=png&name=large)
![Room seeding](https://pbs.twimg.com/media/FeA1h6LXoAECfrV?format=png&name=large)
