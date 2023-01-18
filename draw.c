#include "vector.h"
#include "draw.h" // g_colors
#include "objects.h"
#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <limits.h> // INT_MIN
#include <unistd.h>

// colors for each face of the cube
char g_colors[6] = {'~', '.', '=', '+', '%', '|'};

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


void draw_init() {
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
void draw_pixel(int x, int y, char c) {
    int y_scaled = y/(g_aspect_ratio_screen/g_aspect_ratio_char);
    mvaddch(y_scaled+g_rows/2, x+g_cols/2, c);
}

void draw_end() {
    getch();
    endwin();
}


void draw_cube(cube_t* cube) {
/*
 * This function renders the given cube by the basic ray tracing principle.
 *
 * A ray is shot from the origin to every pixel on the screen row by row.
 * For each screen coordinate, there can zero to two intersections with the cube.
 * If there is one, render the (x, y) of the intersection (not the x,y of the screen!).
 * If there are two, render the (x, y) of the closer intersection. In the figure below,
 * z_hit are the z of the two intersections and z_rend is the closer one.
 *
 * Note: the ray is not necessarily horizontal - it's only drawn this way for illustration
 * purposes. It intersects faces (p0, p1, p2, p3) and  (p4, p5, p6, p7)
 * 
 *                      O camera origin  
 *                       \
 *                        \
 *                         V ray
 *                    p3    \            p2                      o cube's centre 
 *                    +-------------------+                      + cube's vertices
 *                    | \     \           | \                    # ray-cube intersections
 *                    |    \   # z_rend   |    \                   (z_hit)
 *                    |      \  p7        |       \
 *                    |         +-------------------+ p6         ^ y
 *                    |         | \       .         |            |
 *                    |         |  \      .         |            |
 *                    |         |   \     .         |            o-------> x
 *                    |         |    \    .         |             \
 *                    |         |     \   .         |              \
 *                 p0 +---------|......\..+ p1      |               V z
 *                     \        |       \    .      |
 *                       \      |        #     .    |
 *                          \   |         \      .  |
 *                             \+----------\--------+
 *                              p4          \        p5
 *                                           \
 *                                            V
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
    ray_t* ray = obj_ray_new(0, 0, 0);
    //plane_t* plane = obj_plane_new(pt1, pt2, pt3);
    for (int i = g_min_rows; i <= g_max_rows; ++i) {
        for (int j = g_min_cols; j <= g_max_cols; ++j) {
            // we test whether the ray has hit the following surafaces:
            // (p0, p1, p2, p3), (p0, p4, p7, p3)
            // (p4, p5, p6, p7), (p5, p1, p2, p6)
            // (p7, p6, p2, p3), (p0, p4, p5, p1)

            // the final z where a pixel is to be drawn
            int z_rendered = INT_MIN;
            // which z the ray hits the plane - can be up to two hits
            int z_hit;
            // through (p0, p1, p2)
            plane_t* plane = obj_plane_new(p0, p1, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p1, p2, p3) && (z_hit > z_rendered)) {
                draw_pixel(j, i, g_colors[0]);
                z_rendered = z_hit;
            }
            // through (p0, p4, p7);
            obj_plane_set(plane, p0, p4, p7);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p4, p7, p3) && (z_hit > z_rendered)) {
                draw_pixel(j, i, g_colors[1]);
                z_rendered = z_hit;
            }
            // through (p4, p5, p6);
            obj_plane_set(plane, p4, p5, p6);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p4, p5, p6, p7) && (z_hit > z_rendered)) {
                draw_pixel(j, i, g_colors[2]);
                z_rendered = z_hit;
            }
            // through (p5, p1, p2);
            obj_plane_set(plane, p5, p1, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p5, p1, p2, p6) && (z_hit > z_rendered)) {
                draw_pixel(j, i, g_colors[3]);
                z_rendered = z_hit;
            }
            // through (p7, p6, p2);
            obj_plane_set(plane, p7, p6, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p7, p6, p2, p3) && (z_hit > z_rendered)) {
                draw_pixel(j, i, g_colors[4]);
                z_rendered = z_hit;
            } 
            // through (p0, p4, p5);
            obj_plane_set(plane, p0, p4, p5);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p4, p5, p1) && (z_hit > z_rendered)) {
                draw_pixel(j, i, g_colors[5]);
                z_rendered = z_hit;
            }
            obj_plane_free(plane);
        } /* for columns */
    } /* for rows */
    obj_ray_free(ray);
    // render with with ncurse's `refresh`
    refresh();
}
