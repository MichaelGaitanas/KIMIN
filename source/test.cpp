#include <vector>
#include <iostream>
#include <cmath>

template <typename T>
void reduce_vector(std::vector<T>& original, size_t final_size)
{
    // Create a new vector to store the reduced data
    std::vector<T> reduced;
    
    // Calculate the step size to achieve the desired final size
    size_t n = std::ceil(static_cast<double>(original.size()) / final_size);
    
    // Reserve space for the reduced vector (equal to final_size)
    reduced.reserve(final_size);

    // Iterate over the original vector and add elements based on the calculated step size
    for (size_t i = 0; i < original.size(); i += n)
    {
        reduced.push_back(original[i]);
        if (reduced.size() == final_size)
            break;
    }
    
    // Assign the reduced vector back to the original
    original = std::move(reduced);
}

int main()
{
    // Example usage
    std::vector<double> original = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 219.0, -12.0, 0.232, 0.0031231};
    size_t final_size = 3;
    reduce_vector(original, final_size);

    // Print out the reduced vector
    for (const auto& value : original)
    {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    return 0;
}