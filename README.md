# zpg - Zealous Procedural Generator

Library to hold various routines for procedural level generation for use in games.

Output is purely grid based so far. Currently features random walk for creating paths and cellular automata/perlin noise for creating random caves, and some experiments on creating rooms by joining randomly scattered numbers.

## Status

Very much work in progress. Has not been properly "dogfooded" with a game yet.
Interface and build and means of reading output are to be rewritten.


## Modes

### preset

Runs a hardcoded sequence of procedures, with some ability to customise the output (eg size, or initial seed)

Example command lines:

```zpg preset offset_around_4_prefabs -p -w 96 -h 48```

```zpg preset test_room_connections -p -w 20 -h 20 -v```

### script

Runs a line separated set of commands. An example script:

```
set verbosity 0
init_stack 2 64 32
seed
grid_set_all 0 2
stencil 0
caves 0 -1 1 2
drunk 0 -1 0 31 15 40 -1 0
drunk 0 -1 0 31 15 40 0 -1
drunk 0
drunk 0
drunk 0 -1 0 0 0
grid_save 0 output.txt
grid_print 0
```

To run the script:
```zpg script my_script.txt```

### REPL

Starts a shell that allows script commands to be entered at runtime. Useful for experimentation.

```zpg repl```

## Example output

![Drunken walks](https://pbs.twimg.com/media/FeA1h6JWQAABrAz?format=png&name=large)
![Room seeding](https://pbs.twimg.com/media/FeA1h6LXoAECfrV?format=png&name=large)
