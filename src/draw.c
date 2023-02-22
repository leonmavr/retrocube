#include "vector.h"
#include "draw.h" // g_colors
#include "objects.h"
#ifndef _WIN32
#include <sys/ioctl.h> // ioctl
#else
#include <windows.h>
#endif
#include <stdio.h> // printf, fgets
#include <limits.h> // INT_MIN
#include <unistd.h>
#include <stdlib.h> // exit
#include <stdbool.h> // true/false
#include <string.h> // memset

#define SIZE_INVALID 0
// printf macros to manipulate the terminal
#ifndef _WIN32
#define SCREEN_CLEAR() printf("\033[H\033[J")
#define SCREEN_GOTO_TOPLEFT() printf("\033[0;0H")
#define SCREEN_HIDE_CURSOR() printf("\e[?25l")
#define SCREEN_SHOW_CURSOR() printf("\e[?25h");
#else
HANDLE g_cons_out;

// Credits to @Jerry Coffin: https://stackoverflow.com/a/2732327
static void gotoxy(int x, int y) {
    COORD pos = {x, y};
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(output, pos);
}

// Credits to @oogabooga:
// https://cboard.cprogramming.com/c-programming/161186-undefined-reference.html
static void clrscr() {
    COORD top_left = {0, 0};
    DWORD c_chars_written;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(g_cons_out, &csbi);
    DWORD dw_con_size = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacter(g_cons_out, ' ', dw_con_size,
            top_left, &c_chars_written);
    FillConsoleOutputAttribute(g_consOut, csbi.wAttributes,
            dw_con_size, top_left, &c_chars_written);
    SetConsoleCursorPosition(g_consOut, top_left);
}
#define SCREEN_CLEAR() clrscr()
#define SCREEN_GOTO_TOPLEFT() gotoxy(0, 0)
#define SCREEN_HIDE_CURSOR() ;
#define SCREEN_SHOW_CURSOR() ;
#endif

// colors for each face of the cube
static color_t g_colors[6] = {'~', '.', '=', '@', '%', '|'};

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
int g_screen_buffer_size;

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
 * @brief Attempt to get the screen resolution in three ways:
 *            1.`ioctl` call - fails on some terminals
 *            2. (fallback) xrandr command
 *            3. (fallback) assume a common screen resolution, e.g. 1920/1080
 *        Writes to global variables `g_screen_res` and `g_cols_over_rows`,
 *        `g_rows`, `g_cols`, `g_min_rows`, `g_min_cols`, `g_max_rows`,
 *        `g_max_cols`
 */
static void draw__get_screen_info() {
#ifndef _WIN32
    FILE *fp;
    char path[512];

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
    if ((wsize.ws_xpixel != SIZE_INVALID) && (wsize.ws_ypixel != SIZE_INVALID)) {
        g_screen_res = (float)wsize.ws_xpixel/wsize.ws_ypixel;
        return;
    }

    //// 2nd way - xrandr command
    // Open the command for reading
    fp = popen("echo `xrandr --current | grep \'*\' | uniq | awk \'{print $1}\' | cut -d \'x\' -f1` / `xrandr --current | grep \'*\' | uniq | awk \'{print $1}\' | cut -d \'x\' -f2` | bc -l", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    // parse the output - it should only be the resolution
    while (fgets(path, sizeof(path), fp) != NULL) {
        if (draw__is_decimal(path)) {
            g_screen_res = atof(path);
            pclose(fp);
            return;
        }
    }
    //// 3rd way - assume a common resolution
    g_screen_res = 1920.0/1080.0;
#else
    // get terminal info on Windows - credits @quantum:
    // https://stackoverflow.com/a/12642749
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    g_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    g_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    g_cols_over_rows = (float)g_cols/g_rows;
    g_min_rows = -g_rows;
    g_max_rows = g_rows/2;
    g_min_cols = -g_cols/2+1;
    g_max_cols = g_cols/4;
    //// assume a common resolution on Windows
    g_screen_res = 1920.0/1080.0;
#endif
}


void draw_init() {
    SCREEN_HIDE_CURSOR();
    SCREEN_CLEAR();
    // get terminal's size info
    draw__get_screen_info();
    g_screen_buffer_size = g_rows*g_cols;
    g_screen_buffer = malloc(sizeof(color_t) * g_screen_buffer_size);
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
void draw_write_pixel(int x, int y, color_t c) {
    int y_scaled = y/(g_cols_over_rows/g_screen_res) + g_rows/2;
    x += g_cols/2;
    *(g_screen_buffer + y_scaled*g_cols + x) = c;
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
    SCREEN_CLEAR();
    SCREEN_SHOW_CURSOR();
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
    // plane (cube's face) the ray will hit - initialised with some dummy values
    plane_t* plane = obj_plane_new(p0, p0, p0);
    for (int i = g_min_rows; i <= g_max_rows; ++i) {
        for (int j = g_min_cols; j <= g_max_cols; ++j) {
            // we test whether the ray has hit the following surafaces:
            // (p0, p1, p2, p3), (p0, p4, p7, p3)
            // (p4, p5, p6, p7), (p5, p1, p2, p6)
            // (p7, p6, p2, p3), (p0, p4, p5, p1)

            // the final pixel to render
            // we keep the z to find the furthest one from the origin and we draw its x and y
            vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MIN};
            // the color of the rendered pixel
            color_t rendered_color = ' ';
            // which z the ray currently hits the plane - can be up to two hits
            int z_hit;
            // through (p0, p1, p2)
            obj_plane_set(plane, p0, p1, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p1, p2, p3) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[0];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p0, p4, p7);
            obj_plane_set(plane, p0, p4, p7);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p4, p7, p3) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[1];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p4, p5, p6);
            obj_plane_set(plane, p4, p5, p6);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p4, p5, p6, p7) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[2];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p5, p1, p2);
            obj_plane_set(plane, p5, p1, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p5, p1, p2, p6) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[3];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p7, p6, p2);
            obj_plane_set(plane, p7, p6, p2);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p7, p6, p2, p3) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[4];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            // through (p0, p4, p5);
            obj_plane_set(plane, p0, p4, p5);
            z_hit = obj_plane_z_at_xy(plane, j, i);
            obj_ray_send(ray, j, i, z_hit);
            if (obj_ray_hits_rectangle(ray, p0, p4, p5, p1) && (z_hit > rendered_point.z)) {
                rendered_color = g_colors[5];
                rendered_point = (vec3i_t) {j, i, z_hit};
            }
            draw_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
        } /* for columns */
    } /* for rows */
    // free ray-tracing-related constructs
    obj_plane_free(plane);
    obj_ray_free(ray);
}
