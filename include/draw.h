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
// stores the pixels to be drawn on the screen
extern color_t* g_screen_buffer;
extern int g_screen_buffer_size;

/**
 * @brief Initialises the screen buffer and prepares terminal for writing
 */
void draw_init();
void draw_write_pixel(int x, int y, color_t c);
void draw_flush_screen();
void draw_end();
void draw_cube(cube_t* cube);


#endif /* DRAW_H */
