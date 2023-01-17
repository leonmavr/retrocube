#include "draw.h"
#include "vector.h"
#include "objects.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main() {
    draw__init();
    int x = 20, y = 20, z = 80;
    cube_t* cube = obj__cube_new(x, y, z, 14);
    ray_t* ray = obj__ray_new(0, 0, 0);
    plane_t* plane = obj__plane_new(cube->vertices[0], cube->vertices[1], cube->vertices[2]);
    obj__cube_rotate(cube, 2.8, 2.2, 4.7);
    draw__cube(cube);
    refresh();
    clear();

    int j = 0;
    for (int n = 0; n < 20; ++n) {
        clear();
        for (int i = 0; i < 999999; i++)
            j += 0.00001*cos(2*i); 
        obj__cube_rotate(cube, 0.05, 0.05, 0.05);
        draw__cube(cube);
        refresh();
    }

    draw__end();

    return 0;
}
