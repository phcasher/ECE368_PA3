#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This is the structure definition for a binary tree node
typedef struct TreeNode {
    char type; // 'V' for vertical cut, 'H' for horizontal cut, or numeric for leaf
    int label; // The label for leaf nodes
    int width, height; // The dimensions for rectangles
    struct TreeNode *left, *right;
} TreeNode;

// This function creates a new tree node
TreeNode* createNode(char type, int label, int width, int height) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->type = type;
    node->label = label;
    node->width = width;
    node->height = height;
    node->left = node->right = NULL;
    return node;
}

// This function builds a tree from post-order traversal
TreeNode* buildTreeFromPostOrder(FILE *input) {
    char buffer[20];
    if (!fgets(buffer,sizeof(buffer), input)) return NULL;
    if (buffer[0] == 'H' || buffer [0] == 'V') {
        TreeNode* node = createNode(buffer[0], 0, 0);
        node->right = buildTreeFromPostOrder(input);
        node->left = buildTreeFromPostOrder(input);
        return node;
    } else {
        int label, width, height;
        sscanf(buffer, "%d(%d,%d)", &label, &width, &height);
        return createNode(label, width, height);
    }
}

// This function performs pre-order traversal, and then writes to a file
void preOrderTraversal(TreeNode *root, FILE *output) {
    if (root == NULL) return;
    if (root->type == 'V' || root->type == 'H') {
        fprintf(output, "%c\n", root->type);
    } else {
        fprintf(output, "%d(%d,%d)\n", root->label, root->width, root->height);
    }
    preOrderTraversal(root->left, output);
    preOrderTraversal(root->right, output);
}

// This function re-roots the tree for the first output (left to right traversal)
TreeNode* reRootTreeLeft(TreeNode *root) {
    if (!root || (!root->left && !root->right)) return root;
    TreeNode *newRoot = root;
    while (newRoot->left) newRoot = newRoot->left;

    if (newRoot->right) {
        TreeNode *temp = newRoot->right;
        newRoot->right = root;
        root->left = temp;
    }
    return newRoot;
}

// This function re-roots the tree for the second output (right to left traversal)
TreeNode* reRootTreeRight(TreeNode *root) {
    if (!root || (!root->left && !root->right)) return root;
    TreeNode *newRoot = root;
    while (newRoot->right) newRoot = newRoot->right;

    if (newRoot->right) {
        TreeNode *temp = newRoot->right;
        newRoot->right = root;
        root->left = temp;
    }
    return newRoot;
}

// This function computes the dimensions of the smallest enclosing rectangle
void computeSmallestRectangle(TreeNode *root, FILE *output) {
    if (!root) return;
    if (!root->left && !root->right) {
        fprintf(output, "%d(%d,%d)\n", root->type, root->width, root->height);
        return;
    }
    computeSmallestRectangle(root->left, output);
    computeSmallestRectangle(root->right, output);
    fprintf(output, "%c\n", root->type);
}

// The main function
int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf("");
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    TreeNode *root = buildTreeFromPostOrder(input);
    fclose(input);

    if (!root) {
        fprintf(stderr, "Error constructing tree\n");
        return EXIT_FAILURE;
    }

    FILE *out1 = fopen(argv[2], "w");
    TreeNode *root1 = reRootTreeLeft(root);
    preOrderTraversal(root1, out1);
    fclose(out1);

    FILE *out2 = fopen(argv[3], "w");
    TreeNode *root2 = reRootTreeRight(root);
    preOrderTraversal(root2, out2);
    fclose(out2);

    FILE *out3 = fopen(argv[4], "w");
    TreeNode *root1 = reRootTreeLeft(root);
    computeSmallestRectangle(root, out3);
    fclose(out3);

    FILE *out4 = fopen(argv[5], "w");
    TreeNode *root1 = reRootTreeLeft(root);
    preOrderTraversal(root, out4); // This is a placeholder for optimal re-rooting
    fclose(out4);

    // Free memory
    free(root);
    return EXIT_SUCCESS;
}