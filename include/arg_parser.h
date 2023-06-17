#include <stdbool.h> // bool
#include <limits.h> // UINT_MAX

#ifndef CFG_DIR
#define CFG_DIR "/usr/share/retrocube"
#endif

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

//// default command line arguments
// rotation speed around x, y, z axes (-1 to 1)
extern float g_rot_speed_x;
extern float g_rot_speed_y;
extern float g_rot_speed_z;
// maximum fps at which to render the cube
extern unsigned g_fps;
extern bool g_use_random_rotation;
// centre (x, y, z) of the cube
extern int g_cx;
extern int g_cy;
extern int g_cz;
// size of each dimension in "pixels" (rendered characters)
extern unsigned g_width;
extern unsigned g_height;
extern unsigned g_depth;
// how many frames to run the program for
extern unsigned g_max_iterations;
extern char g_mesh_file[256];
// defines the max and min values of random rotation bias
extern float rand_min;
// random rotation biases - the higher, the faster the rotation around x, y, 
extern float random_bias_x;
extern float random_bias_y;
extern float random_bias_z;
extern bool render_from_file;

void arg_parse(int argc, char** argv);
