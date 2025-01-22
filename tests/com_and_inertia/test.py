import numpy as np

with open('inertia.txt', 'r') as file:
    lines = file.readlines()

matrix = []
for line in lines:
    row = list(map(float, line.split()))
    matrix.append(row)

matrix = np.array(matrix)
print('Inertia :')
print(matrix)

print('\nDeterminant :')
print(np.linalg.det(matrix))

eigenvalues, eigenvectors = np.linalg.eigh(matrix)

print('\nInertia eigenvalues :')
print('%.15f  %.15f  %.15f'%(eigenvalues[0],eigenvalues[1],eigenvalues[2]))

print('\nInertia eigenvectors :')
print(eigenvectors[0])
print(eigenvectors[1])
print(eigenvectors[2])