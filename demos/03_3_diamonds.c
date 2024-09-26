#include "arg_parser.h"
#include "objects.h"
#include "renderer.h"
#include "arg_parser.h"
#include "xtrig.h"
#include "utils.h" // UT_MAX
#include <math.h> // sin, cos
#include <unistd.h> // for usleep
#include <stdbool.h> // bool
#include <stdlib.h> // exit
#include <time.h> // time
#include <signal.h> // signal
#include <assert.h> // assert 
#include <stdio.h> // sprintf

// absolute path to mesh file
char mesh_filepath[256];

/* Callback that clears the screen and makes the cursor visible when the user hits Ctr+C */
static void interrupt_handler(int int_num) {
    if (int_num == SIGINT) {
        render_end();
        exit(SIGINT);
    }
}

int main(int argc, char** argv) {
    // change the variables below to configure the demo
    // width, height, depth for all three
    const unsigned w =  120, h = 150, d = 120;
    // focal length - the higher, the bigger the objects
    const unsigned focal_length = 300;

    // make sure we end gracefully if the user hits Ctr+C
    signal(SIGINT, interrupt_handler);

    // path to directory where meshes are stored - stored in CFG_DIR prep. constant
    const char* mesh_dir = STRINGIFY(CFG_DIR);
    // select file from mesh directory
    const char* mesh_filename = "rhombus.scl";
    sprintf(mesh_filepath, "%s/%s", mesh_dir, mesh_filename);
    assert(access(mesh_filepath, F_OK) == 0);
    ftrig_init_lut();

    // draw the same object in 3 different depths to showcase perspective
    mesh_t* obj1 = obj_mesh_from_file(mesh_filepath, -80, 0, 600, w, h, d);
    mesh_t* obj2 = obj_mesh_from_file(mesh_filepath, 0, 0, 500, w, h, d);
    mesh_t* obj3 = obj_mesh_from_file(mesh_filepath, 80, 0, 400, w, h, d);
    render_use_perspective(0, 0, focal_length);
    // do the actual rendering
    render_init();
    for (size_t t = 0; t < UINT_MAX; ++t) {
        obj_mesh_rotate_to(obj1, 1.0/10*t, 0*t, 1.0/15*t);
        obj_mesh_rotate_to(obj2, 1.0/10*t, 0*t, 1.0/15*t);
        obj_mesh_rotate_to(obj3, 1.0/10*t, 0*t, 1.0/15*t);
        render_write_shape(obj1);
        render_write_shape(obj2);
        render_write_shape(obj3);
        render_flush();
#ifndef _WIN32
        // nanosleep does not work on Windows
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / g_fps * 1e9)}}, NULL);
#endif
    }
    obj_mesh_free(obj1);
    obj_mesh_free(obj2);
    obj_mesh_free(obj3);

    render_end();
}
