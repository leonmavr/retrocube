#ifndef OBJECTS_H
#define OBJECTS_H 

#include "vector.h"
#include <stdbool.h>

typedef struct cube {
    vec3i_t** vertices;
    vec3i_t* center;
} cube_t;

typedef struct ray {
    // origin is the centre of perspective in pinhole camera model
    // for now it is always set to (0, 0, 0)
    vec3i_t* orig;
    vec3i_t* end;
    // pixel color encoded as a character
    char color;
} ray_t;

/* 
 * plane in 3D
 * assuming its equation is:
 * ax + by + cz + d = 0
 * or n.X + d = 0
 * , where n = (a, b, c) is the normal
 * and d is the offset from the origin
 */
typedef struct plane {
    int offset;
    vec3i_t* normal;
} plane_t;

cube_t* cubeNew(int cx, int cy, int cz, int size);
// the pixel in the screen where the ray points to
ray_t* rayNew(int x, int y, int z);
void raySend(ray_t* ray, int x, int y, int z); 
void raySetColor(ray_t* ray, char color);
vec3i_t rayplane_tIntersection(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3);
// planar oparations
plane_t*   plane_new          (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2);
vec3i_t  plane_intersectray_t (plane_t* plane, ray_t* ray);
bool     plane_rayHitsSurface(ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3);


#endif /* OBJECTS_H */
