#include "draw.h"
#include "types.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>


//#define DEBUG


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
#endif 
#ifndef DEBUG
#if 0
        for (int i = -g_rows/2-1; i < g_rows/2; ++i) {
            for (int j = -g_cols/2-1; j < g_cols/2; ++j) {
                Vec3i pt = (Vec3i) {j, i, 20};
                rayPointTo(ray, j, i, 20);
                Vec3i inters = rayPlaneIntersection(ray, cube->vertices[4], cube->vertices[5], cube->vertices[6], cube->vertices[7]);
                if ((inters.x != 0) && (inters.y != 0) && (inters.z != 0))
                    drawPixel(j, i, '#');
            }
        }
#endif
#endif
        printf("(Debugging mode)\n");

#ifndef DEBUG
        refresh();
        drawEnd();
#endif
        return 0;
}
