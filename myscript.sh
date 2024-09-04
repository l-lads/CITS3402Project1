#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --partition=work
#SBATCH --account=courses0101
#SBATCH --mem=200G
#SBATCH --time=00:05:00

gcc openmp_sparse_multiplication.c -o openmp_sparse_multiplication -O2 -fopenmp
srun openmp_sparse_multiplication