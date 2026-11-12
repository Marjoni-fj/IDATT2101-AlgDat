#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

typedef struct {
    int distance;
    int previous;
    int processed;
} SearchInfo;

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
        int new_capacity = heap->capacity * 2;

        HeapElement *new_elements =
            realloc(heap->elements, new_capacity * sizeof(HeapElement));

        if (new_elements == NULL) {
            fprintf(stderr, "Kunne ikke utvide heapen\n");
            exit(EXIT_FAILURE);
        }

        heap->elements = new_elements;
        heap->capacity = new_capacity;
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

int dijkstra(Node *nodes, SearchInfo *searchInfo, int number_of_nodes,
             int start, int target) {
    for (int i = 0; i < number_of_nodes; i++) {
        searchInfo[i].distance = INT_MAX;
        searchInfo[i].previous = -1;
        searchInfo[i].processed = 0;
    }
    MinHeap heap = createMinHeap(number_of_nodes);
    searchInfo[start].distance = 0;
    pushToHeap(&heap, start, 0);
    int processed_nodes = 0;

    while (heap.size > 0) {
        HeapElement current = popOffHeap(&heap);
        int current_node = current.node_id;
        if (searchInfo[current_node].processed) {
            continue;
        }
        searchInfo[current_node].processed = 1;
        processed_nodes++;

        if (current_node == target) {
            break;
        }
        Edge *edge = nodes[current_node].first_edge;
        while (edge != NULL) {
            int neighbour = edge->destination;
            int new_time =
                searchInfo[current_node].distance + edge->travel_time;
            if (new_time < searchInfo[neighbour].distance) {
                searchInfo[neighbour].distance = new_time;
                searchInfo[neighbour].previous = current_node;
                pushToHeap(&heap, neighbour, new_time);
            }
            edge = edge->next;
        }
    }
    freeMinHeap(&heap);
    return processed_nodes;
}

int *findRoute(SearchInfo *searchInfo, int number_of_nodes, int start,
               int target, int *route_length) {
    int *route = malloc(number_of_nodes * sizeof(int));
    if (route == NULL) {
        return NULL;
    }
    *route_length = 0;
    int current = target;
    while (current != -1) {
        route[*route_length] = current;
        (*route_length)++;
        if (current == start) {
            break;
        }
        current = searchInfo[current].previous;
    }
    return route;
}

double elapsed_seconds(struct timespec start, struct timespec end) {
    double seconds = end.tv_sec - start.tv_sec;
    double nanoseconds = end.tv_nsec - start.tv_nsec;

    return seconds + nanoseconds / 1e9;
}

void print_time(int hundredths) {
    int total_seconds = hundredths / 100;

    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;

    printf("%02d:%02d:%02d\n", hours, minutes, seconds);
}

void run_Dijkstra_search(Node *nodes, SearchInfo *searchInfo,
                         int number_of_nodes, int start, int target) {
    struct timespec start_time;
    struct timespec end_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int processed_nodes =
        dijkstra(nodes, searchInfo, number_of_nodes, start, target);

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double search_time = elapsed_seconds(start_time, end_time);

    int route_length;
    int *route =
        findRoute(searchInfo, number_of_nodes, start, target, &route_length);

    printf("\nRute fra %d til %d\n", start, target);
    printf("Kjøretid: ");
    print_time(searchInfo[target].distance);
    printf("Søketid: %.6f sekunder\n", search_time);
    printf("Prosesserte noder: %d\n", processed_nodes);
    printf("Noder i ruten: %d\n", route_length);

    free(route);
}

int main(void) {
    /*LES IN NODER*/
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

    SearchInfo *searchInfo = calloc(number_of_nodes, sizeof(SearchInfo));
    if (searchInfo == NULL) {
        fprintf(stderr, "Kunne ikke allokere søksinformasjon\n");
        free(nodes);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < number_of_nodes; i++) {
        searchInfo[i].distance = INT_MAX;
        searchInfo[i].previous = -1;
        searchInfo[i].processed = 0;
    }

    /*EDGE FILE*/
    FILE *edges_file = fopen("kanter.txt", "r");
    if (edges_file == NULL) {
        perror("Kunne ikke åpne kanter.txt");
        free(searchInfo);
        free(nodes);
        return EXIT_FAILURE;
    }

    int number_of_edges;
    if (fscanf(edges_file, "%d", &number_of_edges) != 1) {
        fprintf(stderr, "Kunne ikke lese antall kanter\n");
        fclose(edges_file);
        free(searchInfo);
        free(nodes);
        return EXIT_FAILURE;
    }
    Edge *edges = calloc(number_of_edges, sizeof(Edge));
    if (edges == NULL) {
        perror("Couldn't allocate memory for edges");
        fclose(edges_file);
        free(searchInfo);
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
            free(searchInfo);
            fclose(edges_file);
            return EXIT_FAILURE;
        }
        edges[i].destination = to;
        edges[i].travel_time = travel_time;
        edges[i].next = nodes[from].first_edge;
        nodes[from].first_edge = &edges[i];
    }

    run_Dijkstra_search(nodes, searchInfo, number_of_nodes, 2001238, 1987066);

    run_Dijkstra_search(nodes, searchInfo, number_of_nodes, 2001238, 2763740);

    fclose(edges_file);
    free(edges);
    free(searchInfo);
    free(nodes);
    return EXIT_SUCCESS;
}