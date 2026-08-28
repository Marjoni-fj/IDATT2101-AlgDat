#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*DISCLAIMER:
Denne koden er ikke samarbeidet med andre og alt innhold er kun skrevet av meg,
Fredrik Jonathan Marjoni. Noen av footnotene som er skrevet i koden var kun for
å hjelpe meg selv med å forstå koden og er ikke ment for å være en del av
innleveringen
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

void add(IntArray *arr, int value);
void randomlist(IntArray *arr, int size);

int getHighestReturn(IntArray *arr, int size, int *result);
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

/*
Vi har ei tallrekke som angir forandring i kurs
for en aksje fra dag til dag
Vi ønsker å finne hvordan vi kunne fått best fortjeneste
dvs maksimal positiv differanse mellom kjøpspris
og salgspris. kjøp må selvsalgt skje før salg
*/

int main() {
    srand(time(NULL));
    int size1 = 100000, size2 = 200000, size3 = 1000000;

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
    const int trials = 5;

    // --- EXAMPLE ARRAY FROM THE CURRICULUM ---
    int example[] = {-1, 3, -9, 2, 2, -1, 2, -1, -5};
    IntArray exampleArray;

    exampleArray.data = example;
    exampleArray.length = 9;
    exampleArray.capacity = 9;

    int result[2] = {0, 0};

    int highestReturn = getHighestReturn(&exampleArray, 9, result);

    printf("=== BOKEKSEMPEL ===\n");
    printf("Høyeste fortjeneste: %d\n", highestReturn);
    printf("Kjøp etter dag: %d, Salg etter dag: %d\n\n", result[0], result[1]);

    /*
    Algoritmen finner fortjenesten 5 ved å summere kursendringene fra dag 4 til
    dag 7: (2 + 2 - 1 + 2 = 5). Dette tilsvarer å kjøpe etter kursfallet på dag
    3 og selge etter kursoppgangen på dag 7, slik eksempelet i boka beskriver.
    */
    // --- ARRAY 1 ---
    int return1 = 0;
    double best_time1 = 1e9;

    for (int trial = 0; trial < trials; trial++) {
        reps = 1000;
        start = clock();
        for (int i = 0; i < reps; i++) {
            return1 = getHighestReturn(arr1, arr1->length, result_1);
        }
        end = clock();

        total_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        single_run_time = total_time_used / reps;

        if (single_run_time < best_time1) {
            best_time1 = single_run_time;
        }
    }

    printf("=== ARRAY 1 (%d elementer) ===\n", arr1->length);
    printf("Høyeste fortjeneste: %d\n", return1);
    printf("Kjøp etter dag: %d, Salg etter dag: %d\n", result_1[0],
           result_1[1]);
    printf("Antall test-repetisjoner per forsøk: %d\n", reps);
    printf("Antall forsøk (beste tid brukes): %d\n", trials);
    printf("Beste tid per kjøring: %11.9f sekunder\n\n", best_time1);

    // --- ARRAY 2 ---
    int return2 = 0;
    double best_time2 = 1e9;

    for (int trial = 0; trial < trials; trial++) {
        reps = 1000;
        start = clock();
        for (int i = 0; i < reps; i++) {
            return2 = getHighestReturn(arr2, arr2->length, result_2);
        }
        end = clock();

        total_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        single_run_time = total_time_used / reps;

        if (single_run_time < best_time2) {
            best_time2 = single_run_time;
        }
    }

    printf("=== ARRAY 2 (%d elementer) ===\n", arr2->length);
    printf("Høyeste fortjeneste: %d\n", return2);
    printf("Kjøp etter dag: %d, Salg etter dag: %d\n", result_2[0],
           result_2[1]);
    printf("Antall test-repetisjoner per forsøk: %d\n", reps);
    printf("Antall forsøk (beste tid brukes): %d\n", trials);
    printf("Beste tid per kjøring: %11.9f sekunder\n\n", best_time2);

    // --- ARRAY 3 ---
    int return3 = 0;
    double best_time3 = 1e9;

    for (int trial = 0; trial < trials; trial++) {
        reps = 1000;
        start = clock();
        for (int i = 0; i < reps; i++) {
            return3 = getHighestReturn(arr3, arr3->length, result_3);
        }
        end = clock();

        total_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        single_run_time = total_time_used / reps;

        if (single_run_time < best_time3) {
            best_time3 = single_run_time;
        }
    }

    printf("=== ARRAY 3 (%d elementer) ===\n", arr3->length);
    printf("Høyeste fortjeneste: %d\n", return3);
    printf("Kjøp etter dag: %d, Salg etter dag: %d\n", result_3[0],
           result_3[1]);
    printf("Antall test-repetisjoner per forsøk: %d\n", reps);
    printf("Antall forsøk (beste tid brukes): %d\n", trials);
    printf("Beste tid per kjøring: %11.9f sekunder\n\n", best_time3);

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

/*
startIdx is the first course change included in the return.
Therefore, the purchase happens after the previous day,
while i + 1 represents the day after the last included change.
*/
int getHighestReturn(IntArray *arr, int size, int *result) {
    int highestReturn = 0;
    int currentReturn = 0;
    int startIdx = 0;

    for (int i = 0; i < size; i++) {
        if (currentReturn <= 0) {
            currentReturn = arr->data[i];
            startIdx = i;
            // Reset the current return and update the buy day when the current
            // return is non-positive
        } else {
            currentReturn += arr->data[i];
            // Add today's current chnage to the current return.
        }

        if (currentReturn > highestReturn) {
            highestReturn = currentReturn;

            result[0] = startIdx; // Day to buy (after the previous day)
            result[1] = i + 1;    // Day to sell (after the current day)
        }
    }
    return highestReturn;
}

void randomlist(IntArray *arr, int size) {
    for (int i = 0; i < size; i++) {
        add(arr, rand() % 21 - 10); // Random values between -10 and 10
    }
}

/* 1-2
Algoritmen går gjennom hele arrayet én gang med en for-løkke.
Løkka utføres n ganger (input-størrelsen varierer og er ikke konstant)
og går over alle elementene én gang. Det finnes derfor en konstant c1
slik at 0 <= f(n) <= c1*g(n), og f(n) er i O(n), g(n) = n.

Hver iterasjon gjør et konstant antall operasjoner uansett (if/else-
sjekk, sammenligning, evt. addisjon/tilordning) og hopper ikke over
noen indekser i løkka. Dermed finnes det en konstant c2 slik at
0 <= c2*g(n) <= f(n), og f(n) er i Ω(n), g(n) = n.

Siden algoritmen både er O(n) og Ω(n), er algoritmen Θ(n), altså lineær
tidskompleksitet (O(n) og Ω(n) => Θ(n)), siden man uansett må gå gjennom alle
elementene i lista for å garantere at man finner riktig maks-fortjeneste, og
øvre og nedre grense begge er lineære. f(n) er i Θ(n).

Fra asymptotisk analyse telles ikke konstante faktorer med, så if- og
else-grenene i løkka (enten reset av currentReturn, eller addisjon til
currentReturn, pluss sammenligningen mot highestReturn) påvirker bare
konstantene c1 og c2 - ikke selve vekstordenen.
*/

/* 1-3

Tidsmåling gjøres ved å ta beste (laveste) tid av 5 uavhengige forsøk
per array-størrelse, istedenfor kun ett forsøk. Dette gjøres fordi
enkeltmålinger kan bli forstyrret av ting utenfor selve algoritmen,
som CPU-frekvensskalering eller andre prosesser som kjører på
maskinen samtidig. Slike forstyrrelser kan bare gjøre en kjøring
tregere enn den reelle hastigheten, aldri raskere - derfor er minste
målte tid av flere forsøk et mer stabilt og representativt estimat
på algoritmens faktiske kjøretid.

Tidsmålinger:

n = 100 000:    0.000133931 sekunder
n = 200 000:    0.000266638 sekunder
n = 1 000 000:  0.001348648 sekunder

Når n dobles fra 100 000 til 200 000, øker kjøretiden med omtrent
1.99 ganger.

Når n økes fra 100 000 til 1 000 000, økes n med 10 ganger, mens
kjøretiden øker med omtrent 10.07 ganger.

Dette stemmer svært godt med det vi forventer av en lineær algoritme
(dobling av n -> ~dobling av tid, tidobling av n -> ~tidobling av tid).

Målingene bekrefter dermed den teoretiske analysen fra 1-2 om at
algoritmen har tidskompleksitet Θ(n).
*/