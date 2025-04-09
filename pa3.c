/*
 * pa3.c - ECE368 Programming Assignment 3
 *
 * This program reads the post-order traversal of a strictly binary tree representing a packing of rectangles.
 * It reconstructs the binary tree, computes all possible re-rootings at valid edges, and writes:
 * - out_file1: Re-rooted strictly binary tree from alternating left-right traversal
 * - out_file2: Re-rooted strictly binary tree from alternating right-left traversal
 * - out_file3: Dimensions for all valid re-rooted trees
 * - out_file4: Most optimal re-rooted tree (smallest area)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <stdbool.h>

#define MAX_NODES 1024

// This defines the structure for a tree node
typedef struct TreeNode {
    char type; // 'H' for horizontal cut, 'V' for vertical cut, or a diit for a rectangle label
    int label; // Block label (for the leaf nodes)
    int width, height; // Dimensions for rectangle nodes
    struct TreeNode *left, *right;
} TreeNode;

// Function prototypes
TreeNode* createNode(char type, int label, int width, int height);
TreeNode* buildFromLines(char* lines[], int* idx);
TreeNode* buildTree(FILE* fp);
void computeDimensions(TreeNode* root);
void writePreOrder(TreeNode* root, FILE* fp);
void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst);
TreeNode* cloneTree(TreeNode* root);
void rerootAtEdge(TreeNode* parent, TreeNode* child, TreeNode** newRoot, char direction);
void simulate(TreeNode* root, TreeNode* parent, FILE* fp, int* minArea, int* bestIndex, int* index, TreeNode** bestTree, char dir);
void freeTree(TreeNode* root);

// Function to create a new node
TreeNode* createNode(char type, int label, int width, int height) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->type = type;
    node->label = label;
    node->width = width;
    node->height = height;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// *NEW*
TreeNode* buildFromLines(char* lines[], int* idx) {
    if (*idx < 0) return NULL;

    char* line = lines[*idx];
    (*idx)--;

    if (line[0] == 'H' || line[0] == 'V') {
        TreeNode* right = buildFromLines(lines, idx);
        TreeNode* left = buildFromLines(lines, idx);
        TreeNode* node = createNode(line[0], -1, 0, 0);
        node->left = left;
        node->right = right;
        return node;
    } else {
        int label, width, height;
        sscanf(line, "%d(%d,%d)", &label, &width, &height);
        return createNode('B', label, width, height);
    }
}
 
// Function to build tree from post-order file
TreeNode* buildTree(FILE* fp) {
    char buffer[64];
    char* lines[MAX_NODES];
    int count = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        lines[count] = strdup(buffer);
        if (lines[count][strlen(lines[count]) - 1] == '\n') {
            lines[count][strlen(lines[count]) - 1] = '\0';
        }
        count++;
    }

    int idx = count - 1;
    return buildFromLines(lines, &idx);
}
 
// Function to compute dimensions recursively
void computeDimensions(TreeNode* root) {
    if (!root || root->type == 'B') return;
    computeDimensions(root->left);
    computeDimensions(root->right);

    if (root->type == 'H') {
        root->width = root->left->width > root->right->width ? root->left->width : root->right->width;
        root->height = root->left->height + root->right->height;
    } else {
        root->width = root->left->width + root->right->width;
        root->height = root->left->height > root->right->height ? root->left->height : root->right->height;
    }
}

// Function to write pre-order traversal
void writePreOrder(TreeNode* root, FILE* fp) {
    if (!root) return;
    if (root->type == 'B') fprintf(fp, "%d(%d,%d)\n", root->label, root->width, root->height);
    else fprintf(fp, "%c\n", root->type);
    writePreOrder(root->left, fp);
    writePreOrder(root->right, fp);
}

// Alternating path: left-right for out_file1, right-left for out_file2
void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    TreeNode *parent = NULL, *child = root;
    TreeNode *lastParent = NULL, *lastChild = NULL;
    char lastDirection = ' ';
    int toggle = leftFirst;

    while (child && child->type != 'B') {
        parent = child;
        if (toggle % 2 == 0) {
            child = child->left;
            lastDirection = 'L';
        } else {
            child = child->right;
            lastDirection = 'R';
        }
        toggle++;

        //if (child && child->type == 'B') break; // stop before leaf
    }

    if (lastParent && lastChild) { // removed "&& grand"
        TreeNode* newRoot = NULL;
        rerootAtEdge(parent, child, &newRoot, lastDirection);
        computeDimensions(newRoot);
        writePreOrder(newRoot, fp);
        freeTree(newRoot);
    } else {
        // Not enough depth to reroot, output original
        computeDimensions(root);
        writePreOrder(root, fp);
    }

    //if (curr) fprintf(fp, "%d(%d,%d)\n", curr->label, curr->width, curr->height);
}

// Function to deep copy a tree
TreeNode* cloneTree(TreeNode* root) {
    if (!root) return NULL;
    TreeNode* newNode = createNode(root->type, root->label, root->width, root->height);
    newNode->left = cloneTree(root->left);
    newNode->right = cloneTree(root->right);
    return newNode;
}
 
// Function to simulate rerooting at a specific edge
void rerootAtEdge(TreeNode* parent, TreeNode* child, TreeNode** newRoot, char direction) {
    // if (!parent || !child) return;
 
    TreeNode* pClone = cloneTree(parent);
    TreeNode* cClone = cloneTree(child);
 
    if (direction == 'L') {
        pClone->left = NULL;
        //cClone->left = cloneTree(child->left); // preserve original left
        cClone->right = pClone;
    } else {
        pClone->right = NULL;
        //cClone->right = cloneTree(child->right); // preserve original right
        cClone->left = pClone;
    }

    *newRoot = cClone;
}
 
// Function to explore and simulate rerooting at each valid edge
void simulate(TreeNode* root, TreeNode* parent, FILE* fp, int* minArea, int* bestIndex, int* index, TreeNode** bestTree, char dir) {
    if (!root) return;
 
    // Re-root at current edge (if parent exists)
    if (parent) {
        TreeNode* newRoot = NULL;
        rerootAtEdge(parent, root, &newRoot, dir);
        computeDimensions(newRoot);
        int area = newRoot->width * newRoot->height;
        printf("[DEBUG] Reroot #%d -> w: %d, h: %d, area: %d\n", *index, newRoot->width, newRoot->height, area);

        if (area < *minArea) {
            *minArea = area;
            *bestIndex = *index;
            if (*bestTree) freeTree(*bestTree);
            *bestTree = cloneTree(newRoot);
        }

        if (newRoot->type == 'B') fprintf(fp, "%d\n", newRoot->label);
        else fprintf(fp, "%c(%d,%d)\n", newRoot->type, newRoot->width, newRoot->height);
        (*index)++;

        freeTree(newRoot);
    }

    simulate(root->left, root, fp, minArea, bestIndex, index, bestTree, 'L');
    simulate(root->right, root, fp, minArea, bestIndex, index, bestTree, 'R');
}

// Free memory recursively
void freeTree(TreeNode* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// DEBUG
void debugPrintTree(TreeNode* root) {
    if (!root) return;

    if (root->type == 'B') {
        printf("Node: Leaf Label = %d, Width %d, Height = %d\n", root->label, root->width, root->height);
    } else {
        printf("Node: Internal Type = %c\n", root->type);
    }

    debugPrintTree(root->left);
    debugPrintTree(root->right);
}

// Main function
int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s in_file out_file1 out_file2 out_file3 out_file4\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE* inFile = fopen(argv[1], "r");
    if (!inFile) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    TreeNode* root = buildTree(inFile);
    fclose(inFile);
    if (!root) return EXIT_FAILURE;

    printf("DEBUG: Pre-order traversal of constructed tree:\n");
    debugPrintTree(root);
    
    FILE* out1 = fopen(argv[2], "w");
    FILE* out2 = fopen(argv[3], "w");
    FILE* out3 = fopen(argv[4], "w");
    FILE* out4 = fopen(argv[5], "w");

    if (!out1 || !out2 || !out3 || !out4) {
        perror("Error opening output files");
        return EXIT_FAILURE;
    }

    computeDimensions(root);
    followAlternatingPath(root, out1, 0); // out_file1: left-right path
    // followAlternatingPath(root, out2, 1); // out_file2: right-left path

    // int minArea = INT_MAX;
    // int bestIndex = -1;
    // int index = 0;
    // TreeNode* bestTree = NULL;
    // simulate(root, NULL, out3, &minArea, &bestIndex, &index, &bestTree, ' ');

    // //printf("[DEBUG] Minimum area: %d at index %d\n", minArea, bestIndex);

    // computeDimensions(bestTree);
    // writePreOrder(bestTree, out4);

    fclose(out1);
    fclose(out2);
    fclose(out3);
    fclose(out4);

    freeTree(root);
    // freeTree(bestTree);
    return EXIT_SUCCESS;
}