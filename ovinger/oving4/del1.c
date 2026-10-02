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

void appendDigit(LongNumber *number, int digit) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->value = digit;
    newNode->next = NULL;

    int wasEmpty = (number->head == NULL);

    if (wasEmpty) {
        newNode->prev = NULL;
        number->head = newNode;
    } else {
        newNode->prev = number->tail;
        number->tail->next = newNode;
    }
    number->tail = newNode;
}
void prependDigit(LongNumber *number, int digit) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->value = digit;
    newNode->prev = NULL;

    int wasEmpty = (number->head == NULL);
    if (wasEmpty) {
        newNode->next = NULL;
        number->tail = newNode;
    } else {
        newNode->next = number->head;
        number->head->prev = newNode;
    }
    number->head = newNode;
}

LongNumber buildFromString(char *s) {
    LongNumber number;
    number.head = NULL;
    number.tail = NULL;
    number.negative = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        int digit = s[i] - '0';
        appendDigit(&number, digit);
    }
    return number;
}

void printNumber(LongNumber number) {
    if (number.negative) {
        printf("-");
    }
    Node *curr = number.head;
    while (curr != NULL) {
        printf("%d", curr->value);
        curr = curr->next;
    }
    printf("\n");
}

int length(LongNumber number) {
    int len = 0;
    Node *curr = number.head;
    while (curr != NULL) {
        len++;
        curr = curr->next;
    }
    return len;
}

int compare(LongNumber number1, LongNumber number2) {
    Node *p1 = number1.head;
    Node *p2 = number2.head;

    int len1 = length(number1);
    int len2 = length(number2);

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

void fjernLedendeNuller(LongNumber *number) {
    while (number->head->next != NULL && number->head->value == 0) {
        Node *temp = number->head;
        number->head = number->head->next;
        number->head->prev = NULL;
        free(temp);
    }
}

LongNumber pluss(LongNumber number1, LongNumber number2) {
    LongNumber resultat;
    resultat.head = NULL;
    resultat.tail = NULL;
    resultat.negative = 0;

    Node *p1 = number1.tail;
    Node *p2 = number2.tail;
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

    if (carry > 0) {
        prependDigit(&resultat, carry);
    }
    return resultat;
}

LongNumber minus(LongNumber number1, LongNumber number2) {
    LongNumber result;
    result.head = NULL;
    result.tail = NULL;
    result.negative = 0;
    int cmp = compare(number1, number2);
    Node *p1;
    Node *p2;
    int carry = 0;

    if (cmp >= 0) {
        p1 = number1.tail;
        p2 = number2.tail;
    } else {
        result.negative = 1;
        p1 = number2.tail;
        p2 = number1.tail;
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
        prependDigit(&result, diff);
        if (p1 != NULL)
            p1 = p1->prev;
        if (p2 != NULL)
            p2 = p2->prev;
    }
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Use: %s <number1> <+|-> <number2>\n", argv[0]);
        return 1;
    }

    LongNumber number1 = buildFromString(argv[1]);
    LongNumber number2 = buildFromString(argv[3]);
    LongNumber result;

    if (strcmp(argv[2], "+") == 0) {
        result = pluss(number1, number2);
    } else if (strcmp(argv[2], "-") == 0) {
        result = minus(number1, number2);
    } else {
        printf("Illegal operator\n");
        return 1;
    }

    fjernLedendeNuller(&result);
    printNumber(result);
    return 0;
}