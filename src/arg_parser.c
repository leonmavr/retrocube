#include "arg_parser.h"
#include "renderer.h"
#include "utils.h" // UT_MAX
#include <math.h> // sin, cos
#include <stdlib.h> // atof, atoi, random, exit
#include <unistd.h> // for usleep
#include <assert.h> // assert
#include <stdbool.h> // bool
#include <string.h> // strcmp
#include <time.h> // time
#include <stdio.h> // sprintf

//// default command line arguments
// rotation speed around x, y, z axes (-1 to 1)
float g_rot_speed_x = 0.7;
float g_rot_speed_y = 0.4;
float g_rot_speed_z = 0.6;
// maximum fps at which to render the cube
unsigned g_fps = 40;
bool g_use_random_rotation = true;
// centre (x, y, z) of the cube
int g_cx = 0;
int g_cy = 0;
int g_cz = 250;
// size of each dimension in "pixels" (rendered characters)
unsigned g_width = 60;
unsigned g_height = 60;
unsigned g_depth = 60;
// how many frames to run the program for
unsigned g_max_iterations = UINT_MAX;
char g_mesh_file[256] = {'\0'};
// defines the max and min values of random rotation bias
float rand_min = 0.75, rand_max = 2.25;
// random rotation biases - the higher, the faster the rotation around x, y, 
float random_bias_x = 0;
float random_bias_y = 0;
float random_bias_z = 0;
bool render_from_file = false;


void arg_parse(int argc, char** argv) {
    // initialise pseudo randomness generator for random rotations
    srand(time(NULL));
    rand_min = 0.75, rand_max = 2.25;
    random_bias_x = rand_min + (rand_max - rand_min)*rand()/(double)RAND_MAX;
    random_bias_y = rand_min + (rand_max - rand_min)*rand()/(double)RAND_MAX;
    random_bias_z = rand_min + (rand_max - rand_min)*rand()/(double)RAND_MAX;
    render_from_file = false;
    // parse command line arguments - if followed by an argument, e.g. -sx 0.9, increment `i`
    int i = 0;
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
        } else if ((strcmp(argv[i], "--width") == 0) || (strcmp(argv[i], "-wi") == 0)) {
            g_width = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--height") == 0) || (strcmp(argv[i], "-he") == 0)) {
            g_height = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--depth") == 0) || (strcmp(argv[i], "-de") == 0)) {
            g_depth = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--max-iterations") == 0) || (strcmp(argv[i], "-mi") == 0)) {
            g_max_iterations = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--use-perspective") == 0) || (strcmp(argv[i], "-up") == 0)) {
            render_use_perspective(0, 0, -200);
        } else if ((strcmp(argv[i], "--use-reflection") == 0) || (strcmp(argv[i], "-ur") == 0)) {
            render_use_reflectance();
        } else if ((strcmp(argv[i], "--from-file") == 0) || (strcmp(argv[i], "-ff") == 0)) {
            i++;
            strcpy(g_mesh_file, argv[i]);
            render_from_file = true;
        } else {
            printf("Uknown option: %s\n", argv[i++]);
        }
    }
    assert(( -1.0001 < g_rot_speed_x) && (g_rot_speed_x < 1.0001) &&
            (-1.0001 < g_rot_speed_y) && (g_rot_speed_y < 1.0001) &&
            (-1.0001 < g_rot_speed_z) && (g_rot_speed_z < 1.0001));
    // default file to render
    // define in preprocessor constant CFG_DIR
    if (!render_from_file) {
        const char* cfg_dir = STRINGIFY(CFG_DIR);
        const char* mesh_filename = "cube.scl";
        // TODO: check boundaries
        sprintf(g_mesh_file, "%s/%s", cfg_dir, mesh_filename);
    }
    // we should have a valid filepath by now
    assert(access(g_mesh_file, F_OK) == 0);

}
