#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define VALID_PROBABILITIES_COUNT 3
const float VALID_PROBABILITIES[VALID_PROBABILITIES_COUNT] = {0.01, 0.02, 0.05};

// Convert schedule name to a string for environment variable
void set_omp_schedule_env(const char* schedule_name) {
    char schedule_env[100];
    snprintf(schedule_env, sizeof(schedule_env), "OMP_SCHEDULE=%s", schedule_name);
    putenv(strdup(schedule_env));
}

// Function to check if the schedule name is valid
void check_schedule_name(const char* schedule_name) {
    const char* valid_schedules[] = {"guided", "static", "dynamic", "auto"};
    int is_valid = 0;

    for (int i = 0; i < 4; i++) {
        if (strcmp(schedule_name, valid_schedules[i]) == 0) {
            is_valid = 1;
            break;
        }
    }

    if (!is_valid) {
        fprintf(stderr, "Invalid schedule name: %s. Must be one of {guided, static, dynamic, auto}.\n", schedule_name);
        exit(EXIT_FAILURE);
    }
}

// Function to check if the probability is valid
int is_valid_probability(float probability) {
    for (int i = 0; i < VALID_PROBABILITIES_COUNT; i++) {
        if (probability == VALID_PROBABILITIES[i]) {
            return 1; // Probability is valid
        }
    }
    return 0; // Probability is invalid
}

// Function to check if the number of threads is valid
int is_valid_thread_count(int num_threads) {
    return num_threads > 0; // Validate that the number of threads is positive
}

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
    *B_sizes = (int*)malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        int row_count = 0;

        // Count non-zero elements in the row
        for (int j = 0; j < size; j++) {
            if (matrix[i][j] != 0) {
                row_count++;
            }
        }

        // Allocate memory for B and C arrays
        if (row_count == 0) {
            row_count = 2; // Default size for empty rows
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
void multiply_sparse_matrices(int** B1, int** C1, int* B1_sizes, int** B2, int** C2, int* B2_sizes, int size, int** result, const char* schedule_name) {
    
    // Set the environment variable for OpenMP schedule
    set_omp_schedule_env(schedule_name);
    //printf("$OMP_SCHEDULE: %s\n", getenv("OMP_SCHEDULE"));

    
    #pragma omp parallel
    {
        #pragma omp for schedule(runtime)
        for (int i = 0; i < size; i++) {
            printf("Thread %d is executing\n", omp_get_thread_num());

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
void log_results(const char* filename, int num_threads, const char* schedule_name, double time_taken, int size, float probability) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    fprintf(fp, "Size:%d\n", size);
    fprintf(fp, "Probability:%.2f\n", probability);
    fprintf(fp, "Threads:%d\n", num_threads);
    fprintf(fp, "Schedule:%s\n", schedule_name);
    fprintf(fp, "Time taken:%.5f seconds\n", time_taken);

    fclose(fp);
}

// Function to create the main and subdirectories
void create_directories(const char* size_dir) {
    // Create the main directory named after the size
    struct stat st;
    if (stat(size_dir, &st) == -1) {
        mkdir(size_dir, 0700);
    }

    // Create subdirectories within the main directory
    char sub_dirs[4][20] = {"guided", "static", "dynamic", "runtime"};
    for (int i = 0; i < 4; i++) {
        char dir_path[256];
        snprintf(dir_path, sizeof(dir_path), "%s/%s", size_dir, sub_dirs[i]);
        if (stat(dir_path, &st) == -1) {
            mkdir(dir_path, 0700);
        }
    }
}

// Function to print a matrix (debugging purposes)
void print_matrix(int** matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (j < size - 1) {
                printf("%d ", matrix[i][j]);
            } else {
                printf("%d", matrix[i][j]);
            }
        }
        printf("\n");
    }
}

// Function to print compressed matrix B (debugging purposes)
void print_compressed_B(int** B, int* B_sizes, int size) {
    for (int i = 0; i < size; i++) {
        printf("Row %d: ", i);
        int row_size = B_sizes[i];
        for (int j = 0; j < row_size; j++) {
            printf("%d ", B[i][j]);
        }
        printf("\n");
    }
}

// Function to print compressed matrix C (debugging purposes)
void print_compressed_C(int** C, int* B_sizes, int size) {
    for (int i = 0; i < size; i++) {
        printf("Row %d: ", i);
        int row_size = B_sizes[i];
        for (int j = 0; j < row_size; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    // Seed the random number generator
    srand(time(NULL));
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <matrix_size> <schedule_type> <probability> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int size = atoi(argv[1]);
    const char* schedule_name = argv[2];
    float probability = atof(argv[3]);
    int num_threads = atoi(argv[4]);
    const char* output_dir = argv[1];

    if (!is_valid_thread_count(num_threads)) {
        fprintf(stderr, "Invalid number of threads. Must be positive.\n");
        return EXIT_FAILURE;
    }

    if (!is_valid_probability(probability)) {
        fprintf(stderr, "Invalid probability. Must be one of 0.01, 0.02, or 0.05.\n");
        return EXIT_FAILURE;
    }

    check_schedule_name(schedule_name);

    omp_set_num_threads(num_threads);
    create_directories(output_dir);

    int** X = generate_sparse_matrix(size, probability);
    int** Y = generate_sparse_matrix(size, probability);
    int** BX;
    int** CX;
    int* BX_sizes;
    int** BY;
    int** CY;
    int* BY_sizes;
    int** result = (int**)calloc(size, sizeof(int*));
    for (int i = 0; i < size; i++) {
        result[i] = (int*)calloc(size, sizeof(int));
    }

    // Generate B and C matrices for X and Y
    generate_B_and_C(X, size, &BX, &CX, &BX_sizes);
    generate_B_and_C(Y, size, &BY, &CY, &BY_sizes);

    //printf("Matrix X:\n");
    //print_matrix(X, size);
    //printf("Matrix Y:\n");
    //print_matrix(Y, size);

    //printf("Compressed Matrix BX:\n");
    //print_compressed_B(BX, BX_sizes, size);
    //printf("Compressed Matrix CX:\n");
    //print_compressed_C(CX, BX_sizes, size);
    //printf("Compressed Matrix BY:\n");
    //print_compressed_B(BY, BY_sizes, size);
    //printf("Compressed Matrix CY:\n");
    //print_compressed_C(CY, BY_sizes, size);

    // Start timing
    double start_time = omp_get_wtime();

    // Multiply matrices
    multiply_sparse_matrices(BX, CX, BX_sizes, BY, CY, BY_sizes, size, result, schedule_name);

    // Stop timing
    double end_time = omp_get_wtime();
    double time_taken = end_time - start_time;

    // Extract B and C from result matrix
    int** B_result;
    int** C_result;
    int* B_result_sizes;
    generate_B_and_C(result, size, &B_result, &C_result, &B_result_sizes);

    char B_result_filename[50];
    snprintf(B_result_filename, sizeof(B_result_filename), "%s/%s/FileB.txt", output_dir, schedule_name);
    
    char C_result_filename[50];
    snprintf(C_result_filename, sizeof(C_result_filename), "%s/%s/FileC.txt", output_dir, schedule_name);
    
    char log_filename[80];
    snprintf(log_filename, sizeof(log_filename), "%s/%s/prob=%.2f_thread=%d_sched=%s.txt", output_dir, schedule_name, probability, num_threads, schedule_name);

    write_to_file(B_result_filename, B_result, B_result_sizes, size);
    write_to_file(C_result_filename, C_result, B_result_sizes, size);
    log_results(log_filename, num_threads, schedule_name, time_taken, size, probability);

    // Free allocated memory
    free_matrix(X, size);
    free_matrix(Y, size);
    free_matrix(BX, size);
    free_matrix(CX, size);
    free_matrix(BY, size);
    free_matrix(CY, size);
    free_matrix(result, size);
    free_matrix(B_result, size);
    free_matrix(C_result, size);
    free(BX_sizes);
    free(BY_sizes);
    free(B_result_sizes);

    return 0;
}
