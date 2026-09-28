#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct TreeNodeStruct {
    char *word;
    struct TreeNodeStruct *left;
    struct TreeNodeStruct *right;
} TreeNode;

TreeNode *createNewNode(char *word) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->word = strdup(word);
    node->left = NULL;
    node->right = NULL;
    return node;
}
TreeNode *insertNode(TreeNode *root, char *word) {
    if (root == NULL) {
        return createNewNode(word);
    }
    int cmp = strcmp(word, root->word);
    if (cmp < 0) {
        root->left = insertNode(root->left, word);
    } else if (cmp > 0) {
        root->right = insertNode(root->right, word);
    }
    return root;
}

void freeTree(TreeNode *root) {
    if (root == NULL) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root->word);
    free(root);
}

void printNodesAtLevel(TreeNode *root, int level, int width) {
    if (root == NULL) {
        printf("%*s", width, "");
        return;
    }
    if (level == 0) {
        int totalSpacing = width - strlen(root->word);
        int left = totalSpacing / 2;
        int right = totalSpacing - left;
        printf("%-*s", left, "");
        printf("%s", root->word);
        printf("%-*s", right, "");
    } else {
        printNodesAtLevel(root->left, level - 1, width / 2);
        printNodesAtLevel(root->right, level - 1, width / 2);
    }
}

void printTree(TreeNode *root) {
    printNodesAtLevel(root, 0, 64);
    printf("\n");
    printNodesAtLevel(root, 1, 64);
    printf("\n");
    printNodesAtLevel(root, 2, 64);
    printf("\n");
    printNodesAtLevel(root, 3, 64);
    printf("\n");
}

int main(int argc, char *argv[]) {
    TreeNode *root = NULL;
    for (int i = 1; i < argc; i++) {
        root = insertNode(root, argv[i]);
    }
    printTree(root);
    freeTree(root);
    return 0;
}