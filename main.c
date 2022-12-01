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
#else
        printf("(Debugging mode)\n");
#endif
#ifndef DEBUG
        refresh();
        drawEnd();
#endif
        return 0;
}
