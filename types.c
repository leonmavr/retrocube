#include "types.h"
#include <math.h>
#include <stdlib.h>

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
