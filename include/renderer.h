#ifndef RENDERER_H
#define RENDERER_H
#include "renderer.h"
#include "objects.h"
#include "draw.h"
#include <stdbool.h>


// checks whether the ray hits each pixel
extern plane_t* g_plane_test;
// camera where rays are shot from 
extern camera_t g_camera;
extern color_t g_colors_refl[32];
extern bool g_use_perspective;
extern bool g_use_reflectance;


/**
 * @brief Use perspective transform (pinhole camera model) when rendering shapes. 
 *        After calling this function, call `render_init()` for the changes to
 *        take place.
 *
 * @param center_x0    x-coordinate of the perspective center - aka the point
 *                     where rays are shot from
 * @param center_y0    y-coordinate of the perspective center - aka the point
 *                     where rays are shot from
 * @param focal_length Focal length of pinhole camera
 */
void render_use_perspective(int center_x0, int center_y0, float focal_length);

/**
 * @brief Reflect rays based on the hit angle
 */
void render_use_reflectance();

/**
 * @brief Initializes renderer by setting the point of persperctive and focal length
 *        if projection is to be used
 */
void render_init();

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
