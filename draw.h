#ifndef DRAW_H
#define DRAW_H

extern int g_rows;
extern int g_cols;


void drawInit();
void drawPixel(int x, int y, char c);
void drawEnd();

#endif /* DRAW_H */
