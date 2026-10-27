#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int frequency;
    unsigned char ch;
    struct Node *left;
    struct Node *right;
} Node;

unsigned char *readFile(const char *filename, size_t *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *buffer = malloc(*size);
    if (!buffer) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    size_t bytesRead = fread(buffer, 1, *size, file);
    if (bytesRead != *size) {
        perror("Failed to read file");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Bruk: %s filnavn\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t size;
    unsigned char *data = readFile(argv[1], &size);
    int freq[256] = {0};
    for (const unsigned char *p = data; p < data + size; p++) {
        freq[*p]++;
    }
    Node *nodes[256];
    int n = 0;
    for (int c = 0; c < 256; c++) {
        if (freq[c] > 0) {
            Node *x = malloc(sizeof(Node));
            x->frequency = freq[c];
            x->ch = c;
            x->left = x->right = NULL;
            nodes[n++] = x;
        }
    }
    printf("antall noder: %d\n", n);

    for (int i = 0; i < n; i++) {
        free(nodes[i]);
    }
    free(data);
    return 0;
}