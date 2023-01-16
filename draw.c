#include "vector.h"
#include "draw.h" // g_colors
#include "objects.h"
#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
// colors for each face of the cube
char g_colors[6] = {'#', '.', '=', '+', 'o', 'H'};

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
void draw__surface(vec3i_t* pt1, vec3i_t* pt2, vec3i_t* pt3, vec3i_t* pt4, char color) {
    ray_t* ray = obj__ray_new(0, 0, 0);
    plane_t* plane = obj__plane_new(pt1, pt2, pt3);
    for (int i = g_min_rows; i <= g_max_rows; ++i) {
        for (int j = g_min_cols; j <= g_max_cols; ++j) {
            // which z the ray hits the plane
            int z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            if (obj__ray_hits_rectangle(ray, pt1, pt2, pt3, pt4)) {
                draw__pixel(j, i, color);
            }
        }
    }
    // TODO: free ray and its members
}

void draw__cube(cube_t* cube) {
/*
 *          p3                  p2 
 *           +-------------------+
 *           | \                 | \
 *           |    \              |    \                  ^y
 *           |      \  p7        |       \               |
 *           |         +-------------------+ p6          |
 *           |         |         |         |             |
 *           |         |*(cx,xy,cz)        |             o-------> x
 *           |         |         |         |              \
 *           |         |         |         |               \
 *           |         |         |         |                v z
 *        p0 +---------|---------+ p1      |
 *            \        |           \       |
 *              \      |             \     |
 *                 \   |                \  |
 *                    \+-------------------+
 *                     p4                   p5
 */
    // aliases for cube's vertices
    vec3i_t* p0 = cube->vertices[0];
    vec3i_t* p1 = cube->vertices[1];
    vec3i_t* p2 = cube->vertices[2];
    vec3i_t* p3 = cube->vertices[3];
    vec3i_t* p4 = cube->vertices[4];
    vec3i_t* p5 = cube->vertices[5];
    vec3i_t* p6 = cube->vertices[6];
    vec3i_t* p7 = cube->vertices[7];
    ray_t* ray = obj__ray_new(0, 0, 0);
    //plane_t* plane = obj__plane_new(pt1, pt2, pt3);
    for (int i = g_min_rows; i <= g_max_rows; ++i) {
        for (int j = g_min_cols; j <= g_max_cols; ++j) {
            // we test whether the ray has hit the following surafaces:
            // (p0, p1, p2, p3), (p0, p4, p7, p3)
            // (p4, p5, p6, p7), (p5, p1, p2, p6)
            // (p7, p6, p2, p3), (p0, p4, p5, p1)
            size_t hit_count = 0;
            char rendered_color;
            int z_hit = -999999999;
            int z;
            plane_t* plane = obj__plane_new(p0, p1, p2);
            z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            //draw__pixel(10, 10, g_colors[0]);
            if (obj__ray_hits_rectangle(ray, p0, p1, p2, p3)) {
                draw__pixel(10, 10, g_colors[0]);
                // which z the ray hits the plane
                if (z > z_hit) {
                    draw__pixel(j, i, g_colors[0]);
                    z_hit = z;
                }
            }
            plane = obj__plane_new(p0, p4, p7);
            z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            if (obj__ray_hits_rectangle(ray, p0, p4, p7, p3)) {
                // which z the ray hits the plane
                if (z > z_hit) {
                    draw__pixel(j, i, g_colors[1]);
                    z_hit = z;
                }
            }
            plane = obj__plane_new(p4, p5, p6);
            z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            if (obj__ray_hits_rectangle(ray, p4, p5, p6, p7)) {
                // which z the ray hits the plane
                if (z > z_hit) {
                    draw__pixel(j, i, g_colors[2]);
                    z_hit = z;
                }
            }
            plane = obj__plane_new(p5, p1, p2);
            z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            if (obj__ray_hits_rectangle(ray, p5, p1, p2, p6)) {
                // which z the ray hits the plane
                if (z > z_hit) {
                    draw__pixel(j, i, g_colors[3]);
                    z_hit = z;
                }
            }
            plane = obj__plane_new(p7, p6, p2);
            z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            if (obj__ray_hits_rectangle(ray, p7, p6, p2, p3)) {
                // which z the ray hits the plane
                if (z > z_hit) {
                    draw__pixel(j, i, g_colors[4]);
                    z_hit = z;
                }
            } 
            plane = obj__plane_new(p0, p4, p5);
            z = obj__plane_z_at_xy(plane, j, i);
            obj__ray_send(ray, j, i, z);
            if (obj__ray_hits_rectangle(ray, p0, p4, p5, p1)) {
                // which z the ray hits the plane
                if (z > z_hit) {
                    draw__pixel(j, i, g_colors[5]);
                    z_hit = z;
                }
            }
        }
    }
}
