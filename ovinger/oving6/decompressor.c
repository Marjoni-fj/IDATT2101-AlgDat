#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int frequency;
    unsigned char ch;
    struct Node *left;
    struct Node *right;
} Node;

Node *take_min(Node **a, int *n) {
    int best = 0;
    for (int i = 1; i < *n; i++) {
        if (a[i]->frequency < a[best]->frequency) {
            best = i;
        }
    }
    Node *result = a[best];

    a[best] = a[*n - 1];
    (*n)--;
    return result;
}

Node *build_tree(const int *freq) {
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
    while (n > 1) {
        Node *a = take_min(nodes, &n);
        Node *b = take_min(nodes, &n);
        Node *parent = malloc(sizeof(Node));
        parent->frequency = a->frequency + b->frequency;
        parent->ch = 0; // ikke relevant for interne noder
        parent->left = a;
        parent->right = b;
        nodes[n++] = parent;
    }
    return nodes[0];
}

int read_bit(FILE *file, unsigned char *buffer, int *bit_count) {
    if (*bit_count == 0) {
        fread(buffer, 1, 1, file);
        *bit_count = 8;
    }
    int bit = (*buffer >> 7) & 1;
    *buffer <<= 1;
    (*bit_count)--;
    return bit;
}

void free_tree(Node *node) {
    if (node == NULL) {
        return;
    }
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Bruk: %s filnavn\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *input = fopen(argv[1], "rb");
    if (!input) {
        perror("Failed to open input");
        return EXIT_FAILURE;
    }

    size_t original_size;
    int freq[256];

    if (fread(&original_size, sizeof(original_size), 1, input) != 1) {
        perror("Failed to read original size");
        fclose(input);
        return EXIT_FAILURE;
    }
    if (fread(freq, sizeof(freq[0]), 256, input) != 256) {
        perror("Failed to read frequency table");
        fclose(input);
        return EXIT_FAILURE;
    }

    printf("Original size: %zu bytes\n", original_size);
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            printf("Byte %d: Frequency %d\n", i, freq[i]);
        }
    }
    Node *root = build_tree(freq);
    FILE *output = fopen("decompressed_output.bin", "wb");
    if (!output) {
        perror("Failed to open output");
        fclose(input);
        return EXIT_FAILURE;
    }
    unsigned char buffer = 0;
    int bit_count = 0;
    Node *current = root;
    size_t written = 0;

    while (written < original_size) {
        int bit = read_bit(input, &buffer, &bit_count);
        if (bit == 0) {
            current = current->left;
        } else {
            current = current->right;
        }
        if (current->left == NULL && current->right == NULL) {
            fwrite(&current->ch, 1, 1, output);
            written++;
            current = root;
        }
    }

    printf("Tree frequency: %d\n", root->frequency);
    fclose(input);
    fclose(output);
    free_tree(root);
    return 0;
}