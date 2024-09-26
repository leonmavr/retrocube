#include "objects.h"
#include "renderer.h"
#include "arg_parser.h" // args_parse, CFG_DIR
#include "xtrig.h"
#include <math.h> // sin, cos
#include <unistd.h> // for usleep
#include <stdbool.h> // bool
#include <stdlib.h> // exit
#include <time.h> // time
#include <signal.h> // signal
#include <assert.h> // assert 
#include <stdio.h> // sprintf

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

    init_lookup_tables();
    render_init();

    mesh_t* shape = obj_mesh_from_file(g_mesh_file, g_cx, g_cy, g_cz, g_width, g_height, g_depth);
    // spinning parameters in case random rotation was selected
#ifndef _WIN32
    const float random_rot_speed_x = 0.002, random_rot_speed_y = 0.002, random_rot_speed_z = 0.002;
    const float amplitude_x = 4.25, amplitude_y = 4.25, amplitude_z = 4.25;
#else
    // make it spin faster on Windows because terminal refresh functions are sluggish there
    const float random_rot_speed_x = 0.01, random_rot_speed_y = 0.01, random_rot_speed_z = 0.01;
    const float amplitude_x = 6.0, amplitude_y = 6.0, amplitude_z = 6.0;
#endif
    for (size_t t = 0; t < g_max_iterations; ++t) {
        if (g_use_random_rotation)
            obj_mesh_rotate_to(shape, amplitude_x*xsin(random_rot_speed_x*xsin(random_rot_speed_x*t) + 2*random_bias_x),
                                   amplitude_y*xsin(random_rot_speed_y*random_bias_y*t               + 2*random_bias_y),
                                   amplitude_z*xsin(random_rot_speed_z*random_bias_z*t               + 2*random_bias_z));
        else
            obj_mesh_rotate_to(shape, g_rot_speed_x/20*t, g_rot_speed_y/20*t, g_rot_speed_z/20*t);
        if ((t % 100) >= 50)
            obj_mesh_translate_by(shape, 1, 1, 1);
        else
            obj_mesh_translate_by(shape, -1, -1, -1);
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
