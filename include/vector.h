#ifndef VECTOR_H
#define VECTOR_H 

#include <stdbool.h>

typedef struct vec3i {
    int x, y, z;
} vec3i_t;

typedef struct vec3f {
    float x, y, z;
} vec3f_t;

// alias for floating vector type
typedef vec3f_t vec3_t;

// basic operations between floating vectors
vec3_t*  vec_vec3_new           (float x, float y, float z);
void     vec_vec3_copy          (vec3f_t* dest, vec3f_t* src);
bool     vec_vec3_are_equal     (vec3_t* vec, float x, float y, float z);
void     vec_vec3_add           (vec3_t* dest, vec3_t* src1, vec3_t* src2);
void     vec_vec3_sub           (vec3_t* dest, vec3_t* src1, vec3_t* src2);
void     vec_vec3_mul_scalar    (vec3_t* dest, vec3_t* src, float scalar);
float    vec_vec3_dotprod       (vec3_t* src1, vec3_t* src2);
void     vec_vec3_crossprod     (vec3_t* dest, vec3_t* src1, vec3_t* src2);
/**
 * @brief Rotates a vector about a point 
 *
 * @param src[in/out] Pointer to vector to rotate, we write to that
 * @param angle_x_rad Angle to rotate about x axis in radians
 * @param angle_y_rad Angle to rotate about y ayis in radians
 * @param angle_z_rad Angle to rotate about z azis in radians
 * @param x0 x-coordinate of point to rotate about
 * @param y0 y-coordinate of point to rotate about
 * @param z0 z-coordinate of point to rotate about
 */
void     vec_vec3_rotate        (vec3_t* src, float angle_x_rad, float angle_y_rad, float angle_z_rad,
int x0, int y0, int z0);
// basic operations between integral vectors
vec3i_t* vec_vec3i_new          (int x, int y, int z);
void     vec_vec3i_copy         (vec3i_t* dest, vec3i_t* src);
bool     vec_vec3i_are_equal    (vec3i_t* vec, int x, int y, int z);
void     vec_vec3i_add          (vec3i_t* dest, vec3i_t* src1, vec3i_t* src2);
void     vec_vec3i_sub          (vec3i_t* dest, vec3i_t* src1, vec3i_t* src2);
void     vec_vec3i_mul_scalar   (vec3i_t* dest, vec3i_t* src, float scalar);
int      vec_vec3i_dotprod      (vec3i_t* src1, vec3i_t* src2);
void     vec_vec3i_crossprod    (vec3i_t* dest, vec3i_t* src1, vec3i_t* src2);
/**
 * @brief Rotates a vector about a point 
 *
 * @param src[in/out] Pointer to vector to rotate, we write to that
 * @param angle_x_rad Angle to rotate about x axis in radians
 * @param angle_y_rad Angle to rotate about y ayis in radians
 * @param angle_z_rad Angle to rotate about z azis in radians
 * @param x0 x-coordinate of point to rotate about
 * @param y0 y-coordinate of point to rotate about
 * @param z0 z-coordinate of point to rotate about
 */
void     vec_vec3i_rotate       (vec3i_t* src, float angle_x_rad, float angle_y_rad, float angle_z_rad,
int x0, int y0, int z0);

#endif /* VECTOR_H */
