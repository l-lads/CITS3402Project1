#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SIZE 100000          // Change to 100000 for actual use
#define PROBABILITY 0.01   // Probability of non-zero entries

// Function to generate a random sparse matrix
int** generate_sparse_matrix(int size, double probability) {
    int** matrix = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        matrix[i] = (int*)calloc(size, sizeof(int));
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if ((double)rand() / RAND_MAX < probability) {
                matrix[i][j] = rand() % 10 + 1;
            }
        }
    }
    return matrix;
}

// Function to generate B and C matrices from a sparse matrix
void generate_B_and_C(int** matrix, int size, int*** B, int*** C, int** B_sizes) {
    *B = (int**)malloc(size * sizeof(int*));
    *C = (int**)malloc(size * sizeof(int*));
    *B_sizes = (int*)calloc(size, sizeof(int));

    for (int i = 0; i < size; i++) {
        int row_count = 0;

        // Count non-zero elements in the row
        for (int j = 0; j < size; j++) {
            if (matrix[i][j] != 0) {
                row_count++;
            }
        }

        // Handle empty rows
        if (row_count == 0) {
            row_count = 2;
            (*B)[i] = (int*)calloc(row_count, sizeof(int));
            (*C)[i] = (int*)calloc(row_count, sizeof(int));
        } else {
            (*B)[i] = (int*)malloc(row_count * sizeof(int));
            (*C)[i] = (int*)malloc(row_count * sizeof(int));
        }

        // Fill B and C arrays with non-zero values and their indices
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

// Function to multiply matrices using their B and C compressed forms
void multiply_sparse_matrices(int** B1, int** C1, int* B1_sizes, int** B2, int** C2, int* B2_sizes, int size, int** result, omp_sched_t schedule_type) {
    omp_set_schedule(schedule_type, 0);

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (int i = 0; i < size; i++) {
            // Temporary local array to accumulate results
            int* local_result = (int*)calloc(size, sizeof(int));

            for (int k = 0; k < B1_sizes[i]; k++) {
                int col_B1 = C1[i][k];
                int val_B1 = B1[i][k];

                for (int j = 0; j < B2_sizes[col_B1]; j++) {
                    int col_B2 = C2[col_B1][j];
                    int val_B2 = B2[col_B1][j];

                    local_result[col_B2] += val_B1 * val_B2;
                }
            }

            // Combine local results into global result matrix
            #pragma omp critical
            {
                for (int j = 0; j < size; j++) {
                    result[i][j] += local_result[j];
                }
            }

            free(local_result);
        }
    }
}

// Function to free allocated matrix memory
void free_matrix(int** matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Function to write B and C matrices to files
void write_to_file(const char* filename, int** matrix, int* sizes, int size) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < sizes[i]; j++) {
            fprintf(fp, "%d ", matrix[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

// Function to log results to a file
void log_results(const char* filename, int num_threads, const char* schedule_name, double time_taken) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    fprintf(fp, "Threads:%d\n", num_threads);
    fprintf(fp, "Schedule:%s\n", schedule_name);
    fprintf(fp, "Probability:%.2f\n", PROBABILITY);
    fprintf(fp, "Time taken:%f seconds\n", time_taken);

    fclose(fp);
}

int main() {
    srand(time(NULL));

    // Create the Results directory if it doesn't exist
    struct stat st;
    if (stat("Results", &st) == -1) {
        mkdir("Results", 0700);
    }

    // Generate two sparse matrices X and Y
    printf("Generating two sparse matrices of size %d with probability %.2f\n", SIZE, PROBABILITY);
    int** matrix_X = generate_sparse_matrix(SIZE, PROBABILITY);
    int** matrix_Y = generate_sparse_matrix(SIZE, PROBABILITY);

    // Generate B and C matrices for X and Y
    int **BX, **CX, **BY, **CY;
    int *BX_sizes, *CX_sizes;

    generate_B_and_C(matrix_X, SIZE, &BX, &CX, &BX_sizes);
    generate_B_and_C(matrix_Y, SIZE, &BY, &CY, &CX_sizes);

    // Free the original sparse matrices X and Y
    free_matrix(matrix_X, SIZE);
    free_matrix(matrix_Y, SIZE);

    // Allocate memory for the result matrix
    int** result = (int**)malloc(SIZE * sizeof(int*));
    for (int i = 0; i < SIZE; i++) {
        result[i] = (int*)calloc(SIZE, sizeof(int));
    }

    // Choose specific thread count and scheduling type
    int num_threads = 8; // Set number of threads
    omp_sched_t schedule_type = omp_sched_static; // Set scheduling type to static
    const char* schedule_name = "static"; // Name of scheduling type

    omp_set_num_threads(num_threads); // Set number of threads for OpenMP
    double start_time = omp_get_wtime();

    multiply_sparse_matrices(BX, CX, BX_sizes, BY, CY, CX_sizes, SIZE, result, schedule_type);

    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Log results to file
    char log_filename[50];
    snprintf(log_filename, sizeof(log_filename), "results_threads_%d_schedule_%s.txt", num_threads, schedule_name);
    log_results(log_filename, num_threads, schedule_name, time_taken);

    // Write the final B and C matrices to files
    write_to_file("FileB.txt", BX, BX_sizes, SIZE);
    write_to_file("FileC.txt", CX, CX_sizes, SIZE);

    // Free allocated memory
    free_matrix(BX, SIZE);
    free_matrix(CX, SIZE);
    free_matrix(BY, SIZE);
    free_matrix(CY, SIZE);
    free(BX_sizes);
    free(CX_sizes);
    free_matrix(result, SIZE);

    return 0;
}
