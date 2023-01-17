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


void draw__init();
void draw__pixel(int x, int y, char c);
void draw__clear();
void draw__end();
void draw__cube(cube_t* cube);


#endif /* DRAW_H */
