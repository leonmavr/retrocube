#ifndef DRAW_H
#define DRAW_H

#include "vector.h"
#include "objects.h"

extern int g_rows;
extern int g_cols;
extern int g_min_rows;
extern int g_max_rows;
extern int g_min_cols;
extern int g_max_cols;


void draw_init();
void draw_pixel(int x, int y, char c);
void draw_clear();
void draw_end();
void draw_cube(cube_t* cube);


#endif /* DRAW_H */
