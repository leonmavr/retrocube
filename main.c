#include "draw.h"
#include "types.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>


#define DEBUG


int main(char** argv, int argc) {
        drawInit();
#ifndef DEBUG
        Cube* cube = cubeNew(0, 0, 20, 6);
        Ray* ray = rayNew(10, 5, 2);
        drawPixel(0, 0, 'O');
        drawPixel(1, 0, '#');
        drawPixel(2, 0, '#');
        drawPixel(3, 0, '#');
        drawPixel(40, 0, '#');
        drawPixel(40, 40, '#');
#else
        vec3i_t* p0 = vec3i_new(-1, 1, 2);
        vec3i_t* p1 = vec3i_new(-4, 2, 2);
        vec3i_t* p2 = vec3i_new(-2, 1, 5);
        Plane* pl = plane_new(p0, p1, p2);
        printf("%d, %d, %d, %d\n", pl->normal->x, pl->normal->y, pl->normal->z, pl->offset);
        printf("(Debugging mode)\n");
#endif
#ifndef DEBUG
        refresh();
        drawEnd();
#endif
        return 0;
}
