#include "vector.h"
#include "objects.h"
#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

// rows, columns and aspect ratio of the terminal
int g_rows;
int g_cols;
int g_min_rows;
int g_max_rows;
int g_min_cols;
int g_max_cols;
float g_aspect_ratio_screen;
// aspect ratio of each character
float g_aspect_ratio_char;


void draw__init() {
    // start the curses mode
    initscr();
    curs_set(0);
    // get the number of rows and columns
    getmaxyx(stdscr, g_rows, g_cols);
    g_min_rows = -g_rows;
    g_max_rows = g_rows + 1;
    g_min_cols = -g_cols/2 + 1;
    g_max_cols = g_cols/2;
    // find terminal window's aspect ratio
    struct winsize wsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
    g_aspect_ratio_screen = (float)wsize.ws_col/wsize.ws_row;
    g_aspect_ratio_char = (float)wsize.ws_xpixel/wsize.ws_ypixel;
    printf("screen:\th = %d, w = %d, %.2f\n", wsize.ws_row, wsize.ws_col, g_aspect_ratio_screen);
    printf("char:\th = %d, w = %d, %.2f\n", wsize.ws_xpixel, wsize.ws_ypixel, g_aspect_ratio_char);
}

/* Uses the following coordinate system:
 *
 *      ^ y
 *      |
 *      |
 *      |
 *      |
 *      o---------> x
 *       \
 *        \
 *         v z
 */
void draw__pixel(int x, int y, char c) {
    int yScaled = y/(g_aspect_ratio_screen/g_aspect_ratio_char);
    mvaddch(yScaled+g_rows/2, x+g_cols/2, c);
}

void draw__end() {
    getch();
    endwin();
}

// TODO: static
void draw__surface(vec3i_t* pt1, vec3i_t* pt2, vec3i_t* pt3, vec3i_t* pt4) {
    int z;
    ray_t* ray = obj__ray_new(0, 0, 0);
    plane_t* plane = obj__plane_new(pt1, pt2, pt3);
    for (int i = g_min_rows; i <= g_max_rows; ++i) {
        for (int j = g_min_cols; j <= g_max_cols; ++j) {
            // which z the ray hits the plane
            z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            if (obj__ray_hits_rectangle(ray, pt1, pt2, pt3, pt4)) {
                draw__pixel(j, i, '#');
            }
        }
    }

}

void draw__cube(int x, int y, int z, int side) {

}
