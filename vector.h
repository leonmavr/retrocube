#ifndef VECTOR_H
#define VECTOR_H 

#include <stdbool.h>

typedef struct vec3i {
    int x, y, z;
} vec3i_t;

typedef struct vec3f {
    float x, y, z;
} vec3f_t;

typedef vec3f_t vec3_t;

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

#endif /* VECTOR_H */
