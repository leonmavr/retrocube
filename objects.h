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
 * ax + by + cz + d = 0 (1)
 * or n.X + d = 0       (2)
 * , where n = (a, b, c) is the normal
 * and d is the offset from the origin
 */
typedef struct plane {
    // d from eq. (2)
    int offset;
    // n from eq. (2)
    vec3i_t* normal;
    // coefficients of plane eq. (1)
    float a, b, c, d;
} plane_t;

//----------------------------------------------------------------------------------------------------------
// Cube
//----------------------------------------------------------------------------------------------------------
cube_t*     obj__cube_new               (int cx, int cy, int cz, int size);
void obj__cube_rotate (cube_t* cube, float angle_x_deg, float angle_y_deg, float angle_z_deg);
//----------------------------------------------------------------------------------------------------------
// Ray
//----------------------------------------------------------------------------------------------------------
// the pixel in the screen where the ray points to
ray_t*      obj__ray_new                (int x, int y, int z);
void        obj__ray_send               (ray_t* ray, int x, int y, int z); 
void        obj__ray_set_color          (ray_t* ray, char color);
//----------------------------------------------------------------------------------------------------------
// Plane
//----------------------------------------------------------------------------------------------------------
plane_t*    obj__plane_new              (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2);
vec3i_t     obj__ray_plane_intersection (plane_t* plane, ray_t* ray);
bool        obj__ray_hits_rectangle     (ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3);

#endif /* OBJECTS_H */
