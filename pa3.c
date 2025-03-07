/*
 * What is not currently working:
 * The preOrder function
 * The reroot (left and right) functions
 * The argv[5] file is causing errors
*/

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

    /*if (root->type == 'H' || root->type == 'V') { // This checks if the node is an internal node (cutline node 'H' or 'V')
        fprintf(output, "%c\n", root->type); // This prints the type of the internal node (either 'H' or 'V') to the output file
    } else { // Otherwise, it is a leaf node (a rectangle with width and height)
        fprintf(output, "%c(%d,%d)\n", root->label, root->width, root->height); // This prints the rectangle's type, width, and height in the format 'C(W,H)'
    }*/

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
    if (!node || (!node->left && !node->right)) {
        return node;
    }

    // return node->left ? findLeftmost(node->left) : findLeftmost(node->right);
    if (node->left) {
        return findLeftmost(node->left);
    } else {
        return findLeftmost(node->right);
    }
}

TreeNode* findRightmost(TreeNode *node) {
    if (!node || (!node->left && !node->right)) {
        return node;
    }

    // return node->right ? findRightmost(node->right) : findRightmost(node->left);
    if (node->right) {
        return findRightmost(node->right);
    } else {
        return findRightmost(node->left);
    }
}

TreeNode* reRootLeft(TreeNode *root) {
    if (!root) {
        return NULL;
    }

    TreeNode *newRoot = findLeftmost(root);
    if (newRoot == root) {
        return root;
    }

    TreeNode *parent = NULL, *curr = root;
    while (curr->left != newRoot && curr->right != newRoot) {
        parent = curr;

        // curr = curr->left ? curr->left : curr->right;
        if (curr->left) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }

    if (parent) {
        if (parent->left == newRoot) {
            parent->left = NULL;
        } else {
            parent->right = NULL;
        }
    }

    newRoot->left = root;
    return newRoot;
}

TreeNode* reRootRight(TreeNode *root) {
    if (!root) {
        return NULL;
    }

    TreeNode *newRoot = findRightmost(root);
    if (newRoot == root) {
        return root;
    }

    TreeNode *parent = NULL, *curr = root;
    while (curr->left != newRoot && curr->right != newRoot) {
        parent = curr;

        // curr = curr->right ? curr->right : curr->left;
        if (curr->right) {
            curr = curr->right;
        } else {
            curr = curr->left;
        }
    }

    if (parent) {
        if (parent->right == newRoot) {
            parent->right = NULL;
        } else {
            parent->left = NULL;
        }
    }

    newRoot->right = root;
    return newRoot;
}

TreeNode* reRootOptimal(TreeNode *root) {
    if (!root) return NULL;
    if (!root->left && !root->right) return root;

    TreeNode *newRoot = root;
    int leftHeight = 0, rightHeight = 0;
    TreeNode *leftmost = findLeftmost(root);
    TreeNode *rightmost = findRightmost(root);

    for (TreeNode *curr = leftmost; curr; curr = curr->left) leftHeight++;
    for (TreeNode *curr = rightmost; curr; curr = curr->right) rightHeight++;

    if (leftHeight > rightHeight) newRoot = leftmost;
    else newRoot = rightmost;

    return newRoot == root ? root : (leftHeight > rightHeight ? reRootLeft(root) : reRootRight(root));
}

// This function outputs the packing dimensions
void outputDimensions(TreeNode* root, FILE *fp) {
    // This function prints the dimensions (the width and height) of the root node, which represents the entire packed layout.
    // This function takes a pointer to the root TreeNode as input.

    if (!root) { // If the root is NULL, then there is nothing to print, so return.
        return;
    }

    outputDimensions(root->left, fp);
    outputDimensions(root->right, fp);

    if (root->type == 'B') { // If type 'B', then print the label, width, and height.
        fprintf(fp, "%d(%d,%d)\n", root->label, root->width, root->height);
    } else { // Else, then print the type, width, and height.
        fprintf(fp, "%c(%d,%d)\n", root->type, root->width, root->height);
    }
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
    fclose(input); // Now, close the input file, because it is no longer needed
    if (!root) {
        return EXIT_FAILURE;
    }
    
    // Open all four of the output files for writing
    FILE *output1 = fopen(argv[2], "w");
    FILE *output2 = fopen(argv[3], "w");
    FILE *output3 = fopen(argv[4], "w");
    FILE *output4 = fopen(argv[5], "w"); // CURRENTLY CAUSING EXIT_FAILURE

    // Note: this line will need to include " || !output4"
    if (!output1 || !output2 || !output3 || !output4) { // This checks if any of the output files have failed to open
        perror("Error opening output files");
        return EXIT_FAILURE; // If any output file cannot be opened, then exit with failure
    }

    // Generates and writes the first pre-order traversal with left-root ordering
    // *The issue appears to be with reRootLeft. Not preOrderTraversal.*
    TreeNode *rootLR = reRootLeft(root); // Re-rooting for left-root order
    preOrderTraversal(rootLR, output1); // Write the pre-order traversal of the tree re-rooted with a left preference to the first output file
    freeTree(rootLR); // Free the re-rooted tree after use
    
    /*// Generate and write the second pre-order traversal with right-root ordering
    TreeNode *rootRL = reRootRight(root); // Re-rooting for right-root order
    // THERE IS AN ISSUE WITH PREORDER CURRENTLY TO BE FIXED
    preOrderTraversal(rootRL, output2); // Write the pre-order traversal of the tree re-rooted with a right preference to the second output file
    freeTree(rootRL); // Free the re-rooted tree after use

    // Generate and write the third output file with rectangle dimensions
    outputDimensions(root, output3);

    // Generate and write the fourth output file with optimal room arrangement
    TreeNode *rootOpt = reRootOptimal(root); // Re-rooting for optimal arrangment
    //preOrder(rootOpt, output4); // This writes the pre-order traversal of the optimally re-rooted tree to the fourth output file
    freeTree(rootOpt); // This frees the re-rooted tree after use

    // This closes all of the output files, in order to free resources
    fclose(output1);
    fclose(output2);
    fclose(output3);
    //fclose(output4);

    freeTree(root); // This frees the memory allocated for the tree, in order to prevent memory leaks*/

    return EXIT_SUCCESS; // This indicates that execution was successful.*/
}