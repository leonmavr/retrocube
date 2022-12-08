#include "draw.h"
#include "types.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>


//#define DEBUG


int main() {
        draw__init();
#ifndef DEBUG
        printf("starting\n");
        cube_t* cube = obj__cube_new(0, 0, 20, 6);
        ray_t* ray = obj__ray_new(10, 5, 2);
        plane_t* plane = obj__plane_new(cube->vertices[0], cube->vertices[1], cube->vertices[2]);
        //draw__pixel(-g_rows/2, -g_cols/2, '*');
        draw__pixel(0, 0, 'O');
        draw__pixel(-10, -10, '*');
        draw__pixel(-g_cols/2, g_rows/1, 'M');
#if 1
        for (int i = g_minRows; i <= g_maxRows; ++i) {
            for (int j = g_minCols; j <= g_maxCols; ++j) {
                obj__ray_send(ray, j, i, 20);
                if (obj__ray_hits_rectangle(ray, cube->vertices[0], cube->vertices[1], cube->vertices[2], cube->vertices[3]))
                    draw__pixel(j, i, '#');
            }
        }
#endif
#else
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
