#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Number of vertices
typedef struct EdgeStruct {
    int target;
    int flow;
    int capacity;
    struct EdgeStruct *next;
    struct EdgeStruct *reverse;
} Edge;

#define MAX_VERTICES 1000
Edge *first[MAX_VERTICES]; // Array of pointers to the first edge of each vertex
int V;
typedef struct {
    int data[MAX_VERTICES];
    int front;
    int rear;
} Queue;

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

/*
 * Leser en graf fra en tekstfil og bygger nabolistene i first[].
 *
 * Antar at filen har dette formatet:
 *   linje 1:      <antall noder V> <antall kanter E>
 *   linje 2..E+1: <franode> <tilnode> <kapasitet>
 *
 * Antakelser:
 *   - Nodene er nummerert 0 til V-1.
 *   - V er høyst MAX_VERTICES.
 *   - Filen er riktig formatert. Returverdiene fra fscanf sjekkes ikke.
 *
 * Hver linje i filen gir to kanter: den ekte (med kapasiteten fra filen)
 * og en motsatt kant med kapasitet 0, koblet sammen med reverse.
 *
 * Setter de globale variablene V og first[]. Kalleren må kalle freeGraph()
 * før neste readGraph, ellers lekkes minne.
 *
 * Avslutter programmet hvis filen ikke kan åpnes.
 */
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

void freeGraph() {
    for (int i = 0; i < V; i++) {
        Edge *current = first[i];
        while (current != NULL) {
            Edge *temp = current;
            current = current->next;
            free(temp);
        }
    }
}

int bfs(int startnode, int endnode, Edge *pred[]) {
    int visited[MAX_VERTICES] = {0};
    Queue queue;
    queue.front = 0;
    queue.rear = 0;
    for (int i = 0; i < V; i++) {
        pred[i] = NULL;
    }
    visited[startnode] = 1;
    queue.data[queue.rear++] = startnode;

    while (queue.front < queue.rear) {
        int currentNode = queue.data[queue.front++];
        if (currentNode == endnode) {
            return 1;
        }
        for (Edge *e = first[currentNode]; e != NULL; e = e->next) {
            if (!visited[e->target] && e->capacity - e->flow > 0) {
                pred[e->target] = e;
                visited[e->target] = 1;
                queue.data[queue.rear++] = e->target;
            }
        }
    }

    return 0;
}

void printPath(Edge *pred[], int source, int sink, int bottleneck) {
    int path[MAX_VERTICES];
    int len = 0;

    int node = sink;
    path[len++] = node;
    while (node != source) {
        node = pred[node]->reverse->target;
        path[len++] = node;
    }

    printf("%6d  ", bottleneck);
    for (int i = len - 1; i >= 0; i--) {
        printf("%d ", path[i]);
    }
    printf("\n");
}

int EdmundKarpAlgorithm(int source, int sink) {
    int maxflow = 0;
    Edge *pred[MAX_VERTICES];

    while (bfs(source, sink, pred)) {

        int bottleneck = INT_MAX;
        int currentNode = sink;

        while (currentNode != source) {
            Edge *e = pred[currentNode];
            int residualCapacity = e->capacity - e->flow;
            if (residualCapacity < bottleneck) {
                bottleneck = residualCapacity;
            }
            currentNode = e->reverse->target;
        }

        printPath(pred, source, sink, bottleneck);
        currentNode = sink;

        while (currentNode != source) {
            Edge *e = pred[currentNode];
            e->flow += bottleneck;
            e->reverse->flow -= bottleneck;
            currentNode = e->reverse->target;
        }
        maxflow += bottleneck;
    }
    return maxflow;
}

void runGraph(const char *filename, int source, int sink) {
    readGraph(filename);
    printf("Maksimum flyt fra %d til %d med Edmond-Karp\n", source, sink);
    printf("Økning : Flytøkende vei\n");
    int maxFlow = EdmundKarpAlgorithm(source, sink);
    printf("Maksimal flyt for %s ble %d\n\n", filename, maxFlow);
    freeGraph();
}

int main(void) {
    runGraph("flytgraf1.txt", 0, 7);
    runGraph("flytgraf2.txt", 0, 1);
    runGraph("flytgraf3.txt", 0, 1);
    runGraph("flytgraf4.txt", 0, 7);
    runGraph("flytgraf5.txt", 0, 7);

    return 0;
}