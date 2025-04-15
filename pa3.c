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

// #define MAX_NODES 2000000

typedef struct TreeNode {
    char type; // 'H', 'V', or 'B'
    int label, width, height;
    struct TreeNode *left, *right;
} TreeNode;

typedef struct Edge {
    TreeNode* parent;
    TreeNode* child;
    char direction; // 'L' or 'R'
} Edge;

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
void debugPrintTree(TreeNode* root);

TreeNode* createNode(char type, int label, int width, int height) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->type = type;
    node->label = label;
    node->width = width;
    node->height = height;
    node->left = node->right = NULL;
    return node;
}

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

TreeNode* buildTree(FILE* fp) {
    char buffer[64];

    int capacity = 1024;
    char** lines = malloc(sizeof(char*) * capacity);
    if (!lines) {
        fprintf(stderr, "Memory allocation failed for lines[]\n");
        return NULL;
    }

    int count = 0;
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            char** temp = realloc(lines, sizeof(char*) * capacity);
            if (!temp) {
                fprintf(stderr, "Realloc failed while expanding lines[]\n");
                for (int i = 0; i < count; ++i) free(lines[i]);
                free(lines);
                return NULL;
            }
            lines = temp;
        }

        lines[count] = strdup(buffer);
        if (!lines[count]) {
            fprintf(stderr, "Memory allocation failed for lines[%d]\n", count);
            for (int i = 0; i < count; ++i) free(lines[i]);
            free(lines);
            return NULL;
        }

        size_t len = strlen(lines[count]);
        if (len > 0 && lines[count][len - 1] == '\n') {
            lines[count][len - 1] = '\0';
        }

        count++;
    }

    int idx = count - 1;
    TreeNode* root = buildFromLines(lines, &idx);

    // Cleanup line storage
    for (int i = 0; i < count; ++i) free(lines[i]);
    free(lines);

    return root;
}

void computeDimensions(TreeNode* root) {
    if (!root || root->type == 'B') return;
    computeDimensions(root->left);
    computeDimensions(root->right);

    if (root->type == 'H') {
        root->width = (root->left->width > root->right->width) ? root->left->width : root->right->width;
        root->height = root->left->height + root->right->height;
    } else {
        root->width = root->left->width + root->right->width;
        root->height = (root->left->height > root->right->height) ? root->left->height : root->right->height;
    }
}

void writePreOrder(TreeNode* root, FILE* fp) {
    if (!root) return;
    if (root->type == 'B') fprintf(fp, "%d(%d,%d)\n", root->label, root->width, root->height);
    else fprintf(fp, "%c\n", root->type);
    writePreOrder(root->left, fp);
    writePreOrder(root->right, fp);
}

TreeNode* cloneTree(TreeNode* root) {
    if (!root) return NULL;
    TreeNode* newNode = createNode(root->type, root->label, root->width, root->height);
    newNode->left = cloneTree(root->left);
    newNode->right = cloneTree(root->right);
    return newNode;
}

TreeNode* rerootFromPath(Edge path[], int length) {
    if (length == 0) return NULL;

    // Initialize the subtree as the child node of the first edge
    Edge first = path[0];
    printf("first edge: parent type = %c, child type = %c, direction = %c\n",
           first.parent->type, first.child->type, first.direction);
    TreeNode* newRoot = cloneTree(first.parent);
    printf("Cloned first.parent into newRoot:\n");
    debugPrintTree(newRoot);

    // Implement your requested fix:
    if (first.direction == 'L') {
        newRoot->left = cloneTree(first.child->left);
        printf("Assigned newRoot->left from first.child->left:\n");
        debugPrintTree(newRoot->left);
    } else {
        newRoot->right = cloneTree(first.child->right);
        printf("Assigned newRoot->right from first.child->right:\n");
        debugPrintTree(newRoot->right);
    }

    // Set up current root for further reroot layering
    for (int i = 1; i < length; i++) {
        Edge e = path[i];
        printf("Processing edge %d: parent type = %c, child type = %c, direction = %c\n",
               i, e.parent->type, e.child->type, e.direction);

        TreeNode* parentClone = cloneTree(e.parent);
        printf("Cloned parent:\n");
        debugPrintTree(parentClone);

        TreeNode* sibling = (e.direction == 'L') ? cloneTree(e.parent->right)
                                                 : cloneTree(e.parent->left);
        printf("Cloned sibling (opposite child):\n");
        debugPrintTree(sibling);

        TreeNode* newInternal = createNode(e.parent->type, -1, 0, 0);
        if (e.direction == 'L') {
            newInternal->left = newRoot;
            newInternal->right = sibling;
            printf("Constructed new internal node with left=newRoot and right=sibling\n");
        } else {
            newInternal->left = sibling;
            newInternal->right = newRoot;
            printf("Constructed new internal node with left=sibling and right=newRoot\n");
        }
        newRoot = newInternal;
        printf("Updated newRoot after edge %d:\n", i);
        debugPrintTree(newRoot);
    }

    return newRoot;
}

void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root || root->type == 'B') {
        writePreOrder(root, fp);
        return;
    }

    // Step 1: Dynamically allocated path array
    int capacity = 1024;
    Edge* path = malloc(sizeof(Edge) * capacity);
    if (!path) {
        fprintf(stderr, "Memory allocation failed for path array.\n");
        return;
    }

    int top = 0;
    TreeNode* curr = root;
    int toggle = leftFirst;

    // Step 2: Build the alternating path
    while (curr && curr->type != 'B') {
        if (top >= capacity) {
            capacity *= 2;
            Edge* temp = realloc(path, sizeof(Edge) * capacity);
            if (!temp) {
                fprintf(stderr, "Realloc failed while expanding path array.\n");
                free(path);
                return;
            }
            path = temp;
        }

        TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
        char dir = (toggle % 2 == 0) ? 'L' : 'R';
        path[top++] = (Edge){curr, next, dir};

        if (next->type == 'B') break;
        curr = next;
        toggle++;
    }

    // If only one edge exists, skip rerooting
    if (top <= 1) {
        writePreOrder(root, fp);
        free(path);
        return;
    }

    // Step 3: Clone the last edge’s nodes
    Edge lastEdge = path[top - 1];
    printf("lastEdge: parent type = %c, child type = %c, direction = %c\n",
           lastEdge.parent->type, lastEdge.child->type, lastEdge.direction);

    TreeNode* h1 = cloneTree(lastEdge.parent);
    printf("Cloned h1 (lastEdge.parent):\n");
    debugPrintTree(h1);

    TreeNode* one = cloneTree(lastEdge.child);
    printf("Cloned one (lastEdge.child):\n");
    debugPrintTree(one);

    // Step 4: Reroot the upper portion of the path
    TreeNode* rerootedSubtree = rerootFromPath(path, top - 1);
    printf("rerootedSubtree:\n");
    debugPrintTree(rerootedSubtree);

    // Step 5: Attach rerooted subtree to h1
    if (toggle % 2 != 0) {
        h1->left = rerootedSubtree;
        h1->right = one;
    } else {
        h1->right = rerootedSubtree;
        h1->left = one;
    }
    
    printf("newTree (h1 after attaching rerootedSubtree and one):\n");
    debugPrintTree(h1);

    TreeNode* newTree = h1;
    computeDimensions(newTree);
    writePreOrder(newTree, fp);
    freeTree(newTree);
    free(path);
}

void rerootAtEdge(TreeNode* parent, TreeNode* child, TreeNode** newRoot, char direction) {
    TreeNode* pClone = cloneTree(parent);
    TreeNode* cClone = cloneTree(child);

    if (direction == 'L') {
        pClone->left = NULL;
        cClone->right = pClone;
    } else {
        pClone->right = NULL;
        cClone->left = pClone;
    }
    *newRoot = cClone;
}

void simulate(TreeNode* root, TreeNode* parent, FILE* fp, int* minArea, int* bestIndex, int* index, TreeNode** bestTree, char dir) {
    if (!root) return;

    if (parent) {
        TreeNode* newRoot = NULL;
        rerootAtEdge(parent, root, &newRoot, dir);
        computeDimensions(newRoot);
        int area = newRoot->width * newRoot->height;

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

void freeTree(TreeNode* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

void debugPrintTree(TreeNode* root) {
    if (!root) return;
    if (root->type == 'B')
        printf("Node: Leaf Label = %d, Width %d, Height = %d\n", root->label, root->width, root->height);
    else
        printf("Node: Internal Type = %c\n", root->type);
    debugPrintTree(root->left);
    debugPrintTree(root->right);
}

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

    FILE* out1 = fopen(argv[2], "w");
    FILE* out2 = fopen(argv[3], "w");
    FILE* out3 = fopen(argv[4], "w");
    FILE* out4 = fopen(argv[5], "w");

    if (!out1 || !out2 || !out3 || !out4) {
        perror("Error opening output files");
        return EXIT_FAILURE;
    }

    computeDimensions(root);
    followAlternatingPath(root, out1, 0);
    followAlternatingPath(root, out2, 1);

    //int minArea = INT_MAX;
    //int bestIndex = -1;
    //int index = 0;
    //TreeNode* bestTree = NULL;
    //simulate(root, NULL, out3, &minArea, &bestIndex, &index, &bestTree, ' ');
    //computeDimensions(bestTree);
    //writePreOrder(bestTree, out4);

    fclose(out1);
    fclose(out2);
    fclose(out3);
    fclose(out4);

    freeTree(root);
    // freeTree(bestTree);
    return EXIT_SUCCESS;
}
