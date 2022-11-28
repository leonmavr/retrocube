#include "types.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define SQRT_TWO 1.414213
#define HALF_SQRT_TWO 0.7071065 



Vec3i* vec3iNew(int x, int y, int z) {
    Vec3i* new = malloc(sizeof(Vec3i));
    new->x = x;
    new->y = y;
    new->z = z;
    return new;
}

Vec3f* vec3fNew(int x, int y, int z) {
    Vec3f* new = malloc(sizeof(Vec3f));
    new->x = x;
    new->y = y;
    new->z = z;
    return new;
}

void vec3iCopy(Vec3i* dest, Vec3i* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

void vec3fCopy(Vec3f* dest, Vec3f* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

void vec3fMakeUnit(Vec3f* vec) {
    float norm = sqrt(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z);
    vec->x /= norm;
    vec->y /= norm;
    vec->z /= norm;
}

Cube* cubeNew(int cx, int cy, int cz, int side) {
        Cube* new = malloc(sizeof(Cube));
        new->center = vec3iNew(cx, cy, cz);
        new->vertices = (Vec3i**) malloc(sizeof(Vec3i*) * 8);
        int diag = round(HALF_SQRT_TWO * side); 
        new->vertices[0] = vec3iNew(-diag, -diag, -diag);
        new->vertices[1] = vec3iNew(+diag, -diag, -diag);
        new->vertices[2] = vec3iNew(+diag, +diag, -diag);
        new->vertices[3] = vec3iNew(-diag, +diag, -diag);
        new->vertices[4] = vec3iNew(-diag, -diag, +diag);
        new->vertices[5] = vec3iNew(+diag, -diag, +diag);
        new->vertices[6] = vec3iNew(+diag, +diag, +diag);
        new->vertices[7] = vec3iNew(-diag, +diag, +diag);
        return new;
}

Ray* rayNew(int x, int y, int z) {
    Ray* new = malloc(sizeof(Ray));
    new->orig = vec3iNew(0, 0, 0);
    new->dir = vec3fNew(x, y, z);
    vec3fMakeUnit(new->dir);
    return new;
}


void raySetColor(Ray* ray, char color) {
    ray->color = color;
}


static inline Vec3f vec3fCrossProd(Vec3i* v1, Vec3i* v2) {
    Vec3f ret;
    ret.x = v1->y*v2->z - v1->z*v2->y;  
    ret.y = v1->z*v2->x - v1->x*v2->z;
    ret.z = v1->x*v2->y - v1->y*v1->x; 
    return ret;
}

static inline float vec3fDotprod(Vec3f* v1, Vec3f* v2) {
    return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

static inline float vec3iDotprod(Vec3i* v1, Vec3i* v2) {
    return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

static inline bool pointInRec(Vec3i* m, Vec3i* a, Vec3i* b, Vec3i* c, Vec3i* d) {
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
    Vec3i ab = (Vec3i) {b->x - a->x, b->y - a->y, b->z - a->z};
    Vec3i ad = (Vec3i) {d->x - a->x, d->y - a->y, d->z - a->z};
    Vec3i am = (Vec3i) {m->x - a->x, m->y - a->y, m->z - a->z};
    return (0 < vec3iDotprod(&am, &ad)) && (vec3iDotprod(&am, &ab) < vec3iDotprod(&ab, &ab)) &&
           (0 < vec3iDotprod(&am, &ad)) && (vec3iDotprod(&am, &ad) < vec3iDotprod(&ad, &ad));
}

Vec3i rayPlaneIntersection(Ray* ray, Vec3i* p0, Vec3i* p1, Vec3i* p2, Vec3i* p3) {
    // find the equation of the plane defined by p0, p1, p2
    Vec3i p1p0 = (Vec3i) {p1->x - p0->x, p1->y - p0->y, p1->z - p0->z};
    Vec3i p2p0 = (Vec3i) {p2->x - p0->x, p2->y - p0->y, p2->z - p0->z};
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
    Vec3i ret = (Vec3i) {round(a.x + t*(b.x - a.x)), 
                         round(a.y + t*(b.y - a.y)), 
                         round(a.z + t*(b.z - a.z))};
    // TODO: if ret in p0p1p2p3, return ret, else return (0, 0, 0)
    if (!pointInRec(&ret, p0, p1, p2, p3) && t > 0.0)
        ret = (Vec3i) {0, 0, 0}; 
    return ret;
}

void rayPointTo(Ray* ray, int x, int y, int z) {
    ray->dir->x = x;
    ray->dir->y = y;
    ray->dir->z = z;
    vec3fMakeUnit(ray->dir);
}
