#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*DISCLAIMER:
Denne koden er ikke samarbeidet med andre og alt innhold er kun skrevet av meg,
Fredrik Jonathan Marjoni Footnotene som er skrevet i koden var kun for å hjelpe
meg selv med å forstå koden og er ikke ment for å være en del av innleveringen
*/

/*
Structure to represent a dynamic array of integers
This is kind of the equivalent of a class in Java where you define the
attributes of the object
*/
typedef struct {
    int *data;
    int capacity;
    int length;
} IntArray;

/* this is kind of the equivalent of a constructor in Java where you asign the
 * attributes of the object */
IntArray *createIntArray(int initialCapacity) {
    IntArray *arr = malloc(sizeof(IntArray));
    arr->data = malloc(initialCapacity *
                       sizeof(int)); // specifically an arry containing
                                     // integers, hence the "sizeof(int)"
    arr->capacity = initialCapacity;
    arr->length = 0;

    return arr;
}

void add(IntArray *arr, int value) {
    if (arr->length >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(int));
    }
    arr->data[arr->length++] = value;
} /* Literally: hvis vi har nådd kapasiteten, dobbel kapasitet, reallocate i
     memory, og legg til verdi */

void randomlist(IntArray *arr, int size);
int getHighestReturn(IntArray *arr, int size, int *result);

/*
Vi har ei tallrekke som angir forandring i kurs
for en aksje fra dag til dag
Vi ønsker å finne hvordan vi kunne fått best fortjeneste
dvs maksimal positiv differanse mellom kjøpspris
og salgspris. kjøp må selvsalgt skje før salg
*/

int main() {
    srand(time(NULL));
    int size1 = 10000, size2 = 20000, size3 = 100000;

    IntArray *arr1 = createIntArray(size1);
    randomlist(arr1, size1);
    IntArray *arr2 = createIntArray(size2);
    randomlist(arr2, size2);
    IntArray *arr3 = createIntArray(size3);
    randomlist(arr3, size3);

    int result_1[2] = {0, 0};
    int result_2[2] = {0, 0};
    int result_3[2] = {0, 0};

    clock_t start, end;
    double total_time_used;
    double single_run_time;
    int reps;

    // --- ARRAY 1 ---
    reps = 1000;
    int return1 = 0;

    start = clock();
    for (int i = 0; i < reps; i++) {
        return1 = getHighestReturn(arr1, arr1->length, result_1);
    }
    end = clock();

    total_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    single_run_time = total_time_used / reps;

    printf("=== ARRAY 1 (%d elementer) ===\n", arr1->length);
    printf("Høyeste fortjeneste: %d\n", return1);
    printf("Kjøp dag: %d, Salg dag: %d\n", result_1[0], result_1[1]);
    printf("Antall test-repetisjoner: %d\n", reps);
    printf("Tid brukt per kjøring: %11.9f sekunder\n\n", single_run_time);

    // --- ARRAY 2 ---
    reps = 1000;
    int return2 = 0;

    start = clock();
    for (int i = 0; i < reps; i++) {
        int return2 = getHighestReturn(arr2, arr2->length, result_2);
    }
    end = clock();

    total_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    single_run_time = total_time_used / reps;

    printf("=== ARRAY 2 (%d elementer) ===\n", arr2->length);
    printf("Høyeste fortjeneste: %d\n", return2);
    printf("Kjøp dag: %d, Salg dag: %d\n", result_2[0], result_2[1]);
    printf("Antall test-repetisjoner: %d\n", reps);
    printf("Tid brukt per kjøring: %11.9f sekunder\n\n", single_run_time);

    // --- ARRAY 3 ---
    reps = 1000;
    int return3 = 0;

    start = clock();
    for (int i = 0; i < reps; i++) {
        return3 = getHighestReturn(arr3, arr3->length, result_3);
    }
    end = clock();
    total_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    single_run_time = total_time_used / reps;

    printf("=== ARRAY 3 (%d elementer) ===\n", arr3->length);
    printf("Høyeste fortjeneste: %d\n", return3);
    printf("Kjøp dag: %d, Salg dag: %d\n", result_3[0], result_3[1]);
    printf("Antall test-repetisjoner: %d\n", reps);
    printf("Tid brukt per kjøring: %11.9f sekunder\n\n", single_run_time);

    free(arr1->data);
    free(arr1);
    free(arr2->data);
    free(arr2);
    free(arr3->data);
    free(arr3);

    return 0;
}
/*
The function takes in
IntArray *arr: A pointer to an IntArray structure that contains the array of
integers representing daily stock price changes. int size: The size of the array
(number of elements). int *result: A pointer to an integer array of size 2,
where the function will store the indices of the best buy and sell days.
*/
int getHighestReturn(IntArray *arr, int size, int *result) {
    int highestReturn = 0;
    int buyDay = 0;
    int sellDay = 0;
    int currentReturn = 0;
    int currentIdx = 0;

    for (int i = 0; i < size; i++) {
        if (currentReturn <= 0) {
            currentReturn = arr->data[i];
            currentIdx = i;
            // Reset the current return and update the buy day when the current
            // return is non-positive
        } else {
            currentReturn += arr->data[i];
            // in the case of a positive current return, we add the current
            // day's change to it
        }

        if (currentReturn > highestReturn) {
            highestReturn = currentReturn;
            buyDay = currentIdx;
            sellDay = i;

            result[0] = buyDay;
            result[1] = sellDay;
        }
    }
    return highestReturn;
}

void randomlist(IntArray *arr, int size) {
    for (int i = 0; i < size; i++) {
        add(arr, rand() % 20 - 10); // Random values between -10 and 9
    }
}

/*
1-2
Kompleksiteten for tidsforbruket til algoritmen kan bli eksakt estimert som T(n)
= 9n + 6 Dette kan man finne ved å analysere hele koden og finne den verste
tilfelle kjøretiden:

Deklarasjon av variabler på starten av algoritmen har tidskompleksitet på O(1)
så på starten så har vi O(1) for alle disse 5 deklarasjonene Forløkke looper
gjennom hele arrayet, så den har O(n) kompleksitet

Løkken har i verste tilfelle måtte kjøre 9 operasjoner per runde noe kan skje
dersom currentReturn er mindre eller lik 0 og at currentReturn er større enn
highestReturn. Dette kan for eksempel skje dersom vi har en rekke med negative
tall som gjør at currentReturn blir mindre eller lik 0, og deretter får vi et
positivt tall som gjør at currentReturn blir større enn highestReturn.

Til slutt returnerer algoritmen den høyeste avkastningen samt lagrer resultatene
i result-arrayet.

Derfor kan algoritmen estimeres til å ha en tidskompleksitet beskrevet som T(n)
= 9n + 6

Bruker vi asymptotisk analyse finner vi ut av at T(n) kan forkortes til O(n)
lineær tid ettersom n vokser og 9-konstanten har ikke så mye å si


For å finne Θ(n) så kan vi bruke definisjonen til Θ(n) og finne en n_0, c_1 og
c_2 som passer til ulikheten c_1 * g(n) ≤ T(n) ≤ c_2 * g(n) c_1 * n ≤ 9n + 6 ≤
c_2 * n c_1 ≤ 9 + 6/n ≤ c_2

Her kan vi velge for eksempel n_0 = 10 og c_1 = 9 og c_2 = 11
noe som også betyr at alle n større en n_0 vil også fungere */
