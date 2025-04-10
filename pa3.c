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

typedef struct Edge {
    TreeNode* parent;
    TreeNode* child;
    char direction; // 'L' or 'R'
} Edge;

// Function prototypes
TreeNode* createNode(char type, int label, int width, int height);
TreeNode* buildFromLines(char* lines[], int* idx);
TreeNode* buildTree(FILE* fp);
void computeDimensions(TreeNode* root);
void writePreOrder(TreeNode* root, FILE* fp);
//TreeNode* rerootAlongPath(TreeNode* curr, int toggle, TreeNode ** leafTarget, char* lastDirection);
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
/*void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    TreeNode *parent = NULL, *child = root;
    TreeNode *lastParent = NULL, *lastChild = NULL;
    char lastDirection = ' ';
    int toggle = leftFirst;

    while (child && child->type != 'B') {
        parent = child;

        if (toggle % 2 == 0) {
            printf("Moving left\n");
            child = parent->left;
            lastDirection = 'L';
        } else {
            printf("Moving right\n");
            child = parent->right;
            lastDirection = 'R';
        }

        if (child) {
            printf("Child!\n");
            lastParent = parent;
            lastChild = child;
        }

        printf("Toggle!\n");
        toggle++;
    }

    // if (child && child->type == 'B') {
    //     printf("%d\n", child->label);
    // } else if (child) {
    //     printf("%c\n", child->type);
    // }

    if (lastParent && lastChild) { // removed "&& grand"
        printf("lastParent && lastChild\n");
        printf("[DEBUG] Last edge before leaf: parent=%c, child=%s, direction=%c\n", lastParent->type, lastChild->type == 'B' ? "BLOCK" : (lastChild->type == 'H' ? "H" : "V"), lastDirection);

        TreeNode* newRoot = NULL;
        rerootAtEdge(lastParent, lastChild, &newRoot, lastDirection);
        computeDimensions(newRoot);
        writePreOrder(newRoot, fp);
        freeTree(newRoot);
    } else {
        printf("Else!");
        // Not enough depth to reroot, output original
        printf("[DEBUG] No valid rerooting edge found. Outputting original tree.\n");
        computeDimensions(root);
        writePreOrder(root, fp);
    }
}*/

// New recursive rerooting helper
// TreeNode* rerootAlongPath(TreeNode* curr, int toggle, TreeNode ** leafTarget, char* lastDirection) {
//     if (!curr) return NULL;

//     if (curr->type == 'B') {
//         *leafTarget = createNode('B', curr->label, curr->width, curr->height); // Save leaf for final reroot
//         return *leafTarget;
//     }

//     TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
//     TreeNode* sibling = (toggle % 2 == 0) ? curr->right : curr->left;

//     TreeNode* childSubtree = rerootAlongPath(next, toggle + 1, leafTarget, lastDirection);

//     if (!childSubtree) return NULL;

//     TreeNode* clone = createNode(curr->type, -1, 0, 0);

//     if (toggle % 2 == 0) {
//         clone->right = childSubtree;
//         clone->left = cloneTree(sibling);
//         *lastDirection = 'L';
//     } else {
//         clone->left = childSubtree;
//         clone->right = cloneTree(sibling);
//         *lastDirection = 'R';
//     }

//     return clone;
//     // return NULL;
// }

// void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
//     TreeNode* leaf = NULL;
//     char lastDir = ' ';
//     TreeNode* rerootedPath = rerootAlongPath(root, leftFirst, &leaf, &lastDir);

//     if (!rerootedPath || !leaf) {
//         computeDimensions(root);
//         writePreOrder(root, fp);
//         return;
//     }

//     // Attach leaf as the correct child of rerootedPath
//     TreeNode* clonedLeaf = cloneTree(leaf);
//     if (lastDir == 'L') {
//         rerootedPath->left = clonedLeaf;
//     } else {
//         rerootedPath->right = clonedLeaf;
//     }

//     computeDimensions(rerootedPath);
//     writePreOrder(rerootedPath, fp);
//     freeTree(rerootedPath);
// }

/*void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root) return;

    printf("[INFO] Starting followAlternatingPath, leftFirst = %d\n", leftFirst);

    TreeNode *curr = root;
    TreeNode *parent = NULL;
    TreeNode *lastParent = NULL;
    TreeNode *lastChild = NULL;
    char lastDir = ' ';
    int toggle = leftFirst;
    int depth = 0;

    // Follow alternating path
    while (curr && curr->type != 'B') {
        parent = curr;
        printf("[INFO] Level %d: At node %c\n", depth, curr->type);

        if (toggle % 2 == 0) {
            curr = curr->left;
            lastDir = 'L';
            printf("[INFO] Going LEFT to node %c\n", curr ? curr->type : '?');
        } else {
            curr = curr->right;
            lastDir = 'R';
            printf("[INFO] Going RIGHT to node %c\n", curr ? curr->type : '?');
        }

        lastParent = parent;
        lastChild = curr;
        toggle++;
        depth++;

        // Stop traversal when next node is a leaf
        if (curr && curr->type == 'B') {
            printf("[INFO] Reached leaf node: %d\n", curr->label);
            break;
        }
    }

    if (lastParent && lastChild) {
        printf("[INFO] Rerooting at edge: parent type=%c, child label=%d, direction=%c\n",
               lastParent->type, lastChild->label, lastDir);

        TreeNode* newRoot = NULL;
        rerootAtEdge(lastParent, lastChild, &newRoot, lastDir);
        computeDimensions(newRoot);
        writePreOrder(newRoot, fp);
        freeTree(newRoot);
    } else {
        printf("[INFO] No reroot performed, writing original tree\n");
        computeDimensions(root);
        writePreOrder(root, fp);
    }
}*/

// Updated followAlternatingPath
/*void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root || root->type == 'B') {
        writePreOrder(root, fp);
        return;
    }

    // 1. Traverse alternating path and store edges
    Edge stack[1024];
    int top = 0;
    TreeNode* curr = root;
    int toggle = leftFirst;

    while (curr && curr->type != 'B') {
        TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
        char dir = (toggle % 2 == 0) ? 'L' : 'R';
        stack[top++] = (Edge){curr, next, dir};
        toggle++;
        curr = next;
    }

    if (top == 0) {
        writePreOrder(root, fp);
        return;
    }

    // 2. Reroot at the last edge
    Edge last = stack[--top];
    TreeNode* newRoot = cloneTree(last.child);
    TreeNode* parentClone = cloneTree(last.parent);

    printf("[DEBUG] Initial reroot at edge: parent type %c, child label %d, dir %c\n",
           last.parent->type, last.child->label, last.direction);

    if (last.direction == 'L') {
        parentClone->left = NULL;
        newRoot->right = parentClone;
    } else {
        parentClone->right = NULL;
        newRoot->left = parentClone;
    }

    TreeNode* currentSubtree = newRoot;

    // 3. Rebuild tree upward
    printf("[DEBUG] Stack contains %d edges\n", top + 1);
    for (int i = 0; i <= top; i++) {
        printf("[STACK %d] parent: %c, child: %s, dir: %c\n", i,
            stack[i].parent->type,
            (stack[i].child->type == 'B') ? "BLOCK" : (stack[i].child->type == 'H' ? "H" : "V"),
            stack[i].direction);
    }

    while (top > 0) {
        Edge e = stack[--top];
        TreeNode* parent = cloneTree(e.parent);
        TreeNode* sibling = (e.direction == 'L') ? cloneTree(e.parent->right)
                                                 : cloneTree(e.parent->left);

        TreeNode* newInternal = createNode(e.parent->type, -1, 0, 0);

        if (e.direction == 'L') {
            newInternal->left = currentSubtree;
            newInternal->right = sibling;
        } else {
            newInternal->left = sibling;
            newInternal->right = currentSubtree;
        }

        printf("[DEBUG] Rerooting up: parent type %c, dir %c\n", e.parent->type, e.direction);
        currentSubtree = newInternal;
    }

    // 4. Compute and write output
    computeDimensions(currentSubtree);
    writePreOrder(currentSubtree, fp);
    freeTree(currentSubtree);
}*/

/*void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root || root->type == 'B') {
        writePreOrder(root, fp);
        return;
    }

    // 1. Traverse alternating path and store edges
    Edge stack[1024];
    int top = 0;
    TreeNode* curr = root;
    int toggle = leftFirst;

    while (curr && curr->type != 'B') {
        TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
        char dir = (toggle % 2 == 0) ? 'L' : 'R';
        stack[top++] = (Edge){curr, next, dir};
        toggle++;
        curr = next;
    }

    if (top == 0) {
        writePreOrder(root, fp);
        return;
    }

    // 2. Debug: print stack contents
    printf("[DEBUG] Stack contains %d edges\n", top);
    for (int i = 0; i < top; i++) {
        printf("[STACK %d] parent: %c, child: %s, dir: %c\n", i,
               stack[i].parent->type,
               (stack[i].child->type == 'B') ? "BLOCK" :
               (stack[i].child->type == 'H' ? "H" : "V"),
               stack[i].direction);
    }

    // 3. Initialize reroot at the last edge
    Edge last = stack[--top];
    printf("[DEBUG] Initial reroot at edge: parent type %c, child label %d, dir %c\n",
           last.parent->type, last.child->label, last.direction);

    TreeNode* currentSubtree = NULL;
    rerootAtEdge(last.parent, last.child, &currentSubtree, last.direction);

    // 4. Reroot up the stack
    while (top > 0) {
        Edge e = stack[--top];
        printf("[DEBUG] Rerooting up: parent type %c, dir %c\n", e.parent->type, e.direction);
        rerootAtEdge(e.parent, currentSubtree, &currentSubtree, e.direction);
    }

    // 5. Compute and write output
    computeDimensions(currentSubtree);
    writePreOrder(currentSubtree, fp);
    freeTree(currentSubtree);
}*/

/*void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root || root->type == 'B') {
        writePreOrder(root, fp);
        return;
    }

    // 1. Traverse alternating path and store edges
    Edge stack[1024];
    int top = 0;
    TreeNode* curr = root;
    int toggle = leftFirst;

    while (curr && curr->type != 'B') {
        TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
        char dir = (toggle % 2 == 0) ? 'L' : 'R';
        stack[top++] = (Edge){curr, next, dir};
        toggle++;
        curr = next;
    }

    if (top == 0) {
        writePreOrder(root, fp);
        return;
    }

    // 2. Debug: print stack contents
    printf("[DEBUG] Stack contains %d edges\n", top);
    for (int i = 0; i < top; i++) {
        printf("[STACK %d] parent: %c, child: %s, dir: %c\n", i,
               stack[i].parent->type,
               (stack[i].child->type == 'B') ? "BLOCK" :
               (stack[i].child->type == 'H' ? "H" : "V"),
               stack[i].direction);
    }

    // 3. Initial rerooting at the deepest edge using rerootAtEdge
    Edge last = stack[--top];
    printf("[DEBUG] Initial reroot at edge: parent type %c, child label %d, dir %c\n",
           last.parent->type, last.child->label, last.direction);

    TreeNode* currentSubtree = NULL;
    rerootAtEdge(last.parent, last.child, &currentSubtree, last.direction);

    // 4. Rebuild tree upward from the stack
    while (top > 0) {
        Edge e = stack[--top];
        printf("[DEBUG] Rerooting up: parent type %c, dir %c\n", e.parent->type, e.direction);

        TreeNode* newInternal = createNode(e.parent->type, -1, 0, 0);
        TreeNode* sibling = NULL;

        if (e.direction == 'L') {
            // We came from the left — sibling is on the right
            sibling = cloneTree(e.parent->right);
            newInternal->left = currentSubtree;
            newInternal->right = sibling;
        } else {
            // We came from the right — sibling is on the left
            sibling = cloneTree(e.parent->left);
            newInternal->left = sibling;
            newInternal->right = currentSubtree;
        }

        currentSubtree = newInternal;
    }

    // 5. Compute dimensions and write output
    computeDimensions(currentSubtree);
    writePreOrder(currentSubtree, fp);
    freeTree(currentSubtree);
}*/

/*void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root || root->type == 'B') {
        writePreOrder(root, fp);
        return;
    }

    // 1. Traverse alternating path and store edges
    Edge stack[1024];
    int top = 0;
    TreeNode* curr = root;
    int toggle = leftFirst;

    while (curr && curr->type != 'B') {
        TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
        char dir = (toggle % 2 == 0) ? 'L' : 'R';
        stack[top++] = (Edge){curr, next, dir};
        toggle++;
        curr = next;
    }

    if (top == 0) {
        writePreOrder(root, fp);
        return;
    }

    // 2. Debug: print stack contents
    printf("[DEBUG] Stack contains %d edges\n", top);
    for (int i = 0; i < top; i++) {
        printf("[STACK %d] parent: %c, child: %s, dir: %c\n", i,
               stack[i].parent->type,
               (stack[i].child->type == 'B') ? "BLOCK" :
               (stack[i].child->type == 'H' ? "H" : "V"),
               stack[i].direction);
    }

    // 3. Initialize reroot at the last edge (leaf edge)
    Edge last = stack[--top];
    printf("[DEBUG] Initial reroot at edge: parent type %c, child label %d, dir %c\n",
           last.parent->type, last.child->label, last.direction);

    TreeNode* currentSubtree = NULL;
    rerootAtEdge(last.parent, last.child, &currentSubtree, last.direction);

    // // 4. Reroot upward by building new internal nodes manually
    // while (top > 0) {
    //     Edge e = stack[--top];
    //     printf("[DEBUG] Rerooting up: parent type %c, dir %c\n", e.parent->type, e.direction);

    //     TreeNode* sibling = (e.direction == 'L') ? cloneTree(e.parent->right)
    //                                              : cloneTree(e.parent->left);

    //     TreeNode* newInternal = createNode(e.parent->type, -1, 0, 0);

    //     if (e.direction == 'L') {
    //         newInternal->left = currentSubtree;
    //         newInternal->right = sibling;
    //     } else {
    //         newInternal->left = sibling;
    //         newInternal->right = currentSubtree;
    //     }

    //     printf("[DEBUG] New internal node created of type %c with left = %s, right = %s\n",
    //            e.parent->type,
    //            (newInternal->left->type == 'B') ? "BLOCK" :
    //            (newInternal->left->type == 'H' ? "H" : "V"),
    //            (newInternal->right->type == 'B') ? "BLOCK" :
    //            (newInternal->right->type == 'H' ? "H" : "V"));

    //     currentSubtree = newInternal;
    // }

   // REPLACE ONLY THE REROOTING LOOP in your existing followAlternatingPath function:
   // 4. Reroot up the stack
   while (top > 0) {
        Edge e = stack[--top];
        printf("[DEBUG] Rerooting up: parent type %c, dir %c\n", e.parent->type, e.direction);

        TreeNode* parentClone = cloneTree(e.parent);
        TreeNode* sibling = (e.direction == 'L') ? cloneTree(e.parent->right)
                                                : cloneTree(e.parent->left);

        TreeNode* newInternal = createNode(parentClone->type, -1, 0, 0);

        if (e.direction == 'L') {
            newInternal->left = currentSubtree;
            newInternal->right = sibling;
        } else {
            newInternal->left = sibling;
            newInternal->right = currentSubtree;
        }

        currentSubtree = newInternal;
    }

    // 5. Compute and write output
    computeDimensions(currentSubtree);
    writePreOrder(currentSubtree, fp);
    freeTree(currentSubtree);
}*/

/*void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root || root->type == 'B') {
        writePreOrder(root, fp);
        return;
    }

    // 1. Traverse alternating path and store edges
    Edge stack[1024];
    int top = 0;
    TreeNode* curr = root;
    int toggle = leftFirst;

    while (curr && curr->type != 'B') {
        TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
        char dir = (toggle % 2 == 0) ? 'L' : 'R';
        stack[top++] = (Edge){curr, next, dir};
        toggle++;
        curr = next;
    }

    if (top == 0) {
        writePreOrder(root, fp);
        return;
    }

    // 2. Debug: print stack contents
    printf("[DEBUG] Stack contains %d edges\n", top);
    for (int i = 0; i < top; i++) {
        printf("[STACK %d] parent: %c, child: %s, dir: %c\n", i,
               stack[i].parent->type,
               (stack[i].child->type == 'B') ? "BLOCK" :
               (stack[i].child->type == 'H' ? "H" : "V"),
               stack[i].direction);
    }

    // 3. Initial reroot at last edge
    Edge last = stack[--top];
    printf("[DEBUG] Initial reroot at edge: parent type %c, child label %d, dir %c\n",
           last.parent->type, last.child->label, last.direction);

    TreeNode* currentSubtree = NULL;
    rerootAtEdge(last.parent, last.child, &currentSubtree, last.direction);

    // 4. Reroot up the stack
    while (top > 0) {
        Edge e = stack[--top];
        printf("[DEBUG] Rerooting up: parent type %c, dir %c\n", e.parent->type, e.direction);

        TreeNode* parentClone = cloneTree(e.parent);
        TreeNode* sibling = (e.direction == 'L') ? cloneTree(e.parent->right)
                                                 : cloneTree(e.parent->left);

        TreeNode* newInternal = createNode(e.parent->type, -1, 0, 0);

        // FIX: attach current subtree to the SAME SIDE as it was on before
        // and the sibling to the opposite side
        if (e.direction == 'L') {
            newInternal->left = currentSubtree;
            newInternal->right = sibling;
        } else {
            newInternal->left = sibling;
            newInternal->right = currentSubtree;
        }

        currentSubtree = newInternal;
    }

    // 5. Compute and write
    computeDimensions(currentSubtree);
    writePreOrder(currentSubtree, fp);
    freeTree(currentSubtree);
}*/

void followAlternatingPath(TreeNode* root, FILE* fp, int leftFirst) {
    if (!root || root->type == 'B') {
        printf("[DEBUG] Root is NULL or a block. Writing pre-order and returning.\n");
        writePreOrder(root, fp);
        return;
    }

    // 1. Traverse alternating path and store edges
    Edge stack[1024];
    int top = 0;
    TreeNode* curr = root;
    int toggle = leftFirst;

    printf("[DEBUG] Starting traversal of alternating path (leftFirst = %d)...\n", leftFirst);

    while (curr && curr->type != 'B') {
        TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
        char dir = (toggle % 2 == 0) ? 'L' : 'R';
        stack[top++] = (Edge){curr, next, dir};

        printf("[DEBUG] Step %d: Pushed edge: parent type = %c, child type = %c, direction = %c\n",
               top - 1, curr->type, (next ? next->type : 'X'), dir);

        if (next->type == 'B') break;

        toggle++;
        curr = next;
    }

    // while (curr) {
    //     if (curr->type == 'B') break;
    
    //     TreeNode* next = (toggle % 2 == 0) ? curr->left : curr->right;
    //     char dir = (toggle % 2 == 0) ? 'L' : 'R';
    
    //     // Add edge even if next is a leaf
    //     if (next == NULL) {
    //         printf("[ERROR] Attempted to push edge to NULL child! Parent type: %c, toggle %d\n", curr->type, toggle);
    //     }
    //     stack[top++] = (Edge){curr, next, dir};
    //     printf("[DEBUG] Step %d: Pushed edge: parent type = %c, child type = %s, direction = %c\n",
    //            toggle, curr->type,
    //            (next ? (next->type == 'B' ? "BLOCK" : (next->type == 'H' ? "H" : "V")) : NULL),
    //            dir);
    
    //     toggle++;
    //     curr = next;
    // }

    if (top == 0) {
        printf("[DEBUG] Tree too shallow to reroot. Writing original tree.\n");
        writePreOrder(root, fp);
        return;
    }

    // 2. Print stack summary
    printf("[DEBUG] Stack captured %d edges:\n", top);
    for (int i = 0; i < top; i++) {
        printf("[STACK %d] Parent: %c, Child: %s, Dir: %c\n", i,
               stack[i].parent->type,
               (stack[i].child->type == 'B') ? "BLOCK" :
               (stack[i].child->type == 'H' ? "H" : "V"),
               stack[i].direction);
    }

    // 3. Initial reroot
    Edge last = stack[--top];
    printf("[DEBUG] Initial reroot at edge: parent type %c, child label/type %d/%c, dir %c\n",
           last.parent->type, last.child->label, last.child->type, last.direction);

    TreeNode* currentSubtree = NULL;
    rerootAtEdge(last.parent, last.child, &currentSubtree, last.direction);
    printf("[DEBUG] Subtree after initial reroot constructed.\n");

    // 4. Iteratively reroot upward through stack
    while (top > 0) {
        Edge e = stack[--top];
        printf("\n[DEBUG] Rerooting UP at edge: parent type %c, direction %c\n",
               e.parent->type, e.direction);

        TreeNode* parentClone = cloneTree(e.parent);
        TreeNode* sibling = (e.direction == 'L') ? cloneTree(e.parent->right)
                                                 : cloneTree(e.parent->left);
        printf("[DEBUG] Cloned parent and sibling. Parent type = %c, Sibling type = %c\n",
               parentClone->type, sibling ? sibling->type : 'X');

        TreeNode* newInternal = createNode(e.parent->type, -1, 0, 0);

        // ATTACH with debug
        if (e.direction == 'L') {
            // newInternal->left = sibling;
            // newInternal->right = currentSubtree;

            newInternal->left = currentSubtree;
            newInternal->right = sibling;
            printf("[DEBUG] Attached currentSubtree to LEFT, sibling to RIGHT\n");
        } else {
            // newInternal->left = currentSubtree;
            // newInternal->right = sibling;

            newInternal->left = sibling;
            newInternal->right = currentSubtree;
            printf("[DEBUG] Attached sibling to LEFT, currentSubtree to RIGHT\n");
        }

        currentSubtree = newInternal;
        printf("[DEBUG] New currentSubtree root is type %c\n", currentSubtree->type);
    }

    // 5. Final output
    printf("[DEBUG] Finished rerooting. Computing dimensions and writing output...\n");
    computeDimensions(currentSubtree);
    writePreOrder(currentSubtree, fp);
    freeTree(currentSubtree);
    printf("[DEBUG] Done with followAlternatingPath.\n");
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
// *OLD* void rerootAtEdge(TreeNode* parent, TreeNode* child, TreeNode** newRoot, char direction) {
//     // if (!parent || !child) return;
 
//     TreeNode* pClone = cloneTree(parent);
//     TreeNode* cClone = cloneTree(child);
 
//     if (direction == 'L') {
//         pClone->left = NULL;
//         //cClone->left = cloneTree(child->left); // preserve original left
//         cClone->right = pClone;
//     } else {
//         pClone->right = NULL;
//         //cClone->right = cloneTree(child->right); // preserve original right
//         cClone->left = pClone;
//     }

//     *newRoot = cClone;
// }

// Function to simulate rerooting at a specific edge
void rerootAtEdge(TreeNode* parent, TreeNode* child, TreeNode** newRoot, char direction) {
    printf("[DEBUG] Entering rerootAtEdge. Parent type: %c, Child label/type: %c/%d, Direction: %c\n",
           parent->type, child->type, child->label, direction);
 
    TreeNode* pClone = cloneTree(parent);
    TreeNode* cClone = cloneTree(child);
 
    printf("[DEBUG] Cloned parent and child. Parent clone type: %c, Child clone label/type: %c/%d\n",
           pClone->type, cClone->type, cClone->label);
 
    if (direction == 'L') {
        printf("[DEBUG] Direction is L. Disconnecting parent->left.\n");
        pClone->left = NULL;
        cClone->right = pClone;
        printf("[DEBUG] Attached parent clone to cClone->right.\n");
    } else {
        printf("[DEBUG] Direction is R. Disconnecting parent->right.\n");
        pClone->right = NULL;
        cClone->left = pClone;
        printf("[DEBUG] Attached parent clone to cClone->left.\n");
    }
 
    *newRoot = cClone;
    printf("[DEBUG] New root set. Type: %c, Label: %d\n", cClone->type, cClone->label);
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