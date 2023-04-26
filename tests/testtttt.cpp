#include<iostream>
#include<vector>

void print_vector(double *vec, int size)
{
    for (int i = 0; i < size; ++i)
        std::cout << vec[i] << "\n";
    return;
}

int main()
{
    std::vector<std::vector<double>> matrix;
    for (int i = 0; i < 10; ++i)
    {
        matrix.push_back({(double)i, (double)2.0*i, (double)3.0*i, (double)4.0*i});
    }

    std::vector<double> column(matrix.size());
    std::transform(matrix.begin(), matrix.end(), column.begin(), [](const auto& row) { return row[1]; });
    print_vector(&column[0], column.size());

    print_vector(... , matrix.size());


    return 0;
}