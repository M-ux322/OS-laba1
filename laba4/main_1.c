#include <stdio.h>
#include "contracts.h"

float sin_integral(float, float, float);
float e_func(int);

int main() {
    int cmd;
    while (scanf("%d", &cmd) == 1) {
        if (cmd == 1) {
            float a,b,e;
            scanf("%f %f %f", &a, &b, &e);
            printf("%f\n", sin_integral(a,b,e));
        } else if (cmd == 2) {
            int x;
            scanf("%d", &x);
            printf("%f\n", e_func(x));
        }
    }
}