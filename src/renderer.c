#include "renderer.h"
#include "draw.h"
#include "objects.h"
#include "vector.h"
#include "utils.h"
#include <stdio.h>
#include <limits.h> // INT_MAX


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


// defines a plane each time we're about to hit a pixel
plane_t* g_plane_test;
// camera where rays are shot from 
camera_t g_camera;

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
 * The dot product alone doesn't tell us whether b  |
 * is (c)cw of a. We need the pdot for that.        |  ^ a_perp        angle(a, b) > 90
 * As shown in the sketch on the right half:        |  |               a_perp . b < 0
 *                                                  |   \
 * a_perp . b < 0 when b is ccw from a and the      |   |       ----->
 * angle between a, b is obtuse and                 |    |-----/     a 
 * a_perp . b < 0 when b is ccw from a and the      |    |
 * angle between a, b is acute.                     |    |
 *                                                  |    v b
 * Therefore a_perp . b < 0 when b is ccw from a.   |         
 * Similarly, a_perp . b > when b is cw from a.     |  ^ a_perp         angle(a, b) < 90
 * .                                               .|   \               a_perp . b > 0
 * .                                               .|   |       ----->
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
    vec3i_t ma = vec_vec3i_sub(m, a);
    vec3i_t mb = vec_vec3i_sub(m, b);
    vec3i_t mc = vec_vec3i_sub(m, c);
    return 
        // cw case
        (((VEC_PERP_DOT_PROD(ma, mb) < 0) &&
        (  VEC_PERP_DOT_PROD(mb, mc) < 0) &&
        (  VEC_PERP_DOT_PROD(mc, ma) < 0)) ||
        // ccw case
        (( VEC_PERP_DOT_PROD(ma, mb) > 0) &&
        (  VEC_PERP_DOT_PROD(mb, mc) > 0) &&
        (  VEC_PERP_DOT_PROD(mc, ma) > 0)));
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


static vec3i_t obj_ray_plane_intersection(plane_t* plane, ray_t* ray) {
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

static bool render__ray_hits_rectangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    obj_plane_set(g_plane_test, p0, p1, p2);
    vec3i_t ray_plane_intersection = obj_ray_plane_intersection(g_plane_test, ray);
    return render__is_point_in_rect(&ray_plane_intersection, p0, p1, p2, p3);
}

static bool render__ray_hits_triangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // Find the intersection between the ray and the triangle (p0, p1, p2).
    // Return whether the intersection is whithin that triangle
    obj_plane_set(g_plane_test, p0, p1, p2);
    vec3i_t ray_plane_intersection = obj_ray_plane_intersection(g_plane_test, ray);
    return render__is_point_in_triangle(&ray_plane_intersection, p0, p1, p2);
}

/* find the z-coordinate on a plane give x and y */
static inline int plane_z_at_xy(plane_t* plane, int x, int y) {
    // solve for z in plane's eq/n: n.x*x + n.y*y + n.z*z + offset = 0
    vec3i_t coeffs = (vec3i_t) {plane->normal->x, plane->normal->y, plane->offset};
    vec3i_t xyz = (vec3i_t) {x, y, 1};
    return round(1.0/plane->normal->z*(-vec_vec3i_dotprod(&coeffs, &xyz)));
}


void render_init(int cam_x0, int cam_y0, float focal_length) {
    vec3i_t dummy = {0, 0, 0};
    g_plane_test = obj_plane_new(&dummy, &dummy, &dummy);
    obj_camera_set(&g_camera, cam_x0, cam_y0, focal_length);
}

void render_write_shape(shape_t* shape) {
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
    const bool use_persp = !ut_float_equal(g_camera.focal_length, 0.f);
    const float focal_length = g_camera.focal_length;
    const vec3i_t ray_origin = (vec3i_t) {g_camera.x0, g_camera.y0, g_camera.focal_length};
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
        for (int y = ymin; y <= ymax; ++y) {
            for (int x = xmin; x <= xmax; ++x) {
                // the final pixel and color to render
                vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MAX};
                color_t rendered_color = background;
                for (size_t isurf = 0; isurf < 6; ++isurf) {
                    obj_plane_set(plane, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]);
                    // we keep the z to find the furthest one from the origin and we draw its x and y
                    // which z the ray currently hits the plane - can be up to two hits
                    int z_hit = plane_z_at_xy(plane, x, y);
                    obj_ray_send(ray, x, y, z_hit);
                    if (render__ray_hits_rectangle(ray, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2], surfaces[isurf][3]) &&
                    (z_hit < rendered_point.z)) {
                        rendered_color = shape->colors[isurf];
                        // use perspective transform if its flag is set:
                        // x' = x*f/z, y' = y*f/z
                        rendered_point = (vec3i_t) {x*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*x,
                                                    y*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*y,
                                                    z_hit};
                    }
                }
                screen_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
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
        for (int y = ymin; y <= ymax; ++y) {
            for (int x = xmin; x <= xmax; ++x) {
                // the final pixel and color to render
                vec3i_t rendered_point = (vec3i_t) {0, 0, INT_MAX};
                color_t rendered_color = background;
                for (size_t isurf = 0; isurf < 8; ++isurf) {
                    obj_plane_set(plane, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]);
                    // we keep the z to find the furthest one from the origin and we draw its x and y
                    // which z the ray currently hits the plane - can be up to two hits
                    int z_hit = plane_z_at_xy(plane, x, y);
                    obj_ray_send(ray, x, y, z_hit);
                    if (render__ray_hits_triangle(ray, surfaces[isurf][0], surfaces[isurf][1], surfaces[isurf][2]) &&
                    (z_hit < rendered_point.z)) {
                        rendered_color = shape->colors[isurf];
                        rendered_point = (vec3i_t) {x*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*x,
                                                    y*(1 + (!!use_persp)*focal_length/(z_hit + 1e-4)) - (!!use_persp)*y,
                                                    z_hit};
                    }
                }
                screen_write_pixel(rendered_point.x, rendered_point.y, rendered_color);
            } /* for columns */
        } /* for rows */
    } else if (shape->type == TYPE_TRIANGLE) {
        // render a simple triangle - no need to account for surfaces
        vec3i_t* p0 = shape->vertices[0];
        vec3i_t* p1 = shape->vertices[1];
        vec3i_t* p2 = shape->vertices[2];
        for (int y = ymin; y <= ymax; ++y) {
            for (int x = xmin; x <= xmax; ++x) {
                obj_plane_set(plane, p0, p1, p2);
                int z_hit = plane_z_at_xy(plane, x, y);
                obj_ray_send(ray, x, y, z_hit);
                if (render__ray_hits_triangle(ray, p0, p1, p2)) {
                    screen_write_pixel(x*(1 + (!!use_persp)*focal_length*x/(z_hit + 1e-4)) - (!!use_persp)*x,
                                       y*(1 + (!!use_persp)*focal_length*y/(z_hit + 1e-4)) - (!!use_persp)*y,
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

void render_end() {
    obj_plane_free(g_plane_test);
}
