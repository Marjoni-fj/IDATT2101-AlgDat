#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SEARCH_WINDOW 32768
#define LOOKAHEAD_SIZE 258
#define MIN_MATCH 3

#define LITERAL 0
#define REFERENCE 1

typedef struct Node {
    int frequency;
    unsigned char ch;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct Match {
    int distance;
    int length;
} Match;

typedef struct LZToken {
    int type;
    union {
        unsigned char literal;
        Match match;
    } LiteralOrMatch;
} LZToken;

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
        parent->ch = 0; // not relevant for internal nodes
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

size_t lz77_decompress(const LZToken *tokens, size_t token_count,
                       unsigned char *output, size_t output_capacity) {
    size_t output_size = 0;

    for (size_t i = 0; i < token_count; i++) {
        LZToken token = tokens[i];
        if (token.type == LITERAL) {
            if (output_size >= output_capacity) {
                return SIZE_MAX;
            }
            output[output_size] = token.LiteralOrMatch.literal;
            output_size++;
        } else if (token.type == REFERENCE) {
            int distance = token.LiteralOrMatch.match.distance;
            int length = token.LiteralOrMatch.match.length;
            if (distance <= 0 || (size_t)distance > output_size || length < 0 ||
                (size_t)length > output_capacity - output_size) {
                return SIZE_MAX;
            }
            for (int i = 0; i < token.LiteralOrMatch.match.length; i++) {
                output[output_size] = output[output_size - distance];
                output_size++;
            }
        } else {
            return SIZE_MAX;
        }
    }
    return output_size;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *input = fopen(argv[1], "rb");
    if (!input) {
        perror("Failed to open input");
        return EXIT_FAILURE;
    }

    size_t original_size;
    size_t serialized_size;
    int freq[256];

    if (fread(&original_size, sizeof(original_size), 1, input) != 1) {
        perror("Failed to read original size");
        fclose(input);
        return EXIT_FAILURE;
    }

    if (fread(&serialized_size, sizeof(serialized_size), 1, input) != 1) {
        perror("Failed to read serialized size");
        fclose(input);
        return EXIT_FAILURE;
    }

    if (fread(freq, sizeof(freq[0]), 256, input) != 256) {
        perror("Failed to read frequency table");
        fclose(input);
        return EXIT_FAILURE;
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
    unsigned char *serialized = malloc(serialized_size);
    if (serialized == NULL) {
        perror("Failed to allocate serialized data");
        fclose(input);
        free_tree(root);
        return EXIT_FAILURE;
    }
    size_t decoded_size = 0;

    while (decoded_size < serialized_size) {
        int bit = read_bit(input, &buffer, &bit_count);
        if (bit == 0) {
            current = current->left;
        } else {
            current = current->right;
        }
        if (current->left == NULL && current->right == NULL) {
            serialized[decoded_size] = current->ch;
            decoded_size++;
            current = root;
        }
    }

    size_t position = 0;
    size_t token_count = 0;
    LZToken *tokens = malloc(serialized_size * sizeof(LZToken));
    if (tokens == NULL) {
        perror("Failed to allocate tokens");
        free(serialized);
        fclose(input);
        fclose(output);
        free_tree(root);
        return EXIT_FAILURE;
    }

    while (position < serialized_size) {
        unsigned char type = serialized[position];
        if (type == LITERAL) {
            if (serialized_size - position < 2) {
                fprintf(stderr, "Invalid literal token\n");
                free(tokens);
                free(serialized);
                fclose(input);
                fclose(output);
                free_tree(root);
                return EXIT_FAILURE;
            }
            tokens[token_count].type = LITERAL;
            tokens[token_count].LiteralOrMatch.literal =
                serialized[position + 1];
            position += 2;
        } else if (type == REFERENCE) {
            if (serialized_size - position < 5) {
                fprintf(stderr, "Invalid reference token\n");
                free(tokens);
                free(serialized);
                fclose(input);
                fclose(output);
                free_tree(root);
                return EXIT_FAILURE;
            }
            tokens[token_count].type = REFERENCE;
            tokens[token_count].LiteralOrMatch.match.distance =
                serialized[position + 1] | (serialized[position + 2] << 8);

            tokens[token_count].LiteralOrMatch.match.length =
                serialized[position + 3] | (serialized[position + 4] << 8);
            position += 5;
        } else {
            fprintf(stderr, "Unknown token type: %u\n", type);
            free(tokens);
            free(serialized);
            fclose(input);
            fclose(output);
            free_tree(root);
            return EXIT_FAILURE;
        }
        token_count++;
    }

    unsigned char *reconstructed = malloc(original_size);
    if (reconstructed == NULL) {
        perror("Failed to allocate memory to reconstructed file");
        free(tokens);
        free(serialized);
        fclose(input);
        fclose(output);
        free_tree(root);
        return EXIT_FAILURE;
    }
    size_t reconstructed_size =
        lz77_decompress(tokens, token_count, reconstructed, original_size);
    if (reconstructed_size == SIZE_MAX || reconstructed_size != original_size) {
        fprintf(stderr, "Incorrect size after LZ decompression\n");
        free(reconstructed);
        free(tokens);
        free(serialized);
        fclose(input);
        fclose(output);
        free_tree(root);
        return EXIT_FAILURE;
    }

    if (fwrite(reconstructed, 1, reconstructed_size, output) !=
        reconstructed_size) {
        perror("Failed to write decompressed output");
        return EXIT_FAILURE;
    }

    printf("Decompression successful.\n");
    printf("Output written to decompressed_output.bin\n");
    printf("Decompressed size: %zu bytes\n", reconstructed_size);

    fclose(input);
    fclose(output);
    free_tree(root);
    free(reconstructed);
    free(tokens);
    free(serialized);
    return 0;
}