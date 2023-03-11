#ifndef RENDERER_H
#define RENDERER_H
#include "renderer.h"
#include "objects.h"
#include "draw.h"

// checks whether the ray hits each pixel
extern plane_t* g_plane_test;
// camera where rays are shot from 
extern camera_t g_camera;

/**
 * @brief Initializes renderer by setting the point of persperctive and focal length
 *        if projection is to be used
 *
 * @param x0 x-coordinate of the camera where rays are shot from
 * @param y0 y-coordinate of the camera where rays are shot from
 * @param focal_length Focal length of pinhole camera if projection is to be used.
 *                     Pass 0 to disable the projection. 
 */
void render_init(int cam_x0, int cam_y0, float focal_length);

/**
 * @brief Writes shape to screen buffer before it's rendered.
 *        Once shapes have been written, they can be displayed with `screen_flush()`
 *        (the latter is defined in screen.h)
 *
 * @param shape Pointer to the shape to write to the renderer. Note that it must be
 *              initialised 
 */
void render_write_shape(shape_t* shape);

/**
 * @brief TODO: render blender object from .obj file 
 *
 * @param filepath Filepath to .obj file to render 
 */
void render_from_obj_file(char* filepath);

/**
 * @brief Closes the renderer and deallocates its structures 
 */
void render_end();

#endif /* RENDERER_H */
