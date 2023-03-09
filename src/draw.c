#include "vector.h"
#include "draw.h"
#include "objects.h"
#include "utils.h"
#include <sys/ioctl.h>
#include <stdio.h>
#include <limits.h> // INT_MIN
#include <unistd.h> // STDOUT_FILENO
#include <stdlib.h> // exit
#include <stdbool.h> // true/false 
#include <string.h> // memset
#include <stddef.h> // size_t 

#ifndef _WIN32
#define IOCTL_SIZE_INVALID 0
//----------------------------------------------------------------------------------
// Linux POSIX terminal manipulation macros
//----------------------------------------------------------------------------------
#define SCREEN_CLEAR() printf("\033[H\033[J")
#define SCREEN_GOTO_TOPLEFT() printf("\033[0;0H")
#define SCREEN_HIDE_CURSOR() printf("\e[?25l")
#define SCREEN_SHOW_CURSOR() printf("\e[?25h")
#else
//----------------------------------------------------------------------------------
// Windows terminal manipulation macros
//----------------------------------------------------------------------------------
// Credits to @oogabooga:
// https://cboard.cprogramming.com/c-programming/161186-undefined-reference.html
#define SCREEN_CLEAR() do {                                  \
    COORD top_left = {0, 0};                                 \
    DWORD c_chars_written;                                   \
    CONSOLE_SCREEN_BUFFER_INFO csbi;                         \
    GetConsoleScreenBufferInfo(g_cons_out, &csbi);           \
    DWORD dw_con_size = csbi.dwSize.X * csbi.dwSize.Y;       \
    FillConsoleOutputCharacter(g_cons_out, ' ', dw_con_size, \
            top_left, &c_chars_written);                     \
    FillConsoleOutputAttribute(g_cons_out, csbi.wAttributes, \
            dw_con_size, top_left, &c_chars_written);        \
    SetConsoleCursorPosition(g_cons_out, top_left);          \
} while(0)
// Credits to @Jerry Coffin: https://stackoverflow.com/a/2732327
#define SCREEN_GOTO_TOPLEFT() do {                           \
    COORD pos = {0, 0};                                      \
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);         \
    SetConsoleCursorPosition(output, pos);                   \
} while(0)
#define SCREEN_HIDE_CURSOR() ;
#define SCREEN_SHOW_CURSOR() ;
#endif
//----------------------------------------------------------------------------------

// rows, columns and aspect ratio of the terminal
int g_rows;
int g_cols;
int g_min_rows;
int g_max_rows;
int g_min_cols;
int g_max_cols;
// columns over rows for the terminal 
static float g_cols_over_rows;
// screen resolution (pixels over pixels) 
static float g_screen_res;
color_t* g_screen_buffer;
size_t g_screen_buffer_size;
// defines a plane each time we're about to hit a pixel
plane_t* g_plane_test;

/**
 * @brief Checks whether a null-terminated array of characters represents
 *        a positive decimal number, e.g. 1.8999 or 1,002
 *
 * @param string A null-terminated array of chars
 *
 * @return true if the given string is numerical
 */
static bool draw__is_decimal(char* string) {
    bool ret = false;
    for (char* s = string; *s != '\0'; ++s) {
        if (((*s >= '0') && (*s <= '9')) ||
            (*s == '.') || (*s == ',') ||
            (*s == '\n'))
            ret = true;
        else
            return false;
    }
    return ret;
}

/**
 * @brief Attempt to get the screen info (size and resolution) in three ways:
 *            1.`ioctl` call - fails on some terminals
 *            2. (fallback) xrandr command
 *            3. (fallback) assume a common screen resolution, e.g. 1920/1080
 *        Writes to global variables `g_screen_res` and `g_cols_over_rows`,
 *        `g_rows`, `g_cols`, `g_min_rows`, `g_min_cols`, `g_max_rows`,
 *        `g_max_cols`
 */
static void draw__get_screen_info() {
    //// 1st way - ioctl call
    struct winsize wsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
    g_rows = wsize.ws_row;
    g_cols = wsize.ws_col;
    g_cols_over_rows = (float)g_cols/g_rows;
    g_min_rows = -g_rows;
    g_max_rows = g_rows+1;
    g_min_cols = -g_cols/2+1;
    g_max_cols = g_cols/2;
    if ((wsize.ws_xpixel != IOCTL_SIZE_INVALID) || (wsize.ws_ypixel != IOCTL_SIZE_INVALID)) {
        g_screen_res = (float)wsize.ws_xpixel/wsize.ws_ypixel;
        return;
    }

    //// 2nd way - xrandr command
    // Open the command for reading
    FILE *fp;
    char line[512];
    fp = popen("echo `xrandr --current | grep \'*\' | uniq | awk \'{print $1}\' | cut -d \'x\' -f1` / `xrandr --current | grep \'*\' | uniq | awk \'{print $1}\' | cut -d \'x\' -f2` | bc -l", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    // parse the output - it should only be the resolution
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (draw__is_decimal(line)) {
            g_screen_res = atof(line);
            pclose(fp);
            return;
        }
    }
    //// 3rd way - assume a common resolution
    g_screen_res = 1920.0/1080.0;
}

void draw_init() {
    SCREEN_HIDE_CURSOR();
    SCREEN_CLEAR();
    // get terminal's size info
    draw__get_screen_info();
    g_screen_buffer_size = g_rows*g_cols;
    g_screen_buffer = malloc(sizeof(color_t) * g_screen_buffer_size);
    vec3i_t dummy = {0, 0, 0};
    g_plane_test = obj_plane_new(&dummy, &dummy, &dummy);
}

void draw_write_pixel(int x, int y, color_t c) {
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
    int y_scaled = y/(g_cols_over_rows/g_screen_res) + g_rows/2;
    x += g_cols/2;
    if ((y_scaled*g_cols + x < g_screen_buffer_size) && (y_scaled*g_cols + x >= 0))
        g_screen_buffer[y_scaled*g_cols + x] = c;
}

void draw_flush_screen() {
    // BUG: central pixel is colored as background - mitigate it by copying from the left
    for (size_t i = 1; i < g_screen_buffer_size - 1; ++i)
        if ((g_screen_buffer[i-1] != ' ') && (g_screen_buffer[i] == ' ') && (g_screen_buffer[i+1]  != ' ')) {
            g_screen_buffer[i] = g_screen_buffer[i-1];
    }
    // render the screen buffer
    for (size_t i = 0; i < g_screen_buffer_size; ++i)
        putchar(g_screen_buffer[i]);
    memset(g_screen_buffer, ' ', sizeof(color_t) * g_screen_buffer_size);
    SCREEN_GOTO_TOPLEFT();
}

void draw_end() {
    free(g_screen_buffer);
    obj_plane_free(g_plane_test);
    SCREEN_CLEAR();
    SCREEN_SHOW_CURSOR();
}

void draw_shape(shape_t* shape, camera_t* camera) {
/*
 * This function renders the given cube by the basic ray tracing principle.
 *
 * A ray is shot from the origin to every pixel on the screen row by row.
 * For each screen coordinate, there can zero to two intersections with the cube.
 * If there is one, render the (x, y) of the intersection (not the x,y of the screen!).
 * If there are two, render the (x, y) of the closer intersection. In the figure below,
 * z_hit are the z of the two intersections and z_rend is the closest one.
 *
 * The ray below intersects faces (p0, p1, p2, p3) and  (p4, p5, p6, p7)
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
    // whether we want to use the perspective transform or not
    const bool use_persp = camera != NULL;
    const unsigned focal_length = (camera != NULL) ? camera->focal_length : 1;
    const vec3i_t ray_origin = (camera != NULL) ?
        (vec3i_t) {camera->x0, camera->y0, camera->focal_length} :
        (vec3i_t) {0, 0, 0};
    vec3i_t dummy_vec = {0, 0, 0};
    ray_t* ray = obj_ray_new(ray_origin.x, ray_origin.y, ray_origin.z,
        dummy_vec.x, dummy_vec.y, dummy_vec.z);
    plane_t* plane = obj_plane_new(&dummy_vec, &dummy_vec, &dummy_vec);
    const color_t background = ' ';
    // bounding box pixel indexes
    int xmin = UT_MIN(shape->bounding_box.x0, shape->bounding_box.x1);
    int ymin = UT_MIN(shape->bounding_box.y0, shape->bounding_box.y1);
    int xmax = UT_MAX(shape->bounding_box.x0, shape->bounding_box.x1);
    int ymax = UT_MAX(shape->bounding_box.y0, shape->bounding_box.y1);

    if (shape->type == TYPE_CUBE) {
        //// initialisations
        vec3i_t* p0 = shape->vertices[0];
        vec3i_t* p1 = shape->vertices[1];
        vec3i_t* p2 = shape->vertices[2];
        vec3i_t* p3 = shape->vertices[3];
        vec3i_t* p4 = shape->vertices[4];
        vec3i_t* p5 = shape->vertices[5];
        vec3i_t* p6 = shape->vertices[6];
        vec3i_t* p7 = shape->vertices[7];
        // each quad of points p0 to p5 represents a cube's face
        vec3i_t* surfaces[6][4] = {
            {p0, p1, p2, p3},
            {p0, p4, p7, p3},
            {p4, p5, p6, p7},
            {p5, p1, p2, p6},
            {p7, p6, p2, p3},
            {p0, p4, p5, p1}
        };
        //// main processing
        for (int r = ymin; r <= ymax; ++r) {
            for (int c = xmin; c <= xmax; ++c) {
                // the final pixel and color to render
                vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MAX};
                color_t rendered_color = background;
                for (size_t isurf = 0; isurf < 6; ++isurf) {
                    obj_plane_set(plane, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]);
                    // we keep the z to find the furthest one from the origin and we draw its x and y
                    // which z the ray currently hits the plane - can be up to two hits
                    int z_hit = obj_plane_z_at_xy(plane, c, r);
                    obj_ray_send(ray, c, r, z_hit);
                    if (obj_ray_hits_rectangle(ray, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2], surfaces[isurf][3]) &&
                    (z_hit < rendered_point.z)) {
                        rendered_color = shape->colors[isurf];
                        // use perspective transform if passed camera struct wasn't NULL:
                        // x' = x*f/z, y' = y*f/z
                        rendered_point = (vec3i_t) {c*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*c,
                                                    r*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*r,
                                                    z_hit};
                    }
                }
                draw_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
            } /* for columns */
        } /* for rows */
    } else if (shape->type == TYPE_RHOMBUS) {
        //// initialisations
        vec3i_t* p0 = shape->vertices[0];
        vec3i_t* p1 = shape->vertices[1];
        vec3i_t* p2 = shape->vertices[2];
        vec3i_t* p3 = shape->vertices[3];
        vec3i_t* p4 = shape->vertices[4];
        vec3i_t* p5 = shape->vertices[5];
        // each quad of points p0 to p5 represents a rhombus' face
        vec3i_t* surfaces[8][3] = {
            {p3, p4, p0},
            {p0, p4, p1},
            {p4, p2, p1},
            {p4, p2, p3},
            {p3, p0, p5},
            {p0, p1, p5},
            {p1, p5, p2},
            {p3, p2, p5}
        };
        //// main processing
        for (int r = ymin; r <= ymax; ++r) {
            for (int c = xmin; c <= xmax; ++c) {
                // the final pixel and color to render
                vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MAX};
                color_t rendered_color = background;
                for (size_t isurf = 0; isurf < 8; ++isurf) {
                    obj_plane_set(plane, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]);
                    // we keep the z to find the furthest one from the origin and we draw its x and y
                    // which z the ray currently hits the plane - can be up to two hits
                    int z_hit = obj_plane_z_at_xy(plane, c, r);
                    obj_ray_send(ray, c, r, z_hit);
                    if (obj_ray_hits_triangle(ray, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]) &&
                    (z_hit < rendered_point.z)) {
                        rendered_color = shape->colors[isurf];
                        rendered_point = (vec3i_t) {c*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*c,
                                                    r*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*r,
                                                    z_hit};
                    }
                }
                draw_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
            } /* for columns */
        } /* for rows */
    } else if (shape->type == TYPE_TRIANGLE) {
        // render a simple triangle - no need to account for surfaces
        vec3i_t* p0 = shape->vertices[0];
        vec3i_t* p1 = shape->vertices[1];
        vec3i_t* p2 = shape->vertices[2];
        for (int r = g_min_rows; r <= g_max_rows; ++r) {
            for (int c = g_min_cols; c <= g_max_cols; ++c) {
                obj_plane_set(plane, p0, p1, p2);
                int z_hit = obj_plane_z_at_xy(plane, c, r);
                obj_ray_send(ray, c, r, z_hit);
                if (obj_ray_hits_triangle(ray, p0, p1, p2)) {
                    draw_write_pixel(c*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*c,
                                     r*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*r,
                                     shape->colors[0]);
                    }
            }
        }
    }
    // free ray-tracing-related constructs
    obj_plane_free(plane);
    obj_ray_free(ray);
}
