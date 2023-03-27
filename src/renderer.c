#include "renderer.h"
#include "draw.h"
#include "objects.h"
#include "vector.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h> // malloc, free
#include <string.h>
#include <limits.h> // INT_MAX, INT_MIN


// perpendicular 2D vector, i.e. rotated by 90 degrees ccw
#define VEC_PERP(src) (      \
{                            \
    __typeof__ (src) _ret;   \
    _ret.x = -src.y;         \
    _ret.y = src.x;          \
    _ret.z = 0;              \
    _ret;                    \
}                            \
)

#define VEC_PERP_DOT_PROD(a, b) a.x*b.y - a.y*b.x

#define VEC_MAGN_SQUARED(vec) vec->x*vec->x + vec->y*vec->y + vec->z*vec->z

bool g_use_perspective = false;
bool g_use_reflectance = false;

/*
 * Note for programmers:
 *
 * The table below maps each connection type (defined as enum in renderer.h)
 * to a render function. Thefore if you want to render a new 2D shape, define
 * your connection type and refine your custom rendering function.
 *
 * It uses the X macro pattern for the mapping. X macro does not need to be
 * defined yet. However it needs to be defined every time we want to expand
 * the table.
 *
 * We typically define X one time to expand the first column into an enum
 * type in order to get the indexes (this is not done in this case as the
 * indexes are already defined). We then expand the second column to get
 * an array of pointers to functions.
 *
 * After defining the render functions, it generates a function table whose
 * index is the connection type and the value a pointer to its corresponding
 * render function. As a final note, all functions must take the same parameter
 * types.
 */
#define CONNECTION_TO_FUNC                                 \
        X(CONNECTION_RECT,     render__ray_hits_rectangle) \
        X(CONNECTION_TRIANGLE, render__ray_hits_triangle)


static inline float render__cosine_squared(vec3i_t* vec1, vec3i_t* vec2) {
    const unsigned m1 = vec1->x*vec1->x + vec1->y*vec1->y + vec1->z*vec1->z;
    const unsigned m2 = vec2->x*vec2->x + vec2->y*vec2->y + vec2->z*vec2->z;
#if 0
    printf("v1 = %d, %d, %d\n", vec1->x, vec1->y, vec1->z);
    printf("v2 = %d, %d, %d\n", vec2->x, vec2->y, vec2->z);
    printf("angle = %.2f\n", ((float)vec_vec3i_dotprod(vec1, vec2))*vec_vec3i_dotprod(vec1, vec2) /
        ((float)m1*m2)
);
#endif
    return ((float)vec_vec3i_dotprod(vec1, vec2))*vec_vec3i_dotprod(vec1, vec2) /
           ((float)m1*m2);
}


// defines a plane each time we're about to hit a pixel
plane_t* g_plane_test;
ray_t* g_ray_test;
// camera where rays are shot from 
camera_t g_camera;
// stores the colors of a surfaces after it reflects light - from brightest to darkest
color_t g_colors_refl[32];

//------------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------------

// Whether a point m is inside a triangle (a, b, c)
static inline bool render__is_point_in_triangle(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c) {
/*
 * To test whether a point is inside a triangle,    | a_perp(-a_y, a,x)
 * we use the concept of perpendicular (perp)       | ^                     <----
 * vectors and perpendicular dot product. Perp      |  \                        |
 * dot product (pdot) formulates whether vector b is|  |                        |
 * clockwise (cw) or counterclockwise (ccw) of a.   |   \
 * Given vector a(a_x, a_y), its perp vector a_perp |    \                  a(a_x, a_y)
 * is defined as the same vector rotated by 90      |     \             ---->
 * degrees ccw:                                     |     |   ---------/     
 * a_perp = (-a_y, a_x)                             |      \-/               
 *                                                  |
 * The dot product (.) alone doesn't tell us whether|
 * b is (c)cw of a. We need the pdot for that.      |  ^ a_perp        b cw from a
 * As shown in the sketch on the right half:        |  |               angle(a, b) > 90
 *                                                  |   \              a_perp . b < 0
 * a_perp . b < 0 when b is cw from a and the       |   |       ----->
 * angle between a, b is obtuse and                 |    |-----/     a 
 * a_perp . b < 0 when b is cw from a and the       |    |
 * angle between a, b is acute.                     |    |
 *                                                  |    v b
 * Therefore a_perp . b < 0 when b is cw from a.    |         
 * Similarly, a_perp . b > 0 when b is ccw from a.  |  ^ a_perp         b cw from a
 * .                                               .|   \               angle(a, b) < 90
 * .                                               .|   |       ----->  a_perp . b < 0
 * .                                               .|   -------/     a
 * .                                               .|    \
 * .               (cont'ed)                       .|     \-  
 * .                                               .|       \
 * .                                               .|        > b
 * .                                               .|
 * The scematic below shows that for point M to be  | For M to be inside triangle (ABC),
 * inside triangle (ABC) the following condition    | MB needs to be (c)cw from MA, MC
 * must be satisfied:                               | (c)cw from MB and MA (c)cw from MC
 *                                                  |                   A
 * (MB ccw from MA) => MA_perp . MB > 0 and         |                  _+         
 * (MC ccw from MB) => MB_perp . MC > 0 and         |                 / ^\_       
 * (MA ccw from MC) => MC_perp . MA > 0 and         |               _/ /   \
 * or                                               |              /   |    \_    
 * (MB cw from MA) => MA_perp . MB < 0 and          |             /   /       \
 * (MC cw from MB) => MB_perp . MC < 0 and          |           _/  M*------   \_ 
 * (MA cw from MC) => MC_perp . MA < 0 and          |          /  --/       \---->
 * .                                               .|         / -/     ______/   + 
 * .                                               .|       _--/______/           C
 * .                                               .|      </_/                   
 * .                                               .|      +
 * .                                               .|      B
 */
    const vec3i_t ma = vec_vec3i_sub(m, a);
    const vec3i_t mb = vec_vec3i_sub(m, b);
    const vec3i_t mc = vec_vec3i_sub(m, c);
    // cw = clockwise, ccw = counter-clockwise
    const bool are_all_cw =  ((VEC_PERP_DOT_PROD(ma, mb) < 0) &&
                              (VEC_PERP_DOT_PROD(mb, mc) < 0) &&
                              (VEC_PERP_DOT_PROD(mc, ma) < 0));
    const bool are_all_ccw = ((VEC_PERP_DOT_PROD(ma, mb) > 0) &&
                              (VEC_PERP_DOT_PROD(mb, mc) > 0) &&
                              (VEC_PERP_DOT_PROD(mc, ma) > 0));
    return are_all_cw || are_all_ccw;
}

static inline bool render__is_point_in_rect(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c, vec3i_t* d) {
   /* 
    * The diagram below visualises the conditions for M to be inside rectangle ABCD:
    *
    *                  A        (AM.AB).unit(AB)     B    AM.AB > 0
    *                  +---------->-----------------+     AM.AB < AB.AB 
    *                  |          .                 |
    *                  |          .                 |
    *                  |          .                 |
    *                  |          .                 |     AM.AD > 0
    * (AD.AM).unit(AD) v. . . . . *M                |     AM.AD < AD.AD 
    *                  |                            |
    *                  |                            |
    *                  |                            |
    *                  +----------------------------+ 
    *                  D                            C
    *
    */
    vec3i_t ab = vec_vec3i_sub(a, b);
    vec3i_t ad = vec_vec3i_sub(a, d);
    vec3i_t am = vec_vec3i_sub(a, m);
    return (0 < vec_vec3i_dotprod(&am, &ab)) &&
           (vec_vec3i_dotprod(&am, &ab) < vec_vec3i_dotprod(&ab, &ab)) &&
           (0 < vec_vec3i_dotprod(&am, &ad)) &&
           (vec_vec3i_dotprod(&am, &ad) < vec_vec3i_dotprod(&ad, &ad));
}

static vec3i_t rander__ray_plane_intersection(plane_t* plane, ray_t* ray) {
   /*
    * The parametric line of a ray from from the origin O through 
    * point B ('end' of the ray) is:
    * R(t) = O + t(B - O) = tB
    * This ray meets the plane for some t=t0 such that:
    * R(t0) = B*t0
    * Therefore R(t0) validates the equation of the plane.
    * For the plane we know the normal vector n and the offset
    * from the origin d. Any point X on the plane validates its
    * equation, which is:
    * n.X = d
    * Since R(t0) lies on the plane:
    * n.R(t0) = d =>
    * n.B*t0 = d =>
    * t0 = d/(n.B)
    * Finally, the ray meets the plane at point
    * R(t0) = (d/(n.B))*B
    * This is what this function returns.
    */
    float t0 = (float)plane->offset / vec_vec3i_dotprod(plane->normal, ray->end);
    // only interested in intersections along the positive direction
    t0 = (t0 < 0.0) ? -t0 : t0;
    vec3i_t ray_at_intersection = vec_vec3i_mul_scalar(ray->end, t0);
    return ray_at_intersection;
}

static bool render__ray_hits_rectangle(ray_t* ray, vec3i_t** points) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    vec3i_t* p0 = points[0];
    vec3i_t* p1 = points[1];
    vec3i_t* p2 = points[2];
    vec3i_t* p3 = points[3];
    obj_plane_set(g_plane_test, p0, p1, p2);
    vec3i_t ray_plane_intersection = rander__ray_plane_intersection(g_plane_test, ray);
    return render__is_point_in_rect(&ray_plane_intersection, p0, p1, p2, p3);
}

static bool render__ray_hits_triangle(ray_t* ray, vec3i_t** points) {
    // Find the intersection between the ray and the triangle (p0, p1, p2).
    // Return whether the intersection is whithin that triangle
    vec3i_t* p0 = points[0];
    vec3i_t* p1 = points[1];
    vec3i_t* p2 = points[2];
    obj_plane_set(g_plane_test, p0, p1, p2);
    vec3i_t ray_plane_intersection = rander__ray_plane_intersection(g_plane_test, ray);
    return render__is_point_in_triangle(&ray_plane_intersection, p0, p1, p2);
}

/* find the z-coordinate on a plane give x and y */
static inline int plane_z_at_xy(plane_t* plane, int x, int y) {
    // solve for z in plane's eq/n: n.x*x + n.y*y + n.z*z + offset = 0
    vec3i_t coeffs = (vec3i_t) {plane->normal->x, plane->normal->y, plane->offset};
    vec3i_t xyz = (vec3i_t) {x, y, 1};
    return round(1.0/plane->normal->z*(-vec_vec3i_dotprod(&coeffs, &xyz)));
}


// expand the second column of `CONNECTION_TO_FUNC`, mapping connections
// to intersection functions in an 1-1 manner
static bool (*func_table_intersection[NUM_CONNECTIONS])(ray_t* ray, vec3i_t** points) = {
#define X(a, b) b,
    CONNECTION_TO_FUNC
#undef X
};

/* perspective trasnform to map world point (3D) to screen (2D) */
static inline vec3i_t render__persp_transform(vec3i_t* xyz) {
    return (vec3i_t) {xyz->x*g_camera.focal_length/(xyz->z + 1e-8),
                      xyz->y*g_camera.focal_length/(xyz->z + 1e-8),
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
    g_plane_test = obj_plane_new(&dummy, &dummy, &dummy);
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
    //vec3i_t dummy_vec = {0, 0, 0};
    //iray_t* ray = obj_ray_new(ray_origin.x, ray_origin.y, ray_origin.z,
    //    dummy_vec.x, dummy_vec.y, dummy_vec.z);
    const color_t background = ' ';
    // bounding box pixel indexes
    const int xmin = UT_MIN(shape->bounding_box.x0, shape->bounding_box.x1);
    const int ymin = UT_MIN(shape->bounding_box.y0, shape->bounding_box.y1);
    const int xmax = UT_MAX(shape->bounding_box.x0, shape->bounding_box.x1);
    const int ymax = UT_MAX(shape->bounding_box.y0, shape->bounding_box.y1);
    // stores the 4 surface points to connect together
    vec3i_t** surf_points = malloc(sizeof(vec3i_t*) * 4);

    for (int y = ymin; y <= ymax; ++y) {
        for (int x = xmin; x <= xmax; ++x) {
            // the final pixel and color to render
            vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MAX};
            color_t rendered_color = background;
            for (size_t isurf = 0; isurf < shape->n_faces; ++isurf) {
                // unpack surface info, hence define surface from shape->vertices
                const size_t ipoint0 = shape->connections[isurf][0];
                const size_t ipoint1 = shape->connections[isurf][1];
                const size_t ipoint2 = shape->connections[isurf][2];
                const size_t ipoint3 = shape->connections[isurf][3];
                const int connection_type = shape->connections[isurf][4];
                const color_t surf_color = shape->connections[isurf][5];
                surf_points[0] = shape->vertices[ipoint0];
                surf_points[1] = shape->vertices[ipoint1];
                surf_points[2] = shape->vertices[ipoint2];
                surf_points[3] = shape->vertices[ipoint3];
                
                // find intersections of ray and surface and set colour accordingly
                obj_plane_set(g_plane_test, surf_points[0], surf_points[1], surf_points[2]);
                // we keep the z to find the furthest one from the origin and we draw its x and y
                // which z the ray currently hits the plane - can be up to two hits
                int z_hit = plane_z_at_xy(g_plane_test, x, y);
                obj_ray_send(g_ray_test, x, y, z_hit);
                if ((*func_table_intersection[connection_type])(g_ray_test, surf_points) &&
                (z_hit < rendered_point.z)) {
                    rendered_color = surf_color;
                    rendered_point = (vec3i_t) {x, y, z_hit};
                    // modern compilers (gcc >= 4.0, clang >= 3.0) know how to optimize this:
                    if (g_use_perspective)
                        rendered_point = render__persp_transform(&rendered_point);
                    if (g_use_reflectance)
                        rendered_color = render__reflect(g_ray_test, g_plane_test, shape);
                }
            } /* for surfaces */
            screen_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
        } /* for x */
    } /* for y */
    // free ray-tracing-related constructs
    free(surf_points);
}


void render_from_obj_file(char* filepath) {
    // TODO
}

void render_end() {
    obj_plane_free(g_plane_test);
}
