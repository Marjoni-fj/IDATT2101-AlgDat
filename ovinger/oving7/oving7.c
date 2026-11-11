#include <stdio.h>
#include <stdlib.h>

typedef struct Edge {
    int destination;
    int travel_time;
    struct Edge *next;
} Edge;

typedef struct Node {
    double latitude;
    double longitude;
    Edge *first_edge;
} Node;

typedef struct {
    int node_id;
    int priority;
} HeapElement;

typedef struct {
    HeapElement *elements;
    int size;
    int capacity;
} MinHeap;

MinHeap createMinHeap(int capacity) {
    MinHeap heap;

    heap.elements = calloc(capacity, sizeof(HeapElement));
    heap.size = 0;
    heap.capacity = capacity;

    return heap;
}

void freeMinHeap(MinHeap *heap) {
    free(heap->elements);
    heap->elements = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

void swapElements(HeapElement *first, HeapElement *second) {
    HeapElement temporary = *first;
    *first = *second;
    *second = temporary;
}

void shiftUp(MinHeap *heap, int index) {
    while (index > 0) {
        int parent_index = (index - 1) / (2);
        if (heap->elements[parent_index].priority <=
            heap->elements[index].priority) {
            break;
        }
        swapElements(&heap->elements[parent_index], &heap->elements[index]);
        index = parent_index;
    }
}
void shiftDown(MinHeap *heap, int index) {
    while (1) {
        int left_child = 2 * index + 1;
        int right_child = 2 * index + 2;
        int smallest = index;

        if (left_child < heap->size && heap->elements[left_child].priority <
                                           heap->elements[smallest].priority) {
            smallest = left_child;
        }
        if (right_child < heap->size && heap->elements[right_child].priority <
                                            heap->elements[smallest].priority) {
            smallest = right_child;
        }
        if (smallest == index) {
            break;
        }
        swapElements(&heap->elements[index], &heap->elements[smallest]);
        index = smallest;
    }
}

void pushToHeap(MinHeap *heap, int node_id, int priority) {
    if (heap->size >= heap->capacity) {
        fprintf(stderr, "Heap is full\n");
        return;
    }
    int index = heap->size;
    heap->elements[index].node_id = node_id;
    heap->elements[index].priority = priority;
    heap->size++;

    shiftUp(heap, index);
}

HeapElement popOffHeap(MinHeap *heap) {
    if (heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        return (HeapElement){-1, -1};
    }
    HeapElement popped = heap->elements[0];
    heap->elements[0] = heap->elements[heap->size - 1];
    heap->size--;
    if (heap->size > 0) {
        shiftDown(heap, 0);
    }
    return popped;
}

int main(void) {
    FILE *nodes_file = fopen("noder.txt", "r");

    if (nodes_file == NULL) {
        perror("Kunne ikke åpne noder.txt");
        return EXIT_FAILURE;
    }

    int number_of_nodes;

    if (fscanf(nodes_file, "%d", &number_of_nodes) != 1) {
        fprintf(stderr, "Kunne ikke lese antall noder\n");
        fclose(nodes_file);
        return EXIT_FAILURE;
    }
    Node *nodes = calloc(number_of_nodes, sizeof(Node));
    if (nodes == NULL) {
        perror("Couldn't allocate memory for nodes");
        fclose(nodes_file);
        return EXIT_FAILURE;
    }

    printf("Antall noder: %d\n", number_of_nodes);

    for (int i = 0; i < number_of_nodes; i++) {
        int node_id;
        double latitude;
        double longitude;

        if (fscanf(nodes_file, "%d %lf %lf", &node_id, &latitude, &longitude) !=
            3) {
            fprintf(stderr, "Kunne ikke lese node %d\n", i);
            free(nodes);
            fclose(nodes_file);
            return EXIT_FAILURE;
        }

        nodes[node_id].latitude = latitude;
        nodes[node_id].longitude = longitude;
        nodes[node_id].first_edge = NULL;
    }

    fclose(nodes_file);

    FILE *edges_file = fopen("kanter.txt", "r");

    if (edges_file == NULL) {
        perror("Kunne ikke åpne kanter.txt");
        fclose(edges_file);
        free(nodes);
        return EXIT_FAILURE;
    }

    int number_of_edges;

    if (fscanf(edges_file, "%d", &number_of_edges) != 1) {
        fprintf(stderr, "Kunne ikke lese antall kanter\n");
        fclose(edges_file);
        return EXIT_FAILURE;
    }

    Edge *edges = calloc(number_of_edges, sizeof(Edge));
    if (edges == NULL) {
        perror("Couldn't allocate memory for edges");
        fclose(edges_file);
        free(nodes);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < number_of_edges; i++) {
        int from;
        int to;
        int travel_time;
        int length;
        int speed_limit;

        if (fscanf(edges_file, "%d %d %d %d %d", &from, &to, &travel_time,
                   &length, &speed_limit) != 5) {
            fprintf(stderr, "Kunne ikke lese kant %d\n", i);
            free(edges);
            free(nodes);
            fclose(edges_file);
            return EXIT_FAILURE;
        }
        edges[i].destination = to;
        edges[i].travel_time = travel_time;
        edges[i].next = nodes[from].first_edge;
        nodes[from].first_edge = &edges[i];
    }
    printf("Kanter fra node 1:\n");

    Edge *edge = nodes[1].first_edge;

    while (edge != NULL) {
        printf("1 -> %d, tid: %d\n", edge->destination, edge->travel_time);

        edge = edge->next;
    }

    fclose(edges_file);
    free(edges);
    free(nodes);
    return EXIT_SUCCESS;
}