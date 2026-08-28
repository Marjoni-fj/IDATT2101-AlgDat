#include <math.h>
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
    }
    return x * method2((n - 1) / 2, x * x);
}

float method3(int n, float x) { return (float)pow((double)x, (double)n); }

float method1(int number, float value);
float method2(int number, float value);
float method3(int number, float value);

typedef float (*FunctionPointer)(int, float);

void time_tracking(FunctionPointer methodx, int n, float x, int reps) {
    float result;
    clock_t start, end;

    start = clock();
    for (int i = 0; i < reps; i++) {
        result = methodx(n, x);
    }
    end = clock();

    double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    double time_per_call = total_time / reps;
    printf("n = %d, resultat = %f, tid per kall = %.9f sekunder\n", n, result,
           time_per_call);
}

int main() {
    // --- TEST: sjekk at metodene regner riktig, jf. 5^11 = 48828125 ---
    printf("=== TEST ===\n");
    printf("method1(11, 5.0) = %f (forventet 48828125)\n", method1(11, 5.0f));
    printf("method2(11, 5.0) = %f (forventet 48828125)\n", method2(11, 5.0f));
    printf("method3(11, 5.0) = %f (forventet 48828125)\n\n", method3(11, 5.0f));

    // x settes lavt for å unngå float-overflow ved store n (jf. oppgavetipset)
    const float x = 1.0025f;
    const int reps = 100000;

    // method1 er lineær rekursjon -> stack overflow-fare ved store n, holdes
    // derfor lavere
    int n_values_method1[] = {100, 500, 1000, 2000, 4000};
    int len1 = sizeof(n_values_method1) / sizeof(n_values_method1[0]);

    // method2 og method3 tåler mye høyere n (method2 er logaritmisk i
    // rekursjonsdybde)
    int n_values_method23[] = {100, 1000, 10000, 100000, 1000000};
    int len23 = sizeof(n_values_method23) / sizeof(n_values_method23[0]);

    printf("=== Tidtaking method1 (lineaer rekursjon) ===\n");
    for (int i = 0; i < len1; i++) {
        time_tracking(method1, n_values_method1[i], x, reps);
    }

    printf("\n=== Tidtaking method2 (halvering) ===\n");
    for (int i = 0; i < len23; i++) {
        time_tracking(method2, n_values_method23[i], x, reps);
    }

    printf("\n=== Tidtaking method3 (pow, kun til sammenligning) ===\n");
    for (int i = 0; i < len23; i++) {
        time_tracking(method3, n_values_method23[i], x, reps);
    }

    return 0;
}
