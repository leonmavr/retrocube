#include "objects.h"
#include "renderer.h"
#include "arg_parser.h" // CFG_DIR
#include <math.h> // sin, cos
#include <unistd.h> // for usleep
#include <stdbool.h> // bool
#include <stdlib.h> // exit
#include <time.h> // time
#include <signal.h> // signal
#include <assert.h> // assert 
#include <stdio.h> // sprintf

// absolute path to cube file
char cube_filepath[256];
// absolute path to coffin file
char coffin_filepath[256];

/* Callback that clears the screen and makes the cursor visible when the user hits Ctr+C */
static void interrupt_handler(int int_num) {
    if (int_num == SIGINT) {
        render_end();
        exit(SIGINT);
    }
}

int main(int argc, char** argv) {
    // width, height, depth of the four cubes
    const unsigned w =  100, h = 100, d = 100;
    // focal length - the higher, the bigger the objects
    const unsigned focal_length = 300;

    // make sure we end gracefully if the user hits Ctr+C
    signal(SIGINT, interrupt_handler);

    // path to directory where meshes are stored - stored in CFG_DIR prep. constant
    const char* mesh_dir = STRINGIFY(CFG_DIR);
    // select file from mesh directory
    const char* cube_filename = "cube.scl";
    const char* coffin_filename = "coffin.scl";
    sprintf(cube_filepath, "%s/%s", mesh_dir, cube_filename);
    sprintf(coffin_filepath, "%s/%s", mesh_dir, coffin_filename);
    assert(access(cube_filepath, F_OK) == 0);
    assert(access(coffin_filepath, F_OK) == 0);



/**
 *                  *                  V: viewer
 *                  __                 C: coffin
 *                _/  \_               *: cube
 *              _/      \            v,^: movement directions
 *            _/          \_        
 *          _^              v_      
 *        _/                  \_    
 *      _/                      \_  
 *    _/                          \_
 *  * __            C             __ *
 *      \_                      _/  
 *        \_                 __/    
 *          \__            v/       
 *             ^_       __/         
 *               \_   _/            
 *                 \_/              
 *                  *
 *
 *
 *                  V(0,0,0)
 */

    // create 1 coffin and 4 cubes at 12, 3, 6 and 9 o' clock
    int coffinx = 0, coffiny = 0, coffinz = 700;
    unsigned dist = 300;
    int cubex = coffinx + dist, cubey = coffiny;
    // coffin
    mesh_t* obj1 = obj_mesh_from_file(coffin_filepath, coffinx, coffiny, coffinz, w, 1.3*h, 0.8*d);
    // 3 o'clock cube
    mesh_t* obj2 = obj_mesh_from_file(cube_filepath, coffinx, cubey, coffinz, w, h, d);
    // 9 o'clock cube
    mesh_t* obj3 = obj_mesh_from_file(cube_filepath, -coffinx, cubey, coffinz, w, h, d);
    // 12 o'clock cube
    mesh_t* obj4 = obj_mesh_from_file(cube_filepath, 200, cubey, coffinz+200, w, h, d);
    // 6 o'clock cube
    mesh_t* obj5 = obj_mesh_from_file(cube_filepath, -200, cubey, coffinz-200, w, h, d);

    render_use_perspective(0, 0, focal_length);
    // do the actual rendering
    render_init();
    for (size_t t = 0; t < UINT_MAX; ++t) {
        obj_mesh_rotate_to(obj1, 1.0/10*t, 0*t, 1.0/15*t);
        obj_mesh_rotate_to(obj2, 1.0/10*t, 0*t, 1.0/15*t);
        obj_mesh_rotate_to(obj3, 1.0/10*t, 0*t, 1.0/15*t);
        obj_mesh_rotate_to(obj4, 1.0/10*t, 0*t, 1.0/15*t);
        obj_mesh_rotate_to(obj5, 1.0/10*t, 0*t, 1.0/15*t);
        render_write_shape(obj1);
        render_write_shape(obj2);
        render_write_shape(obj3);
        render_write_shape(obj4);
        render_write_shape(obj5);
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
