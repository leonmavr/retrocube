#ifndef OBJECTS_H
#define OBJECTS_H 

typedef struct Vec3i {
    int x, y, z;
} Vec3i;

typedef struct Vec3f {
    float x, y, z;
} Vec3f;

typedef struct Cube {
    Vec3i** vertices;
    Vec3i* center;
} Cube;

typedef struct Ray {
    // centre of perspective in pinhole camera model
    Vec3i* orig;
    // must be a unit vector
    Vec3f* dir;
    // pixel color rendered as a character
    char color;
} Ray;

Vec3i* vec3iNew(int x, int y, int z);
Vec3f* vec3fNew(int x, int y, int z);
void vec3fMakeUnit(Vec3f* vec);
void vec3iCopy(Vec3i* dest, Vec3i* src);
void vec3fCopy(Vec3f* dest, Vec3f* src);
Cube* cubeNew(int cx, int cy, int cz, int size);
// the pixel in the screen where the ray points to
Ray* rayNew(int x, int y, int z);
void raySetColor(Ray* ray, char color);
Vec3i rayPlaneIntersection(Ray* ray, Vec3i* p0, Vec3i* p1, Vec3i* p2);


#endif /* OBJECTS_H */
