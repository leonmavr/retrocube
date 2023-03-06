#include "vector.h"
#include "objects.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // strncpy
#include <stddef.h> // size_t 

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

// Whether a point m is inside a triangle (a, b, c)
static inline bool obj__is_point_in_triangle(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c) {
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

static inline bool obj__is_point_in_rect(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c, vec3i_t* d) {
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

//----------------------------------------------------------------------------------------------------------
// Renderable shapes 
//----------------------------------------------------------------------------------------------------------
shape_t* obj_shape_new(int cx, int cy, int cz, int width, int height, int type) {

    shape_t* new = malloc(sizeof(shape_t));
    //// common attributes
    new->type = type;
    new->n_vertices = (type == TYPE_CUBE) ? 8 : 6;
    new->center = vec_vec3i_new(cx, cy, cz);
    // colors for each of the maximum possible 8 faces - change the string below to modify them
    strncpy(new->colors, "~.=@%|O+?Tn", 8);
    new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * new->n_vertices);
    //// attributes that depend on number of vertices
    if (type == TYPE_CUBE) {
        /*
        *          p3                  p2 
        *           +-------------------+
        *           | \                 | \
        *           |    \              |    \            ^y
        *           |      \  p7        |       \         |
        *           |         +-------------------+ p6    |
        *           |         |         .         |       |
        *           |         |*(cx,xy,cz)        |       o-------> x
        *           |         |         .         |        \
        *           |         |         .         |         \
        *           |         |         .         |          v z
        *        p0 +---------|.........+ p1      |
        *            \        |           .       |
        *              \      |             .     |
        *                 \   |                .  |
        *                    \+-------------------+
        *                     p4                   p5
        */
        int diag = round(UT_SQRT_TWO * UT_MIN(width/2, height/2)); 
        new->vertices[0] = vec_vec3i_new(-diag, -diag, -diag);
        new->vertices[1] = vec_vec3i_new( diag, -diag, -diag);
        new->vertices[2] = vec_vec3i_new( diag,  diag, -diag);
        new->vertices[3] = vec_vec3i_new(-diag,  diag, -diag);
        new->vertices[4] = vec_vec3i_new(-diag, -diag, diag);
        new->vertices[5] = vec_vec3i_new( diag, -diag, diag);
        new->vertices[6] = vec_vec3i_new( diag,  diag, diag);
        new->vertices[7] = vec_vec3i_new(-diag,  diag, diag);
        // back them up so no floating point error is accumulated affter the rotations
    } else if (type == TYPE_RHOMBUS) {
        /*
        *               p5              * center
        *               X               X vertex
        *              / \
        *             /   \             y 
        *            /     \            ^
        *           /       \           |
        *          /    p2   \          |
        *         /     X     \         |
        *        /  ..     ..  \        o--------> x 
        *       /..          .. \        \
        *   p3 X.       *       .X p1     \
        *       \__           __/          \
        *        \  \__   __/  /            v
        *         \     X     /              z
        *          \    p0   / 
        *           \       / 
        *            \     / 
        *             \   / 
        *              \ / 
        *               X 
        *               p4
        */
        new->vertices[0] = vec_vec3i_new(0,        0,                                width/2);
        new->vertices[1] = vec_vec3i_new(width/2,  0,                                0);
        new->vertices[2] = vec_vec3i_new(0,        0,                                -width/2);
        new->vertices[3] = vec_vec3i_new(-width/2, 0,                                0);
        new->vertices[4] = vec_vec3i_new(0,        -round(UT_PHI/(1+UT_PHI)*height), 0);
        new->vertices[5] = vec_vec3i_new(0,        round(1.0/(1+UT_PHI)*height),     0);
    }
    // finish creating the vertices
    for (int i = 0; i < new->n_vertices; ++i) {
        // shift them to the shape's center 
        *new->vertices[i] = vec_vec3i_add(new->vertices[i], new->center);
        // back them up so no floating point error is accumulated after any rotations
        new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
        vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
    }
    return new;
}

void obj_shape_rotate (shape_t* shape, float angle_x_rad, float angle_y_rad, float angle_z_rad) {
    const size_t n_corners = (shape->type == TYPE_CUBE) ? 8 : 6;
    for (size_t i = 0; i < n_corners; ++i) {
        // first, reset each vertex so no floating point error is accumulated
        vec_vec3i_copy(shape->vertices[i], shape->vertices_backup[i]);

        // point to rotate about
        int x0 = shape->center->x, y0 = shape->center->y, z0 = shape->center->z;
        // rotate around x axis, then y, then z
        // We rotate as follows (* denotes matrix product, C the shape's origin):
        // v = v - C, v = Rz*Ry*Rx*v, v = v + C
        vec_vec3i_rotate(shape->vertices[i], angle_x_rad, angle_y_rad, angle_z_rad, x0, y0, z0);
    }
}

void obj_shape_translate(shape_t* shape, float dx, float dy, float dz) {
    const size_t n_corners = (shape->type == TYPE_CUBE) ? 8 : 6;
    vec3i_t translation = {round(dx), round(dy), round(dz)};
    *shape->center = vec_vec3i_add(shape->center, &translation);
    for (size_t i = 0; i < n_corners; ++i)
        *shape->vertices[i] = vec_vec3i_add(shape->vertices[i], &translation);
}

void obj_shape_free(shape_t* shape) {
    const size_t n_corners = (shape->type == TYPE_CUBE) ? 8 : 6;
    // free the data of the vertices first
    for (size_t i = 0; i < n_corners; ++i) {
        free(shape->vertices[i]);
        free(shape->vertices_backup[i]);
    }
    free(shape->vertices);
    free(shape->vertices_backup);
    free(shape->center);
    free(shape);
}

//----------------------------------------------------------------------------------------------------------
// Ray
//----------------------------------------------------------------------------------------------------------
ray_t* obj_ray_new(int x, int y, int z) {
    ray_t* new = malloc(sizeof(ray_t));
    new->orig = vec_vec3i_new(0, 0, 0);
    new->end = vec_vec3i_new(x, y, z);
    return new;
}

void obj_ray_send(ray_t* ray, int x, int y, int z) {
    ray->end->x = x;
    ray->end->y = y;
    ray->end->z = z;
}

void obj_ray_free(ray_t* ray) {
    free(ray->orig);
    free(ray->end); 
    free(ray);
}
//----------------------------------------------------------------------------------------------------------
// Plane
//----------------------------------------------------------------------------------------------------------

plane_t* obj_plane_new (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    /*
    * Determine the plane through 3 3D points p0, p1, p2 by determining:
    *     1. the normal vector
    *     2. the offset
    *
    *                                     normal
    *                                       ^
    *                                      /
    *                    +----------------/-----------+
    *                   /     *p0        /           /
    *                  /       <_       /           /
    *                 /          \__   /           /
    *                /              \ /           /
    *               /               *p1          /
    *              /             _/             / 
    *             /           _/               /
    *            /           <                /
    *           /         p2*                /
    *          /                            /
    *         +----------------------------+
    * If p0, p1, p2 are co-planar, then the normal through p1 is
    * perpendicular to both * p1p2 = p2 - p1 and p1p0 = p0 - p1.
    * Thererefore it's determined as the cross product of the two:
    * normal = p1p2 x p1p0 = (p2 - p1) x (p0 - p1)
    *
    *                                    normal
    *                                    ^
    *                                   /     
    *                  +---------------/-----------+
    *                 /               /           /
    *                /               /           /
    *               /              *p1          /
    *              /              /            / 
    *             /           ___/            /
    *            /           /               /
    *           /       * <_/               /
    *          /        x                  /
    *         +---------------------------+
    * If x = (x,y,z) is any point on the plane, then the normal
    * through p1 is perpendicular to p1x = x - p1 therefore their
    * dot product is zero:
    * n.(x - p1) = 0 => 
    * n.x - n.p1 = 0
    * -n.p1 is the offset from the origin 
    */
    plane_t* new = malloc(sizeof(plane_t));
    new->normal = malloc(sizeof(vec3_t));
    vec3i_t p1p2 = vec_vec3i_sub(p2, p1);
    vec3i_t p1p0 = vec_vec3i_sub(p0, p1);
    *new->normal = vec_vec3i_crossprod(&p1p2, &p1p0);
    new->offset = -vec_vec3i_dotprod(new->normal, p1);
    return new;
}


vec3i_t obj_ray_plane_intersection(plane_t* plane, ray_t* ray) {
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

bool obj_ray_hits_rectangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    plane_t plane;
    plane.normal = malloc(sizeof(vec3i_t));
    obj_plane_set(&plane, p0, p1, p2);
    vec3i_t ray_plane_intersection = obj_ray_plane_intersection(&plane, ray);
    free(plane.normal);
    return obj__is_point_in_rect(&ray_plane_intersection, p0, p1, p2, p3);
}

bool obj_ray_hits_triangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // Find the intersection between the ray and the triangle (p0, p1, p2).
    // Return whether the intersection is whithin that triangle
    plane_t plane;
    plane.normal = malloc(sizeof(vec3i_t));
    obj_plane_set(&plane, p0, p1, p2);
    vec3i_t ray_plane_intersection = obj_ray_plane_intersection(&plane, ray);
    free(plane.normal);
    return obj__is_point_in_triangle(&ray_plane_intersection, p0, p1, p2);
}


void obj_plane_set(plane_t* plane, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // reset plane's normal and offset like obj_plane_new function computes them
    vec3i_t p1p2 = vec_vec3i_sub(p2, p1);
    vec3i_t p1p0 = vec_vec3i_sub(p0, p1);
    *plane->normal = vec_vec3i_crossprod(&p1p2, &p1p0);
    plane->offset = -vec_vec3i_dotprod(plane->normal, p1);
}

void obj_plane_free (plane_t* plane) {
    free(plane->normal);
    free(plane); 
}
