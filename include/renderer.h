#ifndef RENDERER_H
#define RENDERER_H
#include "renderer.h"
#include "objects.h"
#include "draw.h"

// checks whether the ray hits each pixel
extern plane_t* g_plane_test;


typedef struct camera {
    // origin
    int x0;
    int y0;
    // focal length
    float focal_length;
} camera_t;

// TODO: initialise camera and intersection plane (g_plane_test)
void renderer_init(int cam_x0, int cam_y0, float focal_length);

/**
 * @brief Write the pixels of the given shape in the screen buffer.
 *        The pixels will not be displayed on the screen until
 *        screen_flush() is invoked.
 *
 * @param x x-coordinate of pixel to write
 * @param y y-coordinate of pixel to write
 * @param c "color" of the pixel as an ASCII character
 */
void draw_shape(shape_t* cube, camera_t* camera);

// TODO: render .obj (blender) file by reading vertices (v) and
// their connections (f)
void render_from_obj_file(char* filepath);
#endif /* RENDERER_H */
