#ifndef OBJECTS_H
#define OBJECTS_H 

#include <stdbool.h>

typedef struct vec3i_t {
    int x, y, z;
} vec3i_t;

typedef struct vec3f_t {
    float x, y, z;
} vec3f_t;

typedef vec3f_t vec3_t;

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
// basic operations between floating vectors
vec3_t*  vec3_new           (float x, float y, float z);
void     vec3_copy          (vec3f_t* dest, vec3f_t* src);
bool     vec3_areEqual      (vec3_t* vec, float x, float y, float z);
void     vec3_add           (vec3_t* dest, vec3_t* src1, vec3_t* src2);
void     vec3_sub           (vec3_t* dest, vec3_t* src1, vec3_t* src2);
void     vec3_mul           (vec3_t* dest, vec3_t* src, float scalar);
float    vec3_dotprod       (vec3_t* src1, vec3_t* src2);
void     vec3_crossprod     (vec3_t* dest, vec3_t* src1, vec3_t* src2);
// basic operations between integral vectors
vec3i_t* vec3i_new          (int x, int y, int z);
void     vec3i_copy         (vec3i_t* dest, vec3i_t* src);
bool     vec3i_areEqual     (vec3i_t* vec, int x, int y, int z);
void     vec3i_add          (vec3i_t* dest, vec3i_t* src1, vec3i_t* src2);
void     vec3i_sub          (vec3i_t* dest, vec3i_t* src1, vec3i_t* src2);
void     vec3i_mul          (vec3i_t* dest, vec3i_t* src, float scalar);
int      vec3i_dotprod      (vec3i_t* src1, vec3i_t* src2);
void     vec3i_crossprod    (vec3i_t* dest, vec3i_t* src1, vec3i_t* src2);
// planar oparations
Plane*   plane_new          (vec3i_t* p0, vec3i_t* p1, vec3i_t* p2);
vec3i_t  plane_intersectRay (Plane* plane, Ray* ray);
bool     plane_rayHitsSurface(Ray* ray, vec3i_t* p0, vec3i_t* p1, vec3i_t* p2, vec3i_t* p3);


#endif /* OBJECTS_H */
