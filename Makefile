# Compiler
CC = gcc

# Compiler flags
CFLAGS = -O2 -fopenmp

# Source files
SRC_ORDINARY = ordinary_matrix_multiplication.c
SRC_GENERATE = generate_sparse_matrices.c
SRC_OPENMP = openmp_sparse_multiplication.c

# Executable names
EXE_ORDINARY = ordinary_matrix_multiplication
EXE_GENERATE = generate_sparse_matrices
EXE_OPENMP = openmp_sparse_multiplication

# Default target
all: $(EXE_ORDINARY) $(EXE_GENERATE) $(EXE_OPENMP)

# Ordinary matrix multiplication
$(EXE_ORDINARY): $(SRC_ORDINARY)
	$(CC) $(SRC_ORDINARY) -o $(EXE_ORDINARY) -O2

# Generate sparse matrices and their B, C representation
$(EXE_GENERATE): $(SRC_GENERATE)
	$(CC) $(SRC_GENERATE) -o $(EXE_GENERATE) -O2

# OpenMP-based sparse matrix multiplication
$(EXE_OPENMP): $(SRC_OPENMP)
	$(CC) $(SRC_OPENMP) -o $(EXE_OPENMP) $(CFLAGS)

# Clean target to remove compiled files
clean:
	rm -f $(EXE_ORDINARY) $(EXE_GENERATE) $(EXE_OPENMP)
