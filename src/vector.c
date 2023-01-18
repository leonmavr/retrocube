#include "vector.h"
#include <stdbool.h>
#include <stdlib.h>

// square root tolerance distance when comparing vectors 
#define SQRT_TOL 1e-2


//-----------------------------------------------------------------------------------
// Integral vectors
//-----------------------------------------------------------------------------------
vec3_t* vec_vec3_new(float x, float y, float z) {
    vec3_t* new = malloc(sizeof(vec3_t));
    new->x = x;
    new->y = y;
    new->z = z;
    return new;
}

void vec_vec3_copy(vec3f_t* dest, vec3f_t* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

bool vec_vec3_are_equal(vec3_t* vec, float x, float y, float z) {
    return (vec->x - x)*(vec->x - x) + (vec->y - y)*(vec->y - y) + (vec->z - z)*(vec->z - z) < SQRT_TOL*SQRT_TOL;
}

void vec_vec3_add(vec3_t* dest, vec3_t* src1, vec3_t* src2) {
    dest->x = src1->x + src2->x;
    dest->y = src1->y + src2->y;
    dest->z = src1->z + src2->z;
}

void vec_vec3_sub(vec3_t* dest, vec3_t* src1, vec3_t* src2) {
    dest->x = src1->x - src2->x;
    dest->y = src1->y - src2->y;
    dest->z = src1->z - src2->z;
}

void vec_vec3_mul_scalar (vec3_t* dest, vec3_t* src, float scalar) {
    dest->x = scalar*src->x;
    dest->y = scalar*src->y;
    dest->z = scalar*src->z;
}

float vec_vec3_dotprod(vec3_t* src1, vec3_t* src2) {
    return src1->x*src2->x + src1->y*src2->y + src1->z*src2->z;
}

void vec_vec3_crossprod(vec3_t* dest, vec3_t* src1, vec3_t* src2) {
    vec3_t* a = src1;
    vec3_t* b = src2;
    dest->x = a->y*b->z - a->z*b->y;
    dest->y = -a->x*b->z + a->z*b->x;
    dest->z = a->x*b->y - a->y*b->x;
}


//-----------------------------------------------------------------------------------
// Floating point vectors
//-----------------------------------------------------------------------------------

vec3i_t* vec_vec3i_new(int x, int y, int z) {
    vec3i_t* new = malloc(sizeof(vec3i_t));
    new->x = x;
    new->y = y;
    new->z = z;
    return new;
}

void vec_vec3i_copy(vec3i_t* dest, vec3i_t* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

bool vec_vec3i_are_equal(vec3i_t* vec, int x, int y, int z) {
    return (vec->x == x) && (vec->y == y) && (vec->z == z); 
}

void vec_vec3i_add(vec3i_t* dest, vec3i_t* src1, vec3i_t* src2) {
    dest->x = src1->x + src2->x;
    dest->y = src1->y + src2->y;
    dest->z = src1->z + src2->z;
}

void vec_vec3i_sub(vec3i_t* dest, vec3i_t* src1, vec3i_t* src2) {
    dest->x = src1->x - src2->x;
    dest->y = src1->y - src2->y;
    dest->z = src1->z - src2->z;
}

void vec_vec3i_mul_scalar (vec3i_t* dest, vec3i_t* src, float scalar) {
    dest->x = scalar*src->x;
    dest->y = scalar*src->y;
    dest->z = scalar*src->z;
}

int vec_vec3i_dotprod(vec3i_t* src1, vec3i_t* src2) {
    return src1->x*src2->x + src1->y*src2->y + src1->z*src2->z;
}

void vec_vec3i_crossprod(vec3i_t* dest, vec3i_t* src1, vec3i_t* src2) {
    vec3i_t* a = src1;
    vec3i_t* b = src2;
    dest->x =  a->y*b->z - a->z*b->y;
    dest->y = -a->x*b->z + a->z*b->x;
    dest->z =  a->x*b->y - a->y*b->x;
}
