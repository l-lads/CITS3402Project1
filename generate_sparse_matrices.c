#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100  // Size of the matrix (100000 x 100000)
#define PROBABILITY 0.01  // Probability of a non-zero element

// Function to generate a random sparse matrix
int** generate_sparse_matrix(int size, double probability) {
    // Allocate memory for the matrix
    int** matrix = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        matrix[i] = (int*)calloc(size, sizeof(int));  // Initialize all values to 0
    }

    // Populate the matrix with random non-zero values based on the probability
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            // Generate a random number to decide if the entry is non-zero
            if ((double)rand() / RAND_MAX < probability) {
                matrix[i][j] = rand() % 10 + 1;  // Random value between 1 and 10
            }
        }
    }
    return matrix;
}

// Function to generate B and C matrices from the sparse matrix
void generate_B_and_C(int** matrix, int size, int*** B, int*** C, int** B_sizes) {
    *B = (int**)malloc(size * sizeof(int*));  // Allocate memory for B
    *C = (int**)malloc(size * sizeof(int*));  // Allocate memory for C
    *B_sizes = (int*)calloc(size, sizeof(int));  // Array to store the size of each row in B and C

    for (int i = 0; i < size; i++) {
        int row_count = 0;  // Counter for non-zero elements in the row

        // First count non-zero elements in the row
        for (int j = 0; j < size; j++) {
            if (matrix[i][j] != 0) {
                row_count++;
            }
        }

        // Allocate memory for B and C row, considering empty rows
        if (row_count == 0) {
            row_count = 2;  // For empty rows, we add two consecutive zeros
            (*B)[i] = (int*)calloc(row_count, sizeof(int));
            (*C)[i] = (int*)calloc(row_count, sizeof(int));
        } else {
            (*B)[i] = (int*)malloc(row_count * sizeof(int));
            (*C)[i] = (int*)malloc(row_count * sizeof(int));
        }

        // Populate B and C arrays with non-zero values and their column indices
        int index = 0;
        for (int j = 0; j < size; j++) {
            if (matrix[i][j] != 0) {
                (*B)[i][index] = matrix[i][j];
                (*C)[i][index] = j;
                index++;
            }
        }
        (*B_sizes)[i] = row_count;
    }
}

// Function to free the memory of a matrix
void free_matrix(int** matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main() {
    srand(time(NULL));  // Seed for random number generation

    // Generate sparse matrix with specified probability
    printf("Generating sparse matrix of size %d with probability %.2f\n", SIZE, PROBABILITY);
    int** sparse_matrix = generate_sparse_matrix(SIZE, PROBABILITY);

    // Declare pointers for B and C matrices and their sizes
    int** B = NULL;
    int** C = NULL;
    int* B_sizes = NULL;

    // Generate B and C matrices using the sparse matrix
    generate_B_and_C(sparse_matrix, SIZE, &B, &C, &B_sizes);

    // Output sample row sizes to check correctness
    printf("Sample B size: %d, Sample C size: %d\n", B_sizes[0], B_sizes[0]);
    printf("Sample B row: ");
    for (int i = 0; i < B_sizes[0]; i++) {
        printf("%d ", B[0][i]);
    }
    printf("\nSample C row: ");
    for (int i = 0; i < B_sizes[0]; i++) {
        printf("%d ", C[0][i]);
    }
    printf("\n");

    // Free allocated memory
    free_matrix(sparse_matrix, SIZE);
    free_matrix(B, SIZE);
    free_matrix(C, SIZE);
    free(B_sizes);

    return 0;
}
