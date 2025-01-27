#include <Eigen/Dense>
#include <iostream>
#include <array>

int main() {
    // Example symmetric 3x3 matrix
    Eigen::Matrix3d myMatrix;
    myMatrix << 1, 2, 1,
                2, 3, 2,
                1, 2, 4;

    // Compute eigenvalues and eigenvectors
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(myMatrix);

    // Extract eigenvalues and eigenvectors
    Eigen::Vector3d eigenvalues = solver.eigenvalues();
    Eigen::Matrix3d eigenvectors = solver.eigenvectors();

    // Define custom sorting order for indices
    std::array<int, 3> indices = {2, 0, 1}; // Example: Sort by index 2, 0, 1

    // Containers for sorted eigenvalues and eigenvectors
    Eigen::Vector3d sortedEigenvalues;
    Eigen::Matrix3d sortedEigenvectors;

    // Reorder eigenvalues and eigenvectors according to indices
    for (int i = 0; i < 3; ++i) {
        sortedEigenvalues(i) = eigenvalues(indices[i]);
        sortedEigenvectors.col(i) = eigenvectors.col(indices[i]);
    }

    // Output the results
    std::cout << "Original eigenvalues:\n" << eigenvalues << "\n\n";
    std::cout << "Original eigenvectors:\n" << eigenvectors << "\n\n";
    std::cout << "Sorted eigenvalues:\n" << sortedEigenvalues << "\n\n";
    std::cout << "Sorted eigenvectors:\n" << sortedEigenvectors << "\n";

    return 0;
}