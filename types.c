
#include "types.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define SQRT_TWO 1.414213
#define HALF_SQRT_TWO 0.7071065 
// square root tolerance distance when comparing vectors 
#define SQRT_TOL 1e-2


void vec3i_copy(vec3i_t* dest, vec3i_t* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

void vec3_copy(vec3f_t* dest, vec3f_t* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}


Cube* cubeNew(int cx, int cy, int cz, int side) {
        Cube* new = malloc(sizeof(Cube));
        new->center = vec3i_new(cx, cy, cz);
        new->vertices = (vec3i_t**) malloc(sizeof(vec3i_t*) * 8);
        int diag = round(HALF_SQRT_TWO * side); 
        new->vertices[0] = vec3i_new(-diag, -diag, -diag);
        new->vertices[1] = vec3i_new(+diag, -diag, -diag);
        new->vertices[2] = vec3i_new(+diag, +diag, -diag);
        new->vertices[3] = vec3i_new(-diag, +diag, -diag);
        new->vertices[4] = vec3i_new(-diag, -diag, +diag);
        new->vertices[5] = vec3i_new(+diag, -diag, +diag);
        new->vertices[6] = vec3i_new(+diag, +diag, +diag);
        new->vertices[7] = vec3i_new(-diag, +diag, +diag);
        return new;
}

Ray* rayNew(int x, int y, int z) {
    Ray* new = malloc(sizeof(Ray));
    new->orig = vec3i_new(0, 0, 0);
    new->end = vec3i_new(x, y, z);
    return new;
}


void raySetColor(Ray* ray, char color) {
    ray->color = color;
}




static inline bool pointInRec(vec3i_t* m, vec3i_t* a, vec3i_t* b, vec3i_t* c, vec3i_t* d) {
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
    return (0 <= vec3i_dotprod(&am, &ab)) && (vec3i_dotprod(&am, &ab) <= vec3i_dotprod(&ab, &ab)) &&
           (0 <= vec3i_dotprod(&am, &ad)) && (vec3i_dotprod(&am, &ad) <= vec3i_dotprod(&ad, &ad));
}

vec3i_t rayPlaneIntersection(Ray* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3) {
#if 0
    // find the equation of the plane defined by p0, p1, p2
    vec3i_t p1p0 = (vec3i_t) {p1->x - p0->x, p1->y - p0->y, p1->z - p0->z};
    vec3i_t p2p0 = (vec3i_t) {p2->x - p0->x, p2->y - p0->y, p2->z - p0->z};
    // normal vector to the plane 
    Vec3f n = vec3fCrossProd(&p1p0, &p2p0);
    // see https://9to5science.com/line-and-plane-intersection-in-3d for the intersection derivation
    // ray's origin
    Vec3f a = (Vec3f) {ray->orig->x, ray->orig->y, ray->orig->z};
    // ray's direction
    Vec3f b = (Vec3f) {ray->dir->x, ray->dir->y, ray->dir->z};
    Vec3f ba = (Vec3f) {b.x - a.x, b.y - a.y, b.z - a.z};
    Vec3f fp0 = (Vec3f) {p0->x, p0->y, p0->z};
    // intersection of parametric ray: r(t) = a + t(b-a)
    float t = (vec3fDotprod(&n, &fp0) - vec3fDotprod(&n, &a)) /
        (vec3fDotprod(&n, &b) - vec3fDotprod(&n, &a)); 
    // return the point r(t0) on the ray for the interesection t0: r(t0) = a + t0*(b-a)
    vec3i_t ret = (vec3i_t) {round(a.x + t*(b.x - a.x)), 
                         round(a.y + t*(b.y - a.y)), 
                         round(a.z + t*(b.z - a.z))};
    // TODO: if ret in p0p1p2p3, return ret, else return (0, 0, 0)
    if (!pointInRec(&ret, p0, p1, p2, p3) && t > 0.0)
        return ret
#endif
    
    vec3i_t ret = (vec3i_t) {0, 0, 0}; 
    return ret;
}

void raySend(Ray* ray, int x, int y, int z) {
    ray->end->x = x;
    ray->end->y = y;
    ray->end->z = z;
}

vec3_t* vec3_new(float x, float y, float z) {
    vec3_t* new = malloc(sizeof(vec3_t));
    new->x = x;
    new->y = y;
    new->z = z;
    return new;
}

bool vec3_areEqual(vec3_t* vec, float x, float y, float z) {
    return (vec->x - x)*(vec->x - x) + (vec->y - y)*(vec->y - y) + (vec->z - z)*(vec->z - z) < SQRT_TOL*SQRT_TOL;
}


void vec3_add(vec3_t* dest, vec3_t* src1, vec3_t* src2) {
    dest->x = src1->x + src2->x;
    dest->y = src1->y + src2->y;
    dest->z = src1->z + src2->z;
}


void vec3_sub(vec3_t* dest, vec3_t* src1, vec3_t* src2) {
    dest->x = src1->x - src2->x;
    dest->y = src1->y - src2->y;
    dest->z = src1->z - src2->z;
}

float vec3_dotprod(vec3_t* src1, vec3_t* src2) {
    return src1->x*src2->x + src1->y*src2->y + src1->z*src2->z;
}

void vec3_crossprod(vec3_t* dest, vec3_t* src1, vec3_t* src2) {
    vec3_t* a = src1;
    vec3_t* b = src2;
    dest->x = a->y*b->z - a->z*b->y;
    dest->y = -a->x*b->z + a->z*b->x;
    dest->z = a->x*b->y - a->y*b->x;
}

vec3i_t* vec3i_new(int x, int y, int z) {
    vec3i_t* new = malloc(sizeof(vec3i_t));
    new->x = x;
    new->y = y;
    new->z = z;
    return new;
}

bool vec3i_areEqual(vec3i_t* vec, int x, int y, int z) {
    return (vec->x == x) && (vec->y == y) && (vec->z == z); 
}

void vec3i_add(vec3i_t* dest, vec3i_t* src1, vec3i_t* src2) {
    dest->x = src1->x + src2->x;
    dest->y = src1->y + src2->y;
    dest->z = src1->z + src2->z;
}

void vec3i_sub(vec3i_t* dest, vec3i_t* src1, vec3i_t* src2) {
    dest->x = src1->x - src2->x;
    dest->y = src1->y - src2->y;
    dest->z = src1->z - src2->z;
}

int vec3i_dotprod(vec3i_t* src1, vec3i_t* src2) {
    return src1->x*src2->x + src1->y*src2->y + src1->z*src2->z;
}

void vec3i_crossprod(vec3i_t* dest, vec3i_t* src1, vec3i_t* src2) {
    vec3i_t* a = src1;
    vec3i_t* b = src2;
    dest->x =  a->y*b->z - a->z*b->y;
    dest->y = -a->x*b->z + a->z*b->x;
    dest->z =  a->x*b->y - a->y*b->x;
}

void vec3_mul (vec3_t* dest, vec3_t* src, float scalar) {
    dest->x = scalar*src->x;
    dest->y = scalar*src->y;
    dest->z = scalar*src->z;
}

void vec3i_mul (vec3i_t* dest, vec3i_t* src, float scalar) {
    dest->x = scalar*src->x;
    dest->y = scalar*src->y;
    dest->z = scalar*src->z;
}

Plane* plane_new (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2) {
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
    Plane* new = malloc(sizeof(Plane));
    new->normal = malloc(sizeof(vec3_t));
    vec3i_t p1p2;
    vec3i_t p1p0;
    vec3i_sub(&p1p2, p2, p1);
    vec3i_sub(&p1p0, p0, p1);
    vec3i_crossprod(new->normal, &p1p2, &p1p0);
    new->offset = -vec3i_dotprod(new->normal, p1);
    return new;
}


vec3i_t plane_intersectRay(Plane* plane, Ray* ray) {
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
    int normalDotEnd = vec3i_dotprod(plane->normal, ray->end);
    float t0 = ((float)plane->offset/normalDotEnd);
    // only interested in intersections along the positive direction
    (t0 < 0.0) ? -t0 : t0 ;
    vec3i_t rayAtIntersection;
    rayAtIntersection.x = round(t0*ray->end->x);
    rayAtIntersection.y = round(t0*ray->end->y);
    rayAtIntersection.z = round(t0*ray->end->z);
    return rayAtIntersection;
}


bool plane_rayHitsSurface(Ray* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3) {
    // find the intersection between the ray and the plane segment
    // defined by p0, p1, p2, p3 and if the intersection is whithin
    // that segment, return true
    Plane* plane = plane_new(p0, p1, p2);
    vec3i_t rayPlaneInters = plane_intersectRay(plane, ray);
    if (pointInRec(&rayPlaneInters, p0, p1, p2, p3))
        return true;
    return false;
}
