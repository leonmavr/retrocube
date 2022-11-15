#include "types.h"
#include <math.h>
#include <stdlib.h>

#define SQRT_TWO 1.414213
#define HALF_SQRT_TWO 0.7071065 


Vec3* vec3New(int x, int y, int z) {
    Vec3* new = malloc(sizeof(Vec3));
    new->x = x;
    new->y = y;
    new->z = z;
    return new;
}

void vec3Copy(Vec3* dest, Vec3* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

Cube* cubeNew(int cx, int cy, int cz, int side) {
        Cube* new = malloc(sizeof(Cube));
        new->center = vec3New(cx, cy, cz);
        new->vertices = (Vec3**) malloc(sizeof(Vec3*) * 8);
        int diag = round(HALF_SQRT_TWO * side); 
        new->vertices[0] = vec3New(-diag, -diag, -diag);
        new->vertices[1] = vec3New(+diag, -diag, -diag);
        new->vertices[2] = vec3New(+diag, +diag, -diag);
        new->vertices[3] = vec3New(-diag, +diag, -diag);
        new->vertices[4] = vec3New(-diag, -diag, +diag);
        new->vertices[5] = vec3New(+diag, -diag, +diag);
        new->vertices[6] = vec3New(+diag, +diag, +diag);
        new->vertices[7] = vec3New(-diag, +diag, +diag);
        return new;
}

