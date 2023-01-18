#ifndef OBJECTS_H
#define OBJECTS_H 

#include "vector.h"
#include <stdbool.h>

typedef struct cube {
    vec3i_t** vertices;
    vec3i_t** vertices_backup;
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
 * n_x*x + n_y*y + n_z*z + d = 0 (1)
 * or n.X + d = 0                (2)
 * , where n = (a, b, c) is the normal
 * , X = (x, y, y)
 * and d is the offset from the origin
 */
typedef struct plane {
    // d from eq. (2)
    int offset;
    // n from eq. (2)
    vec3i_t* normal;
} plane_t;

//-------------------------------------------------------------------------------------------------------------
// Cube
//-------------------------------------------------------------------------------------------------------------
cube_t*     obj_cube_new               (int cx, int cy, int cz, int size);
void        obj_cube_rotate            (cube_t* cube, float angle_x_rad, float angle_y_rad, float angle_z_rad);
void        obj_cube_free              (cube_t* cube);
//-------------------------------------------------------------------------------------------------------------
// Ray
//-------------------------------------------------------------------------------------------------------------
// the pixel in the screen where the ray points to
ray_t*      obj_ray_new                (int x, int y, int z);
void        obj_ray_send               (ray_t* ray, int x, int y, int z); 
void        obj_ray_set_color          (ray_t* ray, char color);
void        obj_ray_free               (ray_t* ray);
//-------------------------------------------------------------------------------------------------------------
// Plane
//-------------------------------------------------------------------------------------------------------------
plane_t*    obj_plane_new              (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2);
vec3i_t     obj_ray_plane_intersection (plane_t* plane, ray_t* ray);
bool        obj_ray_hits_rectangle     (ray_t* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3);
inline int  obj_plane_z_at_xy          (plane_t* plane, int x, int y);
void        obj_plane_set              (plane_t* plane, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2);
void        obj_plane_free             (plane_t* plane);

#endif /* OBJECTS_H */
