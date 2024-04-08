# Build
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
where last line is the number of 4-connected pixels.number of connected pixels: 148848