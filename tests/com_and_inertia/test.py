import numpy as np

# Read the matrix from the file
with open('inertia.txt', 'r') as file:
    lines = file.readlines()

# Parse the matrix
matrix = []
for line in lines:
    row = list(map(float, line.split()))
    matrix.append(row)

matrix = np.array(matrix)

# Verify the matrix is symmetric
if not np.allclose(matrix, matrix.T):
    raise ValueError("The input matrix is not symmetric.")

# Compute eigenvalues and eigenvectors
eigenvalues, eigenvectors = np.linalg.eigh(matrix)

# Print results
print("Matrix:")
print(matrix)
print("\nEigenvalues:")
print(eigenvalues)
print("\nEigenvectors:")
print(eigenvectors)