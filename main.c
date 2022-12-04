#include "draw.h"
#include "types.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>


//#define DEBUG


int main() {
        drawInit();
#ifndef DEBUG
        printf("starting\n");
        Cube* cube = cubeNew(0, 0, 20, 6);
        Ray* ray = rayNew(10, 5, 2);
        Plane* plane = plane_new(cube->vertices[0], cube->vertices[1], cube->vertices[2]);
        drawPixel(2, 10, '#');
        drawPixel(0, 0, '#');
#else
        Ray* ray = rayNew(1, 50, 20);
        vec3i_t* p0 = vec3i_new(-1, 1, 2);
        vec3i_t* p1 = vec3i_new(-4, 2, 2);
        vec3i_t* p2 = vec3i_new(-2, 1, 5);
        Plane* pl = plane_new(p0, p1, p2);
        printf("%d, %d, %d, %d\n", pl->normal->x, pl->normal->y, pl->normal->z, pl->offset);
        vec3i_t inters = plane_intersectRay(pl, ray);
        printf("%d, %d, %d\n", inters.x, inters.y, inters.z);
        printf("(Debugging mode)\n");
#endif
#ifndef DEBUG
        refresh();
        drawEnd();
#endif
        return 0;
}
