#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct NodeStruct {
    int value;
    struct NodeStruct *next;
    struct NodeStruct *prev;
} Node;

Node *newNode(int e, Node *next, Node *prev) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->value = e;
    node->next = next;
    node->prev = prev;
    return node;
}
typedef struct {
    Node *head;
    Node *tail;
    int negative;
} LongNumber;

void appendDigit(LongNumber *tall, int siffer) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->value = siffer;
    newNode->next = NULL;

    int wasEmpty = (tall->head == NULL);

    if (wasEmpty) {
        newNode->prev = NULL;
        tall->head = newNode;
    } else {
        newNode->prev = tall->tail;
        tall->tail->next = newNode;
    }
    tall->tail = newNode;
}
void prependDigit(LongNumber *tall, int siffer) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->value = siffer;
    newNode->prev = NULL;

    int wasEmpty = (tall->head == NULL);
    if (wasEmpty) {
        newNode->next = NULL;
        tall->tail = newNode;
    } else {
        newNode->next = tall->head;
        tall->head->prev = newNode;
    }
    tall->head = newNode;
}

LongNumber buildFromString(char *s) {
    LongNumber tall;
    tall.head = NULL;
    tall.tail = NULL;
    tall.negative = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        int digit = s[i] - '0';
        appendDigit(&tall, digit);
    }
    return tall;
}

void printNumber(LongNumber tall) {
    if (tall.negative) {
        printf("-");
    }
    Node *curr = tall.head;
    while (curr != NULL) {
        printf("%d", curr->value);
        curr = curr->next;
    }
    printf("\n");
}

int length(LongNumber tall) {
    int len = 0;
    Node *curr = tall.head;
    while (curr != NULL) {
        len++;
        curr = curr->next;
    }
    return len;
}

int compare(LongNumber tall1, LongNumber tall2) {
    Node *p1 = tall1.head;
    Node *p2 = tall2.head;

    int len1 = length(tall1);
    int len2 = length(tall2);

    if (len1 < len2)
        return -1;
    else if (len1 > len2)
        return 1;
    else {
        while (p1 != NULL && p2 != NULL) {
            if (p1->value < p2->value)
                return -1;
            else if (p1->value > p2->value)
                return 1;
            p1 = p1->next;
            p2 = p2->next;
        }
        return 0; // equal
    }
}

void fjernLedendeNuller(LongNumber *tall) {
    while (tall->head->next != NULL && tall->head->value == 0) {
        Node *temp = tall->head;
        tall->head = tall->head->next;
        tall->head->prev = NULL;
        free(temp);
    }
}

LongNumber pluss(LongNumber tall1, LongNumber tall2) {
    LongNumber resultat;
    resultat.head = NULL;
    resultat.tail = NULL;
    resultat.negative = 0;

    Node *p1 = tall1.tail;
    Node *p2 = tall2.tail;
    int carry = 0;

    while (p1 != NULL || p2 != NULL) {
        int digit1 = (p1 != NULL) ? p1->value : 0;
        int digit2 = (p2 != NULL) ? p2->value : 0;
        int sum = digit1 + digit2 + carry;
        int digit = sum % 10;
        carry = sum / 10;
        prependDigit(&resultat, digit);

        if (p1 != NULL)
            p1 = p1->prev;
        if (p2 != NULL)
            p2 = p2->prev;
    }

    // hva om det er mente igjen etter løkka er ferdig? (f.eks. 999+1)
    if (carry > 0) {
        prependDigit(&resultat, carry);
    }
    return resultat;
}

LongNumber minus(LongNumber tall1, LongNumber tall2) {
    LongNumber resultat;
    resultat.head = NULL;
    resultat.tail = NULL;
    resultat.negative = 0;
    int cmp = compare(tall1, tall2);
    Node *p1;
    Node *p2;
    int carry = 0;

    if (cmp >= 0) {
        p1 = tall1.tail;
        p2 = tall2.tail;
    } else {
        resultat.negative = 1;
        p1 = tall2.tail;
        p2 = tall1.tail;
    }

    while (p1 != NULL || p2 != NULL) {
        int digit1 = (p1 != NULL) ? p1->value : 0;
        int digit2 = (p2 != NULL) ? p2->value : 0;
        int diff = digit1 - digit2 - carry;
        if (diff < 0) {
            diff += 10;
            carry = 1;
        } else {
            carry = 0;
        }
        prependDigit(&resultat, diff);
        if (p1 != NULL)
            p1 = p1->prev;
        if (p2 != NULL)
            p2 = p2->prev;
    }
    return resultat;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Bruk: %s <tall1> <+|-> <tall2>\n", argv[0]);
        return 1;
    }

    LongNumber tall1 = buildFromString(argv[1]);
    LongNumber tall2 = buildFromString(argv[3]);
    LongNumber resultat;

    if (strcmp(argv[2], "+") == 0) {
        resultat = pluss(tall1, tall2);
    } else if (strcmp(argv[2], "-") == 0) {
        resultat = minus(tall1, tall2);
    } else {
        printf("Ugyldig operator\n");
        return 1;
    }

    fjernLedendeNuller(&resultat);
    printNumber(resultat);
    return 0;
}