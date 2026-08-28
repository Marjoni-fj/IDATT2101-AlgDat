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

float method2(int n, float x) {
    if (n == 0 && x != 0) {
        return 1;
    }
    if (n % 2 == 0) {
        return method2(n / 2, x * x);
    } else {
        return x * method2((n - 1) / 2, x * x);
    }
}

int main() {
    printf("%f\n", method1(3, 2));
    printf("%f\n", method2(3, 2));
}