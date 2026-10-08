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

int bfs(int startnode, int endnode, int parent[]) {
    int visited[MAX_VERTICES] = {0};
    visited[startnode] = 1; // Mark the start node as visited
    Queue queue;
    queue.front = 0;
    queue.rear = 0;

    queue.data[queue.rear++] = startnode;

    while (queue.front < queue.rear) {
        int currentNode = queue.data[queue.front++];
        if (currentNode == endnode) {
            return 1;
        }
        for (Edge *e = first[currentNode]; e != NULL; e = e->next) {
            if (!visited[e->target] && e->capacity - e->flow > 0) {
                parent[e->target] = currentNode;
                visited[e->target] = 1;
                queue.data[queue.rear++] = e->target;
            }
        }
    }

    return 0; // Placeholder return value
}

int EdmundKarpAlgorithm(int source, int sink) {
    /* Step 1: Initialize the total flow to zero */
    int maxflow = 0;
    int parent[MAX_VERTICES];

    /* Step 2: Find an augmenting path using BFS */
    while (bfs(source, sink, parent)) {

        /* Step 3: Find the bottleneck capacity of the path */
        int bottleneck = 1000000;
        int currentNode = sink;

        /* Trace the path backwards from sink to source */
        while (currentNode != source) {
            int previousNode = parent[currentNode];

            /* Find the edge from previousNode to currentNode */
            for (Edge *e = first[previousNode]; e != NULL; e = e->next) {
                if (e->target == currentNode) {

                    /* Keep the smallest residual capacity on the path */
                    bottleneck = (e->capacity - e->flow < bottleneck)
                                     ? (e->capacity - e->flow)
                                     : bottleneck;
                }
            }

            currentNode = previousNode;
        }

        /* Step 4: Augment the flow along the path */
        currentNode = sink;

        /* Trace the same path backwards again */
        while (currentNode != source) {
            int previousNode = parent[currentNode];

            /* Find the edge used by the augmenting path */
            for (Edge *e = first[previousNode]; e != NULL; e = e->next) {
                if (e->target == currentNode) {
                    /* Send the bottleneck amount of flow through the edge */
                    e->flow += bottleneck;
                    /* Update the corresponding reverse edge */
                    e->reverse->flow -= bottleneck;
                }
            }
            currentNode = previousNode;
        }
        /* Add the new flow to the total maximum flow */
        maxflow += bottleneck;
    }
    return maxflow;
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