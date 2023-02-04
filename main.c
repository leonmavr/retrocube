#include "draw.h"
#include "vector.h"
#include "objects.h"
#include <ncurses.h> // clear
#include <math.h> // sin, cos
#include <stdlib.h> // atof, atoi, random
#include <unistd.h> // for usleep
#include <assert.h>
#include <stdbool.h>
#include <string.h> // strcmp
#include <limits.h> // UINT_MAX
#include <time.h> // time 


//// default command line arguments
// rotation speed around x, y, z axes (-1 to 1)
static float g_rot_speed_x = 0.7;
static float g_rot_speed_y = 0.4;
static float g_rot_speed_z = 0.6;
// maximum fps at which to render the cube
static unsigned g_fps = 20;
static bool g_use_random_rotation = true;
// centre (x, y, z) of the cube
static int g_cx = 0;
static int g_cy = 0;
static int g_cz = 250;
// size of each face in "pixels" (rendered characters)
static unsigned g_cube_size = 24;
// how many frames to run the program for
static unsigned g_max_iterations = UINT_MAX;


int main(int argc, char** argv) {
    int i = 0;
    // initialise pseudo randomness generator for random rotations
    srand(time(NULL));
    const float rand_min = 0.75, rand_max = 2.25;
    const float random_bias_x = rand_min + (rand_max - rand_min)*rand() / (double)RAND_MAX;
    const float random_bias_y = rand_min + (rand_max - rand_min)*rand() / (double)RAND_MAX;
    const float random_bias_z = rand_min + (rand_max - rand_min)*rand() / (double)RAND_MAX;
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

    draw_init();
    cube_t* cube = obj_cube_new(g_cx, g_cy, g_cz, g_cube_size);

    for (size_t t = 0; t < g_max_iterations; ++t) {
        clear();
        if (g_use_random_rotation)
            obj_cube_rotate(cube, 4*sin(random_bias_x*sin(0.0025*t) + 2*random_bias_x),
                                  4*sin(random_bias_y*0.0025*t      + 2*random_bias_y),
                                  4*sin(random_bias_z*0.0025*t      + 2*random_bias_z));
        else
            obj_cube_rotate(cube, g_rot_speed_x/20*t, g_rot_speed_y/20*t, g_rot_speed_z/20*t);
        draw_cube(cube);
        //nanosleep((const struct timespec[]) {{0, (int)(1.0 / g_fps * 1e9)}}, NULL);
        usleep((int)(1.0 / g_fps * 1e6));
    }
    obj_cube_free(cube);
    draw_end();

    return 0;
}
