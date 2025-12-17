#include <math.h>
#include "contracts.h"

float sin_integral(float a, float b, float e) {
    float sum = 0.0f;
    for (float x = a; x < b; x += e)
        sum += (sinf(x) + sinf(x + e)) * 0.5f * e;
    return sum;
}

float e_func(int x) {
    float sum = 1.0f, fact = 1.0f;
    for (int i = 1; i <= x; i++) {
        fact *= i;
        sum += 1.0f / fact;
    }
    return sum;
}
