#include<iostream>

void func(const int for_reps, const char **str_field)
{
    for (int i = 0; i < for_reps; ++i)
    {
        std::cout << str_field[i] << "\n";
    }
    return;
}

int main()
{
    const char *str_aster_mass[] = {"M1 ", "M2 "};
    func(2, str_aster_mass);

    return 0;
}