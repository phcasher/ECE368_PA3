/* Notes from TA help:
When we do a LL, for instance, we are preserving that connection between those two nodes (you follow left, then left, from the root node, and those are your two).

Looking to eventually make a tree where, for example, the root node (the top one) leads to a leaf node on the left, and then a branch node on the right.
Then, that branch node leads to a leaf node on the left, and then a branch node on the right. Etc.

Opposites:
LL -> RR
LR -> LR (yes this isn't a typo)
RL -> RL
RR -> LL

We need to go back using the opposites. (In order to make comparisons I think he said?)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This defines the structure for a tree node
typedef struct TreeNode {
    char type; // 'H' for horizontal cut, 'V' for vertical cut, or a diit for a rectangle label
    int label; // Block label (for the leaf nodes)
    int width, height; // Dimensions for rectangle nodes
    struct TreeNode *left, *right;
} TreeNode;

TreeNode* reRootRight(TreeNode *root);

// This function creates a new node
TreeNode* createNode(char type, int label, int width, int height) {
    TreeNode *node = (TreeNode*)malloc(sizeof(TreeNode)); // This allocates memory for the new node
    node->type = type; // Set the label (identifier for the leaf nodes)
    node->label = label; // This sets the label field, which is relevant for the leaf nodes, but is ignored for the internal nodes.
    node->width = width; // Set the width of the rectangle (if it is a leaf node)
    node->height = height; // Set the height of the rectangle (if it is a leaf node)
    node->left = node->right = NULL; // Initialize the child pointers to NULL, since a newly created node does not yet have children
    return node; // Now return the pointer to the newly created node
}

/* reconstructTree
 * ---------------
 * This function reconstructs a strictly binary tree from its post-order traversal representation.
 * 
 * Parameters:
 * - postorder: An array of node values that represents the post-order traversal of the tree.
 * - index: A pointer to an integer which tracks the current position in the post-order array.
 * - size: The total number of elements within the post-order array.
 * 
 * Return Values:
 * - A pointer to the root node of the newly reconstructed strictly binary tree.
*/
// Alternate version
/*TreeNode* reconstructTree(FILE *input) {
    char line[20]; // This buffer stores a single line read from the file
    if (fgets(line, sizeof(line), input) == NULL) { // Reads a line from the input file: if reading fails (due to EOF or error), then return NULL (an empty tree)
        return NULL;
    }

    char type; // This stores the type of the node ('H', 'V', or a number representing a leaf node)
    int label, width, height; // This stores the width and height of the rectangular block (this is only relevant for leaf nodes)

    if (sscanf(line, "%c(%d,%d)", &type, &width, &height) == 3) { // This attempts to parse a line formatted as 'C(W,H)', where C is either 'H' or 'V', and W,H are integers.
        // Successfully extracted a cutline node with width and height

        TreeNode *node = createNode(type, label, width, height); // This creates an internal node
        node->right = reconstructTree(input); // This recursively reconstructs the right subtree
        node->left = reconstructTree(input); // This recursively reconstructs the left subtree
        return node; // This returns the constructed subtree
    } else if (sscanf(line, "%c", &type) == 1) { // If the line does not match the previous format, then check if it is a single-character type (either 'H' or 'V')
        return createNode(type, 0, 0, 0); // This creates and returns a node with no width and no height (a non-leaf cutline node)
    }

    return NULL; // If the line does not match any expected format, then return NULL.
}*/
// Old from pa2
TreeNode* reconstructTree(FILE *input) {
    char line[20];
    if (!fgets(line, sizeof(line), input)) {
        return NULL;
    }

    if (line[0] == 'H' || line[0] == 'V') {
        TreeNode* node = createNode(line[0], -1, 0, 0);
        node->left = reconstructTree(input);
        node->right = reconstructTree(input);
        return node;
    } else {
        int label, width, height;
        sscanf(line, "%d(%d,%d)", &label, &width, &height);
        return createNode('B', label, width, height);
    }
}

// New to pa3
// This function performs pre-order traversal, and then writes to output
void preOrderTraversal(TreeNode *root, FILE *output) {
    if (!root) { // The base case: if the current node is NULL, then return (aka end recursion)
        return;
    }

    if (root->type == 'B') { // If the current node is a leaf node (block 'B'), then print its label, width, and height.
        fprintf(output, "%d(%d,%d)\n", root->label, root->width, root->height);
    } else { // Else, then this means that the current node is an internal node ('H' or 'V'), so print its type.
        fprintf(output, "%c\n", root->type);
    }

    preOrderTraversal(root->left, output); // This recursively traverses the left subtree in pre-order fashion
    preOrderTraversal(root->right, output); // This recursively traverses the right subtree in pre-order fashion
}
// Old from pa2. Remove this later!!
void postOrderTraversal(TreeNode* root, FILE *output) {
    // Postorder traversal means visting the nodes in left -> right -> root order.
    // This function recursively processes the left and right children first, prior to printing the current node.
    // Leaf nodes (block 'B') print their label, width, and height.
    // Internal nodes (horizontal 'H' or vertical 'V') print their type.

    if (!root) { // If the root is NULL, then the function returns immediately, because there is nothing to traverse.
        return;
    }
    postOrderTraversal(root->left, output); // This recursively traverses the left subtree first.
    postOrderTraversal(root->right, output); // This recursively traverses the right subtree second.
    if (root->type == 'B') { // If the current node is a leaf node (block 'B'), then print its label, width, and height.
        fprintf(output, "%d(%d,%d)\n", root->label, root->width, root->height);
    } else { // Else, then this means that the current node is an internal node ('H' or 'V'), so print its type.
        fprintf(output, "%c\n", root->type);
    }
}

// This function frees the memory allocated for the tree
void freeTree(TreeNode *root) {
    if (!root) { // The base case: if the current node is NULL, then return (aka end recursion)
        return;
    }
    
    freeTree(root->left); // This recursively frees the left subtree
    freeTree(root->right); // This recursively frees the right subtree
    free(root); // This frees the memory allocated for the current node
}

/* These are the new rerooting functions */
TreeNode* findLeftmost(TreeNode *node) {
    if (!node || (!node->left && !node->right)) { // If the node is either NULL or a leaf node, then return the node itself.
        return node;
    }

    if (node->left) { // If the left child exists, then continue searching down the left subtree.
        return findLeftmost(node->left);
    } else { // Otherwise, seach down the right subtree instead.
        return findLeftmost(node->right);
    }
}

TreeNode* findRightmost(TreeNode *node) {
    if (!node || (!node->left && !node->right)) {// If the node is either NULL or a leaf node, then return the node itself.
        return node;
    }

    if (node->right) { // If the right child exists, then continue searching down the right subtree.
        return findRightmost(node->right);
    } else { // Otherwise, seach down the left subtree instead.
        return findRightmost(node->left);
    }
}

TreeNode* reRootLeft(TreeNode *root) {
    if (!root) { // If the tree is empty, then return NULL.
        return NULL;
    }

    TreeNode *curr = root, *child = root->left;
    if (child != NULL && child->type != 'B') {
        // Re-root the parent and child nodes
        root->left = child->left;
        child->left = root;
        if (child->right->type == 'H' || child->right->type == 'V') { // If the right node is not a leaf node, then return the root.
            return reRootRight(child);
        }
    }

    return root;
}

TreeNode* reRootRight(TreeNode *root) {
    if (!root) { // If the tree is empty, then return NULL.
        return NULL;
    }

    TreeNode *curr = root, *child = root->right;
    if (child != NULL && child->type != 'B') {
        // Re-root the parent and child nodes
        root->right = child->right;
        child->right = root;

        if (child->left->type == 'H' || child->left->type == 'V') { // If the left node is not a leaf node, then return the root.
            return reRootLeft(child);
        }
    }
    return root;
}

TreeNode* reRootOptimal(TreeNode *root) {
    if (!root) { // If the tree is empty, then return NULL.
        return NULL;
    }

    if (!root->left && !root->right) { // If the tree only has one node, return it as the root.
        return root;
    }

    TreeNode *newRoot = root; // This initializes newRoot as the current root.
    int leftHeight = 0, rightHeight = 0; // Variables to track the height of the leftmost and rightmost paths.

    TreeNode *leftmost = findLeftmost(root); // Find the leftmost (the deepest left) node.
    TreeNode *rightmost = findRightmost(root); // Find the rightmost (the deepest right) node.

    // This calculates the height of the leftmost path by following left child pointers.
    for (TreeNode *curr = leftmost; curr; curr = curr->left) {
        leftHeight++;
    }

    // This calculates the height of the rightmost path by following right child pointers.
    for (TreeNode *curr = rightmost; curr; curr = curr->right) {
        rightHeight++;
    }

    // This determines which node should become the new root based on which one has the greater height.
    if (leftHeight > rightHeight) {
        newRoot = leftmost;
    } else {
        newRoot = rightmost;
    }

    // If the selected new root is already the current root, then return it.
    // Otherwise, then call either reRootLeft or reRootRight to perform the actual re-rooting.
    if (newRoot == root) {
        return root;
    } else {
        if (leftHeight > rightHeight) {
            return reRootLeft(root);
        } else {
            return reRootRight(root);
        }
    }
    // return newRoot == root ? root : (leftHeight > rightHeight ? reRootLeft(root) : reRootRight(root));
}

// This function outputs the packing dimensions
void outputDimensions(TreeNode* root, FILE *fp) {
    if (!root) { // Base case: If the root is NULL, then there is nothing to print, so return.
        return;
    }

    static int isFirstCall = 1; // Track if this is the first call (root node)

    if (isFirstCall) {
        isFirstCall = 0;
        // Root node must be printed differently:
        if (root->type == 'B') {
            fprintf(fp, "%d\n", root->label); // Leaf node only print label.
        } else {
            fprintf(fp, "%c\n", root->type); // Internal node prints only 'V' or 'H'.
        }
    }

    // Special case - Left child: If it is the immediate left child of the root, then print without dimensions.
    if (root->left) {
        //if (root->left->parent == root && root == root->parent) {
            if (root->left->type == 'B') {
                fprintf(fp, "%d\n", root->left->label);
            } else {
                fprintf(fp, "%c\n", root->left->type);
            }
        //}
    }

    // Special case - Right child: If it is the immediate right child of the root, then print without dimensions.
    if (root->right) {
        //if (root->right->parent == root && root == root->parent) {
            if (root->left->type == 'B') {
                fprintf(fp, "%d\n", root->right->label);
            } else {
                fprintf(fp, "%c\n", root->right->type);
            }
        //}
    }

    // Pre-order traversal: process the current node before the children
    //if (root->parent) { // Nodes with parent represent re-rooted cases
        if (root->type == 'B') {
            fprintf(fp, "%d(%d,%d)\n", root->label, root->width, root->height);
        } else {
            fprintf(fp, "%c(%d,%d)\n", root->type, root->width, root->height);
        }
    //}

    outputDimensions(root->left, fp);
    outputDimensions(root->right, fp);
}

// The following are all for output 4:

// Function that computes the bounding box of a tree
void computeBoundingBox(TreeNode* root, int* width, int* height) {
    if (!root) { // Base case: if the node is NULL, then the width and height are zero
        *width = 0;
        *height = 0;
        return;
    }

    int leftWidth = 0, leftHeight = 0; // Variables that store left subtree dimensions.
    int rightWidth = 0, rightHeight = 0; // Variables that store right subtree dimensions.

    // This recursively computes the bounding box for the left and right subtrees.
    computeBoundingBox(root->left, &leftWidth, &leftHeight);
    computeBoundingBox(root->right, &rightWidth, &rightHeight);

    if (root->left || root->right) { // If the node has children (internal node)
        *width = leftWidth + rightWidth; // The total width is the sum of both subtrees
        if (leftHeight > rightHeight) { // Height is the max of both subtrees
            *height = leftHeight;
        } else {
            *height = rightHeight;
        }
    } else { // If the node is a leaf node
        *width = root->width; // The width is the stored width of the node
        *height = root->height; // The height is the stored height of the node
    }
}

// Function that finds the optimal re-rooted representation
void findOptimalTree(TreeNode** rerootedTrees, int numTrees, FILE* outputFile) {
    int minArea = 9999; // Variable that tracks the minimum bounding box area.
    int minIndex = -1; // The index of the tree with the smallest area.

    for (int i = 0; i < numTrees; i++) { // Iterate over all re-rooted trees
        int width, height;
        computeBoundingBox(rerootedTrees[i], &width, &height); // This computes the bounding box for each tree.
        int area = width * height; // This calculates the area of the bounding box.

        if (area < minArea) { // If this tree has a smaller area, then update the minimum values.
            minArea = area;
            minIndex = i;
        }
    }
    
    /*// Note: this likely will need to be changed/removed.
    if (minIndex != -1) { // This ensures that a valid tree was found
        // Function that performs pre-order traversal of the optimal tree, and output node labels.
        void preorder(TreeNode* node, FILE* outputFile) {
            if (!node) { // Base case: if the node is NULL, then stop.
                return;
            }
            fprintf(outputFile, "%d ", node->label); // This was marked as "id", I assume they meant label // Prints the current node label
            preorder(node->left, outputFile); // This traverses the left subtree.
            preorder(node->right, outputFile); // This traverses the right subtree.
        }
        preorder(rerootedTrees[minIndex], outputFile); // This outputs the pre-order traversal of the optimal tree.
    }*/

    preOrderTraversal(rerootedTrees[minIndex], outputFile); // This outputs the pre-order traversal of the optimal tree.
}

int generateReRootedTrees(TreeNode *root, TreeNode *rerootedTrees[], int maxTrees) {
    if (!root || maxTrees <= 0) { // Base case: if the tree is empty, then return 0 since no re-rooting can be done.
        return 0;
    }

    int count = 0; // Initialize a counter to track the number of generated trees.
    rerootedTrees[count++] = root; // Includes the original tree as one possibility. // Store the original tree as the first re-rooted version.

    // This generates re-rooted trees by traversing the tree and applying reRootLeft and reRootRight. // Iterates over all nodes in the tree in order to generate re-rooted versions.
    void traverseAndReRoot(TreeNode *node) {
        if (!node || count >= maxTrees) { // Ensures that the maximum allowed trees has not been exceeded.
            return;
        }

        // Generates a left-rooted tree if a left child exists
        if (node->left && count < maxTrees) {
            rerootedTrees[count] = reRootLeft(node);
            if (rerootedTrees[count]) {
                count++;
            }
        }

        // Recursively traverse the left and right subtrees
        traverseAndReRoot(node->left);
        traverseAndReRoot(node->right);
    }

    traverseAndReRoot(root);
    return count; // Returns the total number of generated trees.
}

/*
This program excpectsa 5 command line arguments: an input filename, and four output filenames.
It first verifies that the correct number of arguments has been provided.
It opens the input file to read the tree's post-order traversal data.
It then calls reconstructTree to rebuild the binary tree structure.
It then opens four output files, and checks if they all were successfully opened.
The pre-order traversal of the tree is then written to each output file.
After writing, all of the files are closed to free system resources.
The tree's memory is then freed using freeTree in order to avoid memory leaks.
The program returns success when all of the operations are able to complete correctly.
*/

void treePrint(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    fprintf(stdout, "%d(%d,%d)\n", root->label, root->width, root->height); //print node
    treePrint(root->left);
    treePrint(root->right);
}

int main(int argc, char *argv[]) {
    if (argc != 6) { // This checks if the correct number of command-line arguments is provided
        fprintf(stderr, "Usage: %s in_file out_file1 out_file2 out_file3 out_file4\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r"); // This opens the input file for reading
    if (!input) { // If the file could not be opened, then print an error and exit
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    TreeNode *root = reconstructTree(input); // This reconstructs the tree from the input file using post-order traversal
    //treePrint(root);
    fclose(input); // Now, close the input file, because it is no longer needed
    if (!root) {
        return EXIT_FAILURE;
    }
    
    // Open all four of the output files for writing
    FILE *output1 = fopen(argv[2], "w");
    FILE *output2 = fopen(argv[3], "w");
    FILE *output3 = fopen(argv[4], "w");
    FILE *output4 = fopen(argv[5], "w");

    // Note: this line will need to include " || !output4"
    if (!output1 || !output2 || !output3 || !output4) { // This checks if any of the output files have failed to open
        perror("Error opening output files");
        return EXIT_FAILURE; // If any output file cannot be opened, then exit with failure
    }
    //postOrderTraversal(root, output1);
    // Generates and writes the first pre-order traversal with left-root ordering
    // *The issue appears to be with reRootLeft. Not preOrderTraversal.*
    TreeNode *rootLR = reRootLeft(root); // Re-rooting for left-root order
    preOrderTraversal(rootLR, output1); // Write the pre-order traversal of the tree re-rooted with a left preference to the first output file
    //freeTree(rootLR); // Free the re-rooted tree after use
    
    // Generate and write the second pre-order traversal with right-root ordering
    TreeNode *rootRL = reRootRight(root); // Re-rooting for right-root order
    //// THERE IS AN ISSUE WITH PREORDER CURRENTLY TO BE FIXED
    preOrderTraversal(rootRL, output2); // Write the pre-order traversal of the tree re-rooted with a right preference to the second output file
    //freeTree(rootRL); // Free the re-rooted tree after use
    
    // Generate and write the third output file with rectangle dimensions
    outputDimensions(root, output3);

    // Generate and write the fourth output file with optimal room arrangement. This generates multiple re-rooted trees and finds the optimal one.
    TreeNode *rerootedTrees[100]; // Placeholder for re-rooted trees.
    int numTrees = generateReRootedTrees(root, rerootedTrees, 100); // Generates all possible re-rooted trees.
    findOptimalTree(rerootedTrees, numTrees, output4); // Finds and writes the optimal re-rooted tree.
    
    // This is to free the allocated re-rooted trees.
    for (int i = 0; i < numTrees; i++) {
        freeTree(rerootedTrees[i]);
    }

    // This closes all of the output files, in order to free resources
    fclose(output1);
    fclose(output2);
    fclose(output3);
    fclose(output4);

    //freeTree(root); // This frees the memory allocated for the tree, in order to prevent memory leaks

    return EXIT_SUCCESS; // This indicates that execution was successful.*/
}
