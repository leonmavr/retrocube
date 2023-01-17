#include "draw.h"
#include "vector.h"
#include "objects.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main() {
    draw__init();
    int x = 0, y = 0, z = 300;
    cube_t* cube = obj__cube_new(x, y, z, 40);
    ray_t* ray = obj__ray_new(0, 0, 0);
    plane_t* plane = obj__plane_new(cube->vertices[0], cube->vertices[1], cube->vertices[2]);
    obj__cube_rotate(cube, 2.8, 2.2, 4.7);
    draw__cube(cube);
    refresh();
    clear();

    int j = 0;
    int n = 0;
    while(1) {
        clear();
        for (int i = 0; i < 9999; i++)
            j += 0.00001*cos(2*i); 
        obj__cube_rotate(cube, cos(n/100.0)*4, sin(1.5*n/100.0)*4, 4*sin(0.5*n/100.0));
        draw__cube(cube);
        refresh();
        n++;
        if (n == 2000000) n = 0;
    }

    draw__end();

    return 0;
}
