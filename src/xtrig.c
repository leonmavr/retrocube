#include "xtrig.h"

double sine_lut[LUT_SIZE];

void ftrig_init_lut() {
    for (int i = 0; i < LUT_SIZE; i++) 
        sine_lut[i] = sin(i * LUT_BIN_SIZE);
}

/** Get the index of an angle in the LUT */
static int get_lookup_index(double rad) {
    return (int) round(rad / LUT_BIN_SIZE);
}

double fsin(double rad) {
    // use the period and symmetry to compute based off [0, pi/2]
    rad = fmod(rad, 2*M_PI);
    if (rad < 0) rad += 2*M_PI;
    if (rad <= HALF_PI) {
        return sine_lut[get_lookup_index(rad)];
    } else if (rad <= M_PI) {
        return sine_lut[get_lookup_index(M_PI - rad)];
    } else if (rad <= 3*HALF_PI) {
        return -sine_lut[get_lookup_index(rad - M_PI)];
    } else {
        return -sine_lut[get_lookup_index(2*M_PI - rad)];
    }
}

double fcos(double rad) {
    return fsin(HALF_PI - rad);
}

