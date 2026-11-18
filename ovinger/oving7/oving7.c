#define _POSIX_C_SOURCE 200809L
#define LANDMARK_COUNT 4
#define NEARBY_POINT_COUNT 4
#define GLOSHAUGEN_NODE 2001238
#define OTILIENBORG_NODE 1987066
#define ODENSE_NODE 2763740
#define OSLO_NODE 2948202
#define TRONDHEIM_NODE 7826348
#define KOBENHAVN_NODE 545911
#define HELSINKI_NODE 925129
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct {
    int node_id;
    int type_code;
    char *name;
} InterestPoint;

typedef struct {
    int node_id;
    int distance;
    const char *name;
} NearbyPoint;

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
void save_route_geojson(const char *filename, Node *nodes, int *route,
                        int route_length) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Couldn't create GeoJSON-fil");
        return;
    }

    fprintf(file, "{\n"
                  "  \"type\": \"Feature\",\n"
                  "  \"geometry\": {\n"
                  "    \"type\": \"LineString\",\n"
                  "    \"coordinates\": [\n");

    for (int i = route_length - 1; i >= 0; i--) {
        int node_id = route[i];

        fprintf(file, "      [%.8f, %.8f]%s\n", nodes[node_id].longitude,
                nodes[node_id].latitude, i > 0 ? "," : "");
    }

    fprintf(file, "    ]\n"
                  "  }\n"
                  "}\n");

    fclose(file);
}

void save_points_geojson(const char *filename, Node *nodes, NearbyPoint *points,
                         int count) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Couldn't create POI-GeoJSON");
        return;
    }

    fprintf(file, "{\n"
                  "  \"type\": \"FeatureCollection\",\n"
                  "  \"features\": [\n");

    for (int i = 0; i < count; i++) {
        int node_id = points[i].node_id;

        fprintf(file,
                "    {\n"
                "      \"type\": \"Feature\",\n"
                "      \"properties\": {\n"
                "        \"name\": \"%s\"\n"
                "      },\n"
                "      \"geometry\": {\n"
                "        \"type\": \"Point\",\n"
                "        \"coordinates\": [%.8f, %.8f]\n"
                "      }\n"
                "    }%s\n",
                points[i].name, nodes[node_id].longitude,
                nodes[node_id].latitude, i < count - 1 ? "," : "");
    }

    fprintf(file, "  ]\n"
                  "}\n");

    fclose(file);
}
void run_search(Node *nodes, SearchInfo *searchInfo, int number_of_nodes,
                int start, int target, int *from_landmark, int *to_landmark,
                const char *geojson_filename) {
    struct timespec start_time;
    struct timespec end_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int processed_nodes =
        shortest_path_search(nodes, searchInfo, number_of_nodes, start, target,
                             from_landmark, to_landmark);

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double search_time = elapsed_seconds(start_time, end_time);

    if (searchInfo[target].distance == INT_MAX) {
        printf("No route from %d to %d\n", start, target);
        return;
    }

    int route_length;
    int *route =
        findRoute(searchInfo, number_of_nodes, start, target, &route_length);
    if (route == NULL) {
        fprintf(stderr, "Kunne ikke lage rute\n");
        return;
    }

    printf("\nRoute from %d to %d\n", start, target);
    printf("Travel time: ");
    print_time(searchInfo[target].distance);
    printf("Search time: %.6f seconds\n", search_time);
    printf("Processed nodes: %d\n", processed_nodes);
    printf("Nodes in route: %d\n", route_length);

    if (geojson_filename != NULL) {
        save_route_geojson(geojson_filename, nodes, route, route_length);
    }
    free(route);
}

int compare_interest_points(const void *first, const void *second) {
    const InterestPoint *first_point = first;
    const InterestPoint *second_point = second;

    if (first_point->node_id < second_point->node_id) {
        return -1;
    }
    if (first_point->node_id > second_point->node_id) {
        return 1;
    }
    return 0;
}

int first_interest_point_at_node(InterestPoint *interest_points,
                                 int number_of_interest_points, int node_id) {
    int left = 0;
    int right = number_of_interest_points;

    while (left < right) {
        int middle = left + (right - left) / 2;
        if (interest_points[middle].node_id < node_id) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }
    return left;
}

int find_nearest_interest_points(Node *nodes, SearchInfo *searchInfo,
                                 int number_of_nodes, int start,
                                 InterestPoint *interest_points,
                                 int number_of_interest_points, int wanted_type,
                                 int max_results, NearbyPoint *results) {
    for (int i = 0; i < number_of_nodes; i++) {
        searchInfo[i].distance = INT_MAX;
        searchInfo[i].previous = -1;
        searchInfo[i].processed = 0;
    }
    MinHeap heap = createMinHeap(number_of_nodes);
    searchInfo[start].distance = 0;
    int priority = 0;
    int result_count = 0;
    pushToHeap(&heap, start, priority);

    while (heap.size > 0) {
        HeapElement current = popOffHeap(&heap);
        int current_node = current.node_id;
        if (searchInfo[current_node].processed) {
            continue;
        }
        searchInfo[current_node].processed = 1;
        int first_point = first_interest_point_at_node(
            interest_points, number_of_interest_points, current_node);
        for (int i = first_point; i < number_of_interest_points &&
                                  interest_points[i].node_id == current_node;
             i++) {
            InterestPoint *point = &interest_points[i];
            if (point->node_id == current_node &&
                (point->type_code & wanted_type) != 0 &&
                result_count < max_results) {
                results[result_count].node_id = point->node_id;
                results[result_count].distance =
                    searchInfo[current_node].distance;
                results[result_count].name = point->name;
                result_count++;
            }
        }
        if (result_count == max_results) {
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
    return result_count;
}

int load_interest_points(const char *filename,
                         InterestPoint **interest_points) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Couldn't open interessepkt.txt");
        return -1;
    }

    int number_of_interest_points;
    if (fscanf(file, "%d", &number_of_interest_points) != 1) {
        fprintf(stderr, "Couldn't read all interest points\n");
        fclose(file);
        return -1;
    }

    *interest_points =
        malloc(number_of_interest_points * sizeof(InterestPoint));
    if (*interest_points == NULL) {
        fprintf(stderr, "Couldn't allocate interest points\n");
        fclose(file);
        return -1;
    }

    int character;
    while ((character = fgetc(file)) != '\n' && character != EOF) {
    }
    char line[2048];
    for (int i = 0; i < number_of_interest_points; i++) {
        if (fgets(line, sizeof(line), file) == NULL) {
            fprintf(stderr, "Couldn't read interest point %d\n", i);
            fclose(file);
            return -1;
        }
        int node_id;
        int type_code;
        if (sscanf(line, "%d %d", &node_id, &type_code) != 2) {
            fprintf(stderr, "Invalid interest point %d\n", i);
            fclose(file);
            return -1;
        }
        char *first_quote = strchr(line, '"');
        char *last_quote = strrchr(line, '"');
        if (first_quote == NULL || last_quote == first_quote) {
            fprintf(stderr, "Missing name on interest point %d\n", i);
            fclose(file);
            return -1;
        }

        first_quote++;
        size_t name_length = (size_t)(last_quote - first_quote);
        (*interest_points)[i].node_id = node_id;
        (*interest_points)[i].type_code = type_code;
        (*interest_points)[i].name = malloc(name_length + 1);
        if ((*interest_points)[i].name == NULL) {
            fprintf(stderr, "Couldn't allocate name\n");
            fclose(file);
            return -1;
        }
        memcpy((*interest_points)[i].name, first_quote, name_length);
        (*interest_points)[i].name[name_length] = '\0';
    }

    fclose(file);
    return number_of_interest_points;
}

int save_landmarks(const char *filename, int number_of_nodes, int *landmarks,
                   int *from_landmark, int *to_landmark) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Couldn't create landmark file");
        return -1;
    }
    size_t table_size = LANDMARK_COUNT * (size_t)number_of_nodes;
    if (fwrite(&number_of_nodes, sizeof(int), 1, file) != 1 ||
        fwrite(landmarks, sizeof(int), LANDMARK_COUNT, file) !=
            LANDMARK_COUNT ||
        fwrite(from_landmark, sizeof(int), table_size, file) != table_size ||
        fwrite(to_landmark, sizeof(int), table_size, file) != table_size) {
        fprintf(stderr, "Couldn't write landmark-data\n");
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

int load_landmarks(const char *filename, int number_of_nodes, int *landmarks,
                   int *from_landmark, int *to_landmark) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Couldn't open landmark-fil");
        return -1;
    }
    int stored_number_of_nodes;
    if (fread(&stored_number_of_nodes, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Couldn't read amount of nodes\n");
        fclose(file);
        return -1;
    }
    if (stored_number_of_nodes != number_of_nodes) {
        fprintf(stderr, "Landmark-file doesn't fit the map\n");
        fclose(file);
        return -1;
    }
    if (fread(landmarks, sizeof(int), LANDMARK_COUNT, file) != LANDMARK_COUNT) {
        fprintf(stderr, "Couldn't read landemarks\n");
        fclose(file);
        return -1;
    }
    size_t table_size = LANDMARK_COUNT * (size_t)number_of_nodes;
    if (fread(from_landmark, sizeof(int), table_size, file) != table_size ||
        fread(to_landmark, sizeof(int), table_size, file) != table_size) {
        fprintf(stderr, "Couldn't read landmark-tabel\n");
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

int main(void) {
    int exit_code = EXIT_FAILURE;

    FILE *nodes_file = NULL;
    FILE *edges_file = NULL;
    Node *nodes = NULL;
    Node *reversed_nodes = NULL;
    SearchInfo *searchInfo = NULL;
    Edge *edges = NULL;
    Edge *reversed_edges = NULL;
    InterestPoint *interest_points = NULL;
    int *from_landmark = NULL;
    int *to_landmark = NULL;
    int number_of_nodes = 0;
    int number_of_edges = 0;
    int count = 0;

    /*READ IN NODES*/
    nodes_file = fopen("noder.txt", "r");
    if (nodes_file == NULL) {
        perror("Couldn't open noder.txt");
        goto cleanup;
    }

    if (fscanf(nodes_file, "%d", &number_of_nodes) != 1) {
        fprintf(stderr, "Couldn't read amount of nodes\n");
        goto cleanup;
    }

    nodes = calloc(number_of_nodes, sizeof(Node));
    if (nodes == NULL) {
        perror("Couldn't allocate memory for nodes");
        goto cleanup;
    }
    reversed_nodes = calloc(number_of_nodes, sizeof(Node));
    if (reversed_nodes == NULL) {
        perror("Couldn't allocate memory for reverse nodes");
        goto cleanup;
    }

    for (int i = 0; i < number_of_nodes; i++) {
        int node_id;
        double latitude;
        double longitude;
        if (fscanf(nodes_file, "%d %lf %lf", &node_id, &latitude, &longitude) !=
            3) {
            fprintf(stderr, "Couldn't read node %d\n", i);
            goto cleanup;
        }
        nodes[node_id].latitude = latitude;
        nodes[node_id].longitude = longitude;
        nodes[node_id].first_edge = NULL;
    }
    fclose(nodes_file);
    nodes_file = NULL;

    searchInfo = calloc(number_of_nodes, sizeof(SearchInfo));
    if (searchInfo == NULL) {
        fprintf(stderr, "Couldn't allocate search information\n");
        goto cleanup;
    }

    /*READ IN EDGES*/
    edges_file = fopen("kanter.txt", "r");
    if (edges_file == NULL) {
        perror("Couldn't open kanter.txt");
        goto cleanup;
    }

    if (fscanf(edges_file, "%d", &number_of_edges) != 1) {
        fprintf(stderr, "Couldn't read amount of edges\n");
        goto cleanup;
    }
    edges = calloc(number_of_edges, sizeof(Edge));
    if (edges == NULL) {
        perror("Couldn't allocate memory for edges");
        goto cleanup;
    }
    reversed_edges = calloc(number_of_edges, sizeof(Edge));
    if (reversed_edges == NULL) {
        perror("Couldn't allocate memory for reverse edges");
        goto cleanup;
    }
    for (int i = 0; i < number_of_edges; i++) {
        int from;
        int to;
        int travel_time;
        int length;
        int speed_limit;
        if (fscanf(edges_file, "%d %d %d %d %d", &from, &to, &travel_time,
                   &length, &speed_limit) != 5) {
            fprintf(stderr, "Couldn't read edge %d\n", i);
            goto cleanup;
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

    /* INTEREST POINTS AND NEARBY POINTS*/
    count = load_interest_points("interessepkt.txt", &interest_points);
    if (count == -1) {
        goto cleanup;
    }
    qsort(interest_points, count, sizeof(InterestPoint),
          compare_interest_points);

    /* GLOSHAUGEN */
    int gloshaugen = GLOSHAUGEN_NODE;
    int wanted_type = 8 | 16;

    NearbyPoint gloshaugen_points[NEARBY_POINT_COUNT];
    int gloshaugen_found = find_nearest_interest_points(
        nodes, searchInfo, number_of_nodes, gloshaugen, interest_points, count,
        wanted_type, NEARBY_POINT_COUNT, gloshaugen_points);
    for (int i = 0; i < gloshaugen_found; i++) {
        printf("Gloshaugen %d. %s, node %d, time %d\n", i + 1,
               gloshaugen_points[i].name, gloshaugen_points[i].node_id,
               gloshaugen_points[i].distance);
    }
    save_points_geojson("gloshaugen-points.geojson", nodes, gloshaugen_points,
                        gloshaugen_found);

    /* ODENSE */
    NearbyPoint odense_points[NEARBY_POINT_COUNT];

    int odense_found = find_nearest_interest_points(
        nodes, searchInfo, number_of_nodes, ODENSE_NODE, interest_points, count,
        4, NEARBY_POINT_COUNT, odense_points);
    for (int i = 0; i < odense_found; i++) {
        printf("Odense %d. %s, node %d, time %d\n", i + 1,
               odense_points[i].name, odense_points[i].node_id,
               odense_points[i].distance);
    }
    save_points_geojson("odense-points.geojson", nodes, odense_points,
                        odense_found);

    /* LANDMARKS */
    int landmarks[LANDMARK_COUNT] = {OSLO_NODE, TRONDHEIM_NODE, KOBENHAVN_NODE,
                                     HELSINKI_NODE};

    from_landmark = calloc(LANDMARK_COUNT * number_of_nodes, sizeof(int));
    to_landmark = calloc(LANDMARK_COUNT * number_of_nodes, sizeof(int));
    if (from_landmark == NULL || to_landmark == NULL) {
        perror("Couldn't allocate memory for landmarks");
        goto cleanup;
    }

    printf("\nDijkstra search:\n");
    run_search(nodes, searchInfo, number_of_nodes, GLOSHAUGEN_NODE,
               OTILIENBORG_NODE, NULL, NULL,
               "dijkstra-gloshaugen-otilienborg.geojson");
    run_search(nodes, searchInfo, number_of_nodes, GLOSHAUGEN_NODE, ODENSE_NODE,
               NULL, NULL, "dijkstra-gloshaugen-odense.geojson");

    /* LANDMARK STORAGE*/
    FILE *landmark_file = fopen("landmarks.bin", "rb");
    if (landmark_file != NULL) {
        fclose(landmark_file);

        if (load_landmarks("landmarks.bin", number_of_nodes, landmarks,
                           from_landmark, to_landmark) != 0) {
            fprintf(stderr, "Couldn't load landmark data\n");
            goto cleanup;
        }
        printf("\nLoaded landmarks.bin\n");
    } else {
        printf("Calculating landmark tables...\n");

        for (int landmark_index = 0; landmark_index < LANDMARK_COUNT;
             landmark_index++) {
            int landmark = landmarks[landmark_index];

            shortest_path_search(nodes, searchInfo, number_of_nodes, landmark,
                                 -1, NULL, NULL);
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

        if (save_landmarks("landmarks.bin", number_of_nodes, landmarks,
                           from_landmark, to_landmark) != 0) {
            fprintf(stderr, "Couldn't save landmark data\n");
            goto cleanup;
        }
    }

    printf("\nALT search:\n");
    run_search(nodes, searchInfo, number_of_nodes, GLOSHAUGEN_NODE,
               OTILIENBORG_NODE, from_landmark, to_landmark,
               "alt-gloshaugen-otilienborg.geojson");

    run_search(nodes, searchInfo, number_of_nodes, GLOSHAUGEN_NODE, ODENSE_NODE,
               from_landmark, to_landmark, "alt-gloshaugen-odense.geojson");

    exit_code = EXIT_SUCCESS;

cleanup:
    for (int i = 0; i < count; i++) {
        free(interest_points[i].name);
    }
    free(interest_points);
    if (nodes_file != NULL) {
        fclose(nodes_file);
    }
    if (edges_file != NULL) {
        fclose(edges_file);
    }
    free(from_landmark);
    free(to_landmark);
    free(edges);
    free(reversed_edges);
    free(searchInfo);
    free(nodes);
    free(reversed_nodes);

    return exit_code;
}