#include <stdint.h>
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

void make_codes(Node *node, int code, int length, int codes[], int lengths[]) {
    if (node == NULL) {
        return;
    }
    if (node->left == NULL && node->right == NULL) {
        codes[node->ch] = code;
        lengths[node->ch] = length;
        return;
    }
    make_codes(node->left, (code << 1), length + 1, codes, lengths);
    make_codes(node->right, (code << 1) | 1, length + 1, codes, lengths);
}

void write_bit(FILE *file, int bit, unsigned char *buffer, int *bit_count) {
    *buffer = (*buffer << 1) | bit;
    (*bit_count)++;

    if (*bit_count == 8) {
        fwrite(buffer, 1, 1, file);
        *buffer = 0;
        *bit_count = 0;
    }
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

    size_t size;
    unsigned char *data = readFile(argv[1], &size);
    int freq[256] = {0};
    for (const unsigned char *p = data; p < data + size; p++) {
        freq[*p]++;
    }
    Node *root = build_tree(freq);
    int codes[256] = {0};
    int lengths[256] = {0};
    make_codes(root, 0, 0, codes, lengths);

    FILE *output = fopen("compressed.bin", "wb");
    if (!output) {
        perror("Failed to open output file");
        free(data);
        exit(EXIT_FAILURE);
    }
    fwrite(&size, sizeof(size), 1, output);
    fwrite(freq, sizeof(freq[0]), 256, output);
    unsigned char buffer = 0;
    int bit_count = 0;

    for (const unsigned char *p = data; p < data + size; p++) {
        int code = codes[*p];
        int length = lengths[*p];
        for (int i = 0; i < length; i++) {
            int bit = (code >> (length - 1)) & 1;
            write_bit(output, bit, &buffer, &bit_count);
            code <<= 1;
        }
    }
    if (bit_count > 0) {
        buffer <<= (8 - bit_count);
        fwrite(&buffer, 1, 1, output);
    }
    printf("rot: %d, filstorrelse: %zu\n", root->frequency, size);
    fclose(output);
    free_tree(root);
    free(data);
    return 0;
}