#include "vector.h"
#include <stdbool.h> // true/false
#include <stdlib.h> // malloc
#include <math.h> // round

// square root tolerance distance when comparing vectors 
#define SQRT_TOL 1e-2


//-----------------------------------------------------------------------------------
//  Floating point vectors
//-----------------------------------------------------------------------------------
vec3_t* vec_vec3_new() {
    vec3_t* new = malloc(sizeof(vec3_t));
    return new;
}

void vec_vec3_set(vec3_t* vec, float x, float y, float z) {
    vec->x = x; 
    vec->y = y; 
    vec->z = z; 
}

void vec_vec3_copy(vec3f_t* dest, vec3f_t* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

bool vec_vec3_are_equal(vec3_t* vec1, vec3_t* vec2) {
    return (vec1->x - vec2->x)*(vec1->x - vec2->x) + (vec1->y - vec2->y)*(vec1->y - vec2->y) + (vec1->z - vec2->z)*(vec1->z - vec2->z) < SQRT_TOL*SQRT_TOL;
}

vec3_t vec_vec3_add(vec3_t* src1, vec3_t* src2) {
    return (vec3_t) {src1->x + src2->x, src1->y + src2->y, src1->z + src2->z};
}

vec3_t vec_vec3_sub(vec3_t* src1, vec3_t* src2) {
    return (vec3_t) {src1->x - src2->x, src1->y - src2->y, src1->z - src2->z};
}

vec3_t vec_vec3_mul_scalar (vec3_t* src, float scalar) {
    return (vec3_t) {round(scalar*src->x), round(scalar*src->y), round(scalar*src->z)};
}

float vec_vec3_dotprod(vec3_t* src1, vec3_t* src2) {
    return src1->x*src2->x + src1->y*src2->y + src1->z*src2->z;
}

vec3_t vec_vec3_crossprod(vec3_t* src1, vec3_t* src2) {
    return (vec3_t) { src1->y*src2->z - src1->z*src2->y,
                     -src1->x*src2->z + src1->z*src2->x,
                      src1->x*src2->y - src1->y*src2->x};
}

void vec_vec3_rotate(vec3_t* src, float angle_x_rad, float angle_y_rad, float angle_z_rad, int x0, int y0, int z0) {
    // -(x0, y0, z0)
    // bring to zero so we can do the rotation
    src->x -= x0;
    src->y -= y0;
    src->z -= z0;

    float a = angle_x_rad, b = angle_y_rad, c = angle_z_rad;
    float ca = cos(a), cb = cos(b), cc = cos(c);
    float sa = sin(a), sb = sin(b), sc = sin(c);
    float matrix_rotx[3][3] = {
        {1, 0,  0  },
        {0, ca, -sa},
        {0, sa, ca },
    };
    float matrix_roty[3][3] = {
        {cb,  0, sb},
        {0,   1, 0},
        {-sb, 0, cb},
    };
    float matrix_rotz[3][3] = {
        {cc, -sc, 0},
        {sc, cc,  0},
        {0,  0,   1},
    };
    //
    // x, y, z store the previous coordinates as computed by the previous operation
    int x = src->x;
    int y = src->y;
    int z = src->z;
    src->x = matrix_rotx[0][0]*x + matrix_rotx[0][1]*y + matrix_rotx[0][2]*z;
    src->y = matrix_rotx[1][0]*x + matrix_rotx[1][1]*y + matrix_rotx[1][2]*z;
    src->z = matrix_rotx[2][0]*x + matrix_rotx[2][1]*y + matrix_rotx[2][2]*z;
    // Ry
    x = src->x;
    y = src->y;
    z = src->z;
    src->x = matrix_roty[0][0]*x + matrix_roty[0][1]*y + matrix_roty[0][2]*z;
    src->y = matrix_roty[1][0]*x + matrix_roty[1][1]*y + matrix_roty[1][2]*z;
    src->z = matrix_roty[2][0]*x + matrix_roty[2][1]*y + matrix_roty[2][2]*z;
    // Rz
    x = src->x;
    y = src->y;
    z = src->z;
    src->x = matrix_rotz[0][0]*x + matrix_rotz[0][1]*y + matrix_rotz[0][2]*z;
    src->y = matrix_rotz[1][0]*x + matrix_rotz[1][1]*y + matrix_rotz[1][2]*z;
    src->z = matrix_rotz[2][0]*x + matrix_rotz[2][1]*y + matrix_rotz[2][2]*z;

    // +(x0, y0, z0)
    // reset original offset 
    src->x += x0;
    src->y += y0;
    src->z += z0;
}

//-----------------------------------------------------------------------------------
// Integral vectors
//-----------------------------------------------------------------------------------
vec3i_t* vec_vec3i_new() {
    vec3i_t* new = malloc(sizeof(vec3i_t));
    return new;
}

void vec_vec3i_set(vec3i_t* vec, int x, int y, int z) {
    vec->x = x; 
    vec->y = y; 
    vec->z = z; 
}

void vec_vec3i_copy(vec3i_t* dest, vec3i_t* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

bool vec_vec3i_are_equal(vec3i_t* vec1, vec3i_t* vec2) {
    return (vec1->x == vec2->x) && (vec1->y == vec2->y) && (vec1->z == vec2->z); 
}

vec3i_t vec_vec3i_add(vec3i_t* src1, vec3i_t* src2) {
    return (vec3i_t) {src1->x + src2->x, src1->y + src2->y, src1->z + src2->z};
}

vec3i_t vec_vec3i_sub(vec3i_t* src1, vec3i_t* src2) {
    return (vec3i_t) {src1->x - src2->x, src1->y - src2->y, src1->z - src2->z};
}

vec3i_t vec_vec3i_mul_scalar (vec3i_t* src, float scalar) {
    return (vec3i_t) {round(scalar*src->x), round(scalar*src->y), round(scalar*src->z)};
}

int vec_vec3i_dotprod(vec3i_t* src1, vec3i_t* src2) {
    return src1->x*src2->x + src1->y*src2->y + src1->z*src2->z;
}

vec3i_t vec_vec3i_crossprod(vec3i_t* src1, vec3i_t* src2) {
    return (vec3i_t) { src1->y*src2->z - src1->z*src2->y,
                      -src1->x*src2->z + src1->z*src2->x,
                       src1->x*src2->y - src1->y*src2->x};
}

void vec_vec3i_rotate(vec3i_t* src, float angle_x_rad, float angle_y_rad, float angle_z_rad, int x0, int y0, int z0) {
    // -(x0, y0, z0)
    // bring it to zero so we can do the rotation
    src->x -= x0;
    src->y -= y0;
    src->z -= z0;

    const float a = angle_x_rad, b = angle_y_rad, c = angle_z_rad;
    const float ca = cos(a), cb = cos(b), cc = cos(c);
    const float sa = sin(a), sb = sin(b), sc = sin(c);
    const float matrix_rotx[3][3] = {
        {1, 0,  0  },
        {0, ca, -sa},
        {0, sa, ca },
    };
    const float matrix_roty[3][3] = {
        {cb,  0, sb},
        {0,   1, 0},
        {-sb, 0, cb},
    };
    const float matrix_rotz[3][3] = {
        {cc, -sc, 0},
        {sc, cc,  0},
        {0,  0,   1},
    };
    // x, y, z store the previous coordinates as computed by the previous operation
    int x = src->x;
    int y = src->y;
    int z = src->z;
    src->x = round(matrix_rotx[0][0]*x + matrix_rotx[0][1]*y + matrix_rotx[0][2]*z);
    src->y = round(matrix_rotx[1][0]*x + matrix_rotx[1][1]*y + matrix_rotx[1][2]*z);
    src->z = round(matrix_rotx[2][0]*x + matrix_rotx[2][1]*y + matrix_rotx[2][2]*z);
    // Ry
    x = src->x;
    y = src->y;
    z = src->z;
    src->x = round(matrix_roty[0][0]*x + matrix_roty[0][1]*y + matrix_roty[0][2]*z);
    src->y = round(matrix_roty[1][0]*x + matrix_roty[1][1]*y + matrix_roty[1][2]*z);
    src->z = round(matrix_roty[2][0]*x + matrix_roty[2][1]*y + matrix_roty[2][2]*z);
    // Rz
    x = src->x;
    y = src->y;
    z = src->z;
    src->x = round(matrix_rotz[0][0]*x + matrix_rotz[0][1]*y + matrix_rotz[0][2]*z);
    src->y = round(matrix_rotz[1][0]*x + matrix_rotz[1][1]*y + matrix_rotz[1][2]*z);
    src->z = round(matrix_rotz[2][0]*x + matrix_rotz[2][1]*y + matrix_rotz[2][2]*z);

    // +(x0, y0, z0)
    // reset original offset 
    src->x += x0;
    src->y += y0;
    src->z += z0;
}
