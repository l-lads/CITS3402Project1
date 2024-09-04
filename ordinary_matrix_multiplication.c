#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 10    // Number of rows in matrices
#define COLS 10   // Number of columns in matrices

// Function to multiply two matrices
void multiplyMatrices(int A[ROWS][COLS], int B[ROWS][COLS], int C[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = 0;
            for (int k = 0; k < COLS; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Function to initialize a matrix with random values
void initializeMatrix(int matrix[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = rand() % 10; // Random values between 0 and 9
        }
    }
}

// Function to print a matrix
void printMatrix(int matrix[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main() {
    srand(time(NULL)); // Seed random number generator

    int A[ROWS][COLS];
    int B[ROWS][COLS];
    int C[ROWS][COLS];

    // Initialize matrices with random values
    initializeMatrix(A);
    initializeMatrix(B);

    // Print matrices
    printf("Matrix A:\n");
    //printMatrix(A);

    printf("\nMatrix B:\n");
    //printMatrix(B);

    // Multiply matrices
    multiplyMatrices(A, B, C);

    // Print result matrix
    printf("\nResult Matrix C = A * B:\n");
    //printMatrix(C);
    return 0;
}