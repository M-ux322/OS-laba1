#include <math.h>
#include "contracts.h"

float sin_integral(float a, float b, float e) {
    float sum = 0.0f;
    for (float x = a; x < b; x += e)
        sum += sinf(x) * e;
    return sum;
}

float e_func(int x) {
    return powf(1.0f + 1.0f / x, x);
}