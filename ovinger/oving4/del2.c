#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct TreeNodeStruct {
    char *word;
    struct TreeNodeStruct *left;
    struct TreeNodeStruct *right;
} TreeNode;

TreeNode *createNewNode(char *ord) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->word = strdup(ord);
    node->left = NULL;
    node->right = NULL;
    return node;
}
TreeNode *insertNode(TreeNode *root, char *ord) {
    if (root == NULL) {
        return createNewNode(ord);
    }
    int cmp = strcmp(ord, root->word);
    if (cmp < 0) {
        root->left = insertNode(root->left, ord);
    } else if (cmp > 0) {
        root->right = insertNode(root->right, ord);
    }
    return root;
}