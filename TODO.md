# ZPG TODO and notes

## Core changes

### Script mode issues


* Issues arise from the fact that the Context struct and script commands only holds one grid stack.
	* Any process that involves enlarging a grid, such as room generate from random grid values, cannot be performed.


### Room generation

Requirement to enlarge from the room volumes grid by a factor of four means fitting a set of rooms inside another grid would be tricky. It prevents it from generating rooms in abritrary spaces.

eg from room seeds...
```
12
22
```
...to actual grid geometry it becomes:

```
########
#  ##  #
#  ##  #
#####  #
#####  #
#      #
#      #
########
```

In order to create walls around a room.
