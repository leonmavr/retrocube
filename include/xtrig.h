#ifndef XTRIG_H
#define XTRIG_H

#include <stdio.h>
#include <math.h>

#define LUT_BIN_SIZE 0.00174533 // in radians
#define HALF_PI (M_PI / 2.0)
#define LUT_SIZE 901 // (int)(HALF_PI/ LUT_BIN_SIZE + 1)

/** sine lookup table (LUT) with sampled values from 0 to pi/2 */
extern double sine_lut[LUT_SIZE];

/** Initialize lookup tables - must be called before fsin or fcos */
void ftrig_init_lut();
/** fast sine */
double fsin(double angle);
/** fast cosine */
double fcos(double angle);

#endif // XTRIG_H
