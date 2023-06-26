#include "objects.h"
#include "renderer.h"
#include "arg_parser.h"
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
    // make sure we end gracefully if the user hits Ctr+C
    signal(SIGINT, interrupt_handler);

    // path to directory where meshes are stored - dir stored in CFG_DIR prep. constant
    const char* mesh_dir = STRINGIFY(CFG_DIR);
    // select file in CFG_DIR
    const char* mesh_filename = "coffin.scl";
    sprintf(mesh_filepath, "%s/%s", mesh_dir, mesh_filename);
    assert(access(mesh_filepath, F_OK) == 0);

    mesh_t* obj = obj_mesh_from_file(mesh_filepath, 0, 0, 250, 60, 80, 60);
    // do the actual rendering
    render_init();
    for (size_t t = 0; t < g_max_iterations; ++t) {
        obj_mesh_rotate_to(obj, 1.0/60*t, 1.0/100*t, 1.0/100*t);
        render_write_shape(obj);
        render_flush();
#ifndef _WIN32
        // nanosleep does not work on Windows
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / g_fps * 1e9)}}, NULL);
#endif
    }
    obj_mesh_free(obj);

    render_end();
}
