#include "vector.h"
#include "objects.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define SQRT_TWO 1.414213
#define HALF_SQRT_TWO 0.7071065 


static inline bool is_point_in_rec(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c, vec3i_t* d) {
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
    return (0 < vec__vec3i_dotprod(&am, &ab)) && (vec__vec3i_dotprod(&am, &ab) < vec__vec3i_dotprod(&ab, &ab)) &&
           (0 < vec__vec3i_dotprod(&am, &ad)) && (vec__vec3i_dotprod(&am, &ad) < vec__vec3i_dotprod(&ad, &ad));
}


//----------------------------------------------------------------------------------------------------------
// Cube
//----------------------------------------------------------------------------------------------------------
cube_t* obj__cube_new(int cx, int cy, int cz, int side) {
        cube_t* new = malloc(sizeof(cube_t));
        new->center = vec__vec3i_new(cx, cy, cz);
        new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * 8);
        int diag = round(HALF_SQRT_TWO * side); 
        new->vertices[0] = vec__vec3i_new(-diag, -diag, -diag);
        new->vertices[1] = vec__vec3i_new(+diag, -diag, -diag);
        new->vertices[2] = vec__vec3i_new(+diag, +diag, -diag);
        new->vertices[3] = vec__vec3i_new(-diag, +diag, -diag);
        new->vertices[4] = vec__vec3i_new(-diag, -diag, +diag);
        new->vertices[5] = vec__vec3i_new(+diag, -diag, +diag);
        new->vertices[6] = vec__vec3i_new(+diag, +diag, +diag);
        new->vertices[7] = vec__vec3i_new(-diag, +diag, +diag);
        return new;
}

void obj__cube_rotate (cube_t* cube, float angle_x_deg, float angle_y_deg, float angle_z_deg) {
    // to be consistent with wiki article: https://en.wikipedia.org/wiki/Rotation_matrix
    float a = angle_z_deg, b = angle_y_deg, c = angle_x_deg;
    float ca = cos(a), cb = cos(b), cc = cos(c);
    float sa = sin(a), sb = sin(b), sc = sin(c);
    float rotMatrix[3][3] = {
        {cb*cc, sa*sb*cc - ca*sc, ca*sb*cc + sa*sc},
        {cb*sc, sa*sb*sc + ca*cc, ca*sb*sc - sa*cc},
        {-sb  , sa*cb           , ca*cb}
    };
    for (int i = 0; i < 8; ++i) {
        cube->vertices[i]->x = round(rotMatrix[0][0]*cube->vertices[i]->x + rotMatrix[0][1]*cube->vertices[i]->y + rotMatrix[0][2]*cube->vertices[i]->z);
#if 1
        cube->vertices[i]->y = round(rotMatrix[1][0]*cube->vertices[i]->x + rotMatrix[1][1]*cube->vertices[i]->y + rotMatrix[1][2]*cube->vertices[i]->z);
        cube->vertices[i]->z = round(rotMatrix[2][0]*cube->vertices[i]->x + rotMatrix[2][1]*cube->vertices[i]->y + rotMatrix[2][2]*cube->vertices[i]->z);
        printf("v: %d, %d, %d\n", cube->vertices[i]->x,  cube->vertices[i]->y, cube->vertices[i]->z);
#endif
    }
}

//----------------------------------------------------------------------------------------------------------
// Ray
//----------------------------------------------------------------------------------------------------------
ray_t* obj__ray_new(int x, int y, int z) {
    ray_t* new = malloc(sizeof(ray_t));
    new->orig = vec__vec3i_new(0, 0, 0);
    new->end = vec__vec3i_new(x, y, z);
    return new;
}

void obj__ray_set_color(ray_t* ray, char color) {
    ray->color = color;
}


void obj__ray_send(ray_t* ray, int x, int y, int z) {
    ray->end->x = x;
    ray->end->y = y;
    ray->end->z = z;
}

//----------------------------------------------------------------------------------------------------------
// Plane
//----------------------------------------------------------------------------------------------------------

plane_t* obj__plane_new (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
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
    vec__vec3i_sub(&p1p2, p2, p1);
    vec__vec3i_sub(&p1p0, p0, p1);
    vec__vec3i_crossprod(new->normal, &p1p2, &p1p0);
    new->offset = -vec__vec3i_dotprod(new->normal, p1);
    return new;
}


vec3i_t obj__ray_plane_intersection(plane_t* plane, ray_t* ray) {
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
    int nornmal_dot_rayend = vec__vec3i_dotprod(plane->normal, ray->end);
    float t0 = ((float)plane->offset/nornmal_dot_rayend);
    // only interested in intersections along the positive direction
    (t0 < 0.0) ? -t0 : t0 ;
    vec3i_t ray_at_intersection;
    ray_at_intersection.x = round(t0*ray->end->x);
    ray_at_intersection.y = round(t0*ray->end->y);
    ray_at_intersection.z = round(t0*ray->end->z);
    return ray_at_intersection;
}

bool obj__ray_hits_rectangle(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    plane_t* plane = obj__plane_new(p0, p1, p2);
    vec3i_t ray_plane_intersection = obj__ray_plane_intersection(plane, ray);
    if (is_point_in_rec(&ray_plane_intersection, p0, p1, p2, p3))
        return true;
    return false;
}
