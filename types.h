#ifndef OBJECTS_H
#define OBJECTS_H 

#include <stdbool.h>

typedef struct Vec3i {
    int x, y, z;
} Vec3i;

typedef Vec3i vec3i_t;

typedef struct Vec3f {
    float x, y, z;
} Vec3f;

typedef struct vec3_t {
    float x, y, z;
} vec3_t;

typedef struct Cube {
    Vec3i** vertices;
    Vec3i* center;
} Cube;

typedef struct Ray {
    // origin is the centre of perspective in pinhole camera model
    // for now it is always set to (0, 0, 0)
    Vec3i* orig;
    Vec3i* end;
    // must be a unit vector
    //Vec3f* dir;
    // pixel color rendered as a character
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

Vec3i* vec3iNew(int x, int y, int z);
Vec3f* vec3fNew(int x, int y, int z);
void vec3fMakeUnit(Vec3f* vec);
void vec3iCopy(Vec3i* dest, Vec3i* src);
void vec3fCopy(Vec3f* dest, Vec3f* src);
Cube* cubeNew(int cx, int cy, int cz, int size);
// the pixel in the screen where the ray points to
Ray* rayNew(int x, int y, int z);
void raySend(Ray* ray, int x, int y, int z); 
void raySetColor(Ray* ray, char color);
Vec3i rayPlaneIntersection(Ray* ray, Vec3i* p0, Vec3i* p1, Vec3i* p2, Vec3i* p3);
// basic operations between floating vectors
vec3_t*  vec3_new           (float x, float y, float z);
bool     vec3_areEqual      (vec3_t* vec, float x, float y, float z);
void     vec3_add           (vec3_t* dest, vec3_t* src1, vec3_t* src2);
void     vec3_sub           (vec3_t* dest, vec3_t* src1, vec3_t* src2);
void     vec3_mul           (vec3_t* dest, vec3_t* src, float scalar);
float    vec3_dotprod       (vec3_t* src1, vec3_t* src2);
void     vec3_crossprod     (vec3_t* dest, vec3_t* src1, vec3_t* src2);
// basic operations between integral vectors
vec3i_t* vec3i_new          (int x, int y, int z);
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
