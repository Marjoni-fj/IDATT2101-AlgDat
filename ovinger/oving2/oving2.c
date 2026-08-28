#include <stdio.h>
#include <stdlib.h>
#include <time.h>

float method1(int n, float x) {
    if (n == 0 && x != 0) {
        return 1;
    }
    if (n == 1) {
        return x;
    } else {
        return x * method1(n - 1, x);
    }
}

int main() { printf("%f\n", method1(3, 2)); }