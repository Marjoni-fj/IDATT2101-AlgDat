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

Match find_best_match(const unsigned char *data, size_t position, size_t size) {
    Match best = {0, 0};
    size_t search_start =
        (position > SEARCH_WINDOW) ? position - SEARCH_WINDOW : 0;
    for (size_t candidate = search_start; candidate < position; candidate++) {
        size_t match_length = 0;
        while (
            position + match_length < size && match_length < LOOKAHEAD_SIZE &&
            data[candidate + match_length] == data[position + match_length]) {
            match_length++;
        }
        if (match_length > (size_t)best.length) {
            best.length = match_length;
            best.distance = position - candidate;
        }
    }
    return best;
}

size_t lz77_compress(const unsigned char *data, size_t size, LZToken *tokens) {
    size_t position = 0;
    size_t token_count = 0;

    while (position < size) {
        Match match = find_best_match(data, position, size);

        if (match.length >= MIN_MATCH) {
            tokens[token_count].type = REFERENCE;
            tokens[token_count].LiteralOrMatch.match = match;
            token_count++;
            position += (size_t)match.length;
        } else {
            tokens[token_count].type = LITERAL;
            tokens[token_count].LiteralOrMatch.literal = data[position];
            token_count++;
            position++;
        }
    }
    return token_count;
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
                       unsigned char *output) {
    size_t output_size = 0;

    for (size_t i = 0; i < token_count; i++) {
        LZToken token = tokens[i];
        if (token.type == LITERAL) {
            output[output_size] = token.LiteralOrMatch.literal;
            output_size++;
        } else if (token.type == REFERENCE) {
            for (int i = 0; i < token.LiteralOrMatch.match.length; i++) {
                output[output_size] =
                    output[output_size - token.LiteralOrMatch.match.distance];
                output_size++;
            }
        }
    }
    return output_size;
}

unsigned char *serialize_tokens(const LZToken *tokens, size_t token_count,
                                size_t *serialized_size) {
    size_t position = 0;
    unsigned char *result;
    for (size_t i = 0; i < token_count; i++) {
        LZToken token = tokens[i];
        if (token.type == LITERAL) {
            position += 2;
        } else if (token.type == REFERENCE) {
            position += 5;
        }
    }
    *serialized_size = position;
    result = malloc(position);
    if (result == NULL) {
        perror("Failed allocating emory for result array");
        exit(EXIT_FAILURE);
    }

    position = 0;

    for (size_t i = 0; i < token_count; i++) {
        LZToken token = tokens[i];

        if (token.type == LITERAL) {
            result[position] = LITERAL;
            result[position + 1] = token.LiteralOrMatch.literal;
            position += 2;
        } else if (token.type == REFERENCE) {
            result[position] = REFERENCE;

            result[position + 1] = token.LiteralOrMatch.match.distance & 0xFF;
            result[position + 2] =
                (token.LiteralOrMatch.match.distance >> 8) & 0xFF;

            result[position + 3] = token.LiteralOrMatch.match.length & 0xFF;
            result[position + 4] =
                (token.LiteralOrMatch.match.length >> 8) & 0xFF;

            position += 5;
        }
    }

    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Bruk: %s filnavn\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t size;
    unsigned char *data = readFile(argv[1], &size);

    LZToken *tokens = malloc(
        size * sizeof(LZToken)); /* size ganger størrelsen på ett token */
    if (tokens == NULL) {
        perror("Failed to allocate memory for tokens");
        free(data);
        exit(EXIT_FAILURE);
    }
    size_t token_count = lz77_compress(data, size, tokens);
    size_t serialized_size;
    unsigned char *serialized =
        serialize_tokens(tokens, token_count, &serialized_size);

    int freq[256] = {0};
    for (size_t i = 0; i < serialized_size; i++) {
        freq[serialized[i]]++;
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

    for (size_t i = 0; i < serialized_size; i++) {
        int code = codes[serialized[i]];
        int length = lengths[serialized[i]];
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
    printf("Original størrelse: %zu bytes\n", size);
    printf("Serialisert størrelse: %zu bytes\n", serialized_size);
    printf("Huffman-treets frekvenssum: %d\n", root->frequency);
    fclose(output);
    free_tree(root);
    free(serialized);
    free(tokens);
    free(data);
    return 0;
}