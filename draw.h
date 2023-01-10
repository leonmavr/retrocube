#ifndef DRAW_H
#define DRAW_H

extern int g_rows;
extern int g_cols;
extern int g_min_rows;
extern int g_max_rows;
extern int g_min_cols;
extern int g_max_cols;


void draw__init();
void draw__pixel(int x, int y, char c);
void draw__end();
void draw__cube(int x, int y, int z, int side);

#endif /* DRAW_H */
