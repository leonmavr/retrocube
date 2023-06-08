#include "renderer.h"
#include "screen.h"
#include "objects.h"
#include "vector.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <limits.h> // INT_MAX, INT_MIN


#define VEC_MAGN_SQUARED(vec) vec->x*vec->x + vec->y*vec->y + vec->z*vec->z
#define VEC_PERP_DOT_PROD(a, b) a.x*b.y - a.y*b.x


bool g_use_perspective = false;
bool g_use_reflectance = false;
int* g_z_buffer;
vec3i_t** g_surf_points;
// defines a plane each time we're about to hit a pixel
plane_t* g_plane_test;
ray_t* g_ray_test;
// camera where rays are shot from 
camera_t g_camera;
// stores the colors of a surfaces after it reflects light - from brightest to darkest
color_t g_colors_refl[32];
// expand the second column of `CONN_TABLE`, mapping connections
// to intersection functions in an 1-1 manner
bool (*func_table_intersection[NUM_CONNECTIONS])(ray_t* ray, vec3i_t** points) = {
#define X(a, b, c) c,
    CONN_TABLE
#undef X
};

//------------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------------
static inline float render__cosine_squared(vec3i_t* vec1, vec3i_t* vec2) {
    const unsigned m1 = vec1->x*vec1->x + vec1->y*vec1->y + vec1->z*vec1->z;
    const unsigned m2 = vec2->x*vec2->x + vec2->y*vec2->y + vec2->z*vec2->z;
    return ((float)vec_vec3i_dotprod(vec1, vec2))*vec_vec3i_dotprod(vec1, vec2) /
           ((float)m1*m2);
}


/* find the z-coordinate on a plane given x and y */
static inline int plane_z_at_xy(plane_t* plane, int x, int y) {
    // solve for z in plane's eq/n: n.x*x + n.y*y + n.z*z + offset = 0
    vec3i_t coeffs = (vec3i_t) {plane->normal->x, plane->normal->y, plane->offset};
    vec3i_t xyz = (vec3i_t) {x, y, 1};
    return round(1.0/plane->normal->z*(-vec_vec3i_dotprod(&coeffs, &xyz)));
}


/* perspective trasnform to map world point (3D) to screen (2D) */
static inline vec3i_t render__persp_transform(vec3i_t* xyz) {
    // to avoid drawing inverted images
    int sign = (xyz->z > 0) ? -1 : 1;
    return (vec3i_t) {UT_CLIP(sign*xyz->x*g_camera.focal_length/(xyz->z + 1e-8), -g_cols/2, g_cols/2),
                      UT_CLIP(sign*xyz->y*g_camera.focal_length/(xyz->z + 1e-8), -g_rows, g_rows),
                      xyz->z};
}

/**
* @brief Returns a color based on the angle between the ray and plane,
*        simulating reflection
*
* @param[in] ray A pointer to ray
* @param[in] plane A pointer to plane
* @param[in] shape A pointer to shape
*
* @returns Reflected color
*/
static inline color_t render__reflect(ray_t* ray, plane_t* plane, mesh_t* shape) {
    const int z_refl = (g_use_perspective) ? g_camera.focal_length : -shape->center->z/2;
    vec3i_t camera_axis = {g_camera.x0,
                            g_camera.y0,
                            z_refl};
    const vec3i_t plane_normal = *plane->normal;
    const int ray_angle_ccw = VEC_PERP_DOT_PROD(camera_axis, plane_normal);
    const int sign = (ray_angle_ccw > 0) ? 1 : -1;
    const float ray_plane_angle = sign*render__cosine_squared(&camera_axis, plane->normal);
    //-----------------------------------------------------
    // reflectance
    /*
     *
     *   w_a = 2/n
     *   <--------->
     *  -1       -.66      -.33       0         .33       .66        1
     *   +---------+---------+---------+---------+---------+---------+
     *   |         |         |         |         |         |         |
     *   +---------+---------+---------+---------+---------+---------+
     *        |         |         |         |         |         |
     *        |         |         |         |         |         |
     *        |         |         |         |         |         |
     *   0    v         v         v         v         v         v    31
     *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
     *   |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
     *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
     *   <-------->
     *   w_c = floor(32/n)
     *
     * i_angle = floor((angle + 1)/(2/n))
     * w_c = floor(32/n)
     * i_color_start = (32 - (32 mod n))/2 + w_c/2
     * i_color = i_color_start + i_angle * wc
     */
    const int n = 2*shape->n_faces;
    const float w_a = 2.0/n; 
    const size_t w_c = 32/n;
    return g_colors_refl[(size_t)(
                         (32 % n)/2 + w_c/2 +
                         (size_t)((ray_plane_angle+1)/w_a)*w_c)];
}

static void render_reset_zbuffer() {
    for (size_t i = 0; i < g_screen_buffer_size; ++i)
        g_z_buffer[i] = INT_MAX;
}

//------------------------------------------------------------------------------------
// External functions
//------------------------------------------------------------------------------------
void render_use_perspective(int center_x0, int center_y0, float focal_length) {
    g_use_perspective = true;
    obj_camera_set(&g_camera, center_x0, center_y0, focal_length);
}

void render_use_reflectance() {
    g_use_reflectance = true;
}

void render_init() {
    vec3i_t dummy = {0, 0, 0};
    // z buffer that records the depth of each pixel
    g_z_buffer = malloc(sizeof(int) * g_screen_buffer_size);
    render_reset_zbuffer();
    g_plane_test = obj_plane_new(&dummy, &dummy, &dummy);
    g_surf_points = malloc(sizeof(vec3i_t*) * 4);
    g_ray_test = obj_ray_new(0, 0, 0, 0, 0, 0);
    // reflection colors from brightest to darkest
    strncpy(g_colors_refl, "#OT&=@$x%><)(nc+:;qy\"/?|+.,-v^!`", 32);
}


void render_write_shape(mesh_t* shape) {
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
 *                    p3    \            p2                      o cube's centre a
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
    vec3i_t ray_origin = (vec3i_t) {g_camera.x0, g_camera.y0, g_camera.focal_length};
    vec_vec3i_copy(g_ray_test->orig, &ray_origin);
    // screen boundaries
    int xmin, xmax, ymin, ymax;
    if (g_use_perspective) {
        // clip rendering area to bounding box
        xmin = UT_MIN(shape->bounding_box.x0, shape->bounding_box.x1);
        ymin = UT_MIN(shape->bounding_box.y0, shape->bounding_box.y1);
        xmax = UT_MAX(shape->bounding_box.x0, shape->bounding_box.x1);
        ymax = UT_MAX(shape->bounding_box.y0, shape->bounding_box.y1);
    } else {
        // clip rendering area to screen clip to rows and columns
        xmin = UT_MAX(-g_cols/2+1, shape->bounding_box.x0);
        ymin = UT_MAX(-g_rows, shape->bounding_box.y0);
        xmax = UT_MIN(g_cols/2, shape->bounding_box.x1);
        ymax = UT_MIN(g_rows+1, shape->bounding_box.y1);
    }
    // downscale by subsampling if we use perspective
    unsigned step = (g_use_perspective) ?
        UT_MIN(abs(shape->bounding_box.z0), abs(shape->bounding_box.z1))/g_camera.focal_length :
        1;
    step = (step < 1) ? 1 : step;

    for (int y = ymin;  y <= ymax; y += step) {
        for (int x = xmin; x <= xmax; x += step) {
            // -y to avoid drawing inverted images
            size_t buffer_ind = screen_xy2ind(x, -y);
            // the final pixel and color to render
            vec3i_t rendered_point = (vec3i_t) {x, -y, g_z_buffer[buffer_ind]};
            for (size_t isurf = 0; isurf < shape->n_faces; ++isurf) {
                // unpack surface info, hence define surface from shape->vertices
                const size_t ipoint0 = shape->connections[isurf][0];
                const size_t ipoint1 = shape->connections[isurf][1];
                const size_t ipoint2 = shape->connections[isurf][2];
                const size_t ipoint3 = shape->connections[isurf][3];
                const int connection_type = shape->connections[isurf][4];
                const color_t surf_color = shape->connections[isurf][5];
                g_surf_points[0] = shape->vertices[ipoint0];
                g_surf_points[1] = shape->vertices[ipoint1];
                g_surf_points[2] = shape->vertices[ipoint2];
                g_surf_points[3] = shape->vertices[ipoint3];
                
                // find intersections of ray and surface and set colour accordingly
                obj_plane_set(g_plane_test, g_surf_points[0], g_surf_points[1], g_surf_points[2]);
                // we keep the z to find the closest one to the origin and we draw
                // its x and y at the z the ray hits the current surface
                int z_hit = plane_z_at_xy(g_plane_test, x, y);
                obj_ray_send(g_ray_test, x, y, z_hit);
                vec3i_t persp_point; 
                // if we use perspective, we index the depth buffer at the (x,y)
                // of the projected point, not the original one (`persp_point`)
                if (g_use_perspective) {
                    persp_point = (vec3i_t) {x, -y, z_hit};
                    persp_point = render__persp_transform(&persp_point);
                    buffer_ind = screen_xy2ind(persp_point.x, persp_point.y);
                }
                if ((*func_table_intersection[connection_type])(g_ray_test, g_surf_points) &&
                (z_hit < g_z_buffer[buffer_ind])) {
                    color_t rendered_color = surf_color;
                    // modern compilers (gcc >= 4.0, clang >= 3.0) know how to optimize this:
                    if (g_use_reflectance)
                        rendered_color = render__reflect(g_ray_test, g_plane_test, shape);
                    if (g_use_perspective)
                        rendered_point = persp_point;
                    g_z_buffer[buffer_ind] = z_hit;
                    screen_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
                }
            } /* for surfaces */
        } /* for x */
    } /* for y */
}

void render_flush() {
    render_reset_zbuffer();
    screen_flush();
}


void render_end() {
    free(g_surf_points);
    obj_plane_free(g_plane_test);
}
