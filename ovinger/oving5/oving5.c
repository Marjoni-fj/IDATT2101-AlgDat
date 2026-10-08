#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct EdgeStruct {
    int target;
    int flow;
    int capacity;
    struct EdgeStruct *next;
    struct EdgeStruct *reverse;
} Edge;

#define MAX_VERTICES 1000
Edge *first[MAX_VERTICES];
int V;

void addEdge(int from, int to, int capacity) {
    Edge *forward = (Edge *)malloc(sizeof(Edge));
    Edge *backward = (Edge *)malloc(sizeof(Edge));

    forward->target = to;
    forward->flow = 0;
    forward->capacity = capacity;
    forward->reverse = backward;
    forward->next = first[from];
    first[from] = forward;

    backward->target = from;
    backward->flow = 0;
    backward->capacity = 0;
    backward->reverse = forward;
    backward->next = first[to];
    first[to] = backward;
}

void readGraph(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    int E;
    fscanf(file, "%d %d", &V, &E);

    for (int i = 0; i < V; i++) {
        first[i] = NULL;
    }

    for (int i = 0; i < E; i++) {
        int from, to, capacity;
        fscanf(file, "%d %d %d", &from, &to, &capacity);
        addEdge(from, to, capacity);
    }
    fclose(file);
}

int main(void) {
    readGraph("flytgraf1.txt");
    for (int u = 0; u < V; u++) {
        printf("node %d:", u);
        for (Edge *e = first[u]; e != NULL; e = e->next) {
            printf(" -> %d (kap %d)", e->target, e->capacity);
        }
        printf("\n");
    }
    return 0;
}