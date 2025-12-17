#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include "contracts.h"

static sin_integral_func *sin_integral;
static e_func_func *e_func;

static float stub_sin(float a, float b, float e) {
    (void)a; (void)b; (void)e;
    return 0.0f;
}

static float stub_e(int x) {
    (void)x;
    return 0.0f;
}

void load_library(const char *path, void **lib) {
    if (*lib) dlclose(*lib);

    *lib = dlopen(path, RTLD_LOCAL | RTLD_NOW);
    if (*lib == NULL) {
        write(2, "warning: failed to load library\n", 33);
        sin_integral = stub_sin;
        e_func = stub_e;
        return;
    }   

    sin_integral = dlsym(*lib, "sin_integral");
    if (!sin_integral) sin_integral = stub_sin;

    e_func = dlsym(*lib, "e_func");
    if (!e_func) e_func = stub_e;
}

int main() {
    void *lib = NULL;
    int toggle = 0;

    load_library("./lib_1.so", &lib);

    int cmd;
    while (scanf("%d", &cmd) == 1) {
        if (cmd == 0) {
            toggle = !toggle;
            load_library(toggle ? "./lib_2.so" : "./lib_1.so", &lib);
        } else if (cmd == 1) {
            float a,b,e;
            scanf("%f %f %f", &a, &b, &e);
            printf("%f\n", sin_integral(a,b,e));
        } else if (cmd == 2) {
            int x;
            scanf("%d", &x);
            printf("%f\n", e_func(x));
        }
    }

    if (lib) dlclose(lib);
}
