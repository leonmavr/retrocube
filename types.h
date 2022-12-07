#ifndef OBJECTS_H
#define OBJECTS_H 

#include "vector.h"
#include <stdbool.h>

typedef struct Cube {
    vec3i_t** vertices;
    vec3i_t* center;
} Cube;

typedef struct Ray {
    // origin is the centre of perspective in pinhole camera model
    // for now it is always set to (0, 0, 0)
    vec3i_t* orig;
    vec3i_t* end;
    // pixel color encoded as a character
    char color;
} Ray;

/* 
 * plane in 3D
 * assuming its equation is:
 * ax + by + cz + d = 0
 * or n.X + d = 0
 * , where n = (a, b, c) is the normal
 * and d is the offset from the origin
 */
typedef struct Plane {
    int offset;
    vec3i_t* normal;
} Plane;

Cube* cubeNew(int cx, int cy, int cz, int size);
// the pixel in the screen where the ray points to
Ray* rayNew(int x, int y, int z);
void raySend(Ray* ray, int x, int y, int z); 
void raySetColor(Ray* ray, char color);
vec3i_t rayPlaneIntersection(Ray* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3);
// planar oparations
Plane*   plane_new          (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2);
vec3i_t  plane_intersectRay (Plane* plane, Ray* ray);
bool     plane_rayHitsSurface(Ray* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3);


#endif /* OBJECTS_H */
