#include "draw.h"
#include "vector.h"
#include "objects.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//#define DEBUG


int main() {
    draw__init();
#ifndef DEBUG
    printf("starting\n");
    int x = 20, y = 20, z = 40;
    cube_t* cube = obj__cube_new(x, y, z, 14);
    ray_t* ray = obj__ray_new(0, 0, 0);
    plane_t* plane = obj__plane_new(cube->vertices[0], cube->vertices[1], cube->vertices[2]);
    obj__cube_rotate(cube, 4.8, 3.2, 5.7);
    draw__pixel(g_min_cols, g_min_rows, '*');
    draw__pixel(g_max_cols, g_max_rows-1, '*');
    draw__pixel(0, 0, 'O');
    //draw__pixel(-10, -10, '*');
    //draw__surface(cube->vertices[0], cube->vertices[1], cube->vertices[2], cube->vertices[3], 'O');
    draw__cube(cube);
#else
    printf("r = %d, c = %d\n", g_min_rows, g_min_cols);
    cube_t* cube = obj__cube_new(0, 0, 20, 10);
    obj__cube_rotate(cube, 0, 60, 20); 
    ray_t* ray = obj__ray_new(1, 50, 20);
    vec3i_t* p0 = vec__vec3i_new(-1, 1, 2);
    vec3i_t* p1 = vec__vec3i_new(-4, 2, 2);
    vec3i_t* p2 = vec__vec3i_new(-2, 1, 5);
    plane_t* pl = obj__plane_new(p0, p1, p2);
    printf("%d, %d, %d, %d\n", pl->normal->x, pl->normal->y, pl->normal->z, pl->offset);
    vec3i_t inters = obj__ray_plane_intersection(pl, ray);
    printf("%d, %d, %d\n", inters.x, inters.y, inters.z);
    printf("(Debugging mode)\n");
#endif
#ifndef DEBUG
    refresh();
    draw__end();
#endif
    return 0;
}
