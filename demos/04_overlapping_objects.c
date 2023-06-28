#include "objects.h"
#include "renderer.h"
#include "arg_parser.h" // args_parse, CFG_DIR
#include <math.h> // sin, cos
#include <unistd.h> // for usleep
#include <stdbool.h> // bool
#include <stdlib.h> // exit
#include <time.h> // time
#include <signal.h> // signal
#include <assert.h> // assert 
#include <stdio.h> // sprintf

// absolute path to mesh file
static char cube_filepath[256];
static char rhombus_filepath[256];

/* Callback that clears the screen and makes the cursor visible when the user hits Ctr+C */
static void interrupt_handler(int int_num) {
    if (int_num == SIGINT) {
        render_end();
        exit(SIGINT);
    }
}

int main(int argc, char** argv) {
    arg_parse(argc, argv);
    // make sure we end gracefully if the user hits Ctr+C
    signal(SIGINT, interrupt_handler);

    render_init();

    // path to directory where meshes are stored - dir stored in CFG_DIR prep. constant
    const char* mesh_dir = STRINGIFY(CFG_DIR);
    // select files in CFG_DIR
    const char* cube_fname = "cube.scl";
    const char* rhombus_fname = "rhombus.scl";
    // cube's dimensions
    const unsigned w = 50, h = 50, d = 50;
    sprintf(cube_filepath, "%s/%s", mesh_dir, cube_fname);
    sprintf(rhombus_filepath, "%s/%s", mesh_dir, rhombus_fname);
    mesh_t* obj1 = obj_mesh_from_file(cube_filepath, 0, 0, 0, w, h, d);
    mesh_t* obj2 = obj_mesh_from_file(rhombus_filepath, 0, 0, 0, 1.2*w, 1.35*h, 1.2*d);
    assert(access(cube_filepath, F_OK) == 0);
    assert(access(rhombus_filepath, F_OK) == 0);
    // spinning parameters in case random rotation was selected
#ifndef _WIN32
    const float random_rot_speed_x = 0.002, random_rot_speed_y = 0.002, random_rot_speed_z = 0.002;
    const float amplitude_x = 4.25, amplitude_y = 4.25, amplitude_z = 4.25;
#else
    // make it spin faster on Windows because terminal refresh functions are sluggish there
    const float random_rot_speed_x = 0.01, random_rot_speed_y = 0.01, random_rot_speed_z = 0.01;
    const float amplitude_x = 6.0, amplitude_y = 6.0, amplitude_z = 6.0;
#endif
    for (size_t t = 0; t < UINT_MAX; ++t) {
        obj_mesh_rotate_to(obj1, 1.0/80*t, 1.0/40*t, 1.0/60*t);
        obj_mesh_rotate_to(obj2, amplitude_x*sin(random_rot_speed_x*sin(random_rot_speed_x*t) + 2*random_bias_x),
                                    amplitude_y*sin(random_rot_speed_y*random_bias_y*t           + 2*random_bias_y),
                                    amplitude_z*sin(random_rot_speed_z*random_bias_z*t           + 2*random_bias_z));
        render_write_shape(obj1);
        render_write_shape(obj2);
        render_flush();
#ifndef _WIN32
        // nanosleep does not work on Windows
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / 40 * 1e9)}}, NULL);
#endif
    }
    obj_mesh_free(obj1);
    obj_mesh_free(obj2);
    render_end();
}
