
#include "draw.h"
#include "vector.h"
#include "objects.h"
#include <ncurses.h> // clear
#include <time.h> // nanosleep 
#include <math.h> // sin, cos
#include <stdlib.h> // atof, atoi, random
#include <time.h> // time
#include <assert.h>
#include <stdbool.h>
#include <string.h> // strcmp
#include <limits.h> // UINT_MAX


//// default command line arguments
// rotation speed around x, y, z axes (0 to 1)
static float g_rot_speed_x = 0.7;
static float g_rot_speed_y = 0.4;
static float g_rot_speed_z = 0.6;
// maximum fps at which to render the cube
static unsigned g_fps = 40;
static bool g_use_random_rotation = false;
// centre (x, y, z) of the cube
static int g_cx = 0;
static int g_cy = 0;
static int g_cz = 200;
// size of each face in "pixels" (rendered characters)
static unsigned g_cube_size = 24;
// how many frames to run the program for
static unsigned g_max_iterations = UINT_MAX;


int main(int argc, char** argv) {
    int i = 0;
    float random_bias_x, random_bias_y, random_bias_z;
    // parse command line arguments - if followed by an argument, e.g. -sx 0.9, increment `i`
    while (++i < argc) {
        if ((strcmp(argv[i], "--speedx") == 0) || (strcmp(argv[i], "-sx") == 0)) {
            g_rot_speed_x = atof(argv[++i]);
        } else if ((strcmp(argv[i], "--speedy") == 0) || (strcmp(argv[i], "-sy") == 0)) {
            g_rot_speed_y = atof(argv[++i]);
        } else if ((strcmp(argv[i], "--speedz") == 0) || (strcmp(argv[i], "-sz") == 0)) {
            g_rot_speed_z = atof(argv[++i]);
        } else if ((strcmp(argv[i], "--fps") == 0) || (strcmp(argv[i], "-f") == 0)) {
            g_fps = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--random") == 0) || (strcmp(argv[i], "-r") == 0)) {
            g_use_random_rotation = true;
            // initialise pseudo randomness generator
            srand(time(NULL));
            // from 0 to 1.25
            random_bias_x = 1.25*rand() / (double)RAND_MAX;
            random_bias_y = 1.25*rand() / (double)RAND_MAX;
            random_bias_z = 1.25*rand() / (double)RAND_MAX;
        } else if ((strcmp(argv[i], "--cx") == 0) || (strcmp(argv[i], "-cx") == 0)) {
            g_cx = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--cy") == 0) || (strcmp(argv[i], "-cy") == 0)) {
            g_cy = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--cz") == 0) || (strcmp(argv[i], "-cz") == 0)) {
            g_cz = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--size") == 0) || (strcmp(argv[i], "-s") == 0)) {
            g_cube_size = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--max-iterations") == 0) || (strcmp(argv[i], "-mi") == 0)) {
            g_max_iterations = atoi(argv[++i]);
        }
        assert((-1.0 < g_rot_speed_x) && (g_rot_speed_x < 1.0) &&
               (-1.0 < g_rot_speed_y) && (g_rot_speed_y < 1.0) &&
               (-1.0 < g_rot_speed_z) && (g_rot_speed_z < 1.0));
    }

    draw_init();
    cube_t* cube = obj_cube_new(g_cx, g_cy, g_cz, g_cube_size);

    for (size_t t = 0; t < g_max_iterations; ++t) {
        clear();
        if (!g_use_random_rotation)
            obj_cube_rotate(cube, g_rot_speed_x/20*t, g_rot_speed_y/20*t, g_rot_speed_z/20*t);
        else
            obj_cube_rotate(cube, 6*sin((0.5 + random_bias_x)*sin(0.0015*t) + random_bias_x),
                                  6*sin((0.5 + random_bias_y)*0.0015*t + random_bias_y),
                                  6*sin((0.5 + random_bias_z)*0.0015*t + random_bias_z));
        draw_cube(cube);
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / g_fps * 1e9)}}, NULL);
    }
    obj_cube_free(cube);
    draw_end();

    return 0;
}
