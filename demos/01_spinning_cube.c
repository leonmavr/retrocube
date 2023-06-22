//#include "screen.h"
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
    // initialise objects to render
    mesh_t* shape = obj_mesh_from_file(g_mesh_file, g_cx, g_cy, g_cz, g_width, g_height, g_depth);
    // do the actual rendering
    render_init();
    for (size_t t = 0; t < g_max_iterations; ++t) {
        obj_mesh_rotate_to(shape, 0.05*t, 0.01*t, 0.025*t);
        render_write_shape(shape);
        render_flush();
#ifndef _WIN32
        // nanosleep does not work on Windows
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / g_fps * 1e9)}}, NULL);
#endif
    }
    obj_mesh_free(shape);

    render_end();
}
