#ifndef DRAW_H
#define DRAW_H

extern int g_rows;
extern int g_cols;
extern int g_minRows;
extern int g_maxRows;
extern int g_minCols;
extern int g_maxCols;


void draw__init();
void draw__pixel(int x, int y, char c);
void draw__end();

#endif /* DRAW_H */
