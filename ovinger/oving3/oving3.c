#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int median3(int arr[], int left, int right) {
    int mid = left + (right - left) / 2;
    if (arr[left] > arr[mid])
        swap(&arr[left], &arr[mid]);
    if (arr[mid] > arr[right]) {
        swap(&arr[mid], &arr[right]);
        if (arr[left] > arr[mid])
            swap(&arr[left], &arr[mid]);
    }
    return mid;
}

int partition_single(int arr[], int low, int high) {
    int m = median3(arr, low, high);
    int pivot = arr[m];
    swap(&arr[m], &arr[high - 1]);

    int i = low, j = high - 1;
    for (;;) {
        while (arr[++i] < pivot)
            ;
        while (arr[--j] > pivot)
            ;
        if (i >= j)
            break;
        swap(&arr[i], &arr[j]);
    }
    swap(&arr[i], &arr[high - 1]);
    return i;
}

int partition_dual(int *arr, int low, int high, int *lp) {
    int third = (high - low) / 3;
    swap(&arr[low], &arr[low + third]);
    swap(&arr[high], &arr[high - third]);

    if (arr[low] > arr[high])
        swap(&arr[low], &arr[high]);
    // p is the left pivot, and q is the right pivot.
    int j = low + 1;
    int g = high - 1, k = low + 1, p = arr[low], q = arr[high];
    while (k <= g) {

        // if elements are less than thepartition left pivot
        if (arr[k] < p) {
            swap(&arr[k], &arr[j]);
            j++;
        }

        // if elements are greater than or equal
        // to the right pivot
        else if (arr[k] >= q) {
            while (arr[g] > q && k < g)
                g--;
            swap(&arr[k], &arr[g]);
            g--;
            if (arr[k] < p) {
                swap(&arr[k], &arr[j]);
                j++;
            }
        }
        k++;
    }
    j--;
    g++;

    swap(&arr[low], &arr[j]);
    swap(&arr[high], &arr[g]);

    // returning the indices of the pivots.
    *lp = j;

    return g;
}

void quick_sort_single(int arr[], int low, int high) {
    if (high - low > 2) {
        int pi = partition_single(arr, low, high);
        quick_sort_single(arr, low, pi - 1);
        quick_sort_single(arr, pi + 1, high);
    } else {
        median3(arr, low, high);
    }
}

void quick_sort_dual(int arr[], int low, int high) {
    if (low < high) {
        int lp, rp;
        rp = partition_dual(arr, low, high, &lp);
        quick_sort_dual(arr, low, lp - 1); // Before the left pivot
        if (arr[lp] != arr[rp]) {
            quick_sort_dual(arr, lp + 1, rp - 1); // Between the pivots
        }
        quick_sort_dual(arr, rp + 1, high); // After the right pivot
    }
}

// ===== Utility functions for testing and timing =====
long checksum(int arr[], int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int is_sorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0;
        }
    }
    return 1;
}

// ===== Filling functions for different test cases =====
void fill_random(int arr[], int n) {
    for (int i = 0; i < n; ++i)
        arr[i] = rand();
}
void fill_duplicates(int arr[], int n) {
    for (int i = 0; i < n; ++i)
        arr[i] = (i % 2 == 0) ? 1 : 2;
}
void fill_sorted(int arr[], int n) {
    for (int i = 0; i < n; ++i)
        arr[i] = i;
}
void fill_reverse(int arr[], int n) {
    for (int i = 0; i < n; ++i)
        arr[i] = n - i;
}

// ===== Function pointer types for sorting and filling functions =====
typedef void (*SortFunction)(int[], int, int);
typedef void (*GenFunction)(int[], int);

// ==== Timing function for sorting algorithms =====
void run_and_time(const char *label, SortFunction sort_fn, int arr[], int n) {
    long before = checksum(arr, n);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    sort_fn(arr, 0, n - 1);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    long after = checksum(arr, n);

    printf("%-40s: %8.3f s   sjekksum:%s   rekkefolge:%s\n", label, elapsed,
           (before == after) ? "OK" : "FEIL",
           is_sorted(arr, n) ? "OK" : "FEIL");
}

int main(void) {
    srand(time(NULL));
    int n = 50000000;
    int *original = malloc(n * sizeof(int));
    int *work = malloc(n * sizeof(int));

    if (!original || !work) {
        printf("Feil ved allokering av minne\n");
        return 1;
    }

    GenFunction generators[4] = {fill_random, fill_duplicates, fill_sorted,
                                 fill_reverse};
    const char *gen_names[4] = {"Tilfeldige tall", "Mange duplikater",
                                "Sortert fra for", "Baklengs sortert"};

    SortFunction sorters[2] = {quick_sort_single, quick_sort_dual};
    const char *sorter_names[2] = {"single pivot", "dual pivot"};

    for (int g = 0; g < 4; g++) {
        generators[g](original, n);

        for (int s = 0; s < 2; s++) {
            for (int i = 0; i < n; i++)
                work[i] = original[i];

            char label[64];
            snprintf(label, sizeof(label), "%s - %s", gen_names[g],
                     sorter_names[s]);

            run_and_time(label, sorters[s], work, n);
        }
        printf("\n");
    }

    free(original);
    free(work);
    return 0;
}
