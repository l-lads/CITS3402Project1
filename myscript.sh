#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64
#SBATCH --partition=work
#SBATCH --account=courses0101
#SBATCH --mem=220G
#SBATCH --time=01:30:00

# Compile the program
gcc openmp_sparse_multiplication.c -o openmp_sparse_multiplication -O2 -fopenmp

# Run the program with different parameters on different tasks
# ./openmp_sparse_multiplication <matrix_size> <schedule from {static, dynamic, guided, auto}> <probability from {0.01, 0.02, 0.05}> <thread count>
srun ./openmp_sparse_multiplication 10000 auto 0.05 64
srun ./openmp_sparse_multiplication 1000 auto 0.05 64

