#ifndef UTILS_H
#define UTILS_H 

#include <stdbool.h>

// TODO:
// #define INLINE inline __attribute__((always_inline))

#define UT_SQRT_TWO      1.414213
#define UT_HALF_SQRT_TWO 0.7071065 
// the golden ratio
#define UT_PHI           1.6180

// the min below is generic and avoids double evaluation by redefining `a`, `b`
#define UT_MIN(a, b) (          \
{                            \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
}                            \
)

// the max below is generic and avoids double evaluation by redefining `a`, `b`
#define UT_MAX(a, b) (       \
{                            \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
}                            \
)

// float equality
inline bool ut_float_equal(float a, float b) {
    return (-1e-4 < a - b) && (a - b < 1e-4);
}


/**
 * @brief Checks whether a null-terminated array of characters represents
 *        a positive decimal number, e.g. 1.8999 or 1,002
 *
 * @param string A null-terminated array of chars
 *
 * @return true if the given string is numerical
 */
bool ut_is_decimal(char* string);

#endif /* UTILS_H */
