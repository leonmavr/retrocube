#ifndef OBJECTS_H
#define OBJECTS_H 

typedef struct Vec3 {
    int x, y, z;
} Vec3;

typedef struct Cube {
    Vec3** vertices;
    Vec3* center;
} Cube;

Vec3* vec3New(int x, int y, int z);
void vec3Copy(Vec3* dest, Vec3* src);
Cube* cubeNew(int cx, int cy, int cz, int size);


#endif /* OBJECTS_H */
