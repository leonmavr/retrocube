#include "renderer.h"
#include "draw.h"
#include "objects.h"
#include "vector.h"
#include "utils.h"
#include <stdio.h>
#include <limits.h> // INT_MAX

void renderer_init(int cam_x0, int cam_y0, float focal_length) {
    // TODO
}

void draw_shape(shape_t* shape, camera_t* camera) {
/*
 * This function renders the given cube by the basic ray tracing principle.
 *
 * A ray is shot from the origin to every pixel on the screen row by row.
 * For each screen coordinate, there can zero to two intersections with the cube.
 * If there is one, render the (x, y) of the intersection (not the x,y of the screen!).
 * If there are two, render the (x, y) of the closer intersection. In the figure below,
 * z_hit are the z of the two intersections and z_rend is the closest one.
 *
 * The ray below intersects faces (p0, p1, p2, p3) and  (p4, p5, p6, p7)
 * 
 *                      O camera origin  
 *                       \
 *                        \
 *                         V ray
 *                    p3    \            p2                      o cube's centre 
 *                    +-------------------+                      + cube's vertices
 *                    | \     \           | \                    # ray-cube intersections
 *                    |    \   # z_rend   |    \                   (z_hit)
 *                    |      \  p7        |       \
 *                    |         +-------------------+ p6         ^ y
 *                    |         | \       .         |            |
 *                    |         |  \      .         |            |
 *                    |         |   \     .         |            o-------> x
 *                    |         |    \    .         |             \
 *                    |         |     \   .         |              \
 *                 p0 +---------|......\..+ p1      |               V z
 *                     \        |       \    .      |
 *                       \      |        #     .    |
 *                          \   |         \      .  |
 *                             \+----------\--------+
 *                              p4          \        p5
 *                                           \
 *                                            V
 */
    // whether we want to use the perspective transform or not
    const bool use_persp = camera != NULL;
    const unsigned focal_length = (camera != NULL) ? camera->focal_length : 1;
    const vec3i_t ray_origin = (camera != NULL) ?
        (vec3i_t) {camera->x0, camera->y0, camera->focal_length} :
        (vec3i_t) {0, 0, 0};
    vec3i_t dummy_vec = {0, 0, 0};
    ray_t* ray = obj_ray_new(ray_origin.x, ray_origin.y, ray_origin.z,
        dummy_vec.x, dummy_vec.y, dummy_vec.z);
    plane_t* plane = obj_plane_new(&dummy_vec, &dummy_vec, &dummy_vec);
    const color_t background = ' ';
    // bounding box pixel indexes
    int xmin = UT_MIN(shape->bounding_box.x0, shape->bounding_box.x1);
    int ymin = UT_MIN(shape->bounding_box.y0, shape->bounding_box.y1);
    int xmax = UT_MAX(shape->bounding_box.x0, shape->bounding_box.x1);
    int ymax = UT_MAX(shape->bounding_box.y0, shape->bounding_box.y1);

    if (shape->type == TYPE_CUBE) {
        //// initialisations
        vec3i_t* p0 = shape->vertices[0];
        vec3i_t* p1 = shape->vertices[1];
        vec3i_t* p2 = shape->vertices[2];
        vec3i_t* p3 = shape->vertices[3];
        vec3i_t* p4 = shape->vertices[4];
        vec3i_t* p5 = shape->vertices[5];
        vec3i_t* p6 = shape->vertices[6];
        vec3i_t* p7 = shape->vertices[7];
        // each quad of points p0 to p5 represents a cube's face
        vec3i_t* surfaces[6][4] = {
            {p0, p1, p2, p3},
            {p0, p4, p7, p3},
            {p4, p5, p6, p7},
            {p5, p1, p2, p6},
            {p7, p6, p2, p3},
            {p0, p4, p5, p1}
        };
        //// main processing
        for (int r = ymin; r <= ymax; ++r) {
            for (int c = xmin; c <= xmax; ++c) {
                // the final pixel and color to render
                vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MAX};
                color_t rendered_color = background;
                for (size_t isurf = 0; isurf < 6; ++isurf) {
                    obj_plane_set(plane, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]);
                    // we keep the z to find the furthest one from the origin and we draw its x and y
                    // which z the ray currently hits the plane - can be up to two hits
                    int z_hit = obj_plane_z_at_xy(plane, c, r);
                    obj_ray_send(ray, c, r, z_hit);
                    if (obj_ray_hits_rectangle(ray, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2], surfaces[isurf][3]) &&
                    (z_hit < rendered_point.z)) {
                        rendered_color = shape->colors[isurf];
                        // use perspective transform if passed camera struct wasn't NULL:
                        // x' = x*f/z, y' = y*f/z
                        rendered_point = (vec3i_t) {c*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*c,
                                                    r*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*r,
                                                    z_hit};
                    }
                }
                draw_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
            } /* for columns */
        } /* for rows */
    } else if (shape->type == TYPE_RHOMBUS) {
        //// initialisations
        vec3i_t* p0 = shape->vertices[0];
        vec3i_t* p1 = shape->vertices[1];
        vec3i_t* p2 = shape->vertices[2];
        vec3i_t* p3 = shape->vertices[3];
        vec3i_t* p4 = shape->vertices[4];
        vec3i_t* p5 = shape->vertices[5];
        // each quad of points p0 to p5 represents a rhombus' face
        vec3i_t* surfaces[8][3] = {
            {p3, p4, p0},
            {p0, p4, p1},
            {p4, p2, p1},
            {p4, p2, p3},
            {p3, p0, p5},
            {p0, p1, p5},
            {p1, p5, p2},
            {p3, p2, p5}
        };
        //// main processing
        for (int r = ymin; r <= ymax; ++r) {
            for (int c = xmin; c <= xmax; ++c) {
                // the final pixel and color to render
                vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MAX};
                color_t rendered_color = background;
                for (size_t isurf = 0; isurf < 8; ++isurf) {
                    obj_plane_set(plane, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]);
                    // we keep the z to find the furthest one from the origin and we draw its x and y
                    // which z the ray currently hits the plane - can be up to two hits
                    int z_hit = obj_plane_z_at_xy(plane, c, r);
                    obj_ray_send(ray, c, r, z_hit);
                    if (obj_ray_hits_triangle(ray, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]) &&
                    (z_hit < rendered_point.z)) {
                        rendered_color = shape->colors[isurf];
                        rendered_point = (vec3i_t) {c*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*c,
                                                    r*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*r,
                                                    z_hit};
                    }
                }
                draw_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
            } /* for columns */
        } /* for rows */
    } else if (shape->type == TYPE_TRIANGLE) {
        // render a simple triangle - no need to account for surfaces
        vec3i_t* p0 = shape->vertices[0];
        vec3i_t* p1 = shape->vertices[1];
        vec3i_t* p2 = shape->vertices[2];
        for (int r = ymin; r <= ymax; ++r) {
            for (int c = xmin; c <= xmax; ++c) {
                obj_plane_set(plane, p0, p1, p2);
                int z_hit = obj_plane_z_at_xy(plane, c, r);
                obj_ray_send(ray, c, r, z_hit);
                if (obj_ray_hits_triangle(ray, p0, p1, p2)) {
                    draw_write_pixel(c*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*c,
                                     r*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*r,
                                     shape->colors[0]);
                    }
            }
        }
    }
    // free ray-tracing-related constructs
    obj_plane_free(plane);
    obj_ray_free(ray);
}


void render_from_obj_file(char* filepath) {
    // TODO
}
