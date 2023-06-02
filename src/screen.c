#include "vector.h"
#include "screen.h"
#include "objects.h"
#include "utils.h"
#include <sys/ioctl.h>
#include <stdio.h>
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

// rows, columns of the terminal
int g_rows;
int g_cols;
// columns over rows for the terminal 
static float g_cols_over_rows;
// screen resolution (pixels over pixels) 
static float g_screen_res;
color_t* g_screen_buffer;
size_t g_screen_buffer_size;


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
    if ((wsize.ws_xpixel != IOCTL_SIZE_INVALID) || (wsize.ws_ypixel != IOCTL_SIZE_INVALID)) {
        g_screen_res = (float)wsize.ws_xpixel/wsize.ws_ypixel;
        return;
    }

    //// 2nd way - xrandr command
    // Open the command for reading
#if 1
    FILE *fp;
    char line[512];
    fp = popen("echo `xrandr --current | grep \'*\' | uniq | awk \'{print $1}\' | cut -d \'x\' -f1` / `xrandr --current | grep \'*\' | uniq | awk \'{print $1}\' | cut -d \'x\' -f2` | bc -l", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    // parse the output - it should only be the resolution
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (ut_is_decimal(line)) {
            g_screen_res = atof(line);
            pclose(fp);
            return;
        }
    }
#endif
    //// 3rd way - assume a common resolution
    g_screen_res = 1920.0/1080.0;
}

void screen_init() {
    SCREEN_HIDE_CURSOR();
    SCREEN_CLEAR();
    // get terminal's size info
    draw__get_screen_info();
    g_screen_buffer_size = g_rows*g_cols;
    g_screen_buffer = malloc(sizeof(color_t) * g_screen_buffer_size);
}

size_t screen_xy2ind(int x, int y) {
    x += g_cols/2;
    y += g_rows;
    const int y_scaled = round(y/(g_cols_over_rows/g_screen_res));
    const int ind_buffer = round(y_scaled*g_cols + x);
    if ((ind_buffer >= g_screen_buffer_size) || (ind_buffer < 0))
        return 0;
    return ind_buffer;
}

void screen_write_pixel(int x, int y, color_t c) {
   /* Uses the following coordinate system:
    *
    *      ^ y
    *      |
    *      |
    *      |
    *      o--------> x
    *       \
    *        \
    *         v z
    */
    size_t ind_buffer = screen_xy2ind(x, y);
    g_screen_buffer[ind_buffer] = c;
}

void screen_flush() {
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

void screen_end() {
    free(g_screen_buffer);
    SCREEN_CLEAR();
    SCREEN_SHOW_CURSOR();
}

