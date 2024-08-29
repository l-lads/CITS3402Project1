#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100000
#define DENSITY 0.01

// Function to generate a sparse matrix with given density
void generateSparseMatrix(int **matrix, double density, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            // Randomly decide if the element is non-zero
            if (rand() % 100 < density * 100) {
                matrix[i][j] = rand() % 10 + 1; // Random value between 1 and 10
            } else {
                matrix[i][j] = 0;
            }
        }
    }
}

// Function to generate row-compressed matrices B and C and write to files
void generateRowCompressed(int **matrix, int **B, int **C, int size) {
    FILE *fpB = fopen("FileB", "w");
    FILE *fpC = fopen("FileC", "w");

    for (int i = 0; i < size; i++) {
        int bIndex = 0;
        int cIndex = 0;
        for (int j = 0; j < size; j++) {
            if (matrix[i][j] != 0) {
                B[i][bIndex++] = matrix[i][j];
                C[i][cIndex++] = j;
                fprintf(fpB, "%d ", matrix[i][j]);
                fprintf(fpC, "%d ", j);
            }
        }
        if (bIndex == 0) { // No non-zero elements in row
            B[i][0] = 0;
            C[i][0] = 0;
            fprintf(fpB, "0 ");
            fprintf(fpC, "0 ");
        }
        fprintf(fpB, "\n");
        fprintf(fpC, "\n");
    }

    fclose(fpB);
    fclose(fpC);
}

int main() {
    srand(time(NULL)); // Seed the random number generator

    // Dynamic allocation of matrices
    int **matrix = malloc(SIZE * sizeof(int *));
    int **B = malloc(SIZE * sizeof(int *));
    int **C = malloc(SIZE * sizeof(int *));
    for (int i = 0; i < SIZE; i++) {
        matrix[i] = malloc(SIZE * sizeof(int));
        B[i] = malloc(SIZE * sizeof(int));
        C[i] = malloc(SIZE * sizeof(int));
    }

    // Generate the sparse matrix and its compressed representation
    generateSparseMatrix(matrix, DENSITY, SIZE);
    generateRowCompressed(matrix, B, C, SIZE);

    // Free allocated memory
    for (int i = 0; i < SIZE; i++) {
        free(matrix[i]);
        free(B[i]);
        free(C[i]);
    }
    free(matrix);
    free(B);
    free(C);

    return 0;
}
