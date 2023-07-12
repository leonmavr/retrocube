#include "objects.h"
#include "renderer.h"
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
#include <stdio.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>

// absolute path to cube file
char cube_filepath[256];
// absolute path to coffin file
char coffin_filepath[256];

struct termios old_terminal_settings;
struct termios new_terminal_settings;

mesh_t** obj;

/* Callback that clears the screen and makes the cursor visible when the user hits Ctr+C */
static void interrupt_handler(int int_num) {
    // restore the terminal settings to their prvious state
    if (tcsetattr(0, TCSANOW, &old_terminal_settings) < 0)
        perror("tcsetattr ICANON");
    for (int i = 0; i < 5; ++i)
        obj_mesh_free(obj[i]);
    if (int_num == SIGINT) {
        render_end();
        exit(SIGINT);
    }
}

/* Credits to @supirman from https://ubuntuforums.org */
static int is_key_pressed(void)
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

static void set_terminal() {
    // Get the current terminal settings
    if (tcgetattr(0, &old_terminal_settings) < 0)
        perror("tcgetattr()");
    memcpy(&new_terminal_settings, &old_terminal_settings, sizeof(struct termios));
    // disable canonical mode processing in the line discipline driver,
    // disable echoing chracters
    new_terminal_settings.c_lflag &= ~ICANON;
    new_terminal_settings.c_lflag &= ~ECHO;
    // apply our new settings
    if (tcsetattr(0, TCSANOW, &new_terminal_settings) < 0)
        perror("tcsetattr ICANON");
}

static int sign(int x) {
    return (x >= 0) - (x < 0);
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

    set_terminal();
/**
 *                  *                  V: viewer
 *                  __                 C: coffin
 *                _/  \_               *: cube
 *              _/      \            v,^: movement directions
 *            _/          \_
 *          _^              v_            ^y
 *        _/                  \_          |
 *      _/                      \_        o-----> x
 *    _/                          \_       \
 *  * __            C             __ *      \
 *      \_                      _/           V z
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
    int coffinx = 0, coffiny = 0, coffinz = 900;
    unsigned dist = 120;
    obj = malloc(sizeof(mesh_t*) * 5);
    // coffin
    obj[0] = obj_mesh_from_file(coffin_filepath, coffinx, coffiny+50, coffinz, w, 1.3*h, 0.8*d);
    // cubes at 3, 6, 9, 12 o'clock cubes
    obj[1] = obj_mesh_from_file(cube_filepath, dist,  0, coffinz,     w, h, d);
    obj[2] = obj_mesh_from_file(cube_filepath, 0,     0, coffinz+200, w, h, d);
    obj[3] = obj_mesh_from_file(cube_filepath, -dist, 0, coffinz,     w, h, d);
    obj[4] = obj_mesh_from_file(cube_filepath, 0,     0, coffinz-200, w, h, d);

    // do the actual rendering
    render_use_perspective(0, 0, focal_length);
    render_init();

    unsigned rad = 2*dist;
    int sign_x = 1, sign_y = 1;
    for (size_t t = 0; t < UINT_MAX; ++t) {
        int dx[4] = {0}, dy[4] = {0}, dz[4] = {0};
        // Check if a key is pressed.  If it is, call getchar to fetch it.
        if (is_key_pressed()) {
            char ch = getchar();
            if (ch == 'a')
                dx[0] += 10;
            else if (ch == 's')
                dz[0] += 10;
            else if (ch == 'd')
                dx[0] -= 10;
            else if (ch == 'w')
                dz[0] -= 10;
        }

			sign_x = (t % rad == 0) ? -sign_x: sign_x;
			sign_y = ((t + rad/2) % rad == 0) ? -sign_y: sign_y;

			dx[0] += sign_x;
			//dz += -sign(obj[i]->center->z - coffinz)*sign_y;
			dz[0] += sign_y;
			// 1st cube at 1st quarter
			if (dx[0] > 0 && dz[0] > 0) {
				dx[1] = dx[0];
				dz[1] = -dz[0];
				dx[2] = -dx[0];
				dz[2] = -dz[0];
				dx[3] = -dx[0];
				dz[3] = dz[0];
			// 1st cube at 2nd quarter
			} else if (dx[0] > 0 && dz[0] < 0) {
				//dx[1] = dx[0];
				//dz[1] = -dz[0];
			}
			obj_mesh_translate_by(obj[1], dx[0], dy[0], dz[0]);
			if ((obj[1]->center->x <= coffinx) && (obj[1]->center->z <= coffinz + rad))
				printf("dz = %d, z = %d/%d\n", dz[0], obj[1]->center->z, coffinz);
        for (int i = 0; i < 5; ++i) {
            if (obj[i]->center->x != 0)
                obj_mesh_rotate_to(obj[i], atan(obj[i]->center->y/obj[i]->center->x), 0, 0);
        }

        obj_mesh_rotate_to(obj[1], 1.0/10*t, 0*t, 1.0/15*t);
        for (int i = 0; i < 5; ++i)
            render_write_shape(obj[i]);
        render_flush();
#ifndef _WIN32
        // nanosleep does not work on Windows
        nanosleep((const struct timespec[]) {{0, (int)(1.0 / 60 * 1e9)}}, NULL);
#endif
    }
    for (int i = 0; i < 5; ++i)
        obj_mesh_free(obj[i]);

    render_end();
}
