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
    visited[startnode] = 1;               // Mark the start node as visited
    queue.data[queue.rear++] = startnode; // Enqueue the start node

    while (queue.front < queue.rear) {
        int currentNode = queue.data[queue.front++];
        if (currentNode == endnode) {
            return 1;
        }
        for (Edge *e = first[currentNode]; e != NULL; e = e->next) {
            if (!visited[e->target] && e->capacity - e->flow > 0) {
                pred[e->target] = e;
                visited[e->target] = 1;
                queue.data[queue.rear++] = e->target; // Enqueue the target node
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
    /* Step 1: Initialize the total flow to zero */
    int maxflow = 0;
    Edge *pred[MAX_VERTICES];

    /* Step 2: Find an augmenting path using BFS */
    while (bfs(source, sink, pred)) {

        /* Step 3: Find the bottleneck capacity of the path */
        int bottleneck = INT_MAX;
        int currentNode = sink;

        /* Trace the path backwards from sink to source */
        while (currentNode != source) {
            Edge *e = pred[currentNode];
            int residualCapacity = e->capacity - e->flow;
            if (residualCapacity < bottleneck) {
                bottleneck = residualCapacity;
            }
            currentNode =
                e->reverse->target; // Move to the previous node in the path
        }

        printPath(pred, source, sink, bottleneck);
        /* Step 4: Augment the flow along the path */
        currentNode = sink;

        /* Trace the same path backwards again */
        while (currentNode != source) {
            Edge *e = pred[currentNode];
            e->flow += bottleneck;
            e->reverse->flow -= bottleneck;
            currentNode = e->reverse->target;
        }
        /* Add the new flow to the total maximum flow */
        maxflow += bottleneck;
    }
    return maxflow;
}

int main(void) {
    readGraph("flytgraf1.txt");
    printf("Maksimal flyt for flytgraf1.txt ble %d\n\n",
           EdmundKarpAlgorithm(0, 7));
    freeGraph();

    readGraph("flytgraf2.txt");
    printf("Maksimal flyt for flytgraf2.txt ble %d\n\n",
           EdmundKarpAlgorithm(0, 1));
    freeGraph();

    readGraph("flytgraf3.txt");
    printf("Maksimal flyt for flytgraf3.txt ble %d\n\n",
           EdmundKarpAlgorithm(0, 1));
    freeGraph();

    readGraph("flytgraf4.txt");
    printf("Maksimal flyt for flytgraf4.txt ble %d\n\n",
           EdmundKarpAlgorithm(0, 7));
    freeGraph();

    readGraph("flytgraf5.txt");
    printf("Maksimal flyt for flytgraf5.txt ble %d\n\n",
           EdmundKarpAlgorithm(0, 7));
    freeGraph();

    return 0;
}