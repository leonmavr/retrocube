#include "draw.h"
#include "types.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>


//#define DEBUG

int main(char** argv, int argc) {
        Cube* cube = cubeNew(0, 0, 0, 12);
        drawInit();
#ifndef DEBUG
#endif 
        drawPixel(0, 0, 'o');
        drawPixel(-10, -10, '#');
        printf("(Debugging mode)\n");

#ifndef DEBUG
        refresh();
        drawEnd();
#endif
        return 0;
}
