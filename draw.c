#include "types.h"
#include <ncurses.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

// rows, columns and aspect ratio of the terminal
int g_rows;
int g_cols;
float g_aspRatioScreen;
// aspect ratio of each character
float g_aspRatioChar;


void drawInit() {
    // start the curses mode
    initscr();
    curs_set(0);
    // get the number of rows and columns
    getmaxyx(stdscr, g_rows, g_cols);
    // find terminal window's aspect ratio
    struct winsize wsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
    g_aspRatioScreen = (float)wsize.ws_col/wsize.ws_row;
    g_aspRatioChar = (float)wsize.ws_xpixel/wsize.ws_ypixel;
    printf("screen:\th = %d, w = %d, %.2f\n", wsize.ws_row, wsize.ws_col, g_aspRatioScreen);
    printf("char:\th = %d, w = %d, %.2f\n", wsize.ws_xpixel, wsize.ws_ypixel, g_aspRatioChar);
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
    int yScaled = y/(g_aspRatioScreen/g_aspRatioChar);
    mvaddch(-yScaled + g_rows/2, x + g_cols/2, c);
}

void drawEnd() {
    getch();
    endwin();
}
