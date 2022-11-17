#include "types.h"
#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

// rows, columns and aspect ratio of the terminal
int g_rows;
int g_cols;
float g_aspRatio;

void drawInit() {
    // start the curses mode
    initscr();
    curs_set(0);
    // get the number of rows and columns
    getmaxyx(stdscr, g_rows, g_cols);
    // find terminal window's aspect ratio
    struct winsize wsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
    g_aspRatio = (float)wsize.ws_col/wsize.ws_row;
    //printf("h = %d, w = %d, %.2f\n", wsize.ws_row, wsize.ws_col, g_aspRatio);
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
void drawPixel(int x, int y, char c) {
    mvaddch(-y + g_rows/2, g_aspRatio*x + g_cols/2, c);
}

void drawEnd() {
    getch();
    endwin();
}
