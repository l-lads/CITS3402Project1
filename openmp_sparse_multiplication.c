#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE1 1000 // Small size for initial testing
#define SIZE2 5000 // Medium size
#define SIZE3 10000 // Large size

// Function to multiply sparse matrices using OpenMP
void multiplySparse(int **XB, int **XC, int **YB, int **YC, int **result, int size, omp_sched_t schedule_type) {
    omp_set_schedule(schedule_type, 1); // Set the schedule type

    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            for (int k = 0; k < size; k++) {
                if (XC[i][k] == YC[j][k]) {
                    sum += XB[i][k] * YB[j][k];
                }
            }
            result[i][j] = sum;
        }
    }
}

// Function to allocate and initialize matrices
void initializeMatrices(int ***XB, int ***XC, int ***YB, int ***YC, int ***result, int size) {
    *XB = (int **)malloc(size * sizeof(int *));
    *XC = (int **)malloc(size * sizeof(int *));
    *YB = (int **)malloc(size * sizeof(int *));
    *YC = (int **)malloc(size * sizeof(int *));
    *result = (int **)malloc(size * sizeof(int *));
    
    if (!*XB || !*XC || !*YB || !*YC || !*result) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }

    for (int i = 0; i < size; i++) {
        (*XB)[i] = (int *)malloc(size * sizeof(int));
        (*XC)[i] = (int *)malloc(size * sizeof(int));
        (*YB)[i] = (int *)malloc(size * sizeof(int));
        (*YC)[i] = (int *)malloc(size * sizeof(int));
        (*result)[i] = (int *)malloc(size * sizeof(int));

        if (!(*XB)[i] || !(*XC)[i] || !(*YB)[i] || !(*YC)[i] || !(*result)[i]) {
            fprintf(stderr, "Memory allocation failed for row %d!\n", i);
            exit(1);
        }

        // Initialize with some random sparse data
        for (int j = 0; j < size; j++) {
            (*XB)[i][j] = (rand() % 100 < 1) ? rand() % 10 : 0; // Sparse random data
            (*XC)[i][j] = (j % 10 < 1) ? j : -1;             // Index tracking sparse structure
            (*YB)[i][j] = (rand() % 100 < 1) ? rand() % 10 : 0; // Sparse random data
            (*YC)[i][j] = (j % 10 < 1) ? j : -1;             // Index tracking sparse structure
            (*result)[i][j] = 0;                                // Initialize result to zero
        }
    }
}

// Function to write matrix data to file
void writeMatrixToFile(const char *filename, int **matrix, int size) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s for writing.\n", filename);
        exit(1);
    }
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(fp, "%d ", matrix[i][j]);
        }
        fprintf(fp, "\n");
    }
    
    fclose(fp);
}

int main() {
    int **XB, **XC, **YB, **YC, **result;
    int sizes[] = {SIZE1, SIZE2, SIZE3};
    const char *schedule_names[] = {"Static", "Dynamic", "Guided", "Auto"};
    omp_sched_t schedules[] = {omp_sched_static, omp_sched_dynamic, omp_sched_guided, omp_sched_auto};

    srand(time(NULL)); // Seed for random number generation

    for (int matrix_size_idx = 0; matrix_size_idx < 3; matrix_size_idx++) {
        int size = sizes[matrix_size_idx];
        
        initializeMatrices(&XB, &XC, &YB, &YC, &result, size);

        // Evaluate performance with different thread counts
        for (int num_threads = 1; num_threads <= 8; num_threads *= 2) {
            omp_set_num_threads(num_threads);
            printf("Testing with %d threads for matrix size %d\n", num_threads, size);

            for (int s = 0; s < 4; s++) {
                omp_set_schedule(schedules[s], 1);
                double start = omp_get_wtime();
                multiplySparse(XB, XC, YB, YC, result, size, schedules[s]);
                double end = omp_get_wtime();

                printf("Time taken with %s scheduling: %f seconds\n", schedule_names[s], end - start);
            }
        }

        // Write final matrices to files
        writeMatrixToFile("FileB", XB, size);
        writeMatrixToFile("FileC", XC, size);

        // Free allocated memory
        for (int i = 0; i < size; i++) {
            free(XB[i]);
            free(XC[i]);
            free(YB[i]);
            free(YC[i]);
            free(result[i]);
        }
        free(XB);
        free(XC);
        free(YB);
        free(YC);
        free(result);
    }

    return 0;
}
