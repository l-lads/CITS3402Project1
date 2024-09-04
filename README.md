# CITS3402Project1
Matrix Multiplication Project for CITS3402
# Explanation for each file

## ordinary_matrix_multiplication.c
### Description
Algorithm that does matrix multiplication with random integers from 0 to 9 in the matrices A and B. With defined sized rows and columns
### Status
Completed ✅

## generate_sparse_matrices.c
### Description
Convert a sparse matrix into its compressed row-storage format, generating two arrays (B and C) for efficient matrix multiplication.
### Status
Completed ✅

## openmp_sparse_multiplications.c
### Description
When testing with Setonix-
1. Use the shellscript provided with the C file provided
2. When changing thread count change the:
    - #SBATCH --cpus-per-task=
    - Thread count in the c file
3. Use command squeue -u <username> for checking status of job
### Status
Virtually Completed ✅

