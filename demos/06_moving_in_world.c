#include "objects.h"
#include "renderer.h"
#include "utils.h"
#include "arg_parser.h" // CFG_DIR
#include "utils.h" // CFG_DIR
#include <math.h> // sin, cos
#include <unistd.h> // for usleep
#include <stdbool.h> // bool
#include <stdlib.h> // exit
#include <time.h> // time
#include <signal.h> // signal
#include <assert.h> // assert
#include <stdio.h> // sprintf
#include <termios.h> // tcgetattr, tcsetattr
#include <sys/select.h> // select
#include <unistd.h>
#include <string.h> // strcpy

// absolute path to cube file
char cube_filepath[256];
// absolute path to coffin file
char coffin_filepath[256];

struct termios old_terminal_settings;
struct termios new_terminal_settings;

mesh_t** obj;


/**
 * @brief Close renderer, restore terminal and free object constructs
 */
static void cleanup() {
    render_end();
    // restore the terminal settings to their prvious state
    if (tcsetattr(0, TCSANOW, &old_terminal_settings) < 0)
        perror("Error resetting terminal.");
    for (int i = 0; i < 5; ++i)
        obj_mesh_free(obj[i]);
    free(obj);
}

/**
 * @brief Callback that clears the screen and makes the cursor visible when the user hits Ctr+C
 */
static void interrupt_handler(int int_num) {
    if (int_num == SIGINT) {
        cleanup();
        exit(SIGINT);
    }
}

/**
 * @brief Detects whether any key is pressed.
 *        Credits to @supirman from https://ubuntuforums.org
 * @return true is key pressed
 */
static bool is_key_pressed(void)
{
     struct timeval tv;
     fd_set fds;
     tv.tv_sec = 0;
     tv.tv_usec = 0;

     FD_ZERO(&fds);
     FD_SET(STDIN_FILENO, &fds);

     select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
     return FD_ISSET(STDIN_FILENO, &fds);
}

/**
 * @brief Change terminal properties to prepare it for rendering 
 */
static void set_terminal(void) {
    // Get the current terminal settings
    if (tcgetattr(0, &old_terminal_settings) < 0)
        perror("tcgetattr error when getting terminal settings.");
    memcpy(&new_terminal_settings, &old_terminal_settings, sizeof(struct termios));
    // disable canonical mode processing in the line discipline driver,
    // disable echoing chracters
    new_terminal_settings.c_lflag &= ~ICANON;
    new_terminal_settings.c_lflag &= ~ECHO;
    // apply our new settings
    if (tcsetattr(0, TCSANOW, &new_terminal_settings) < 0)
        perror("Error setting the terminal.");
}


int main(void) {
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

    set_terminal();
    /**
    *                  *                  V: viewer
    *                  __                 C: coffin
    *                _/  \_               *: cube
    *              _/      \            v,^: movement directions
    *            _/          \_
    *          _^              v_
    *        _/                  \_                    ^ z
    *      _/                      \_                 /
    *    _/                          \_              /
    *  * __            C             __ *   <------o/
    *      \_                      _/       x      |
    *        \_                 __/                |
    *          \__            v/                   |
    *             ^_       __/                     v y
    *               \_   _/
    *                 \_/
    *                  *
    *
    *
    *                  V(0,0,0)
    */
    // create 1 coffin and 4 cubes at 12, 3, 6 and 9 o' clock
    int coffinx = 0, coffiny = -25, coffinz = 900;
    const unsigned dist = 120;
    const unsigned motion_width = 2*dist;
    obj = malloc(sizeof(mesh_t*) * 5);
    // coffin
    obj[0] = obj_mesh_from_file(coffin_filepath, coffinx, coffiny, coffinz, 1.1*w, 1.35*h, 0.8*d);
    // cubes at 3, 6, 9, 12 o'clock
    obj[1] = obj_mesh_from_file(cube_filepath, dist,  0, coffinz,     w, h, d);
    obj[2] = obj_mesh_from_file(cube_filepath, 0,     0, coffinz+200, w, h, d);
    obj[3] = obj_mesh_from_file(cube_filepath, -dist, 0, coffinz,     w, h, d);
    obj[4] = obj_mesh_from_file(cube_filepath, 0,     0, coffinz-200, w, h, d);

    // start the actual rendering
    render_use_perspective(0, 0, focal_length);
    render_init();

    //----------------------------------------------------------------------------
    // translation due to keystroke
    //----------------------------------------------------------------------------
    int sign_x = 1, sign_z = 1;
    for (size_t t = 0; t < UINT_MAX; ++t) {
        int dx[5] = {0}, dy[5] = {0}, dz[5] = {0};
		// Check if a key is pressed.  If it is, call getchar to fetch it.
		if (is_key_pressed()) {
			char ch = getchar();
			if ((ch == 'a') || (ch == 'h')) {
                for (int i = 0; i < 5; i++)
                    dx[i] += 10;
            }
			else if ((ch == 's') || (ch == 'j')) {
                for (int i = 0; i < 5; i++)
                    dz[i] += 10;
            }
			else if ((ch == 'd') || (ch == 'l')) {
                for (int i = 0; i < 5; i++)
                    dx[i] -= 10;
            }
			else if ((ch == 'w') || (ch == 'k')) {
                for (int i = 0; i < 5; i++)
                    dz[i] -= 10;
            } else if (ch == 'q')
                cleanup();
		}

        //----------------------------------------------------------------------------
        // translation due to orbiting around the coffin 
        //----------------------------------------------------------------------------
        // directions of first cube's motions - every other cube is relatve to it
        // change x and z direction every `motion_width` frames
		sign_x = (t % motion_width == 0) ? -sign_x: sign_x;
		sign_z = ((t + motion_width/2) % motion_width == 0) ? -sign_z: sign_z;
        // dx and dz for first cube - every other cube's motion is relative to it
		dx[1] += sign_x;
		dz[1] += sign_z;
        /**
         *  motion in quadrants:
		 *  ^>    v>
		 *  ^<    v<
         */
        // center of motion
        coffinx = obj[0]->center->x, coffinz = obj[0]->center->z;
		// 1st cube at 1st quadrant
        if ((obj[1]->center->x >= coffinx) && (obj[1]->center->z >= coffinz)) {
			dx[2] +=  sign_x;
			dz[2] += -sign_z;
			dx[3] += -sign_x;
			dz[3] += -sign_z;
			dx[4] += -sign_x;
			dz[4] +=  sign_z;
		// at 2nd quadrant
        } else if ((obj[1]->center->x <= coffinx) && (obj[1]->center->z >= coffinz)) {
			dx[2] += -sign_x;
			dz[2] +=  sign_z;
			dx[3] += -sign_x;
			dz[3] += -sign_z;
			dx[4] +=  sign_x;
			dz[4] += -sign_z;
		// at 3rd quadrant
        } else if ((obj[1]->center->x <= coffinx) && (obj[1]->center->z <= coffinz)) {
			dx[2] +=  sign_x;
			dz[2] += -sign_z;
			dx[3] += -sign_x;
			dz[3] += -sign_z;
			dx[4] += -sign_x;
			dz[4] +=  sign_z;
		// at 4th quadrant
        } else {
			dx[2] += -sign_x;
			dz[2] +=  sign_z;
			dx[3] += -sign_x;
			dz[3] += -sign_z;
			dx[4] +=  sign_x;
			dz[4] += -sign_z;
        }
		for (int i = 0; i < 5; i++)
			obj_mesh_translate_by(obj[i], dx[i], dy[i], dz[i]);
        obj_mesh_translate_by(obj[0], 0, 0.25*motion_width*sin(2*UT_PI*sin(0.05*(t+1))) - 0.25*motion_width*sin(2*UT_PI*sin(0.05*t)), 0);

        //----------------------------------------------------------------------------
        // rotation 
        //----------------------------------------------------------------------------
        float t_norm = (t % 150)/150.0;
        // smoothstep interpolation function value - https://en.wikipedia.org/wiki/Smoothstep
        float smoothstep = 3*t_norm*t_norm - 2*t_norm*t_norm*t_norm;
        obj_mesh_rotate_to(obj[0], 0, UT_PI + 2*UT_PI*(3*t_norm*t_norm - 2*t_norm*t_norm*t_norm), UT_PI);

        for (int i = 1; i < 5; ++i) {
            // uniform rotation about z axis (first, third cube)
            float rot_z_uniform = (i == 1) ? 0 : 0.15*t;
            rot_z_uniform =       (i == 3) ? 0 : 0.15*t + UT_HALF_PI;
            // uniform rotation about y axis (second, fourth cube)
            float rot_y_uniform = (i == 2) ? 0 : 0.1*t;
            rot_y_uniform =       (i == 4) ? 0 : 0.1*t + UT_HALF_PI;
            // rotation to simulate changing the POV
            float rot_x_pov = (obj[i]->center->x != 0) ? 2*atan(obj[i]->center->y/obj[i]->center->x) : 0;
            obj_mesh_rotate_to(obj[i], rot_x_pov, rot_y_uniform, rot_z_uniform);
        }
        for (int i = 0; i < 5; ++i)
            render_write_shape(obj[i]);
        render_flush();
#ifndef _WIN32
        // nanosleep does not work on Windows
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / 60 * 1e9)}}, NULL);
#endif
    }
    // close renderer and reset terminal
    cleanup();

    return 0;
}
