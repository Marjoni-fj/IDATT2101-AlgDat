#define _POSIX_C_SOURCE 200809L
#define LANDMARK_COUNT 4
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

int calculate_heuristic(int current, int target, int number_of_nodes,
                        int *from_landmark, int *to_landmark) {
    int heuristic = 0;

    for (int landmark = 0; landmark < LANDMARK_COUNT; landmark++) {
        int index = landmark * number_of_nodes;

        int landmark_to_target = from_landmark[index + target];
        int landmark_to_current = from_landmark[index + current];

        int current_to_landmark = to_landmark[index + current];
        int target_to_landmark = to_landmark[index + target];

        if (landmark_to_target != INT_MAX && landmark_to_current != INT_MAX) {
            int candidate_from = landmark_to_target - landmark_to_current;

            if (candidate_from > heuristic) {
                heuristic = candidate_from;
            }
        }
        if (current_to_landmark != INT_MAX && target_to_landmark != INT_MAX) {
            int candidate_to = current_to_landmark - target_to_landmark;

            if (candidate_to > heuristic) {
                heuristic = candidate_to;
            }
        }
    }
    return heuristic;
}

int shortest_path_search(Node *nodes, SearchInfo *searchInfo,
                         int number_of_nodes, int start, int target,
                         int *from_landmark, int *to_landmark) {
    for (int i = 0; i < number_of_nodes; i++) {
        searchInfo[i].distance = INT_MAX;
        searchInfo[i].previous = -1;
        searchInfo[i].processed = 0;
    }
    MinHeap heap = createMinHeap(number_of_nodes);
    searchInfo[start].distance = 0;
    int start_heuristic = 0;
    if (from_landmark != NULL && to_landmark != NULL && target != -1) {
        start_heuristic = calculate_heuristic(start, target, number_of_nodes,
                                              from_landmark, to_landmark);
    }
    pushToHeap(&heap, start, start_heuristic);
    int processed_nodes = 0;

    while (heap.size > 0) {
        HeapElement current = popOffHeap(&heap);
        int current_node = current.node_id;
        if (searchInfo[current_node].processed) {
            continue;
        }
        searchInfo[current_node].processed = 1;
        processed_nodes++;

        if (target != -1 && current_node == target) {
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
                int heuristic = 0;
                if (from_landmark != NULL && to_landmark != NULL &&
                    target != -1) {
                    heuristic =
                        calculate_heuristic(neighbour, target, number_of_nodes,
                                            from_landmark, to_landmark);
                }
                int priority = new_time + heuristic;
                pushToHeap(&heap, neighbour, priority);
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

void run_search(Node *nodes, SearchInfo *searchInfo, int number_of_nodes,
                int start, int target, int *from_landmark, int *to_landmark) {
    struct timespec start_time;
    struct timespec end_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int processed_nodes =
        shortest_path_search(nodes, searchInfo, number_of_nodes, start, target,
                             from_landmark, to_landmark);

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double search_time = elapsed_seconds(start_time, end_time);

    int route_length;
    int *route =
        findRoute(searchInfo, number_of_nodes, start, target, &route_length);
    if (route == NULL) {
        fprintf(stderr, "Kunne ikke lage rute\n");
        return;
    }

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
    Node *reversed_nodes = calloc(number_of_nodes, sizeof(Node));
    if (reversed_nodes == NULL) {
        perror("Couldn't allocate memory for reverse nodes");
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
    Edge *reversed_edges = calloc(number_of_edges, sizeof(Edge));
    if (reversed_edges == NULL) {
        perror("Couldn't allocate memory for reverse edges");
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

        reversed_edges[i].destination = from;
        reversed_edges[i].travel_time = travel_time;
        reversed_edges[i].next = reversed_nodes[to].first_edge;
        reversed_nodes[to].first_edge = &reversed_edges[i];
    }

    /* LANDEMERKER */
    int landmarks[LANDMARK_COUNT] = {
        2948202, // Oslo
        7826348, // Trondheim
        545911,  // København
        925129   // Helsinki
    };

    int *from_landmark = calloc(LANDMARK_COUNT * number_of_nodes, sizeof(int));
    int *to_landmark = calloc(LANDMARK_COUNT * number_of_nodes, sizeof(int));
    if (from_landmark == NULL || to_landmark == NULL) {
        perror("Couldn't allocate memory for landmarks");
        free(from_landmark);
        free(to_landmark);
        return EXIT_FAILURE;
    }

    printf("\nDijkstra-søk:\n");
    run_search(nodes, searchInfo, number_of_nodes, 2001238, 1987066, NULL,
               NULL);
    run_search(nodes, searchInfo, number_of_nodes, 2001238, 2763740, NULL,
               NULL);

    for (int landmark_index = 0; landmark_index < LANDMARK_COUNT;
         landmark_index++) {
        int landmark = landmarks[landmark_index];

        shortest_path_search(nodes, searchInfo, number_of_nodes, landmark, -1,
                             NULL, NULL);
        for (int node = 0; node < number_of_nodes; node++) {
            from_landmark[landmark_index * number_of_nodes + node] =
                searchInfo[node].distance;
        }

        shortest_path_search(reversed_nodes, searchInfo, number_of_nodes,
                             landmark, -1, NULL, NULL);
        for (int node = 0; node < number_of_nodes; node++) {
            to_landmark[landmark_index * number_of_nodes + node] =
                searchInfo[node].distance;
        }
    }

    printf("\nALT-søk:\n");
    run_search(nodes, searchInfo, number_of_nodes, 2001238, 1987066,
               from_landmark, to_landmark);
    run_search(nodes, searchInfo, number_of_nodes, 2001238, 2763740,
               from_landmark, to_landmark);

    // printf("Oslo -> Odense: %d\n", from_landmark[0 * number_of_nodes +
    // 2763740]); printf("Gløshaugen -> Oslo: %d\n", to_landmark[0 *
    // number_of_nodes + 2001238]);

    fclose(edges_file);
    free(from_landmark);
    free(to_landmark);
    free(edges);
    free(reversed_edges);
    free(searchInfo);
    free(nodes);
    free(reversed_nodes);

    return EXIT_SUCCESS;
}