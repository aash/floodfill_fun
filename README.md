# Build with msvc (other compilers aren't tested)
```
$ git clone https://github.com/aash/floodfill_fun
$ cd floodfill_fun
$ mkdir build
$ cmake .. -G <generator> # ex. Ninja, see cmake --help
```
# Run
```
$ floodfill_fun 26 2000 2000 1000 1000
bound: 26
width: 2000
height: 2000
px: 1000
py: 1000
number of connected pixels: 148848
```
where last line is the number of 4-connected pixels.
# TODO
- add map reduce concurrency pattern for floodfill, core of the problem is reduction, research needed
- implement scan-line floodfill algorithm
- add tests
- run memory, thread sanitizers
- check popular compilers/environments: clang, gcc, mingw, msys, linux
# Concurrent floodfill
1. calculate floodfill on the chunk where (x, y) belong, mark chunk as processed
2. deduce edges for upper, lower, left and right sides, each of them form a set of points (T, B, L, R), filter non-empty ones
3. spawn new tasks for each non-empty edge
4. for-each pixel on the edge, run floodfill on the left, right, top, bottom chunks touching previously processed
5. repeat from 2
