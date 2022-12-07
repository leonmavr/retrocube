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
        //drawPixel(-g_rows/2, -g_cols/2, '*');
        drawPixel(0, 0, 'O');
        drawPixel(-10, -10, '*');
        drawPixel(-g_cols/2, g_rows/1, 'M');
#if 1
        for (int i = g_minRows; i <= g_maxRows; ++i) {
            for (int j = g_minCols; j <= g_maxCols; ++j) {
                raySend(ray, j, i, 20);
                if (plane_rayHitsSurface(ray, cube->vertices[0], cube->vertices[1], cube->vertices[2], cube->vertices[3]))
                    drawPixel(j, i, '#');
            }
        }
#endif
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
