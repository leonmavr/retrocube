#ifndef RENDERER_H
#define RENDERER_H
#include "renderer.h"
#include "objects.h"
#include "draw.h"

// checks whether the ray hits each pixel
extern plane_t* g_plane_test;
// camera where rays are shot from 
extern camera_t g_camera;

// TODO: initialise camera and intersection plane (g_plane_test)
void renderer_init(int cam_x0, int cam_y0, float focal_length);

void draw_shape(shape_t* cube);

// TODO: render .obj (blender) file by reading vertices (v) and
// their connections (f)
void render_from_obj_file(char* filepath);

void renderer_end();

#endif /* RENDERER_H */
