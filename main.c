#include "draw.h"
#include "objects.h"
#include <math.h> // sin, cos
#include <stdlib.h> // atof, atoi, random, exit
#include <unistd.h> // for usleep
#include <assert.h> // assert
#include <stdbool.h>
#include <string.h> // strcmp
#include <limits.h> // UINT_MAX
#include <time.h> // time 
#include <signal.h> // signal


//// default command line arguments
// rotation speed around x, y, z axes (-1 to 1)
static float g_rot_speed_x = 0.7;
static float g_rot_speed_y = 0.4;
static float g_rot_speed_z = 0.6;
// maximum fps at which to render the cube
static unsigned g_fps = 40;
static bool g_use_random_rotation = true;
// centre (x, y, z) of the cube
static int g_cx = 0;
static int g_cy = 0;
static int g_cz = 250;
// size of each face in "pixels" (rendered characters)
static unsigned g_cube_size = 24;
// how many frames to run the program for
static unsigned g_max_iterations = UINT_MAX;


/* Clears the screen and makes the cursor visible when the user hits Ctr+C */
void interrupt_handler(int int_num) {
    if (int_num == SIGINT) {
        draw_end();
        exit(0);
    }            
}

int main(int argc, char** argv) {
    int i = 0;
    // initialise pseudo randomness generator for random rotations
    srand(time(NULL));
    const float rand_min = 0.75, rand_max = 2.25;
    const float random_bias_x = rand_min + (rand_max - rand_min)*rand() / (double)RAND_MAX;
    const float random_bias_y = rand_min + (rand_max - rand_min)*rand() / (double)RAND_MAX;
    const float random_bias_z = rand_min + (rand_max - rand_min)*rand() / (double)RAND_MAX;
    // TODO: width and height cmd args
    // parse command line arguments - if followed by an argument, e.g. -sx 0.9, increment `i`
    while (++i < argc) {
        if ((strcmp(argv[i], "--speedx") == 0) || (strcmp(argv[i], "-sx") == 0)) {
            g_rot_speed_x = atof(argv[++i]);
            g_use_random_rotation = false;
        } else if ((strcmp(argv[i], "--speedy") == 0) || (strcmp(argv[i], "-sy") == 0)) {
            g_rot_speed_y = atof(argv[++i]);
            g_use_random_rotation = false;
        } else if ((strcmp(argv[i], "--speedz") == 0) || (strcmp(argv[i], "-sz") == 0)) {
            g_rot_speed_z = atof(argv[++i]);
            g_use_random_rotation = false;
        } else if ((strcmp(argv[i], "--fps") == 0) || (strcmp(argv[i], "-f") == 0)) {
            g_fps = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--random") == 0) || (strcmp(argv[i], "-r") == 0)) {
            g_use_random_rotation = true;
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
    // make sure we end gracefully if the user hits Ctr+C
    signal(SIGINT, interrupt_handler);

    draw_init();
    shape_t* cube = obj_cube_new(g_cx, g_cy, g_cz, g_cube_size, 1.5*g_cube_size, TYPE_CUBE);
    // spinning parameters in case random rotation was selected
#ifndef _WIN32
    const float random_rot_speed_x = 0.002, random_rot_speed_y = 0.002, random_rot_speed_z = 0.002;
    const float amplitude_x = 4.25, amplitude_y = 4.25, amplitude_z = 4.25;
#else
    // make it spin faster on Windows because terminal refresh functions are sluggish there
    const float random_rot_speed_x = 0.0125, random_rot_speed_y = 0.0125, random_rot_speed_z = 0.0125;
    const float amplitude_x = 10.0, amplitude_y = 10.0, amplitude_z = 10.0;
#endif
    for (size_t t = 0; t < g_max_iterations; ++t) {
        if (g_use_random_rotation)
            obj_cube_rotate(cube, amplitude_x*sin(random_rot_speed_x*sin(random_rot_speed_x*t) + 2*random_bias_x),
                                  amplitude_y*sin(random_rot_speed_y*random_bias_y*t           + 2*random_bias_y),
                                  amplitude_z*sin(random_rot_speed_z*random_bias_z*t           + 2*random_bias_z));
        else
            obj_cube_rotate(cube, g_rot_speed_x/20*t, g_rot_speed_y/20*t, g_rot_speed_z/20*t);
        draw_cube(cube);
        draw_flush_screen();
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / g_fps * 1e9)}}, NULL);
    }
    obj_cube_free(cube);
    draw_end();

    return 0;
}
