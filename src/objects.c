#include "vector.h"
#include "objects.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> // strncpy
#include <stddef.h> // size_t 

#define SQRT_TWO 1.414213
#define HALF_SQRT_TWO 0.7071065 
// the golden ratio
#define PHI 1.6180

// the min below is generic and avoids double evaluation by redefining `a`, `b`
#define MIN(a, b) (          \
{                            \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
}                            \
)

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
 * TODO:
 * schematic of why this works
 */
    // don't care about z components
    vec3i_t ma = {a->x - m->x, a->y - m->y, 0};
    vec3i_t mb = {b->x - m->x, b->y - m->y, 0};
    vec3i_t mc = {c->x - m->x, c->y - m->y, 0};
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
    vec3i_t ab = (vec3i_t) {b->x - a->x, b->y - a->y, b->z - a->z};
    vec3i_t ad = (vec3i_t) {d->x - a->x, d->y - a->y, d->z - a->z};
    vec3i_t am = (vec3i_t) {m->x - a->x, m->y - a->y, m->z - a->z};
    return (0 < vec_vec3i_dotprod(&am, &ab)) && (vec_vec3i_dotprod(&am, &ab) < vec_vec3i_dotprod(&ab, &ab)) &&
           (0 < vec_vec3i_dotprod(&am, &ad)) && (vec_vec3i_dotprod(&am, &ad) < vec_vec3i_dotprod(&ad, &ad));
}


//----------------------------------------------------------------------------------------------------------
// Cube
//----------------------------------------------------------------------------------------------------------
shape_t* obj_cube_new(int cx, int cy, int cz, int width, int height, int type) {
/*
 *          p3                  p2 
 *           +-------------------+
 *           | \                 | \
 *           |    \              |    \                  ^y
 *           |      \  p7        |       \               |
 *           |         +-------------------+ p6          |
 *           |         |         .         |             |
 *           |         |*(cx,xy,cz)        |             o-------> x
 *           |         |         .         |              \
 *           |         |         .         |               \
 *           |         |         .         |                v z
 *        p0 +---------|.........+ p1      |
 *            \        |           .       |
 *              \      |             .     |
 *                 \   |                .  |
 *                    \+-------------------+
 *                     p4                   p5
 */
    shape_t* new = malloc(sizeof(shape_t));
    //// common attributes
    new->type = type;
    new->center = vec_vec3i_new(cx, cy, cz);
    // colors for each of the maximum possible 8 faces - change the string below to modify them
    strncpy(new->colors, "~.=@%|O+?Tn", 8);
    //// attributes that depend on number of type
    if (type == OBJ_CUBE) {
        new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * 8);
        new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * 8);
        int diag = round(HALF_SQRT_TWO * MIN(width, height)); 
        new->vertices[0] = vec_vec3i_new(cx-diag, cy-diag, cz-diag);
        new->vertices[1] = vec_vec3i_new(cx+diag, cy-diag, cz-diag);
        new->vertices[2] = vec_vec3i_new(cx+diag, cy+diag, cz-diag);
        new->vertices[3] = vec_vec3i_new(cx-diag, cy+diag, cz-diag);
        new->vertices[4] = vec_vec3i_new(cx-diag, cy-diag, cz+diag);
        new->vertices[5] = vec_vec3i_new(cx+diag, cy-diag, cz+diag);
        new->vertices[6] = vec_vec3i_new(cx+diag, cy+diag, cz+diag);
        new->vertices[7] = vec_vec3i_new(cx-diag, cy+diag, cz+diag);
        // back them up so no floating point error is accumulated affter the rotations
        for (int i = 0; i < 8; ++i) {
            new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
            vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
        }
    } else if (type == OBJ_RHOMBUS) {
        // TODO: rhombus schematic
        new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * 6);
        new->vertices_backup = (vec3i_t**) malloc(sizeof(vec3i_t*) * 6);
        const int hw = round(width/2.0);
        new->vertices[0] = vec_vec3i_new(0,        0,                          width/2);
        new->vertices[1] = vec_vec3i_new(width/2,  0,                          0);
        new->vertices[2] = vec_vec3i_new(0,        0,                          -width/2);
        new->vertices[3] = vec_vec3i_new(-width/2, 0,                          0);
        new->vertices[4] = vec_vec3i_new(0,        -round(PHI/(1+PHI)*height), 0);
        new->vertices[5] = vec_vec3i_new(0,        round(1.0/(1+PHI)*height),  0);
        // shift them to the origin 
        for (int i = 0; i < 6; ++i)
            vec_vec3i_add(new->vertices[i], new->vertices[i], new->center);
        // back them up so no floating point error is accumulated affter the rotations
        for (int i = 0; i < 6; ++i) {
            new->vertices_backup[i] = vec_vec3i_new(0, 0, 0);
            vec_vec3i_copy(new->vertices_backup[i], new->vertices[i]);
        }
    }
    return new;
}

void obj_cube_rotate (shape_t* cube, float angle_x_rad, float angle_y_rad, float angle_z_rad) {
    const size_t n_corners = (cube->type == OBJ_CUBE) ? 8 : 6;
    for (size_t i = 0; i < n_corners; ++i) {
        // first, reset each vertex so no floating point error is accumulated
        vec_vec3i_copy(cube->vertices[i], cube->vertices_backup[i]);

        // point to rotate about
        int x0 = cube->center->x, y0 = cube->center->y, z0 = cube->center->z;
        // rotate around x axis, then y, then z
        // We rotate as follows (* denotes matrix product, C the cube's origin):
        // v = v - C, v = Rz*Ry*Rx*v, v = v + C
        vec_vec3i_rotate(cube->vertices[i], angle_x_rad, angle_y_rad, angle_z_rad, x0, y0, z0);
    }
}


void obj_cube_free(shape_t* cube) {
    const size_t n_corners = (cube->type == OBJ_CUBE) ? 8 : 6;
    // free the data of the vertices first
    for (int i = 0; i < n_corners; ++i) {
        free(cube->vertices[i]);
        free(cube->vertices_backup[i]);
    }
    free(cube->vertices);
    free(cube->vertices_backup);
    free(cube->center);
    free(cube);
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
    vec3i_t p1p2;
    vec3i_t p1p0;
    vec_vec3i_sub(&p1p2, p2, p1);
    vec_vec3i_sub(&p1p0, p0, p1);
    vec_vec3i_crossprod(new->normal, &p1p2, &p1p0);
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
    int nornmal_dot_rayend = vec_vec3i_dotprod(plane->normal, ray->end);
    float t0 = ((float)plane->offset/nornmal_dot_rayend);
    // only interested in intersections along the positive direction
    t0 = (t0 < 0.0) ? -t0 : t0 ;
    //if (t0 < 0)
    //    t0 = -t0;
    vec3i_t ray_at_intersection;
    ray_at_intersection.x = round(t0*ray->end->x);
    ray_at_intersection.y = round(t0*ray->end->y);
    ray_at_intersection.z = round(t0*ray->end->z);
    return ray_at_intersection;
}

bool obj_ray_hits_rectangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    // TODO: dont malloc
    plane_t* plane = obj_plane_new(p0, p1, p2);
    vec3i_t ray_plane_intersection = obj_ray_plane_intersection(plane, ray);
    bool ret = false;
#if 1
    if (obj__is_point_in_rect(&ray_plane_intersection, p0, p1, p2, p3))
        ret = true;;
#else
    if (obj__is_point_in_triangle(&ray_plane_intersection, p0, p1, p2))
        ret = true;;
#endif
    obj_plane_free(plane);
    return ret;
}

bool obj_ray_hits_triangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    // TODO: don't calloc
    plane_t* plane = obj_plane_new(p0, p1, p2);
    vec3i_t ray_plane_intersection = obj_ray_plane_intersection(plane, ray);
    bool ret = false;
#if 0
    if (obj__is_point_in_rect(&ray_plane_intersection, p0, p1, p2, p3))
        ret = true;;
#else
    if (obj__is_point_in_triangle(&ray_plane_intersection, p0, p1, p2))
        ret = true;;
#endif
    obj_plane_free(plane);
    return ret;
}



void obj_plane_set(plane_t* plane, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
    // reset plane's normal and offset like obj_plane_new function computes them
    vec3i_t p1p2;
    vec3i_t p1p0;
    vec_vec3i_sub(&p1p2, p2, p1);
    vec_vec3i_sub(&p1p0, p0, p1);
    vec_vec3i_crossprod(plane->normal, &p1p2, &p1p0);
    plane->offset = -vec_vec3i_dotprod(plane->normal, p1);
}

void obj_plane_free (plane_t* plane) {
    free(plane->normal);
    free(plane); 
}
